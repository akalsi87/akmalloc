/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

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

    ak_u32 RELEASE_RATE;
    ak_u32 MAX_PAGES_TO_FREE;
};

#if !defined(AK_SLAB_RELEASE_RATE)
#  define AK_SLAB_RELEASE_RATE 512
#endif

#if !defined(AK_SLAB_MAX_PAGES_TO_FREE)
#  define AK_SLAB_MAX_PAGES_TO_FREE AK_SLAB_RELEASE_RATE
#endif

/**************************************************************/
/* P R I V A T E                                              */
/**************************************************************/

#define ak_slab_unlink(slab)               \
  do {                                     \
    ak_slab* const sU = (slab);            \
    sU->bk->fd = (sU->fd);                 \
    sU->fd->bk = (sU->bk);                 \
    sU->fd = sU->bk = AK_NULLPTR;          \
  } while (0)

#define ak_slab_link_fd(slab, fwd)         \
  do {                                     \
    ak_slab* const sLF = (slab);           \
    ak_slab* const fLF = (fwd);            \
    sLF->fd = fLF;                         \
    fLF->bk = sLF;                         \
  } while (0)

#define ak_slab_link_bk(slab, back)        \
  do {                                     \
    ak_slab* const sLB = (slab);           \
    ak_slab* const bLB = (back);           \
    sLB->bk = bLB;                         \
    bLB->fd = sLB;                         \
  } while (0)

#define ak_slab_link(slab, fwd, back)      \
  do {                                     \
    ak_slab* const sL = (slab);            \
    ak_slab* const fL = (fwd);             \
    ak_slab* const bL = (back);            \
    ak_slab_link_bk(sL, bL);               \
    ak_slab_link_fd(sL, fL);               \
  } while (0)

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

#define ak_slab_init(m, s, av, r)                                             \
  do {                                                                        \
    void* slabmem = (m);                                                      \
    ak_sz slabsz = (s);                                                       \
    ak_sz slabnavail = (av);                                                  \
    ak_slab_root* slabroot = (r);                                             \
                                                                              \
    AKMALLOC_ASSERT(slabmem);                                                 \
    AKMALLOC_ASSERT(slabsz < (AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))); \
    AKMALLOC_ASSERT(slabsz > 0);                                              \
    AKMALLOC_ASSERT(slabsz % 2 == 0);                                         \
                                                                              \
    AKMALLOC_ASSERT(slabnavail < 512);                                        \
    AKMALLOC_ASSERT(slabnavail > 0);                                          \
                                                                              \
    ak_slab* s = (ak_slab*)slabmem;                                           \
    s->fd = s->bk = AK_NULLPTR;                                               \
    s->root = slabroot;                                                       \
    ak_bitset512_clear_all(&(s->avail));                                      \
    int inavail = (int)slabnavail;                                            \
    for (int i = 0; i < inavail; ++i) {                                       \
        ak_bitset512_set(&(s->avail), i);                                     \
    }                                                                         \
    (void)slabsz;                                                             \
  } while (0)

static ak_slab* ak_slab_new_init(char* mem, ak_sz sz, ak_sz navail, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    ak_slab_init(mem, sz, navail, root);
    ak_slab* slab = (ak_slab*)mem;
    ak_slab_link(slab, fd, bk);
    return slab;
}

static ak_slab* ak_slab_new_alloc(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const int NPAGES = root->npages;

    // try to acquire a page and fit as many slabs as possible in
    char* const mem = (char*)ak_os_alloc(NPAGES * AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (ak_unlikely(!mem)) { return AK_NULLPTR; }
    }

    ak_sz navail = root->navail;

    char* cmem = mem;
    for (int i = 0; i < NPAGES - 1; ++i) {
        ak_slab* nextpage = (ak_slab*)(cmem + AKMALLOC_DEFAULT_PAGE_SIZE);
        ak_slab* curr = ak_slab_new_init(cmem, sz, navail, nextpage, bk, root);
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_ones(&(curr->avail)) == (int)navail);
        (void)curr;
        bk = nextpage;
        cmem += AKMALLOC_DEFAULT_PAGE_SIZE;
    }

    ak_slab_new_init(cmem, sz, navail, fd, bk, root);

    return (ak_slab*)mem;
}

static ak_slab* ak_slab_new_reuse(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    AKMALLOC_ASSERT(root->nempty >= 1);

    ak_sz navail = root->navail;
    ak_slab* const curr = root->empty_root.fd;
    ak_slab_unlink(curr);
    ak_slab_new_init((char*)curr, sz, navail, fd, bk, root);

    --(root->nempty);

    return curr;
}

ak_inline static ak_slab* ak_slab_new(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    return (root->nempty > 0)
                ? ak_slab_new_reuse(sz, fd, bk, root)
                : ak_slab_new_alloc(sz, fd, bk, root);
}

#define ak_slab_2_mem(s) (char*)(void*)((s) + 1)

static int ak_slab_all_free(ak_slab* s)
{
    const ak_bitset512* pavail = &(s->avail);
    ak_u32 nto;
    ak_bitset512_fill_num_trailing_ones(pavail, nto);
    return nto == s->root->navail;
}

