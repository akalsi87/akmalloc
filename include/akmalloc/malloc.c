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
#  if !defined(NDEBUG)
#    define AKMALLOC_ASSERT AKMALLOC_DEFAULT_ASSERT
#  else
#    define AKMALLOC_ASSERT(...) do { } while(0)
#  endif
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
    static_cast<void>(pgsz);
    AKMALLOC_ASSERT(pgsz == AKMALLOC_DEFAULT_PAGE_SIZE);
    return AKMALLOC_MMAP(sz);
}

static void ak_os_free(void* p, size_t sz)
{
    AKMALLOC_MUNMAP(p, sz);
}

ak_inline static void* ak_page_start_before(void* p)
{
    return (void*)((ak_sz)p & (~(ak_sz)(AKMALLOC_DEFAULT_PAGE_SIZE - 1)));
}

/***********************************************
 * Slabs
 ***********************************************/

#include "akmalloc/detail/bitset.h"

#define AK_SLAB_MAGIC 0xBADBEEF

typedef struct ak_slab_tag ak_slab;

typedef struct ak_slab_root_tag ak_slab_root;

struct ak_slab_root_tag
{
    ak_slab*     fd;
    ak_slab*     bk;
    ak_u32       bytes;
    ak_u32       sz;
    ak_u32       magic;
    ak_bitset32  avail;
};

struct ak_slab_tag
{
    ak_slab*     fd;
    ak_slab*     bk;
    ak_u32       bytes;
    ak_u32       sz;
    ak_u32       magic;
    ak_bitset32  avail;
};

ak_inline static void ak_slab_init_root(ak_slab_root* s, ak_sz sz)
{
    s->fd = s->bk = (ak_slab*)s;
    s->bytes = 0;
    s->sz = sz;
    s->magic = AK_SLAB_MAGIC;
    ak_bitset_clear_all(&(s->avail));
}

ak_inline static ak_slab* ak_slab_init(void* mem, ak_sz bytes, ak_sz sz)
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
    static const ak_sz maxsz = AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab);
    const ak_sz szofslabdata = sz * 32;
    return (szofslabdata > maxsz) ? 1 : (AKMALLOC_DEFAULT_PAGE_SIZE / szofslabdata);
}

static ak_slab* ak_slab_new(ak_sz sz, ak_slab* fd, ak_slab* bk)
{
    // try to acquire a page and fit as many slabs as possible in
    char* mem = (char*)ak_os_alloc(AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (!mem) { return 0; }
    }

    const ak_sz nslabs = ak_num_slabs_per_page(sz);
    const ak_sz nbytes = AKMALLOC_DEFAULT_PAGE_SIZE/nslabs;

    ak_slab* firstslab = ak_slab_init(mem, nbytes, sz);

    ak_sz i = 0;
    ak_slab* curr = firstslab;
    ak_slab* prev = bk;
    for (; i < nslabs - 1; ++i) {
        curr->bk = prev;
        prev->fd = curr;
        ak_slab* next;
        next = ak_slab_init((char*)curr + nbytes, nbytes, sz);
        curr->fd = next;
        prev = curr;
        curr = next;
    }
    curr->bk = prev;
    curr->fd = fd;
    fd->bk = curr;

    return firstslab;
}

ak_inline static char* ak_slab_2_mem(ak_slab* s)
{
    return (char*)(void*)(s + 1);
}

ak_inline static int ak_slab_all_free(ak_slab* s)
{
    return ak_bitset_num_trailing_ones(&(s->avail)) == (int)((s->bytes - sizeof(ak_slab))/s->sz);
}

ak_inline static void* ak_slab_alloc_idx(ak_slab* s, int idx)
{
    ak_bitset_clear(&(s->avail), idx);
    return ak_slab_2_mem(s) + (idx * s->sz);
}

ak_inline static void ak_slab_free_idx(ak_slab* s, int idx)
{
    AKMALLOC_ASSERT(!ak_bitset_get(&(s->avail), idx));
    ak_bitset_set(&(s->avail), idx);
}

static void* ak_slab_alloc(ak_slab_root* root)
{
    AKMALLOC_ASSERT(root->fd->sz == root->sz);
    ak_slab* s = root->fd;
    ak_u32 ntz = ak_bitset_num_trailing_zeros(&(s->avail));
    if (ntz == 32) {
        // check existing slabs
        while (s->fd != (ak_slab*)root) {
            AKMALLOC_ASSERT(s->fd->sz == s->sz);
            AKMALLOC_ASSERT(s->fd->magic == AK_SLAB_MAGIC);
            s = s->fd;
            ak_u32 ntz = ak_bitset_num_trailing_zeros(&(s->avail));
            if (ntz != 32) {
                return ak_slab_alloc_idx(s, ntz);
            }
        }
        // create a new slab
        ak_slab* nslab = ak_slab_new(root->sz, root->fd, (ak_slab*)root);
        if (nslab) {
            return ak_slab_alloc_idx(nslab, 0);
        }
        // out of memory
        return 0;
    }
    return ak_slab_alloc_idx(s, ntz);
}

static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* firstslab = (ak_slab*)(ak_page_start_before(p));
    AKMALLOC_ASSERT(firstslab->magic == AK_SLAB_MAGIC);

    ak_slab* rightslab = 0;
    {// free to the right slab
        ak_sz nslabsoff = (mem - (char*)firstslab)/firstslab->bytes;
        rightslab = (ak_slab*)((char*)firstslab + nslabsoff*firstslab->bytes);
        ak_sz idx = (mem - (char*)ak_slab_2_mem(rightslab))/rightslab->sz;
        AKMALLOC_ASSERT(rightslab->sz == firstslab->sz);
        ak_slab_free_idx(rightslab, idx);
    }

    if (ak_slab_all_free(rightslab) &&
        ((rightslab == firstslab) || ak_slab_all_free(firstslab))) {
        int allfree = 1;
        ak_slab* curr = firstslab->fd;
        ak_slab* prev = firstslab;
        ak_sz nslabs = ak_num_slabs_per_page(firstslab->sz);
        ak_sz i = 0;
        for (; i < nslabs - 1; ++i) {
            if (!ak_slab_all_free(curr)) {
                allfree = 0;
                break;
            }
            ak_slab* next = (ak_slab*)((char*)curr + curr->bytes);
            AKMALLOC_ASSERT((i == (nslabs - 2)) || (next == curr->fd));
            AKMALLOC_ASSERT(curr->magic == AK_SLAB_MAGIC);
            prev = curr;
            curr = next;
        }
        if (allfree) {
            ak_slab* lastslab = prev;
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
