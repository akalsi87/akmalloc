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
#define AK_SLAB_ROOT_MAGIC 0xFADBEEF

typedef struct ak_slab_tag ak_slab;

typedef struct ak_slab_root_tag ak_slab_root;

struct ak_slab_tag
{
    ak_slab*      fd;
    ak_slab*      bk;
    ak_slab_root* root;
    ak_u32        bytes;
    ak_u32        sz;
    ak_u32        magic;
    ak_bitset32   avail;
};

struct ak_slab_root_tag
{
    ak_u32 sz;
    ak_u32 magic;
    ak_u32 npages;

    ak_slab partial_root;
    ak_slab full_root;
};

ak_inline static void ak_slab_unlink(ak_slab* s)
{
    s->bk->fd = s->fd;
    s->fd->bk = s->bk;
    s->fd = s->bk = 0;
}

ak_inline static void ak_slab_link_fd(ak_slab* s, ak_slab* fd)
{
    s->fd = fd;
    fd->bk = s;
}

ak_inline static void ak_slab_link_bk(ak_slab* s, ak_slab* bk)
{
    s->bk = bk;
    bk->fd = s;
}

ak_inline static void ak_slab_link(ak_slab* s, ak_slab* fd, ak_slab* bk)
{
    ak_slab_link_bk(s, bk);
    ak_slab_link_fd(s, fd);
}

ak_inline static void ak_slab_init_chain_head(ak_slab* s, ak_slab_root* rootp)
{
    s->fd = s->bk = s;
    s->root = rootp;
    s->bytes = 0;
    s->sz = rootp->sz;
    s->magic = AK_SLAB_MAGIC;
    ak_bitset_clear_all(&(s->avail));
}

ak_inline static void ak_slab_init_root(ak_slab_root* s, ak_sz sz, ak_sz npages)
{
    s->sz = sz;
    s->magic = AK_SLAB_ROOT_MAGIC;
    s->npages = npages;

    ak_slab_init_chain_head(&(s->partial_root), s);
    ak_slab_init_chain_head(&(s->full_root), s);
}

