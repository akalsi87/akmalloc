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

#include "akmalloc/setup.h"
#include "akmalloc/slab.h"
#include "akmalloc/coalescingalloc.h"

#if defined(AKMALLOC_BUILD)
#include "akmalloc/malloc.h"
#endif

/***********************************************
 * Exported APIs
 ***********************************************/

static int MALLOC_INIT = 0;
static ak_ca_root MALLOC_ROOT;

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

void* ak_malloc(size_t sz)
{
    if (ak_unlikely(!MALLOC_INIT)) {
        ak_ca_init_root_default(&MALLOC_ROOT);
        MALLOC_INIT = 1;
    }
    return ak_ca_alloc(&MALLOC_ROOT, sz);
}

void* ak_calloc(size_t elsz, size_t numel)
{
    ak_sz sz = elsz*numel;
    void* mem = ak_malloc(sz);
    return ak_memset(mem, 0, sz);
}

void ak_free(void* mem)
{
    if (mem) {
        ak_ca_free(&MALLOC_ROOT, mem);
    }
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

size_t ak_malloc_usable_size(const void* mem)
{
    if (mem) {
        const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
        return ak_ca_to_sz(n->currinfo);
    } else {
        return 0;
    }
}

void* ak_realloc(void* mem, size_t newsz)
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
