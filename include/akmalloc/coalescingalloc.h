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
 * \file coalescingalloc.h
 * \date Jul 07, 2016
 */

#ifndef AKMALLOC_COALESCING_ALLOC_H
#define AKMALLOC_COALESCING_ALLOC_H

#include "akmalloc/assert.h"
#include "akmalloc/inline.h"
#include "akmalloc/constants.h"
#include "akmalloc/setup.h"
#include "akmalloc/utils.h"

/**
 * We choose a minimum alignment of 16. One could increase this, but not decrease.
 *
 * 16 byte alignment buys us a few things:
 * - The 3 low-bits of an address will be 000. Therefore we can store metadata in them.
 * - On x64, we can exactly store two pointers worth of information in any block which
 *   can be used to house an implicit free list.
 */
#define AK_COALESCE_ALIGN 16

#define AK_COALESCE_SEGMENT_GRANULARITY 65536

#if !defined(AK_COALESCE_SEGMENT_SIZE)
/* 64KB */
#  define AK_COALESCE_SEGMENT_SIZE AK_COALESCE_SEGMENT_GRANULARITY
#endif

typedef ak_sz ak_alloc_info;

typedef struct ak_alloc_node_tag ak_alloc_node;

typedef struct ak_free_list_node_tag ak_free_list_node;

typedef struct ak_ca_segment_tag ak_ca_segment;

typedef struct ak_ca_root_tag ak_ca_root;

struct ak_alloc_node_tag
{
    ak_alloc_info previnfo;
    ak_alloc_info currinfo;
};

struct ak_free_list_node_tag
{
    ak_free_list_node* bk;
    ak_free_list_node* fd;
};

struct ak_ca_segment_tag
{
    ak_ca_segment* bk;
    ak_ca_segment* fd;
    ak_sz sz;
    ak_alloc_node* head;
};

struct ak_ca_root_tag
{
    ak_ca_segment main_root;
    ak_ca_segment empty_root;

    ak_free_list_node free_root;

    ak_u32 nempty;
    ak_u32 release;

    ak_u32 RELEASE_RATE;
    ak_u32 MAX_SEGMENTS_TO_FREE;
};

/**************************************************************/
/* P R I V A T E                                              */
/**************************************************************/

#define ak_as_ptr(x) (&(x))

#define ak_ca_to_sz(p) (((ak_sz)(p)) & ~(AK_COALESCE_ALIGN - 1))

#define ak_ca_is_first(p) (((ak_sz)(p)) & (AK_SZ_ONE << 0))

#define ak_ca_is_last(p) (((ak_sz)(p)) & (AK_SZ_ONE << 1))

#define ak_ca_is_free(p) (((ak_sz)(p)) & (AK_SZ_ONE << 2))

ak_inline static void ak_ca_set_sz(ak_alloc_info* p, ak_sz sz)
{
    AKMALLOC_ASSERT(sz == ak_ca_to_sz((ak_alloc_info)sz));
    *p = (ak_alloc_info)((((ak_sz)*p)  &  (AK_COALESCE_ALIGN - 1)) |
                                 (sz   & ~(AK_COALESCE_ALIGN - 1)));
}

ak_inline static void ak_ca_set_is_first(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 0)) | (v ? (AK_SZ_ONE << 0) : 0));
}

ak_inline static void ak_ca_set_is_last(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 1)) | (v ? (AK_SZ_ONE << 1) : 0));
}

ak_inline static void ak_ca_set_is_free(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 2)) | (v ? (AK_SZ_ONE << 2) : 0));
}

ak_inline static ak_alloc_node* ak_ca_next_node(ak_alloc_node* node)
{
    return ak_ca_is_last(node->currinfo)
                ? AK_NULLPTR
                : (ak_alloc_node*)((char*)(node + 1) + ak_ca_to_sz(node->currinfo));
}

ak_inline static ak_alloc_node* ak_ca_prev_node(ak_alloc_node* node)
{
    return ak_ca_is_first(node->currinfo)
                ? AK_NULLPTR
                : (ak_alloc_node*)((char*)(node - 1) - ak_ca_to_sz(node->previnfo));
}

ak_inline static void ak_ca_update_footer(ak_alloc_node* p)
{
    ak_alloc_node* n = ak_ca_next_node(p);
    if (n) {
        n->previnfo = p->currinfo;
    }
}

#define ak_free_list_node_unlink(node)               \
  do {                                               \
    ak_free_list_node* const sU = (node);            \
    sU->bk->fd = (sU->fd);                           \
    sU->fd->bk = (sU->bk);                           \
    sU->fd = sU->bk = AK_NULLPTR;                    \
  } while (0)

