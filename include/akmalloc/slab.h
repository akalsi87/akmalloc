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

#include "akmalloc/spinlock.h"

typedef struct ak_slab_tag ak_slab;

typedef struct ak_slab_root_tag ak_slab_root;

typedef struct ak_slab_free_node_tag ak_slab_free_node;

#if defined(AK_SLAB_USE_LOCKS)
#  define AK_SLAB_LOCK_DEFINE(nm)    ak_spinlock nm
#  define AK_SLAB_LOCK_INIT(root)    ak_spinlock_init(ak_as_ptr((root)->LOCKED))
#  define AK_SLAB_LOCK_ACQUIRE(root) ak_spinlock_acquire(ak_as_ptr((root)->LOCKED))
#  define AK_SLAB_LOCK_RELEASE(root) ak_spinlock_release(ak_as_ptr((root)->LOCKED))
#else
#  define AK_SLAB_LOCK_DEFINE(nm)
#  define AK_SLAB_LOCK_INIT(root)
#  define AK_SLAB_LOCK_ACQUIRE(root)
#  define AK_SLAB_LOCK_RELEASE(root)
#endif

struct ak_slab_tag
{
    ak_slab*           fd;
    ak_slab*           bk;
    ak_slab_root*      root;
    ak_slab_free_node* next_free;
    size_t             ref_count;
    void*              _unused;
#if AKMALLOC_BITNESS == 32
    size_t             _unused0;
    size_t             _unused1;
#endif
};

/*!
 * Slab allocator
 */
struct ak_slab_root_tag
{
    ak_u32 sz;                      /**< the size of elements in this slab */
    ak_u32 navail;                  /**< max number of available bits for the slab size \p sz */
    ak_u32 nempty;                  /**< number of empty pages */
    ak_u32 release;                 /**< number of accumulated free empty pages since last release */

    ak_slab partial_root;           /**< root of the partially filled slab list*/
    ak_slab full_root;              /**< root of the full slab list */
    ak_slab empty_root;             /**< root of the empty slab list */

    ak_u32 RELEASE_RATE;            /**< number of pages moved to empty before a release */
    ak_u32 MAX_PAGES_TO_FREE;       /**< number of pages to free when release happens */
    AK_SLAB_LOCK_DEFINE(LOCKED);    /**< lock for this allocator if locks are enabled */
};

struct ak_slab_free_node_tag
{
    struct ak_slab_free_node_tag* next;
};

#if !defined(AK_SLAB_RELEASE_RATE)
#  define AK_SLAB_RELEASE_RATE 31
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

ak_inline static void ak_slab_init_slots_chain(ak_slab* s)
{
    ak_slab_free_node* n = (ak_slab_free_node*)(s + 1);
    char* cm = ak_ptr_cast(char, n);
    int slabsz = s->root->sz;
    s->next_free = n;
    for (ak_u32 i = 0; i < (ak_u32)s->ref_count - 1; ++i) {
        cm += slabsz;
        n->next = ak_ptr_cast(ak_slab_free_node, cm);
        n = n->next;
    }
    n->next = AK_NULLPTR;
}

ak_inline static void ak_slab_init_chain_head(ak_slab* s, ak_slab_root* rootp)
{
    s->fd = s->bk = s;
    s->root = rootp;
    s->ref_count = 0;
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
    s->ref_count = slabnavail;                                                \
    ak_slab_init_slots_chain(s);                                              \
    (void)slabsz;                                                             \
  } while (0)

ak_inline static ak_slab* ak_slab_new_init(char* mem, ak_sz sz, ak_sz navail, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    ak_slab_init(mem, sz, navail, root);
    ak_slab* slab = ak_ptr_cast(ak_slab, mem);
    ak_slab_link(slab, fd, bk);
    return slab;
}

