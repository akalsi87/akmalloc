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

typedef void* ak_alloc_info;

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

ak_inline static void* ak_ca_to_ptr(ak_alloc_info p)
{
    return (void*)(((ak_sz)p) & ~(AK_COALESCE_ALIGN - 1));
}

ak_inline static ak_sz ak_ca_is_first(ak_alloc_info p)
{
    return (((ak_sz)p) & (AK_SZ_ONE << 0));
}

ak_inline static ak_sz ak_ca_is_last(ak_alloc_info p)
{
    return (((ak_sz)p) & (AK_SZ_ONE << 1));
}

ak_inline static ak_sz ak_ca_is_free(ak_alloc_info p)
{
    return (((ak_sz)p) & (AK_SZ_ONE << 2));
}

ak_inline static void ak_ca_set_ptr(ak_alloc_info* p, void* ptr)
{
    AKMALLOC_ASSERT(ptr == ak_ca_to_ptr((ak_alloc_info)ptr));
    *p = (ak_alloc_info)((((ak_sz)*p)  &  (AK_COALESCE_ALIGN - 1)) |
                         (((ak_sz)ptr) & ~(AK_COALESCE_ALIGN - 1)));
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

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

static void ak_ca_init_root(ak_ca_root* root, ak_u32 relrate, ak_u32 maxsegstofree)
{
    ak_ca_segment_link(&(root->main_root), &(root->main_root), &(root->main_root));
    ak_ca_segment_link(&(root->empty_root), &(root->empty_root), &(root->empty_root));
    ak_free_list_node_link(&(root->free_root), &(root->free_root), &(root->free_root));
    root->nempty = root->release = 0;

    root->RELEASE_RATE = relrate;
    root->MAX_SEGMENTS_TO_FREE = maxsegstofree;
}

ak_inline static void ak_ca_init_root_default(ak_ca_root* root)
{
    ak_ca_init_root(root, 4095, 10);
}

static void ak_ca_destroy(ak_ca_root* root)
{
    root->nempty = root->release = 0;
}

#endif/*AKMALLOC_COALESCING_ALLOC_H*/
