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
 * \file malloc.c
 * \date Apr 08, 2016
 */

#ifndef AKMALLOC_MALLOC_C
#define AKMALLOC_MALLOC_C

#include "akmalloc/slab.h"
#include "akmalloc/coalescingalloc.h"
#include "akmalloc/setup.h"

#if defined(AKMALLOC_BUILD)
#include "akmalloc/malloc.h"
#endif

/***********************************************
 * Exported APIs
 ***********************************************/

// Coalescing allocs give 16-byte aligned memory where the preamble
// uses three bits. The fourth bit is always free. We use that bit
// to distinguish slabs from coalesced outputs, and mmap-outputs.
//
// xxx0 - coalesce
// 0101 - slab
// 1001 - mmap
#define ak_alloc_type_bits(p) \
  ((*(((ak_sz*)(p)) - 1)) & (AK_COALESCE_ALIGN - 1))

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

static const ak_sz MAX_SMALL_REQUEST = 256;

// 1MB
static const ak_sz MMAP_SIZE = (AK_SZ_ONE << 20);

typedef struct ak_malloc_state_tag ak_malloc_state;

struct ak_malloc_state_tag
{
    ak_slab_root slabs[NSLABS];
    ak_ca_root   ca;
    ak_ca_segment map_root;
};

static void ak_malloc_init_state(ak_malloc_state* s)
{
    for (ak_sz i = 0; i != NSLABS; ++i) {
        ak_slab_init_root_default(ak_as_ptr(s->slabs[i]), SLAB_SIZES[i]);
    }
    ak_ca_init_root_default(ak_as_ptr(s->ca));
    ak_ca_segment_link(ak_as_ptr(s->map_root), ak_as_ptr(s->map_root), ak_as_ptr(s->map_root));
}

static int MALLOC_INIT = 0;
static ak_malloc_state MALLOC_ROOT;

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

static void ak_try_reclaim_memory()
{
    // for each slab, reclaim empty pages
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_root* s = ak_as_ptr(MALLOC_ROOT.slabs[i]);
        ak_slab_release_pages(s, ak_as_ptr(s->empty_root), AK_U32_MAX);
        s->nempty = 0;
        s->release = 0;
    }
    // return unused segments in ca
    ak_ca_return_os_mem(ak_as_ptr(MALLOC_ROOT.ca.empty_root), AK_U32_MAX);
    MALLOC_ROOT.ca.nempty = 0;
    MALLOC_ROOT.ca.release = 0;
}

