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
 * \file mallocstate.h
 * \date Jul 12, 2016
 */

#ifndef AKMALLOC_MALLOCSTATE_H
#define AKMALLOC_MALLOCSTATE_H

#if !AKMALLOC_WINDOWS
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#endif

#include "akmalloc/spinlock.h"

#if defined(AK_MALLOCSTATE_USE_LOCKS)
#  define AK_SLAB_USE_LOCKS
#  define AK_CA_USE_LOCKS
#  define AKMALLOC_LOCK_DEFINE(nm)  ak_spinlock nm
#  define AKMALLOC_LOCK_INIT(lk)    ak_spinlock_init((lk))
#  define AKMALLOC_LOCK_ACQUIRE(lk) ak_spinlock_acquire((lk))
#  define AKMALLOC_LOCK_RELEASE(lk) ak_spinlock_release((lk))
#else
#  define AKMALLOC_LOCK_DEFINE(nm)
#  define AKMALLOC_LOCK_INIT(lk)
#  define AKMALLOC_LOCK_ACQUIRE(lk)
#  define AKMALLOC_LOCK_RELEASE(lk)
#endif

#if !defined(AK_COALESCE_SEGMENT_GRANULARITY)
#  define AK_COALESCE_SEGMENT_GRANULARITY (((size_t)1) << 18) /* 256KB */
#endif

#include "akmalloc/slab.h"
#include "akmalloc/coalescingalloc.h"
#include "akmalloc/setup.h"

#if !defined(AK_SEG_CBK_DEFINED)
/**
 * gets pointer to segment, and size of segment.
 * return non-zero to keep continuing
 */
typedef int(*ak_seg_cbk)(const void*, size_t);
#endif

static void* ak_memset(void* m, int v, ak_sz sz)
{
    char* mem = (char*)m;
    for (ak_sz i = 0; i != sz; ++i) {
        mem[i] = v;
    }
    return m;
}

static void ak_memcpy(void* d, const void* s, ak_sz sz)
{
    char* mem = (char*)d;
    const char* srcmem = (const char*)s;
    for (ak_sz i = 0; i != sz; ++i) {
        mem[i] = srcmem[i];
    }
}

// Coalescing allocs give 16-byte aligned memory where the preamble
// uses three bits. The fourth bit is always free. We use that bit
// to distinguish slabs from coalesced outputs, and mmap-outputs.
//
// xxx0 - coalesce
// 0101 - slab
// 1001 - mmap
#define ak_alloc_type_bits(p) \
  ((*(((const ak_sz*)(p)) - 1)) & (AK_COALESCE_ALIGN - 1))

#define ak_alloc_type_coalesce(sz) \
  ((((ak_sz)sz) & ((ak_sz)8)) == 0)

#define ak_alloc_type_slab(sz) \
  ((((ak_sz)sz) & (AK_COALESCE_ALIGN - 1)) == 10)

#define ak_alloc_type_mmap(sz) \
  ((((ak_sz)sz) & (AK_COALESCE_ALIGN - 1)) == 9)

#define ak_alloc_mark_coalesce(p) ((void)(p))

#define ak_alloc_mark_slab(p) \
  *(((ak_sz*)(p)) - 1) = ((ak_sz)10)

#define ak_alloc_mark_mmap(p) \
  *(((ak_sz*)(p)) - 1) = ((ak_sz)9)

#define NSLABS 16

static const ak_sz SLAB_SIZES[NSLABS] = {
    16,   32,   48,   64,   80,   96,  112,  128,
   144,  160,  176,  192,  208,  224,  240,  256
};

