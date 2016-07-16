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
 * \file setup.h
 * \date Jul 04, 2016
 */

#ifndef AKMALLOC_SETUP_H
#define AKMALLOC_SETUP_H

#include "akmalloc/assert.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"
#include "akmalloc/memmap.h"

#if defined(AKMALLOC_GETPAGESIZE)
#  error "Page size can only be set to an internal default."
#else
#  define AKMALLOC_GETPAGESIZE AKMALLOC_DEFAULT_GETPAGESIZE
#endif

#if !defined(AKMALLOC_MMAP) && !defined(AKMALLOC_MUNMAP)
#  define AKMALLOC_MMAP AKMALLOC_DEFAULT_MMAP
#  define AKMALLOC_MUNMAP AKMALLOC_DEFAULT_MUNMAP
#elif defined(AKMALLOC_MMAP) && defined(AKMALLOC_MUNMAP)
/* do nothing */
#else
#  error "AKMALLOC_MMAP and AKMALLOC_MUNMAP not defined simultaneously."
#endif

/***********************************************
 * OS Allocation
 ***********************************************/

static void* ak_os_alloc(size_t sz)
{
    static const ak_sz pgsz = AKMALLOC_DEFAULT_PAGE_SIZE;
    (void)(pgsz);
    AKMALLOC_ASSERT_ALWAYS(pgsz == ak_page_size());
    void* mem = AKMALLOC_MMAP(sz);
    DBG_PRINTF("osmap,%p,%zu,%zu pages,iswhole %d\n", mem, sz, sz/AKMALLOC_DEFAULT_PAGE_SIZE, sz == AKMALLOC_DEFAULT_PAGE_SIZE*(sz/AKMALLOC_DEFAULT_PAGE_SIZE));
    return mem;
}

static void ak_os_free(void* p, size_t sz)
{
    DBG_PRINTF("osunmap,%p,%zu\n", p, sz);
    AKMALLOC_MUNMAP(p, sz);
}

ak_inline static void* ak_page_start_before(void* p)
{
    return (void*)((ak_sz)p & (~(ak_sz)(AKMALLOC_DEFAULT_PAGE_SIZE - 1)));
}

ak_inline static const void* ak_page_start_before_const(const void* p)
{
    return (void*)((ak_sz)p & (~(ak_sz)(AKMALLOC_DEFAULT_PAGE_SIZE - 1)));
}

#endif/*AKMALLOC_SETUP_H*/