ak_inline static ak_slab* ak_slab_init(void* mem, ak_sz bytes, ak_sz sz, ak_slab_root* root)
{
    AKMALLOC_ASSERT(mem);
    AKMALLOC_ASSERT(sz > 0);
    AKMALLOC_ASSERT(sz % 8 == 0);

    ak_sz navail = (bytes - sizeof(ak_slab))/sz;
    AKMALLOC_ASSERT(navail > 0);

    ak_slab* s = (ak_slab*)mem;
    s->fd = s->bk = 0;
    s->root = root;
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

static ak_slab* ak_slab_new_pvt(char* mem, ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const ak_sz nslabs = ak_num_slabs_per_page(sz);
    const ak_sz nbytes = AKMALLOC_DEFAULT_PAGE_SIZE/nslabs;

    ak_slab* firstslab = ak_slab_init(mem, nbytes, sz, root);

    ak_sz i = 0;
    ak_slab* curr = firstslab;
    ak_slab* prev = bk;
    for (; i < nslabs - 1; ++i) {
        ak_slab_link_fd(prev, curr);
        ak_slab* next = ak_slab_init((char*)curr + nbytes, nbytes, sz, root);
        curr->fd = next;
        prev = curr;
        curr = next;
    }
    ak_slab_link_fd(prev, curr);
    ak_slab_link_fd(curr, fd);

    return firstslab;
}

static ak_slab* ak_slab_new(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const int NPAGES = root->npages;
    // try to acquire a page and fit as many slabs as possible in
    char* mem = (char*)ak_os_alloc(NPAGES * AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (ak_unlikely(!mem)) { return 0; }
    }

    for (int i = 0; i < NPAGES - 1; ++i) {
        bk = ak_slab_new_pvt(mem, sz, (ak_slab*)(mem + AKMALLOC_DEFAULT_PAGE_SIZE), bk, root);
        mem += AKMALLOC_DEFAULT_PAGE_SIZE;
    }

    ak_slab_new_pvt(mem, sz, fd, bk, root);

    return (ak_slab*)mem;
}

ak_inline static char* ak_slab_2_mem(ak_slab* s)
{
    return (char*)(void*)(s + 1);
}

ak_inline static int ak_slab_all_free(ak_slab* s)
{
    return ak_bitset_num_trailing_ones(&(s->avail)) == (int)((s->bytes - sizeof(ak_slab))/s->sz);
}

ak_inline static int ak_slab_none_free(ak_slab* s)
{
    return ak_bitset_num_trailing_zeros(&(s->avail)) == 32;
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

ak_inline static void* ak_slab_search(ak_slab* s, ak_slab** pslab, int* pntz)
{
    const ak_slab* const root = s;
    void* mem = 0;
    if (s->fd != root) {
        AKMALLOC_ASSERT(s->fd->sz == s->sz);
        AKMALLOC_ASSERT(s->fd->magic == AK_SLAB_MAGIC);
        AKMALLOC_ASSERT(pslab);
        AKMALLOC_ASSERT(pntz);
        s = s->fd;
        // partial list entry must not be full
        AKMALLOC_ASSERT(ak_bitset_num_trailing_zeros(&(s->avail)) != 32);
        mem = ak_slab_alloc_idx(s, ak_bitset_num_trailing_zeros(&(s->avail)));
        *pslab = s;
        *pntz = ak_bitset_num_trailing_zeros(&(s->avail));
    }
    return mem;
}

static void* ak_slab_alloc(ak_slab_root* root)
{
    int ntz = 0;
    ak_slab* slab = 0;
    void* mem = ak_slab_search(&(root->partial_root), &slab, &ntz);

    if (ak_unlikely(!mem)) {
        slab = ak_slab_new(root->sz, root->partial_root.fd, &(root->partial_root), root);
        if (ak_likely(slab)) {
            mem = ak_slab_alloc_idx(slab, 0);
        }
    } else if (ak_unlikely(ntz == 32)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->full_root.fd, &(root->full_root));
    }

    return mem;
}

static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* firstslab = (ak_slab*)(ak_page_start_before(p));
    AKMALLOC_ASSERT(firstslab->magic == AK_SLAB_MAGIC);
    AKMALLOC_ASSERT(firstslab->root);
    AKMALLOC_ASSERT(firstslab->root->magic == AK_SLAB_ROOT_MAGIC);

    ak_slab* rightslab = 0;
    {// free to the right slab
        ak_sz nslabsoff = (mem - (char*)firstslab)/firstslab->bytes;
        rightslab = (ak_slab*)((char*)firstslab + nslabsoff*firstslab->bytes);

        AKMALLOC_ASSERT(rightslab->root);
        AKMALLOC_ASSERT(rightslab->root->magic == AK_SLAB_ROOT_MAGIC);

        int movetopartial = ak_slab_none_free(rightslab);

        ak_sz idx = (mem - (char*)ak_slab_2_mem(rightslab))/rightslab->sz;
        AKMALLOC_ASSERT(rightslab->sz == firstslab->sz);
        ak_slab_free_idx(rightslab, idx);

        if (ak_unlikely(movetopartial)) {
            // put at the back of the partial list so the full ones
            // appear at the front
            ak_slab_root* root = rightslab->root;
            ak_slab_unlink(rightslab);
            ak_slab_link(rightslab, &(root->partial_root), root->partial_root.bk);
        }
    }

    //
    // to return memory to the OS, we see if all slabs from a page are free.
    // cannot rely on fd/bk as they may be linked to different slabs than
    // their siblings. traverse address wise.
    //
    if (ak_slab_all_free(rightslab) &&
        ((rightslab == firstslab) || ak_slab_all_free(firstslab))) {
        int allfree = 1;
        ak_slab* curr = firstslab->fd;
        ak_slab* prev = firstslab;
        ak_sz nslabs = ak_num_slabs_per_page(firstslab->sz);
        ak_sz i = 0;
        for (; i < nslabs - 1; ++i) {
            if (ak_likely(!ak_slab_all_free(curr))) {
                allfree = 0;
                break;
            }
            ak_slab* next = (ak_slab*)((char*)curr + curr->bytes);
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