static ak_slab* ak_slab_new_alloc(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    char* const mem = (char*)ak_os_alloc(AKMALLOC_DEFAULT_PAGE_SIZE);
    if (ak_unlikely(!mem)) { return AK_NULLPTR; }
    ak_slab_new_init(mem, sz, root->navail, fd, bk, root);
    return ak_ptr_cast(ak_slab, mem);
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

/*!
 * Initialize a slab allocator.
 * \param s; Pointer to the allocator root to initialize (non-NULL)
 * \param sz; Size of the slab elements (maximum allowed is 4000)
 * \param relrate; Release rate, \ref akmallocDox
 * \param maxpagefree; Number of segments to free upon release, \ref akmallocDox
 */
static void ak_slab_init_root(ak_slab_root* s, ak_sz sz, ak_u32 relrate, ak_u32 maxpagefree)
{
    s->sz = (ak_u32)sz;
    s->navail = (ak_u32)(AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/(ak_u32)sz;
    s->nempty = 0;
    s->release = 0;

    ak_slab_init_chain_head(&(s->partial_root), s);
    ak_slab_init_chain_head(&(s->full_root), s);
    ak_slab_init_chain_head(&(s->empty_root), s);

    s->RELEASE_RATE = relrate;
    s->MAX_PAGES_TO_FREE = maxpagefree;
    AK_SLAB_LOCK_INIT(s);
}

/*!
 * Default initialize a slab allocator.
 * \param s; Pointer to the allocator root to initialize (non-NULL)
 * \param sz; Size of the slab elements (maximum allowed is 4000)
 */
ak_inline static void ak_slab_init_root_default(ak_slab_root* s, ak_sz sz)
{
    ak_slab_init_root(s, sz, (ak_u32)(AK_SLAB_RELEASE_RATE), (ak_u32)(AK_SLAB_MAX_PAGES_TO_FREE));
}

/*!
 * Attempt to allocate memory from the slab allocator root.
 * \param root; Pointer to the allocator root
 *
 * \return \c 0 on failure, else pointer to at least \p root->sz bytes of memory.
 */
ak_inline static void* ak_slab_alloc(ak_slab_root* root)
{
    ak_slab* slab = &(root->partial_root);
    const ak_sz sz = root->sz;

    void* mem = AK_NULLPTR;
    AK_SLAB_LOCK_ACQUIRE(root);
    if (ak_likely(slab->fd != slab)) {// try allocation if not pointing to self
        slab = slab->fd;
        // AKMALLOC_ASSERT(slab->ref_count < root->navail);
        // disabled for now because we may overallocate pages so the overallocated pages
        // will have ref_count == root->navail
        AKMALLOC_ASSERT(slab->ref_count != 0);
        mem = slab->next_free;
        slab->next_free = slab->next_free->next;
        --(slab->ref_count);
    }

    if (ak_unlikely(!mem)) {
        slab = ak_slab_new(sz, root->partial_root.fd, &(root->partial_root), root);
        if (ak_likely(slab)) {
            // try allocation
            AKMALLOC_ASSERT(slab->ref_count == root->navail);
            AKMALLOC_ASSERT(slab->ref_count != 0);
            mem = slab->next_free;
            slab->next_free = slab->next_free->next;
            --(slab->ref_count);
            AKMALLOC_ASSERT(slab->ref_count < root->navail);
            AKMALLOC_ASSERT(slab->ref_count != 0);
        }
    } else if (ak_unlikely(slab->ref_count == 0)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->full_root.fd, &(root->full_root));
    }

    AK_SLAB_LOCK_RELEASE(root);

    return mem;
}

/*!
 * Return memory to the slab allocator root.
 * \param p; Pointer to the memory to return.
 */
ak_inline static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* slab = (ak_slab*)(ak_page_start_before(p));
    AKMALLOC_ASSERT(slab->root);

    ak_slab_root* root = slab->root;
    AK_SLAB_LOCK_ACQUIRE(root);

    int move_to_partial = slab->ref_count == 0;
    ak_slab_free_node* n = ak_ptr_cast(ak_slab_free_node, mem);
    n->next = slab->next_free;
    slab->next_free = n;
    ++(slab->ref_count);

    if (ak_unlikely(move_to_partial)) {
        // put at the back of the partial list so the full ones
        // appear at the front
        ak_slab_unlink(slab);
        ak_slab_link(slab, &(root->partial_root), root->partial_root.bk);
    } else if (ak_unlikely(slab->ref_count == root->navail)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->empty_root.fd, &(root->empty_root));
        ++(root->nempty); ++(root->release);
        if (root->release >= root->RELEASE_RATE) {
            ak_slab_release_os_mem(root);
        }
    }

    AK_SLAB_LOCK_RELEASE(root);
}

/*!
 * Destroy the slab allocator root and return all memory to the OS.
 * \param root; Pointer to the allocator root
 */
static void ak_slab_destroy(ak_slab_root* root)
{
    ak_slab_release_pages(root, &(root->empty_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->partial_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->full_root), AK_U32_MAX);
    root->nempty = 0;
    root->release = 0;
}

#endif/*AKMALLOC_SLAB_H*/
