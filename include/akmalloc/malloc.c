/**
 * \file malloc.c
 * \date Apr 08, 2016
 */

#ifndef AKMALLOC_MALLOC_C
#define AKMALLOC_MALLOC_C

#include "akmalloc/assert.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"
#include "akmalloc/memmap.h"

#if defined(AKMALLOC_GETPAGESIZE)
#  error "Page size can only be set to an internal default."
#else
#  define AKMALLOC_GETPAGESIZE AKMALLOC_DEFAULT_GETPAGESIZE
#endif

#if !defined(AKMALLOC_MMAP) && !defined(AKMALLOC_MUNMAP)
#  define AKMALLOC_MMAP AKMALLOC_DEFAULT_MMAP
#  define AKMALLOC_MUNMAP AKMALLOC_DEFAULT_MUNMAP
#elif defined(AKMALLOC_MMAP) && defined(AKMALLOC_MUNMAP)
/* do nothing */
#else
#  error "AKMALLOC_MMAP and AKMALLOC_MUNMAP not defined simultaneously."
#endif

#if !defined(AKMALLOC_LARGE_BLOCK_SIZE)
#  define AKMALLOC_LARGE_BLOCK_SIZE AKMALLOC_DEFAULT_LARGE_BLOCK_SIZE
#endif

#if !defined(AKMALLOC_ASSERT)
#  define AKMALLOC_ASSERT AKMALLOC_DEFAULT_ASSERT
#endif

/***********************************************
 * IMPLEMENTATION
 ***********************************************/

#define ak_offsetof(ty, field) (ak_sz)(((ty *)0)->field)
#define ak_pagealign(sz) (ak_sz)(((sz) + AKMALLOC_DEFAULT_PAGE_SIZE - 1) & (AKMALLOC_DEFAULT_PAGE_SIZE - 1))

/***********************************************
 * OS Allocation
 ***********************************************/

static void* ak_os_alloc(size_t sz)
{
    static const ak_sz pgsz = ak_page_size();
    AKMALLOC_ASSERT(pgsz == AKMALLOC_DEFAULT_PAGE_SIZE);
    return AKMALLOC_MMAP(sz);
}

static void ak_os_free(void* p, size_t sz)
{
    AKMALLOC_MUNMAP(p, sz);
}

ak_inline static void* ak_page_start_before(void* p)
{
    return (void*)((ak_sz)p & (~(AKMALLOC_DEFAULT_PAGE_SIZE - 1)));
}

/***********************************************
 * Slabs
 ***********************************************/

#include "akmalloc/detail/bitset.h"

#define AK_SLAB_MAGIC 0xBADBEEF

typedef struct ak_slab_tag ak_slab;

struct ak_slab_tag
{
    ak_slab*     fd;
    ak_slab*     bk;
    ak_u32       bytes;
    ak_u32       sz;
    ak_u32       magic;
    ak_bitset32  avail;
};

static void ak_slab_init_root(ak_slab* s, ak_sz sz)
{
    s->fd = s->bk = s;
    s->bytes = 0;
    s->sz = sz;
    s->magic = AK_SLAB_MAGIC;
    ak_bitset_clear_all(&(s->avail));
}

static ak_slab* ak_slab_init(void* mem, ak_sz bytes, ak_sz sz)
{
    AKMALLOC_ASSERT(mem);
    AKMALLOC_ASSERT(sz > 0);
    AKMALLOC_ASSERT(sz % 8 == 0);

    ak_sz navail = (bytes - sizeof(ak_slab))/sz;
    AKMALLOC_ASSERT(navail > 0);

    ak_slab* s = (ak_slab*)mem;
    s->fd = s->bk = 0;
    s->bytes = bytes;
    s->sz = sz;
    s->magic = AK_SLAB_MAGIC;
    ak_bitset_clear_all(&(s->avail));
    ak_bitset_set_n(&(s->avail), 0, navail);

    return s;
}

ak_inline static ak_sz ak_num_slabs_per_page(ak_sz sz)
{
    ak_sz noverhead = ((sizeof(ak_slab) + sz - 1) / sz);
    return ((((32 - noverhead) * sz) + 255) / 256) * (AKMALLOC_DEFAULT_PAGE_SIZE / 256);
}

