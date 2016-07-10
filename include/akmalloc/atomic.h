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
 * \file atomic.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_ATOMIC_H
#define AKMALLOC_ATOMIC_H

#include "akmalloc/assert.h"
#include "akmalloc/config.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"

typedef void* ak_atomic_void_ptr;

#if !AKMALLOC_MSVC && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100

ak_inline static int ak_atomic_cas_ptr(ak_atomic_void_ptr* pptr, void* nv, void* ov)
{
    return __sync_bool_compare_and_swap(pptr, ov, nv);
}

#else

#ifndef _M_AMD64
/* These are already defined on AMD64 builds */
AK_EXTERN_C_BEGIN

PVOID __cdecl _InterlockedCompareExchangePointer(PVOID volatile* Dest, PVOID Exchange, PVOID Comp);

AK_EXTERN_C_END

#pragma intrinsic (_InterlockedCompareExchangePointer)

#endif /* _M_AMD64 */


ak_inline static int ak_atomic_cas_ptr(void* volatile* ptr, void* nv, void* ov)
{
    return _InterlockedCompareExchangePointer(ptr, nv, ov) == ov;
}

#endif

static void ak_os_sleep(ak_u32 micros);

static void ak_spin_lock_yield();

static void ak_atomic_spin_lock_acquire(ak_atomic_void_ptr* pptr)
{
    static const ak_u32 SPINS_PER_YIELD = 15;
    ak_u32 spins = 0;
    void* prev = AK_NULLPTR;
    void* newv = (void*)AK_SZ_ONE;
    AKMALLOC_ASSERT(((ak_sz)pptr) % sizeof(ak_sz) == 0);
    if (!ak_atomic_cas_ptr(pptr, newv, prev)) {
        while (!ak_atomic_cas_ptr(pptr, newv, prev)) {
            if ((++spins & SPINS_PER_YIELD) == 0) {
                ak_spin_lock_yield();
            }
        }
    }
    AKMALLOC_ASSERT(*pptr == newv);
}

static void ak_atomic_spin_lock_release(ak_atomic_void_ptr* pptr)
{
    static const ak_u32 SPINS_PER_YIELD = 15;
    ak_u32 spins = 0;
    void* prev = (void*)AK_SZ_ONE;
    void* newv = AK_NULLPTR;
    AKMALLOC_ASSERT(((ak_sz)pptr) % sizeof(ak_sz) == 0);
    AKMALLOC_ASSERT(*pptr == prev);
    if (!ak_atomic_cas_ptr(pptr, newv, prev)) {
        while (!ak_atomic_cas_ptr(pptr, newv, prev)) {
            if ((++spins & SPINS_PER_YIELD) == 0) {
                ak_spin_lock_yield();
            }
        }
    }
}

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

static void ak_os_sleep(ak_u32 micros)
{
    const ak_u32 millis = micros/1000;
    SleepEx(millis ? millis : 1, FALSE);
}

static void ak_spin_lock_yield()
{
    // obtained from dlmalloc
    SleepEx(50, FALSE);
}

#else

#include <sched.h>
#include <unistd.h>

static void ak_os_sleep(ak_u32 micros)
{
    usleep(micros);
}

static void ak_spin_lock_yield()
{
    sched_yield();
}

#endif

#endif/*AKMALLOC_ATOMIC_H*/
