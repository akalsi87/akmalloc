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
#  define AK_COALESCE_SEGMENT_GRANULARITY (((size_t)1) << 22) // 4MB
#endif

#include "akmalloc/slab.h"
#include "akmalloc/coalescingalloc.h"
#include "akmalloc/setup.h"

#if !defined(AK_SEG_CBK_DEFINED)
/**
 * Gets a pointer to a memory segment and its size.
 * \param p; Pointer to segment memory.
 * \param sz; Number of bytes in the segment.
 * 
 * \return \c 0 to stop iteration, non-zero to continue.
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


#define NSLABS 16

/*!
 * Sizes for the slabs in an \c ak_malloc_state
 */
static const ak_sz SLAB_SIZES[NSLABS] = {
    16,   32,   48,   64,   80,   96,  112,  128,
   144,  160,  176,  192,  208,  224,  240,  256
};

#define NCAROOTS 10

/*!
 * Sizes for the coalescing allocators in an \c ak_malloc_state
 *
 * Size here denotes maximum size request for each allocator.
 */
static const ak_sz CA_SIZES[NCAROOTS] = {
    512, 1024, 2048, 4096,
    8192, 16384, 32768, 65536,
    131072,  MMAP_SIZE
};

typedef struct ak_malloc_state_tag ak_malloc_state;

/*!
 * Private malloc like allocator
 */
struct ak_malloc_state_tag
{
    ak_sz         init;             /**< whether initialized */
    ak_slab_root  slabs[NSLABS];    /**< slabs of different sizes */
    ak_ca_root    ca[NCAROOTS];     /**< coalescing allocators of different size ranges */
    ak_ca_segment map_root;         /**< root of list of mmap-ed segments */

    AKMALLOC_LOCK_DEFINE(MAP_LOCK); /**< lock for mmap-ed regions if locks are enabled */
};

#if !defined(AKMALLOC_COALESCING_ALLOC_RELEASE_RATE)
#  define AKMALLOC_COALESCING_ALLOC_RELEASE_RATE 16
#endif

#if !defined(AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE)
#  define AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE AK_U32_MAX
#endif

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
    for (ak_sz i = 0; i < NCAROOTS; ++i) {
        ak_ca_root* ca = ak_as_ptr(m->ca[i]);
        ak_ca_return_os_mem(ak_as_ptr(ca->empty_root), AK_U32_MAX);
        ca->nempty = 0;
        ca->release = 0;
    }

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

ak_inline static ak_ca_root* ak_find_ca_root(ak_malloc_state* m, ak_sz sz)
{
    ak_sz i = 0;
    for (; i < NCAROOTS; ++i) {
        if (CA_SIZES[i] >= sz) {
            break;
        }
    }
    AKMALLOC_ASSERT(i < NCAROOTS);
    return ak_as_ptr(m->ca[i]);
}

ak_inline static void* ak_try_alloc(ak_malloc_state* m, size_t sz)
{
    void* retmem = AK_NULLPTR;
    ak_sz modsz = ak_slab_mod_sz(sz);
    if (modsz <= MIN_SMALL_REQUEST) {
        retmem = ak_try_slab_alloc(m, modsz);
        DBG_PRINTF("a,slab,%p,%llu\n", retmem, modsz);
    } else if (sz < MMAP_SIZE) {
        const ak_sz alnsz = ak_ca_aligned_size(sz);
        ak_ca_root* proot = ak_find_ca_root(m, sz);
        retmem = ak_try_coalesce_alloc(m, proot, alnsz);
        DBG_PRINTF("a,ca[%d],%p,%llu\n", (int)(proot-ak_as_ptr(m->ca[0])), retmem, alnsz);
    } else {
        sz += sizeof(ak_ca_segment);
        const ak_sz actsz = ak_ca_aligned_segment_size(sz);
        retmem = ak_try_alloc_mmap(m, actsz);
        DBG_PRINTF("a,mmap,%p,%llu\n", retmem, actsz);
    }

    return retmem;
}


static void* ak_aligned_alloc_from_state_no_checks(ak_malloc_state* m, size_t aln, size_t sz)
{
    ak_sz req = ak_ca_aligned_size(sz);
    req += aln + (2 * sizeof(ak_alloc_node)) + (2 * sizeof(ak_free_list_node));
    char* mem = AK_NULLPTR;
    // must request from coalesce alloc so we can return the extra piece
    ak_ca_root* ca = ak_find_ca_root(m, req);

    mem = (char*)ak_ca_alloc(ca, req);
    ak_alloc_node* node = ak_ptr_cast(ak_alloc_node, mem) - 1;
    if (ak_likely(mem)) {
        if ((((ak_sz)mem) & (aln - 1)) != 0) {
            // misaligned
            AK_CA_LOCK_ACQUIRE(ca);
            char* alnpos = (char*)(((ak_sz)(mem + aln - 1)) & ~(aln - 1));
            ak_alloc_node* alnnode = ak_ptr_cast(ak_alloc_node, alnpos) - 1;
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

ak_inline static size_t ak_malloc_usable_size_in_state(const void* mem);

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

/*!
 * Initialize a private malloc like allocator.
 * \param s; Pointer to the allocator to initialize (non-NULL)
 */
static void ak_malloc_init_state(ak_malloc_state* s)
{
    AKMALLOC_ASSERT_ALWAYS(sizeof(ak_slab) % AK_COALESCE_ALIGN == 0);

    for (ak_sz i = 0; i != NSLABS; ++i) {
        ak_slab_init_root_default(ak_as_ptr(s->slabs[i]), SLAB_SIZES[i]);
    }

    for (ak_sz i = 0; i != NCAROOTS; ++i) {
        ak_ca_init_root(ak_as_ptr(s->ca[i]), (NCAROOTS-i) > 2 ? (NCAROOTS-i) : 2, AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE);
        s->ca[i].MIN_SIZE_TO_SPLIT = (i == 0) ? SLAB_SIZES[NSLABS-1] : CA_SIZES[i-1];
    }

    ak_ca_segment_link(ak_as_ptr(s->map_root), ak_as_ptr(s->map_root), ak_as_ptr(s->map_root));

    AKMALLOC_LOCK_INIT(ak_as_ptr(s->MAP_LOCK));
    s->init = 1;
}

/*!
 * Destroy the private malloc like allocator and return all memory to the OS.
 * \param m; Pointer to the allocator
 */
static void ak_malloc_destroy_state(ak_malloc_state* m)
{
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_destroy(ak_as_ptr(m->slabs[i]));
    }
    for (ak_sz i = 0; i < NCAROOTS; ++i) {
        ak_ca_destroy(ak_as_ptr(m->ca[i]));
    }
    {// mmaped chunks
        ak_ca_segment temp;
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->map_root)) {
            temp = *seg;
            ak_os_free(seg, seg->sz);
            seg = &temp;
        }
    }
}