static ak_slab* ak_slab_new(ak_sz sz, ak_slab* fd, ak_slab* bk)
{
    // try to acquire a page and fit as many slabs as possible in
    char* mem = (char*)ak_os_alloc(AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (!mem) { return 0; }
    }

    ak_sz nslabs = ak_num_slabs_per_page(sz);
    ak_slab* firstslab = ak_slab_init(mem, AKMALLOC_DEFAULT_PAGE_SIZE/nslabs, sz);

    ak_sz i = 0;
    ak_slab* curr = firstslab;
    ak_slab* prev = bk;
    for (; i < nslabs - 1; ++i) {
        curr->bk = prev;
        prev->fd = curr;
        ak_slab* next = ak_slab_init((char*)curr + curr->bytes, curr->bytes, sz);
        curr->fd = next;
        prev = curr;
        curr = next;
    }
    prev->fd = fd;

    return firstslab;
}

ak_inline static char* ak_slab_2_mem(ak_slab* s)
{
    return (char*)(void*)(s + 1);
}

ak_inline static int ak_slab_all_free(ak_slab* s)
{
    return ak_bitset_num_trailing_ones(&(s->avail)) == ((s->bytes - sizeof(ak_slab))/s->sz);
}

ak_inline static void* ak_slab_alloc_idx(ak_slab* s, int idx)
{
    // found free slot at nlz
    ak_bitset_clear(&(s->avail), idx);
    return ak_slab_2_mem(s) + (idx * s->sz);
}

static void* ak_slab_alloc_pvt(ak_slab* s, ak_slab* root)
{
    ak_u32 ntz = ak_bitset_num_trailing_zeros(&(s->avail));
    if (ntz == 32) {
        if (s->fd != root) {
            return ak_slab_alloc_pvt(s->fd, root);
        }
        ak_slab* nslab = ak_slab_new(s->sz, s->fd, s);
        if (nslab) {
            return ak_slab_alloc_idx(nslab, 0);
        }
        return 0;
    }
    return ak_slab_alloc_idx(s, ntz);
}

static void* ak_slab_alloc(ak_slab* s, size_t sz)
{
    AKMALLOC_ASSERT(sz == s->sz);
    // move ahead from root
    ak_slab* root = s;
    s = s->fd;
    return ak_slab_alloc_pvt(s, root);
}

static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* firstslab = (ak_slab*)(ak_page_start_before(p));
    ak_u32 checkall = 0;
    AKMALLOC_ASSERT(firstslab->magic == AK_SLAB_MAGIC);

    {// free to the right slab
        ak_sz nslabsoff = (mem - (char*)firstslab)/firstslab->bytes;
        ak_slab* rightslab = (ak_slab*)((char*)firstslab + nslabsoff*firstslab->bytes);
        ak_sz idx = (mem - (char*)ak_slab_2_mem(rightslab))/rightslab->sz;
        AKMALLOC_ASSERT(rightslab->sz == firstslab->sz);
        AKMALLOC_ASSERT(!ak_bitset_get(&(rightslab->avail), idx));
        ak_bitset_clear(&(rightslab->avail), idx);
        checkall = ak_slab_all_free(rightslab);
    }

    if (checkall) {
        int allfree = 1;
        ak_slab* curr = firstslab;
        ak_sz nslabs = ak_num_slabs_per_page(firstslab->sz);
        ak_sz i = 0;
        for (; i < nslabs; ++i) {
            if (!ak_slab_all_free(curr)) {
                allfree = 0;
                break;
            }
            ak_slab* next = (ak_slab*)((char*)curr + (AKMALLOC_DEFAULT_PAGE_SIZE / nslabs));
            AKMALLOC_ASSERT(next == curr->fd);
            AKMALLOC_ASSERT(curr->magic == AK_SLAB_MAGIC);
            curr = next;
        }
        if (allfree) {
            ak_slab* lastslab = (ak_slab*)((char*)curr - (AKMALLOC_DEFAULT_PAGE_SIZE / nslabs));
            firstslab->bk->fd = lastslab->fd;
            lastslab->fd->bk = firstslab->bk;
            ak_os_free(firstslab, AKMALLOC_DEFAULT_PAGE_SIZE);
        }
    }
}

/***********************************************
 * Coalescing allocator
 ***********************************************/


/***********************************************
 * Exported APIs
 ***********************************************/

void* ak_malloc(size_t sz)
{
    return 0;
}

void* ak_calloc(size_t elsz, size_t numel)
{
    return 0;
}

void ak_free(void* mem)
{

}

void* ak_aligned_alloc(size_t sz, size_t aln)
{
    return 0;
}

int ak_posix_memalign(void** pmem, size_t sz, size_t aln)
{
    return 0;
}

void* ak_memalign(size_t sz, size_t aln)
{
    return 0;
}

void* ak_realloc(void* mem, size_t newsz)
{
    return 0;
}

size_t ak_malloc_usable_size(const void* mem)
{
    return 0;
}

#endif/*AKMALLOC_MALLOC_C*/