#if defined(AK_MIN_SLAB_ALIGN_16)
#  define ak_slab_mod_sz(x) (ak_ca_aligned_size((x)) + AK_COALESCE_ALIGN)
#  define ak_slab_alloc_2_mem(x) (((ak_sz*)x) + (AK_COALESCE_ALIGN / sizeof(ak_sz)))
#  define ak_slab_mem_2_alloc(x) (((ak_sz*)x) - (AK_COALESCE_ALIGN / sizeof(ak_sz)))
#  define ak_slab_usable_size(x) ((x) - AK_COALESCE_ALIGN)
#else
#  define ak_slab_mod_sz(x) (ak_ca_aligned_size((x) + sizeof(ak_sz)))
#  define ak_slab_alloc_2_mem(x) (((ak_sz*)x) + 1)
#  define ak_slab_mem_2_alloc(x) (((ak_sz*)x) - 1)
#  define ak_slab_usable_size(x) ((x) - sizeof(ak_sz))
#endif

/* cannot be changed. we have fixed size slabs */
#define MIN_SMALL_REQUEST 256

#if !defined(MIN_MEDIUM_REQUEST)
#  define MIN_MEDIUM_REQUEST 16384
#endif

#if !defined(MIN_LARGE_REQUEST)
#  define MIN_LARGE_REQUEST 65536
#endif

#if !defined(MMAP_SIZE)
#  if !AKMALLOC_WINDOWS
#    define MMAP_SIZE (AK_SZ_ONE << 20) /* 1 MB */
#  else/* Windows */
    /**
     * Memory mapping on Windows is slow. Put the entries in the large free list
     * to avoid mmap() calls.
     */
#    define MMAP_SIZE AK_SZ_MAX
#  endif
#endif

typedef struct ak_malloc_state_tag ak_malloc_state;

struct ak_malloc_state_tag
{
    ak_sz init;
    ak_slab_root slabs[NSLABS];
    ak_ca_root   casmall;
    ak_ca_root   camedium;
    ak_ca_root   calarge;
    ak_ca_segment map_root;

    AKMALLOC_LOCK_DEFINE(MAP_LOCK);
};

#if !defined(AKMALLOC_SMALL_ALLOC_RELEASE_RATE)
#  define AKMALLOC_SMALL_ALLOC_RELEASE_RATE 2047
#endif
#if !defined(AKMALLOC_MEDIUM_ALLOC_RELEASE_RATE)
#  define AKMALLOC_MEDIUM_ALLOC_RELEASE_RATE 511
#endif
#if !defined(AKMALLOC_LARGE_ALLOC_RELEASE_RATE)
#  define AKMALLOC_LARGE_ALLOC_RELEASE_RATE 63
#endif

static void ak_malloc_init_state(ak_malloc_state* s)
{
    AKMALLOC_ASSERT_ALWAYS(sizeof(ak_slab) % AK_COALESCE_ALIGN == 0);
    for (ak_sz i = 0; i != NSLABS; ++i) {
        ak_slab_init_root_default(ak_as_ptr(s->slabs[i]), SLAB_SIZES[i]);
    }
    ak_ca_init_root(ak_as_ptr(s->casmall), AKMALLOC_SMALL_ALLOC_RELEASE_RATE, AKMALLOC_SMALL_ALLOC_RELEASE_RATE);
    s->casmall.MIN_SIZE_TO_SPLIT = MIN_SMALL_REQUEST;

    ak_ca_init_root(ak_as_ptr(s->camedium), AKMALLOC_MEDIUM_ALLOC_RELEASE_RATE, AKMALLOC_MEDIUM_ALLOC_RELEASE_RATE);
    s->camedium.MIN_SIZE_TO_SPLIT = MIN_MEDIUM_REQUEST;

    ak_ca_init_root(ak_as_ptr(s->calarge), AKMALLOC_LARGE_ALLOC_RELEASE_RATE, AKMALLOC_LARGE_ALLOC_RELEASE_RATE);
    s->calarge.MIN_SIZE_TO_SPLIT = MIN_LARGE_REQUEST;

    ak_ca_segment_link(ak_as_ptr(s->map_root), ak_as_ptr(s->map_root), ak_as_ptr(s->map_root));

    AKMALLOC_LOCK_INIT(ak_as_ptr(s->MAP_LOCK));
    s->init = 1;
}

