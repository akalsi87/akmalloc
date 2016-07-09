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

#include "akmalloc/config.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100

ak_inline static void* ak_atomic_cas_ptr(void* volatile* pptr, void* nv, void* ov)
{
    return __sync_val_compare_and_swap(pptr, ov, nv);
}

ak_inline static void ak_atomic_clear_ptr(void* volatile* pptr)
{
    __sync_lock_release(pptr);
}

#else

#ifndef _M_AMD64
/* These are already defined on AMD64 builds */
AK_EXTERN_C_BEGIN

PVOID __cdecl _InterlockedCompareExchangePointer(PVOID volatile* Dest, PVOID Exchange, PVOID Comp);
PVOID __cdecl _InterlockedExchangePointer(PVOID volatile* Target, PVOID Value);

AK_EXTERN_C_END

#pragma intrinsic (_InterlockedCompareExchangePointer)
#pragma intrinsic (_InterlockedExchangePointer)

#endif /* _M_AMD64 */


ak_inline static void* ak_atomic_cas_ptr(void* volatile* ptr, void* nv, void* ov)
{
    return _InterlockedCompareExchangePointer(ptr, nv, ov);
}

ak_inline static void ak_atomic_clear_ptr(void* volatile* pptr)
{
    _InterlockedExchangePointer(pptr, 0);
}

#endif

static void ak_atomic_spin_lock_acquire(void* volatile* pptr)
{
    // static const SPINS_PER_YIELD = 127;
    // int spins = 0;
    void* prev = AK_NULLPTR;
    void* newv = (void*)AK_SZ_ONE;
    if (*pptr != newv) {
        void* tmp = AK_NULLPTR;
        while ((tmp = ak_atomic_cas_ptr(pptr, newv, prev))) {
            if (((*pptr) == newv) && (tmp == prev)) {
                break;
            }
        }
    }
}

static void ak_atomic_spin_lock_release(void* volatile* pptr)
{
    ak_atomic_clear_ptr(pptr);
}

#endif/*AKMALLOC_ATOMIC_H*/
