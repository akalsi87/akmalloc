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
 * \file memmap.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_MEMMAP_H
#define AKMALLOC_MEMMAP_H

#include "akmalloc/config.h"
#include "akmalloc/assert.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"
#include "akmalloc/types.h"

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <Windows.h>

ak_inline static ak_sz ak_page_size()
{
    static ak_sz PGSZ = 0;
    if (ak_unlikely(!PGSZ)) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        PGSZ = si.dwPageSize;
    }
    return PGSZ;
}

ak_inline static void* ak_mmap(ak_sz s)
{
    void* mem = VirtualAlloc(0, s, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    //fprintf(stderr, "a: %p\n", mem);
    return mem;
}

ak_inline static void ak_munmap(void* p, ak_sz s)
{
    //fprintf(stderr, "d: %p\n", p);
    BOOL ret = VirtualFree(p, 0, MEM_RELEASE);
    AKMALLOC_ASSERT(ret);
    (void)ret;
}

#else

#include <sys/mman.h>

ak_inline static void* ak_mmap(ak_sz s)
{
    void* addr = mmap(0, s, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    return (addr == (void*)AK_SZ_MAX) ? 0 : addr;
}

ak_inline static void ak_munmap(void* p, ak_sz s)
{
    int rv = munmap(p, s);
    AKMALLOC_ASSERT(rv == 0);
    (void)rv;
}

#include <unistd.h>

ak_inline static ak_sz ak_page_size()
{
    static ak_sz pgsz = 0;
    if (ak_unlikely(!pgsz)) {
        pgsz = sysconf(_SC_PAGESIZE);
    }
    return pgsz;
}

#endif

#define AKMALLOC_DEFAULT_GETPAGESIZE ak_page_size
#define AKMALLOC_DEFAULT_MMAP(s) ak_mmap((s))
#define AKMALLOC_DEFAULT_MUNMAP(a, s) ak_munmap((a), (s))

#endif/*AKMALLOC_MEMMAP_H*/