/*!
 * Attempt to allocate memory containing at least \p n bytes.
 * \param m; The allocator
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory.
 */
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

/*!
 * Return memory to the allocator.
 * \param m; The allocator
 * \param mem; Pointer to the memory to return.
 */
ak_inline static void ak_free_to_state(ak_malloc_state* m, void* mem)
{
    if (ak_likely(mem)) {
#if defined(AKMALLOC_DEBUG_PRINT)
        ak_sz ussize = ak_malloc_usable_size_in_state(mem);
#endif/*defined(AKMALLOC_DEBUG_PRINT)*/
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            DBG_PRINTF("d,slab,%p,%llu\n", mem, ussize);
            ak_slab_free(ak_slab_mem_2_alloc(mem));
        } else if (ak_alloc_type_mmap(ty)) {
            DBG_PRINTF("d,mmap,%p,%llu\n", mem, ussize);
            AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(m->MAP_LOCK));
            ak_ca_segment* seg = ((ak_ca_segment*)mem) - 1;
            ak_ca_segment_unlink(seg);
            ak_os_free(seg, seg->sz);
            AKMALLOC_LOCK_RELEASE(ak_as_ptr(m->MAP_LOCK));
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            ak_ca_root* proot = n->root;
            DBG_PRINTF("d,ca[%d],%p,%llu\n", (int)(proot-ak_as_ptr(m->ca[0])), mem, ak_ca_to_sz(n->currinfo));
            ak_ca_free(proot, mem);
        }
    }
}

/*!
 * Attempt to grow memory at the region pointed to by \p p to a size \p newsz without relocation.
 * \param m; The allocator
 * \param mem; Memory to grow
 * \param newsz; New size to grow to
 *
 * \return \c NULL if no memory is available, or \p mem with at least \p newsz bytes.
 */
ak_inline static void* ak_realloc_in_place_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    const ak_sz usablesize = ak_malloc_usable_size_in_state(mem);
    if (usablesize >= newsz) {
        return mem;
    }
    if (ak_alloc_type_coalesce(ak_alloc_type_bits(mem))) {
        ak_alloc_node* n = ak_ptr_cast(ak_alloc_node, mem) - 1;
        AKMALLOC_ASSERT(ak_ca_is_free(n->currinfo));
        // check if there is a free next, if so, maybe merge
        ak_sz sz = ak_ca_to_sz(n->currinfo);
        ak_ca_root* proot = ak_find_ca_root(m, sz);
        if (ak_ca_realloc_in_place(proot, mem, newsz)) {
            return mem;
        }
    }
    return AK_NULLPTR;
}

/*!
 * Attempt to grow memory at the region pointed to by \p p to a size \p newsz.
 * \param m; The allocator
 * \param mem; Memory to grow
 * \param newsz; New size to grow to
 *
 * This function will copy the old bytes to a new memory location if the old memory cannot be
 * grown in place, and will free the old memory. If no more memory is available it will not
 * destroy the old memory.
 *
 * \return \c NULL if no memory is available, or a pointer to memory with at least \p newsz bytes.
 */
static void* ak_realloc_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    if (ak_unlikely(!mem)) {
        return ak_malloc_from_state(m, newsz);
    }

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


/*!
 * Return the usable size of the memory region pointed to by \p p.
 * \param mem; Pointer to the memory to determize size of.
 *
 * \return The number of bytes that can be written to in the region.
 */
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

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln. \p aln must be a power of two. \p sz must be a multiple of \p aln.
 * \param m; The allocator
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory at an aligned address.
 */
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

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln and assign the address to \p *pmem. \p aln must be a power of two and
 * a multiple of \c sizeof(void*).
 * \param m; The allocator
 * \param pmem; The address where the memory address should be writted.
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on success, 12 if no more memory is available, and 22 if \p aln was not a power
 * of two and a multiple of \c sizeof(void*)
 */
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

/*!
 * Iterate over all memory segments allocated.
 * \param m; The allocator
 * \param cbk; Callback that is given the address of a segment and its size. \see ak_seg_cbk.
 */
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

    {// ca roots
        for (ak_sz i = 0; i < NCAROOTS; ++i) {
            ak_circ_list_for_each(ak_ca_segment, seg, &(m->ca[i].main_root)) {
                if (!cbk(seg->head, seg->sz)) {
                    return;
                }
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
