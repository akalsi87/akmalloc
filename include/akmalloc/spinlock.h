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

#if AKMALLOC_GCC || AKMALLOC_CLANG
// #  define AKMALLOC_USE_CK
#endif

#ifndef AKMALLOC_USE_CK
typedef struct ak_spinlock_tag ak_spinlock;
#endif

struct ak_spinlock_tag
{
    ak_u32 islocked;
};

static void ak_os_sleep(ak_u32 micros);

static void ak_spinlock_yield();

#if !AKMALLOC_MSVC && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
#  define ak_atomic_cas(px, nx, ox) __sync_bool_compare_and_swap((px), (ox), (nx))
#  define ak_atomic_xchg(px, nx) __sync_lock_test_and_set((px), (nx))
#  define ak_atomic_incr(px) __sync_add_and_fetch((px),1)
#else/* Windows */
#  ifndef _M_AMD64
   /* These are already defined on AMD64 builds */
   AK_EXTERN_C_BEGIN
     long __cdecl _InterlockedCompareExchange(long volatile* Target, long NewValue, long OldValue);
     long __cdecl _InterlockedExchange(long volatile* Target, long NewValue);
     long __cdecl _InterlockedIncrement(long volatile* Target);
   AK_EXTERN_C_END
#    pragma intrinsic (_InterlockedCompareExchange)
#    pragma intrinsic (_InterlockedExchange)
#    pragma intrinsic (_InterlockedIncrement)
#  endif /* _M_AMD64 */
#  define ak_atomic_cas(px, nx, ox) (_InterlockedCompareExchange((volatile long*)(px), (nx), (ox)) == (ox))
#  define ak_atomic_xchg(px, nx) _InterlockedExchange((volatile long*)(px), (nx))
#  define ak_atomic_incr(px) _InterlockedIncrement((volatile long*)(px))
#endif/* Windows */

#ifdef AKMALLOC_USE_CK
#include "external/ck/ck_spinlock.h"

typedef ck_spinlock_t ak_spinlock;
#endif

ak_inline static int ak_spinlock_is_locked(ak_spinlock* p)
{
#ifndef AKMALLOC_USE_CK
    return *(volatile ak_u32*)(&(p->islocked));
#else
    return ck_spinlock_locked(p);
#endif
}

ak_inline static void ak_spinlock_init(ak_spinlock* p)
{
#ifndef AKMALLOC_USE_CK
    p->islocked = 0;
#else
    ck_spinlock_init(p);
#endif
}

ak_inline static void ak_spinlock_acquire(ak_spinlock* p)
{
#ifdef AKMALLOC_USE_CK
    ck_spinlock_lock(p);
#else    
    ak_u32 spins = 1;

#if AKMALLOC_WINDOWS
#  define SPINS_PER_YIELD 31
#elif AKMALLOC_MACOS || AKMALLOC_IOS
#  define SPINS_PER_YIELD 15
#else
#  define SPINS_PER_YIELD 3
#endif

    ak_u32 max_spins_per_yield = 255;
    ak_u32 spins_per_yield = SPINS_PER_YIELD;

    if (ak_atomic_xchg(&(p->islocked), 1)) {
        while (ak_atomic_xchg(&(p->islocked), 1)) {
            while ((++spins & spins_per_yield) != 0) { }
            spins = 1;
            spins_per_yield = ((spins_per_yield + 1) << 1) - 1;
            if (spins_per_yield > max_spins_per_yield) {
                spins_per_yield = SPINS_PER_YIELD;
#if AKMALLOC_MACOS || AKMALLOC_IOS
                ak_os_sleep(40);
                // ak_spinlock_yield();
#elif AKMALLOC_LINUX
                // ak_os_sleep(1);
                ak_spinlock_yield();
#else
                ak_spinlock_yield();
#endif
            }
        }
    }

#undef SPINS_PER_YIELD
#endif
    AKMALLOC_ASSERT(ak_spinlock_is_locked(p));
}

ak_inline static void ak_spinlock_release(ak_spinlock* p)
{
    AKMALLOC_ASSERT(ak_spinlock_is_locked(p));
#ifndef AKMALLOC_USE_CK
    ak_atomic_xchg(&(p->islocked), 0);
#else
    ck_spinlock_unlock(p);
#endif
}

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <Windows.h>

static void ak_os_sleep(ak_u32 micros)
{
    SleepEx(micros / 1000, FALSE);
}

static void ak_spinlock_yield()
{
    SwitchToThread();
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
