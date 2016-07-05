/**
 * \file slab.h
 * \date Jul 04, 2016
 */

#ifndef AKMALLOC_SLAB_H
#define AKMALLOC_SLAB_H

/***********************************************
 * Slabs
 ***********************************************/

#include "akmalloc/setup.h"
#include "akmalloc/bitset.h"

typedef struct ak_slab_tag ak_slab;

typedef struct ak_slab_root_tag ak_slab_root;

struct ak_slab_tag
{
    ak_slab*      fd;
    ak_slab*      bk;
    ak_slab_root* root;
    ak_bitset512  avail;
};

struct ak_slab_root_tag
{
    ak_u32 sz;
    ak_u32 npages;
    ak_u32 navail;
    ak_u32 nempty;
    ak_u32 release;

    ak_slab partial_root;
    ak_slab full_root;
    ak_slab empty_root;
};

static const ak_u32 RELEASE_RATE = 512;
static const ak_u32 MAX_PAGES_TO_FREE = 50;

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
    ak_bitset512_clear_all(&(s->avail));
}

ak_inline static ak_sz ak_num_pages_for_sz(ak_sz sz)
{
    return (sz)/4;
}

ak_inline static void ak_slab_init_root(ak_slab_root* s, ak_sz sz)
{
    s->sz = sz;
    s->navail = (AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/sz;
    s->npages = ak_num_pages_for_sz(sz);
    s->nempty = 0;
    s->release = 0;

    ak_slab_init_chain_head(&(s->partial_root), s);
    ak_slab_init_chain_head(&(s->full_root), s);
    ak_slab_init_chain_head(&(s->empty_root), s);
}

ak_inline static ak_slab* ak_slab_init(void* mem, ak_sz sz, ak_sz navail, ak_slab_root* root)
{
    AKMALLOC_ASSERT(mem);
    AKMALLOC_ASSERT(sz < (AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab)));
    AKMALLOC_ASSERT(sz > 0);
    AKMALLOC_ASSERT(sz % 2 == 0);

    AKMALLOC_ASSERT(navail < 512);
    AKMALLOC_ASSERT(navail > 0);

    ak_slab* s = (ak_slab*)mem;
    s->fd = s->bk = 0;
    s->root = root;
    ak_bitset512_clear_all(&(s->avail));
    for (ak_sz i = 0; i < navail; ++i) {
        ak_bitset512_set(&(s->avail), i);
    }

    return s;
}

static ak_slab* ak_slab_new_pvt(char* mem, ak_sz sz, ak_sz navail, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    ak_slab* slab = ak_slab_init(mem, sz, navail, root);
    ak_slab_link(slab, fd, bk);
    return slab;
}

static ak_slab* ak_slab_new_alloc(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const int NPAGES = root->npages;

    // try to acquire a page and fit as many slabs as possible in
    char* const mem = (char*)ak_os_alloc(NPAGES * AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (ak_unlikely(!mem)) { return 0; }
    }

    ak_sz navail = (AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/sz;

    char* cmem = mem;
    for (int i = 0; i < NPAGES - 1; ++i) {
        ak_slab* nextpage = (ak_slab*)(cmem + AKMALLOC_DEFAULT_PAGE_SIZE);
        ak_slab* curr = ak_slab_new_pvt(cmem, sz, navail, nextpage, bk, root);
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_ones(&(curr->avail)) == navail);
        (void)curr;
        bk = nextpage;
        cmem += AKMALLOC_DEFAULT_PAGE_SIZE;
    }

    ak_slab_new_pvt(cmem, sz, navail, fd, bk, root);

    return (ak_slab*)mem;
}

static ak_slab* ak_slab_new_reuse(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const int NPAGES = root->npages;

    ak_sz navail = root->navail;

    ak_slab* const start = root->empty_root.fd;
    ak_slab* curr = start;

    for (int i = 0; i < NPAGES - 1; ++i) {
        ak_slab* nextpage = curr->fd;
        ak_slab_unlink(curr);
        curr = ak_slab_new_pvt((char*)curr, sz, navail, nextpage, bk, root);
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_ones(&(curr->avail)) == navail);
        (void)curr;
        bk = nextpage;
    }

    ak_slab_new_pvt((char*)curr, sz, navail, fd, bk, root);

    return start;
}

