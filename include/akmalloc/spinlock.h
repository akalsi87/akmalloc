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
 * \file spinlock.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_SPINLOCK_H
#define AKMALLOC_SPINLOCK_H

#include "akmalloc/assert.h"
#include "akmalloc/config.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"

typedef struct ak_spinlock_tag ak_spinlock;

struct ak_spinlock_tag
{
    ak_u32 lockA;  
};

static void ak_os_sleep(ak_u32 micros);

static void ak_spinlock_yield();

#define ak_atomic_load(x) (*(volatile ak_u32*)(&(x)))

#if !AKMALLOC_MSVC && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
#  define ak_cas(px, nx, ox) __sync_bool_compare_and_swap((px), (ox), (nx))
#else/* Windows */
#  ifndef _M_AMD64
     /* These are already defined on AMD64 builds */
     AK_EXTERN_C_BEGIN
     LONG __cdecl _InterlockedCompareExchange(LONG volatile* Target, LONG NewValue, LONG OldValue);
     AK_EXTERN_C_END
#    pragma intrinsic (_InterlockedCompareExchange)
#  endif /* _M_AMD64 */
#  define ak_cas(px, nx, ox) (_InterlockedCompareExchange((px), (nx), (ox)) == (ox))
#endif/* Windows */

ak_inline static int ak_spinlock_is_locked(ak_spinlock* p)
{
    return ak_atomic_load(p->lockA);
}

ak_inline static void ak_spinlock_init(ak_spinlock* p)
{
    p->lockA = 0;
    AKMALLOC_ASSERT_ALWAYS(!ak_spinlock_is_locked(p));
}

ak_inline static void ak_spinlock_acquire(ak_spinlock* p)
{
    static const ak_u32 SPINS_PER_YIELD = 31;
    ak_u32 spins = 0;

    if (!ak_cas(&(p->lockA), 1, 0)) {
        while (!ak_cas(&(p->lockA), 1, 0)) {
            if ((++spins & SPINS_PER_YIELD) == 0) {
                ak_spinlock_yield();
            }
        }
    }

    AKMALLOC_ASSERT_ALWAYS(ak_spinlock_is_locked(p));
}

ak_inline static void ak_spinlock_release(ak_spinlock* p)
{
    AKMALLOC_ASSERT_ALWAYS(ak_spinlock_is_locked(p));
    int rvA = ak_cas(&(p->lockA), 0, 1);
    AKMALLOC_ASSERT_ALWAYS(rvA == 1);
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

static void ak_spinlock_yield()
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

static void ak_spinlock_yield()
{
    sched_yield();
}

#endif

#endif/*AKMALLOC_SPINLOCK_H*/