#define ak_free_list_node_link_fd(node, fwd)         \
  do {                                               \
    ak_free_list_node* const sLF = (node);           \
    ak_free_list_node* const fLF = (fwd);            \
    sLF->fd = fLF;                                   \
    fLF->bk = sLF;                                   \
  } while (0)

#define ak_free_list_node_link_bk(node, back)        \
  do {                                               \
    ak_free_list_node* const sLB = (node);           \
    ak_free_list_node* const bLB = (back);           \
    sLB->bk = bLB;                                   \
    bLB->fd = sLB;                                   \
  } while (0)

#define ak_free_list_node_link(node, fwd, back)      \
  do {                                               \
    ak_free_list_node* const sL = (node);            \
    ak_free_list_node* const fL = (fwd);             \
    ak_free_list_node* const bL = (back);            \
    ak_free_list_node_link_bk(sL, bL);               \
    ak_free_list_node_link_fd(sL, fL);               \
  } while (0)

#define ak_ca_segment_unlink(node)                   \
  do {                                               \
    ak_ca_segment* const sU = (node);                \
    sU->bk->fd = (sU->fd);                           \
    sU->fd->bk = (sU->bk);                           \
    sU->fd = sU->bk = AK_NULLPTR;                    \
  } while (0)

#define ak_ca_segment_link_fd(node, fwd)             \
  do {                                               \
    ak_ca_segment* const sLF = (node);               \
    ak_ca_segment* const fLF = (fwd);                \
    sLF->fd = fLF;                                   \
    fLF->bk = sLF;                                   \
  } while (0)

#define ak_ca_segment_link_bk(node, back)            \
  do {                                               \
    ak_ca_segment* const sLB = (node);               \
    ak_ca_segment* const bLB = (back);               \
    sLB->bk = bLB;                                   \
    bLB->fd = sLB;                                   \
  } while (0)

#define ak_ca_segment_link(node, fwd, back)          \
  do {                                               \
    ak_ca_segment* const sL = (node);                \
    ak_ca_segment* const fL = (fwd);                 \
    ak_ca_segment* const bL = (back);                \
    ak_ca_segment_link_bk(sL, bL);                   \
    ak_ca_segment_link_fd(sL, fL);                   \
  } while (0)

#define ak_circ_list_for_each(type, name, list)      \
    type* name = (list)->fd;                         \
    for(type* const iterroot = (list); name != iterroot; name = name->fd)

#define ak_ca_aligned_size(x) (x) ? (((x) + AK_COALESCE_ALIGN - 1) & ~(AK_COALESCE_ALIGN - 1)) : AK_COALESCE_ALIGN

#define ak_ca_aligned_segment_size(x) (((x) + (AK_COALESCE_SEGMENT_SIZE) - 1) & ~((AK_COALESCE_SEGMENT_SIZE) - 1))

static void* ak_ca_search_free_list(ak_free_list_node* root, ak_sz sz)
{
    // walk through list finding the first element that fits and split
    ak_circ_list_for_each(ak_free_list_node, node, root) {
        ak_alloc_node* n = (ak_alloc_node*)(node - 1);
        AKMALLOC_ASSERT(ak_ca_is_free(n->currinfo));
        ak_sz nodesz = ak_ca_to_sz(n->currinfo);
        if (nodesz >= sz) {
            if ((nodesz - sz) > (sizeof(ak_alloc_node) + sizeof(ak_free_list_node))) {
                // split and assign
                ak_alloc_node* newnode = (ak_alloc_node*)(((char*)node) + sz);
                int islast = ak_ca_is_last(n->currinfo);

                ak_ca_set_sz(ak_as_ptr(n->currinfo), sz);
                ak_ca_set_is_last(ak_as_ptr(n->currinfo), 0);
                ak_ca_set_is_free(ak_as_ptr(n->currinfo), 0);
                ak_ca_update_footer(n);

                ak_ca_set_sz(ak_as_ptr(newnode->currinfo), nodesz - sz - sizeof(ak_alloc_node));
                ak_ca_set_is_first(ak_as_ptr(newnode->currinfo), 0);
                ak_ca_set_is_last(ak_as_ptr(newnode->currinfo), islast);
                ak_ca_set_is_free(ak_as_ptr(newnode->currinfo), 1);
                ak_ca_update_footer(newnode);
                
                // copy free list node from node
                ak_free_list_node* fl = (ak_free_list_node*)(newnode + 1);
                ak_free_list_node_link(fl, node->fd, node->bk);
                AKMALLOC_ASSERT(n->currinfo == newnode->previnfo);
            } else {
                // return as is
                ak_ca_set_is_free(ak_as_ptr(n->currinfo), 0);
                ak_ca_update_footer(n);
                ak_free_list_node_unlink(node);
            }
            return node;
        }
    }
    return AK_NULLPTR;
}