ak_inline static char* ak_slab_2_mem(ak_slab* s)
{
    return (char*)(void*)(s + 1);
}

ak_inline static int ak_slab_all_free(ak_slab* s)
{
    return ak_bitset512_num_trailing_ones(&(s->avail)) == s->root->navail;
}

ak_inline static int ak_slab_none_free(ak_slab* s)
{
    return ak_bitset512_num_trailing_zeros(&(s->avail)) == 512;
}

ak_inline static void* ak_slab_alloc_idx(ak_slab* s, ak_sz sz, int idx)
{
    AKMALLOC_ASSERT(ak_bitset512_get(&(s->avail), idx));
    ak_bitset512_clear(&(s->avail), idx);
    return ak_slab_2_mem(s) + (idx * sz);
}

ak_inline static void ak_slab_free_idx(ak_slab* s, int idx)
{
    AKMALLOC_ASSERT(!ak_bitset512_get(&(s->avail), idx));
    ak_bitset512_set(&(s->avail), idx);
}

ak_inline static void* ak_slab_search(ak_slab* s, ak_sz sz, ak_slab** pslab, int* pntz)
{
    const ak_slab* const root = s;
    void* mem = 0;
    if (s->fd != root) {
        AKMALLOC_ASSERT(pslab);
        AKMALLOC_ASSERT(pntz);
        s = s->fd;
        // partial list entry must not be full
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_zeros(&(s->avail)) != 512);
        mem = ak_slab_alloc_idx(s, sz, ak_bitset512_num_trailing_zeros(&(s->avail)));
        *pslab = s;
        *pntz = ak_bitset512_num_trailing_zeros(&(s->avail));
    }
    return mem;
}

static void* ak_slab_alloc(ak_slab_root* root)
{
    int ntz = 0;
    ak_slab* slab = 0;
    const ak_sz sz = root->sz;
    void* mem = ak_slab_search(&(root->partial_root), sz, &slab, &ntz);

    if (ak_unlikely(!mem)) {
        if (root->nempty > root->npages) {
            slab = ak_slab_new_reuse(sz, root->partial_root.fd, &(root->partial_root), root);
        } else {
            slab = ak_slab_new_alloc(sz, root->partial_root.fd, &(root->partial_root), root);
        }
        if (ak_likely(slab)) {
            mem = ak_slab_alloc_idx(slab, sz, 0);
        }
    } else if (ntz == 512) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->full_root.fd, &(root->full_root));
    }

    return mem;
}

static void ak_slab_release_empty_pages(ak_slab_root* root)
{
    ak_u32 ct = 0;
    ak_u32 numtofree = root->nempty/2;
    numtofree = numtofree > MAX_PAGES_TO_FREE ? MAX_PAGES_TO_FREE : numtofree;
    ak_slab* s = (root->empty_root.fd);

    for (; ct != numtofree; ++ct) {
        ak_slab* next = s->fd;
        ak_slab_unlink(s);
        ak_os_free(s, AKMALLOC_DEFAULT_PAGE_SIZE);
        s = next;
    }
    root->nempty -= numtofree;
    root->release = 0;
}

static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* slab = (ak_slab*)(ak_page_start_before(p));
    AKMALLOC_ASSERT(slab->root);

    int movetopartial = ak_slab_none_free(slab);

    ak_slab_root* root = slab->root;
    const ak_sz sz = root->sz;

    ak_sz idx = (mem - (char*)ak_slab_2_mem(slab))/sz;
    ak_slab_free_idx(slab, idx);

    if (movetopartial) {
        // put at the back of the partial list so the full ones
        // appear at the front
        ak_slab_root* root = slab->root;
        ak_slab_unlink(slab);
        ak_slab_link(slab, &(root->partial_root), root->partial_root.bk);
    } else if (ak_slab_all_free(slab)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->empty_root.fd, &(root->empty_root));
        ++(root->nempty);
        ++(root->release);
        if (root->release >= RELEASE_RATE) {
            ak_slab_release_empty_pages(root);
        }
    }
}

#endif/*AKMALLOC_SLAB_H*/
