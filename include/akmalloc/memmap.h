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
#include "akmalloc/constants.h"
#include "akmalloc/types.h"

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

static ak_sz ak_page_size()
{
    static ak_sz PGSZ = 0;
    if (PGSZ == 0) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        PGSZ = si.dwPageSize;
    }
    return PGSZ;
}

static void* ak_mmap(ak_sz s)
{
    return VirtualAlloc(0, s, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

static void ak_munmap(void* p, ak_sz s)
{
    (void)VirtualFree(p, s, MEM_RELEASE);
}

#else

#include <sys/mman.h>
#include <unistd.h>

inline static ak_sz ak_page_size()
{
    return sysconf(_SC_PAGESIZE);
}

static void* ak_mmap(ak_sz s)
{
    void* addr = mmap(0, s, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    return (addr == (void*)AK_SZ_MAX) ? 0 : addr;
}

static void ak_munmap(void* p, ak_sz s)
{
    (void)munmap(p, s);
}

#endif

#define AKMALLOC_DEFAULT_GETPAGESIZE ak_page_size
#define AKMALLOC_DEFAULT_MMAP(s) ak_mmap((s))
#define AKMALLOC_DEFAULT_MUNMAP(a, s) ak_munmap((a), (s))

#endif/*AKMALLOC_MEMMAP_H*/