void* ak_malloc(ak_sz sz)
{
    if (ak_unlikely(!MALLOC_INIT)) {
        ak_malloc_init_state(&MALLOC_ROOT);
        MALLOC_INIT = 1;
    }
    ak_sz modsz = ak_ca_aligned_size(sz + sizeof(ak_sz));
    if (modsz <= MAX_SMALL_REQUEST) {
        AKMALLOC_ASSERT(modsz % AK_COALESCE_ALIGN == 0);
        ak_sz idx = (modsz >> 4) - 1;
        ak_sz* mem = (ak_sz*)ak_slab_alloc(ak_as_ptr(MALLOC_ROOT.slabs[idx]));
        if (ak_likely(mem)) {
            ak_alloc_mark_slab(mem + 1); // we overallocate
            AKMALLOC_ASSERT(ak_alloc_type_slab(ak_alloc_type_bits(mem + 1)));
            return mem + 1;
        }
    } else if (sz < MMAP_SIZE) {
        ak_sz* mem = (ak_sz*)ak_ca_alloc(ak_as_ptr(MALLOC_ROOT.ca), sz);
        if (ak_likely(mem)) {
            ak_alloc_mark_coalesce(mem);
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ak_alloc_type_bits(mem)));
            return mem;
        }
    } else {
        sz += sizeof(ak_ca_segment);
        const ak_sz actsz = ak_ca_aligned_segment_size(sz);
        ak_ca_segment* mem = (ak_ca_segment*)ak_os_alloc(actsz);
        if (ak_likely(mem)) {
            ak_alloc_mark_mmap(mem + 1);
            AKMALLOC_ASSERT(ak_alloc_type_mmap(ak_alloc_type_bits(mem + 1)));
            mem->sz = actsz;
            ak_ca_segment_link(mem, MALLOC_ROOT.map_root.fd, ak_as_ptr(MALLOC_ROOT.map_root));
            return (mem + 1);
        }
    }

    // try reclaiming memory and repeat
    ak_try_reclaim_memory();

    if (modsz <= MAX_SMALL_REQUEST) {
        AKMALLOC_ASSERT(modsz % AK_COALESCE_ALIGN == 0);
        ak_sz idx = (modsz >> 4) - 1;
        ak_sz* mem = (ak_sz*)ak_slab_alloc(ak_as_ptr(MALLOC_ROOT.slabs[idx]));
        if (ak_likely(mem)) {
            ak_alloc_mark_slab(mem + 1); // we overallocate
            AKMALLOC_ASSERT(ak_alloc_type_slab(ak_alloc_type_bits(mem + 1)));
            return mem + 1;
        }
    } else if (sz < MMAP_SIZE) {
        ak_sz* mem = (ak_sz*)ak_ca_alloc(ak_as_ptr(MALLOC_ROOT.ca), sz);
        if (ak_likely(mem)) {
            ak_alloc_mark_coalesce(mem);
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ak_alloc_type_bits(mem)));
            return mem;
        }
    } else {
        const ak_sz actsz = ak_ca_aligned_segment_size(sz);
        ak_ca_segment* mem = (ak_ca_segment*)ak_os_alloc(actsz);
        if (ak_likely(mem)) {
            ak_alloc_mark_mmap(mem + 1);
            AKMALLOC_ASSERT(ak_alloc_type_mmap(ak_alloc_type_bits(mem + 1)));
            mem->sz = actsz;
            ak_ca_segment_link(mem, MALLOC_ROOT.map_root.fd, ak_as_ptr(MALLOC_ROOT.map_root));
            return (mem + 1);
        }
    }

    return AK_NULLPTR;
}

void* ak_calloc(ak_sz elsz, ak_sz numel)
{
    ak_sz sz = elsz*numel;
    void* mem = ak_malloc(sz);
    return ak_memset(mem, 0, sz);
}

void ak_free(void* mem)
{
    if (mem) {
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            ak_slab_free(((ak_sz*)mem) - 1);
        } else if (ak_alloc_type_mmap(ty)) {
            ak_ca_segment* seg = ((ak_ca_segment*)mem) - 1;
            ak_ca_segment_unlink(seg);
            ak_os_free(seg, seg->sz);
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            ak_ca_free(ak_as_ptr(MALLOC_ROOT.ca), mem);
        }
    }
}

void* ak_aligned_alloc(ak_sz sz, ak_sz aln)
{
    return 0;
}

int ak_posix_memalign(void** pmem, ak_sz sz, ak_sz aln)
{
    return 0;
}

void* ak_memalign(ak_sz sz, ak_sz aln)
{
    return 0;
}

ak_sz ak_malloc_usable_size(const void* mem)
{
    if (mem) {
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            // round to page
            ak_slab* slab = (ak_slab*)(ak_page_start_before((void*)mem));
            return (slab->root->sz - sizeof(ak_sz));
        } else if (ak_alloc_type_mmap(ty)) {
            return (((ak_ca_segment*)mem) - 1)->sz - sizeof(ak_ca_segment);
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            return ak_ca_to_sz(n->currinfo);
        }
    } else {
        return 0;
    }
}

void* ak_realloc(void* mem, ak_sz newsz)
{
    void* newmem = ak_malloc(newsz);
    if (!newmem) {
        return 0;
    }
    if (ak_likely(mem)) {
        ak_memcpy(newmem, mem, ak_malloc_usable_size(mem));
        ak_free(mem);
    }
    return newmem;
}

#endif/*AKMALLOC_MALLOC_C*/