static int ak_slab_none_free(ak_slab* s)
{
    const ak_bitset512* pavail = &(s->avail);
    int ntz;
    ak_bitset512_fill_num_trailing_zeros(pavail, ntz);
    return ntz == 512;
}

static void* ak_slab_search(ak_slab* s, ak_sz sz, ak_u32 navail, ak_slab** pslab, int* pntz)
{
    const ak_slab* const root = s;
    void* mem = AK_NULLPTR;
    if (ak_likely(s->fd != root)) {
        AKMALLOC_ASSERT(pslab);
        AKMALLOC_ASSERT(pntz);
        s = s->fd;

        // partial list entry must not be full
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_zeros(&(s->avail)) != 512);

        const ak_bitset512* pavail = &(s->avail);
        int ntz;
        ak_bitset512_fill_num_trailing_zeros(pavail, ntz);

        AKMALLOC_ASSERT(ak_bitset512_get(&(s->avail), ntz));
        ak_bitset512_clear(&(s->avail), ntz);
        mem = ak_slab_2_mem(s) + (ntz * sz);

        *pslab = s;
        *pntz = (ntz == (int)navail - 1) ? 512 : (ntz + 1);
    }
    return mem;
}

static void ak_slab_release_pages(ak_slab_root* root, ak_slab* s, ak_u32 numtofree)
{
    ak_slab* const r = s;
    ak_slab* next = AK_NULLPTR;
    s = s->fd;
    for (ak_u32 ct = 0; ct < numtofree; ++ct) {
        if (s == r) {
            break;
        } else {
            next = s->fd;
        }
        ak_slab_unlink(s);
        ak_os_free(s, AKMALLOC_DEFAULT_PAGE_SIZE);
        s = next;
    }
}

ak_inline static void ak_slab_release_os_mem(ak_slab_root* root)
{
    ak_u32 numtofree = root->nempty;
    numtofree = (numtofree > root->MAX_PAGES_TO_FREE)
                    ? root->MAX_PAGES_TO_FREE
                    : numtofree;
    ak_slab_release_pages(root, &(root->empty_root), numtofree);
    root->nempty -= numtofree;
    root->release = 0;
}

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

static void ak_slab_init_root(ak_slab_root* s, ak_sz sz, ak_u32 npages, ak_u32 relrate, ak_u32 maxpagefree)
{
    s->sz = (ak_u32)sz;
    s->navail = (ak_u32)(AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/(ak_u32)sz;
    s->npages = npages;
    s->nempty = 0;
    s->release = 0;

    ak_slab_init_chain_head(&(s->partial_root), s);
    ak_slab_init_chain_head(&(s->full_root), s);
    ak_slab_init_chain_head(&(s->empty_root), s);

    s->RELEASE_RATE = relrate;
    s->MAX_PAGES_TO_FREE = maxpagefree;
}

ak_inline static void ak_slab_init_root_default(ak_slab_root* s, ak_sz sz)
{
    ak_slab_init_root(s, sz, ak_num_pages_for_sz(sz), (AK_SLAB_RELEASE_RATE), (AK_SLAB_MAX_PAGES_TO_FREE));
}

static void* ak_slab_alloc(ak_slab_root* root)
{
    int ntz = 0;
    ak_slab* slab = AK_NULLPTR;
    const ak_sz sz = root->sz;
    void* mem = ak_slab_search(&(root->partial_root), sz, root->navail, &slab, &ntz);

    if (ak_unlikely(!mem)) {
        slab = ak_slab_new(sz, root->partial_root.fd, &(root->partial_root), root);
        if (ak_likely(slab)) {
            AKMALLOC_ASSERT(ak_bitset512_get(&(slab->avail), 0));
            ak_bitset512_clear(&(slab->avail), 0);
            return ak_slab_2_mem(slab);
        }
    } else if (ak_unlikely(ntz == 512)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->full_root.fd, &(root->full_root));
    }

    return mem;
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

    int idx = (int)(mem - (char*)ak_slab_2_mem(slab))/(int)sz;
    AKMALLOC_ASSERT(!ak_bitset512_get(&(slab->avail), idx));
    ak_bitset512_set(&(slab->avail), idx);

    if (ak_unlikely(movetopartial)) {
        // put at the back of the partial list so the full ones
        // appear at the front
        ak_slab_unlink(slab);
        ak_slab_link(slab, &(root->partial_root), root->partial_root.bk);
    } else if (ak_unlikely(ak_slab_all_free(slab))) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->empty_root.fd, &(root->empty_root));
        ++(root->nempty); ++(root->release);
        if (root->release >= root->RELEASE_RATE) {
            ak_slab_release_os_mem(root);
        }
    }
}

static void ak_slab_destroy(ak_slab_root* root)
{
    ak_slab_release_pages(root, &(root->empty_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->partial_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->full_root), AK_U32_MAX);
    root->nempty = 0;
    root->release = 0;
}

#endif/*AKMALLOC_SLAB_H*/