static void ak_malloc_destroy_state(ak_malloc_state* m)
{
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_destroy(ak_as_ptr(m->slabs[i]));
    }
    ak_ca_destroy(ak_as_ptr(m->casmall));
    ak_ca_destroy(ak_as_ptr(m->camedium));
    ak_ca_destroy(ak_as_ptr(m->calarge));
    {// mmaped chunks
        ak_ca_segment temp;
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->map_root)) {
            temp = *seg;
            ak_os_free(seg, seg->sz);
            seg = &temp;
        }
    }
}

static void ak_try_reclaim_memory(ak_malloc_state* m)
{
    // for each slab, reclaim empty pages
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_root* s = ak_as_ptr(m->slabs[i]);
        ak_slab_release_pages(s, ak_as_ptr(s->empty_root), AK_U32_MAX);
        s->nempty = 0;
        s->release = 0;
    }
    // return unused segments in ca
    ak_ca_return_os_mem(ak_as_ptr(m->casmall.empty_root), AK_U32_MAX);
    m->casmall.nempty = 0;
    m->casmall.release = 0;
    
    ak_ca_return_os_mem(ak_as_ptr(m->camedium.empty_root), AK_U32_MAX);
    m->camedium.nempty = 0;
    m->camedium.release = 0;
    
    ak_ca_return_os_mem(ak_as_ptr(m->calarge.empty_root), AK_U32_MAX);
    m->camedium.nempty = 0;
    m->camedium.release = 0;

    // all memory in mmap-ed regions is being used. we return pages immediately
    // when they are free'd.
}

ak_inline static void* ak_try_slab_alloc(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_ASSERT(sz % AK_COALESCE_ALIGN == 0);
    ak_sz idx = (sz >> 4) - 1;
    ak_sz* mem = (ak_sz*)ak_slab_alloc(ak_as_ptr(m->slabs[idx]));
    if (ak_likely(mem)) {
        ak_alloc_mark_slab(ak_slab_alloc_2_mem(mem)); // we overallocate
        AKMALLOC_ASSERT(ak_alloc_type_slab(ak_alloc_type_bits(ak_slab_alloc_2_mem(mem))));
        mem = ak_slab_alloc_2_mem(mem);
    }
    return mem;
}

ak_inline static void* ak_try_coalesce_alloc(ak_malloc_state* m, ak_ca_root* proot, size_t sz)
{
    ak_sz* mem = (ak_sz*)ak_ca_alloc(proot, sz);
    if (ak_likely(mem)) {
        ak_alloc_mark_coalesce(mem);
        AKMALLOC_ASSERT(ak_alloc_type_coalesce(ak_alloc_type_bits(mem)));
    }
    return mem;
}

ak_inline static void* ak_try_alloc_mmap(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(m->MAP_LOCK));
    ak_ca_segment* mem = (ak_ca_segment*)ak_os_alloc(sz);
    if (ak_likely(mem)) {
        ak_alloc_mark_mmap(mem + 1);
        AKMALLOC_ASSERT(ak_alloc_type_mmap(ak_alloc_type_bits(mem + 1)));
        mem->sz = sz;
        ak_ca_segment_link(mem, m->map_root.fd, ak_as_ptr(m->map_root));
        mem += 1;
    }
    AKMALLOC_LOCK_RELEASE(ak_as_ptr(m->MAP_LOCK));

    return mem;
}

static void* ak_try_alloc(ak_malloc_state* m, size_t sz)
{
    void* retmem = AK_NULLPTR;
    ak_sz modsz = ak_slab_mod_sz(sz);
    if (modsz <= MIN_SMALL_REQUEST) {
        retmem = ak_try_slab_alloc(m, modsz);
    } else if (sz < MMAP_SIZE) {
        const ak_sz alnsz = ak_ca_aligned_size(sz);
        ak_ca_root* proot = AK_NULLPTR;
        if (alnsz < MIN_MEDIUM_REQUEST) {
            proot = ak_as_ptr(m->casmall);
        } else if (alnsz < MIN_LARGE_REQUEST) {
            proot = ak_as_ptr(m->camedium);
        } else {
            proot = ak_as_ptr(m->calarge);
        }
        retmem = ak_try_coalesce_alloc(m, proot, alnsz);
    } else {
        sz += sizeof(ak_ca_segment);
        const ak_sz actsz = ak_ca_aligned_segment_size(sz);
        retmem = ak_try_alloc_mmap(m, actsz);
    }

    return retmem;
}

