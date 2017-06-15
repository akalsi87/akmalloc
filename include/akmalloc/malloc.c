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

#if !defined(AKMALLOC_USE_LOCKS) || AKMALLOC_USE_LOCKS
#  define AK_MALLOCSTATE_USE_LOCKS
#endif

#if !defined(AKMALLOC_INCLUDE_ONLY)
#  include "akmalloc/malloc.h"
#endif

#include "akmalloc/mallocstate.h"

/***********************************************
 * Exported APIs
 ***********************************************/

static int MALLOC_INIT = 0;

static ak_malloc_state MALLOC_ROOT;
static ak_malloc_state* GMSTATE = AK_NULLPTR;
static ak_spinlock MALLOC_INIT_LOCK;

#define ak_ensure_malloc_state_init()                        \
{                                                            \
    if (ak_unlikely(!MALLOC_INIT)) {                         \
        ak_spinlock_init(&MALLOC_INIT_LOCK);                 \
        AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(MALLOC_INIT_LOCK));  \
        if (MALLOC_INIT != 1) {                              \
            GMSTATE = &MALLOC_ROOT;                          \
            ak_malloc_init_state(GMSTATE);                   \
            MALLOC_INIT = 1;                                 \
        }                                                    \
        AKMALLOC_LOCK_RELEASE(ak_as_ptr(MALLOC_INIT_LOCK));  \
    }                                                        \
    AKMALLOC_ASSERT(MALLOC_ROOT.init);                       \
}

AK_EXTERN_C_BEGIN

void* ak_malloc(size_t sz)
{
    ak_ensure_malloc_state_init();
    return ak_malloc_from_state(GMSTATE, sz);
}

void* ak_calloc(size_t elsz, size_t numel)
{
    const ak_sz sz = elsz*numel;
    void* mem = ak_malloc_from_state(GMSTATE, sz);
    return ak_memset(mem, 0, sz);
}

void ak_free(void* mem)
{
    ak_ensure_malloc_state_init();
    ak_free_to_state(GMSTATE, mem);
}

void* ak_aligned_alloc(size_t sz, size_t aln)
{
    ak_ensure_malloc_state_init();
    return ak_aligned_alloc_from_state(GMSTATE, sz, aln);
}

int ak_posix_memalign(void** pmem, size_t aln, size_t sz)
{
    ak_ensure_malloc_state_init();
    return ak_posix_memalign_from_state(GMSTATE, pmem, aln, sz);
}

void* ak_memalign(size_t sz, size_t aln)
{
    ak_ensure_malloc_state_init();
    return ak_aligned_alloc_from_state(GMSTATE, sz, aln);
}

size_t ak_malloc_usable_size(const void* mem)
{
    return ak_malloc_usable_size_in_state(mem);
}

void* ak_realloc(void* mem, size_t newsz)
{
    ak_ensure_malloc_state_init();
    return ak_realloc_from_state(GMSTATE, mem, newsz);
}

void* ak_realloc_in_place(void* mem, size_t newsz)
{
    ak_ensure_malloc_state_init();
    return ak_realloc_in_place_from_state(GMSTATE, mem, newsz);
}

void ak_malloc_for_each_segment(ak_seg_cbk cbk)
{
    ak_ensure_malloc_state_init();
    ak_malloc_for_each_segment_in_state(GMSTATE, cbk);
}

AK_EXTERN_C_END

#endif/*AKMALLOC_MALLOC_C*/
