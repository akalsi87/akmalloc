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
    return (addr == (void*)AKSIZE_MAX) ? 0 : addr;
}

static void ak_munmap(void* p, ak_sz s)
{
    (void)munmap(p, s);
}

#endif

#define AKMALLOC_DEFAULT_GETPAGESIZE ak_page_size()
#define AKMALLOC_DEFAULT_MMAP(s) ak_mmap((s))
#define AKMALLOC_DEFAULT_MUNMAP(a, s) ak_munmap((a), (s))

#endif/*AKMALLOC_MEMMAP_H*/