static void* ak_malloc_from_state(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_ASSERT(m->init);
    void* mem = ak_try_alloc(m, sz);
    if (ak_unlikely(!mem)) {
        ak_try_reclaim_memory(m);
        mem = ak_try_alloc(m, sz);
    }
    return mem;
}

ak_inline static void ak_free_to_state(ak_malloc_state* m, void* mem)
{
    if (ak_likely(mem)) {
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            ak_slab_free(ak_slab_mem_2_alloc(mem));
        } else if (ak_alloc_type_mmap(ty)) {
            AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(m->MAP_LOCK));
            ak_ca_segment* seg = ((ak_ca_segment*)mem) - 1;
            ak_ca_segment_unlink(seg);
            ak_os_free(seg, seg->sz);
            AKMALLOC_LOCK_RELEASE(ak_as_ptr(m->MAP_LOCK));
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            const ak_sz alnsz = ak_ca_to_sz(n->currinfo);
            ak_ca_root* proot = AK_NULLPTR;
            if (alnsz < MIN_MEDIUM_REQUEST) {
                proot = ak_as_ptr(m->casmall);
            } else if (alnsz < MIN_LARGE_REQUEST) {
                proot = ak_as_ptr(m->camedium);
            } else {
                proot = ak_as_ptr(m->calarge);
            }
            ak_ca_free(proot, mem);
        }
    }
}

ak_inline static size_t ak_malloc_usable_size_in_state(const void* mem)
{
    if (ak_likely(mem)) {
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            // round to page
            const ak_slab* slab = (const ak_slab*)(ak_page_start_before_const(mem));
            return ak_slab_usable_size(slab->root->sz);
        } else if (ak_alloc_type_mmap(ty)) {
            return (((const ak_ca_segment*)mem) - 1)->sz - sizeof(ak_ca_segment);
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            AKMALLOC_ASSERT(!ak_ca_is_free(n->currinfo));
            return ak_ca_to_sz(n->currinfo);
        }
    } else {
        return 0;
    }
}

ak_inline static void* ak_realloc_in_place_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    const ak_sz usablesize = ak_malloc_usable_size_in_state(mem);
    if (usablesize >= newsz) {
        return mem;
    }
    return AK_NULLPTR;
}

static void* ak_realloc_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    if (ak_realloc_in_place_from_state(m, mem, newsz)) {
        return mem;
    }

    void* newmem = ak_malloc_from_state(m, newsz);
    if (!newmem) {
        return AK_NULLPTR;
    }
    if (ak_likely(mem)) {
        ak_memcpy(newmem, mem, ak_malloc_usable_size_in_state(mem));
        ak_free_to_state(m, mem);
    }
    return newmem;
}