static int ak_ca_add_new_segment(ak_ca_root* root, char* mem, ak_sz sz)
{
    if (ak_likely(mem)) {
        // make segment
        ak_ca_segment* seg = (ak_ca_segment*)(mem + sz - sizeof(ak_ca_segment));
        ak_ca_segment_link(seg, root->main_root.fd, ak_as_ptr(root->main_root));
        seg->sz = sz;
        seg->head = (ak_alloc_node*)mem;
        {// add to free list
            ak_alloc_node* hd = seg->head;
            ak_ca_set_is_first(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_is_last(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_is_free(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_sz(ak_as_ptr(hd->currinfo), (sz - sizeof(ak_alloc_node) - sizeof(ak_ca_segment)));
            ak_free_list_node* fl = (ak_free_list_node*)(hd + 1);
            ak_free_list_node_link(fl, root->free_root.fd, ak_as_ptr(root->free_root));
        }
        return 1;
    }
    return 0;
}

static int ak_ca_get_new_segment(ak_ca_root* root, ak_sz sz)
{
    // align to segment size multiple
    sz += sizeof(ak_ca_segment) + sizeof(ak_alloc_node) + sizeof(ak_free_list_node);
    sz = ak_ca_aligned_segment_size(sz);

    // search empty_root for a segment that is as big or more
    char* mem = AK_NULLPTR;
    ak_sz segsz = sz;
    ak_circ_list_for_each(ak_ca_segment, seg, ak_as_ptr(root->empty_root)) {
        if (seg->sz >= sz) {
            mem = (char*)(seg->head);
            segsz = seg->sz;
            ak_ca_segment_unlink(seg);
            --(root->nempty);
            break;
        }
    }

    return ak_ca_add_new_segment(root, mem ? mem : ((char*)ak_os_alloc(sz)), segsz);
}

static ak_u32 ak_ca_return_os_mem(ak_ca_segment* r, ak_u32 num)
{
    ak_u32 ct = 0;
    ak_ca_segment* next = r->fd;
    ak_ca_segment* curr = next;
    for(; curr != r; curr = next) {
        if (ct >= num) {
            break;
        }
        next = curr->fd;
        ak_ca_segment_unlink(curr);
        ak_os_free(curr->head, curr->sz);
        ++ct;
    }
    return ct;
}

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

static void ak_ca_init_root(ak_ca_root* root, ak_u32 relrate, ak_u32 maxsegstofree)
{
    AKMALLOC_ASSERT_ALWAYS(AK_COALESCE_SEGMENT_SIZE % AK_COALESCE_SEGMENT_GRANULARITY == 0);
    AKMALLOC_ASSERT_ALWAYS(((AK_COALESCE_SEGMENT_SIZE & (AK_COALESCE_SEGMENT_SIZE - 1)) == 0) && "Segment size must be a power of 2");

    ak_ca_segment_link(&(root->main_root), &(root->main_root), &(root->main_root));
    ak_ca_segment_link(&(root->empty_root), &(root->empty_root), &(root->empty_root));
    ak_free_list_node_link(&(root->free_root), &(root->free_root), &(root->free_root));
    root->nempty = root->release = 0;

    root->RELEASE_RATE = relrate;
    root->MAX_SEGMENTS_TO_FREE = maxsegstofree;
}

ak_inline static void ak_ca_init_root_default(ak_ca_root* root)
{
    ak_ca_init_root(root, 2048, 2048);
}

static void* ak_ca_alloc(ak_ca_root* root, ak_sz s)
{
    // align and round size
    ak_sz sz = ak_ca_aligned_size(s);
    // search free list
    void* mem = ak_ca_search_free_list(ak_as_ptr(root->free_root), sz);
    // add new segment
    if (ak_unlikely(!mem)) {
        // NOTE: could also move segments from empty_root to main_root
        if (ak_likely(ak_ca_get_new_segment(root, sz))) {
            mem = ak_ca_search_free_list(ak_as_ptr(root->free_root), sz);
            AKMALLOC_ASSERT(mem);
        }
    }
    return mem;
}

static void ak_ca_free(ak_ca_root* root, void* m)
{
    // get alloc header before
    ak_alloc_node* node = ((ak_alloc_node*)m) - 1;
    ak_alloc_node* nextnode = ak_ca_next_node(node);
    ak_alloc_node* prevnode = ak_ca_prev_node(node);
    int coalesce = 0;

    // mark as free
    AKMALLOC_ASSERT(!ak_ca_is_free(node->currinfo));
    AKMALLOC_ASSERT(!nextnode || (node->currinfo == nextnode->previnfo));
    ak_ca_set_is_free(ak_as_ptr(node->currinfo), 1);
    ak_ca_update_footer(node);
    
    // NOTE: maybe this should happen at a lower frequency?
    // coalesce if free before or if free after or both
    if (prevnode && ak_ca_is_free(node->previnfo)) {
        // coalesce back
        // update node and the footer
        ak_sz newsz = ak_ca_to_sz(node->previnfo) + ak_ca_to_sz(node->currinfo) + sizeof(ak_alloc_node);
        ak_ca_set_sz(ak_as_ptr(prevnode->currinfo), newsz);
        ak_ca_set_is_last(ak_as_ptr(prevnode->currinfo), nextnode == AK_NULLPTR);
        ak_ca_update_footer(prevnode);
        AKMALLOC_ASSERT(!nextnode || ak_ca_next_node(prevnode) == nextnode);
        AKMALLOC_ASSERT(!nextnode || prevnode->currinfo == nextnode->previnfo);
        coalesce += 1;
        // update free list
    }

    if (nextnode && ak_ca_is_free(nextnode->currinfo)) {
        // coalesce forward
        // update node and the footer
        ak_alloc_node* n = (coalesce) ? prevnode : node;
        ak_sz newsz = ak_ca_to_sz(n->currinfo) + ak_ca_to_sz(nextnode->currinfo) + sizeof(ak_alloc_node);
        ak_ca_set_sz(ak_as_ptr(n->currinfo), newsz);
        ak_ca_set_is_last(ak_as_ptr(n->currinfo), ak_ca_is_last(nextnode->currinfo));
        ak_ca_update_footer(n);
        AKMALLOC_ASSERT(ak_ca_is_last(n->currinfo) || (n->currinfo == ak_ca_next_node(nextnode)->previnfo));
        coalesce += 2;
    }

    // update free lists
    ak_alloc_node* tocheck = AK_NULLPTR;
    switch (coalesce) {
        case 0: {
                // thread directly
                ak_free_list_node* fl = (ak_free_list_node*)(node + 1);
                ak_free_list_node_link(fl, root->free_root.fd, ak_as_ptr(root->free_root));
            }
            break;
        case 1: {
                // prevnode already threaded through
                tocheck = prevnode;
            }
            break;
        case 2: {
                // copy free list entry from nextnode
                ak_free_list_node* fl = (ak_free_list_node*)(node + 1);
                ak_free_list_node* nextfl = (ak_free_list_node*)(nextnode + 1);
                ak_free_list_node_link(fl, nextfl->fd, nextfl->bk);
                tocheck = node;
            }
            break;
        case 3: {
                ak_free_list_node* nextfl = (ak_free_list_node*)(nextnode + 1);
                ak_free_list_node_unlink(nextfl);
                tocheck = prevnode;
            }
            break;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Should not get here!");
            break;
    }

    // move to empty if segment is empty

    if (tocheck && ak_ca_is_first(tocheck->currinfo) && ak_ca_is_last(tocheck->currinfo)) {
        // remove free list entry
        ak_free_list_node* fl = (ak_free_list_node*)(tocheck + 1);
        ak_free_list_node_unlink(fl);
        ak_ca_segment* seg = (ak_ca_segment*)((char*)(tocheck + 1) + ak_ca_to_sz(tocheck->currinfo));
        ak_ca_segment_unlink(seg);
        ak_ca_segment_link(seg, root->empty_root.fd, ak_as_ptr(root->empty_root));
        ++(root->nempty); ++(root->release);
        // check if we should free empties
        if (root->release >= root->RELEASE_RATE) {
            // release segment
            ak_u32 nrem = ak_ca_return_os_mem(ak_as_ptr(root->empty_root), root->MAX_SEGMENTS_TO_FREE);
            root->nempty -= nrem;
            root->release = 0;
        }
    }
}

static void ak_ca_destroy(ak_ca_root* root)
{
    ak_ca_return_os_mem(ak_as_ptr(root->main_root), AK_U32_MAX);
    ak_ca_return_os_mem(ak_as_ptr(root->empty_root), AK_U32_MAX);
    root->nempty = root->release = 0;
}

#endif/*AKMALLOC_COALESCING_ALLOC_H*/