static void* ak_aligned_alloc_from_state_no_checks(ak_malloc_state* m, size_t aln, size_t sz)
{
    ak_sz req = ak_ca_aligned_size(sz);
    req += aln + (2 * sizeof(ak_alloc_node)) + (2 * sizeof(ak_free_list_node));
    char* mem = AK_NULLPTR;
    // must request from coalesce alloc so we can return the extra piece
    ak_ca_root* ca = AK_NULLPTR;
    if (aln < MIN_MEDIUM_REQUEST) {
        ca = ak_as_ptr(m->casmall);
    } else if (aln < MIN_LARGE_REQUEST) {
        ca = ak_as_ptr(m->camedium);
    } else {
        ca = ak_as_ptr(m->calarge);
    }

    mem = (char*)ak_ca_alloc(ca, req);
    ak_alloc_node* node = ((ak_alloc_node*)mem) - 1;
    if (ak_likely(mem)) {
        if ((((ak_sz)mem) & (aln - 1)) != 0) {
            // misaligned
            AK_CA_LOCK_ACQUIRE(ca);
            char* alnpos = (char*)(((ak_sz)(mem + aln - 1)) & ~(aln - 1));
            ak_alloc_node* alnnode = ((ak_alloc_node*)alnpos) - 1;
            AKMALLOC_ASSERT((ak_sz)(alnpos - mem) >= (sizeof(ak_free_list_node) + sizeof(ak_alloc_node)));
            ak_sz actsz = ak_ca_to_sz(node->currinfo);
            int islast = ak_ca_is_last(node);

            ak_ca_set_sz(ak_as_ptr(node->currinfo), alnpos - mem - sizeof(ak_alloc_node));
            ak_ca_set_is_last(ak_as_ptr(node->currinfo), 0);
            ak_ca_set_is_free(ak_as_ptr(node->currinfo), 1);

            ak_ca_set_sz(ak_as_ptr(alnnode->currinfo), actsz - ak_ca_to_sz(node->currinfo));
            ak_ca_set_is_last(ak_as_ptr(alnnode->currinfo), islast);
            ak_ca_set_is_free(ak_as_ptr(alnnode->currinfo), 0);

            ak_ca_update_footer(node);
            ak_ca_update_footer(alnnode);
            mem = alnpos;
            AK_CA_LOCK_RELEASE(ca);
        }
        return mem;
    }
    return AK_NULLPTR;
}

static void* ak_aligned_alloc_from_state(ak_malloc_state* m, size_t aln, size_t sz)
{
    if (aln <= AK_COALESCE_ALIGN) {
        return ak_malloc_from_state(m, sz);
    }
    if ((aln & AK_SZ_ONE) || (aln & (aln - 1))) {
        size_t a = AK_COALESCE_ALIGN << 1;
        while (a < aln)  {
            a  = (a << 1);
        }
        aln = a;
    }
    return ak_aligned_alloc_from_state_no_checks(m, aln, sz);
}

#define AK_EINVAL 22
#define AK_ENOMEM 12

static int ak_posix_memalign_from_state(ak_malloc_state* m, void** pmem, size_t aln, size_t sz)
{
    void* mem = AK_NULLPTR;
    if (aln == AK_COALESCE_ALIGN) {
        mem = ak_malloc_from_state(m, sz);
    } else {
        ak_sz div = (aln / sizeof(ak_sz));
        ak_sz rem = (aln & (sizeof(ak_sz)));
        if (rem != 0 || div == 0 || (div & (div - AK_SZ_ONE)) != 0) {
            return AK_EINVAL;
        }
        aln = (aln <= AK_COALESCE_ALIGN) ? AK_COALESCE_ALIGN : aln;
        mem = ak_aligned_alloc_from_state_no_checks(m, aln, sz);
    }

    if (!mem) {
        return AK_ENOMEM;
    }

    *pmem = mem;
    return 0;
}

static void ak_malloc_for_each_segment_in_state(ak_malloc_state* m, ak_seg_cbk cbk)
{
    // for each slab, reclaim empty pages
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_root* s = ak_as_ptr(m->slabs[i]);
        ak_circ_list_for_each(ak_slab, fslab, &(s->full_root)) {
            if (!cbk(fslab, AKMALLOC_DEFAULT_PAGE_SIZE)) {
                return;
            }
        }
        ak_circ_list_for_each(ak_slab, pslab, &(s->partial_root)) {
            if (!cbk(pslab, AKMALLOC_DEFAULT_PAGE_SIZE)) {
                return;
            }
        }
    }

    {// ca small
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->casmall.main_root)) {
            if (!cbk(seg->head, seg->sz)) {
                return;
            }
        }
    }
    {// ca medium
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->camedium.main_root)) {
            if (!cbk(seg->head, seg->sz)) {
                return;
            }
        }
    }
    {// ca large
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->calarge.main_root)) {
            if (!cbk(seg->head, seg->sz)) {
                return;
            }
        }
    }
    {// mmaped chunks
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->map_root)) {
            if (!cbk(seg, seg->sz)) {
                return;
            }
        }
    }
}

#endif/*AKMALLOC_MALLOCSTATE_H*/
