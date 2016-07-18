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

/*!
 * \mainpage akmalloc - A customizable memory allocator
 *
 * \section intro Introduction
 *
 * <tt><a href="https://github.com/akalsi87/akmalloc">akmalloc</a></tt> is a customizable memory allocator and its constituent parts. It can be a drop in
 * replacement for \p malloc() and \p free() in many cases, and is composed of slabs
 * and coalescing allocators.
 *
 * The inspiration and motivation for this library comes from <tt><a href="ftp://gee.oswego.edu/pub/misc/malloc.c">dlmalloc</a></tt>.
 *
 * The goals for this library are:
 *
 * -# Easy to read and maintain.
 *
 * -# Be more memory conserving.
 *
 * -# High efficiency and good performance.
 *
 * -# Portability.
 *
 *
 * The constituent parts of malloc are:
 *
 * -# \ref slaballoc
 *
 * -# \ref caalloc
 *
 * For the \p malloc() implementation see \ref akmallocDox
 *
 * \section releases Releases page
 *
 * <a href="https://github.com/akalsi87/akmalloc/releases">Link</a>
 *
 * \section build Build instructions
 *
 * The recommended way to build <tt>akmalloc</tt> and use it is to download the source directly.
 *
 * You may then decide to either build a library or include it directly into your source code.
 *
 * <h2> Requirements </h2>
 *
 * A recent version of GCC, Clang or MSVC. Although akmalloc has been qualified on these
 * compilers, if you face issues raise a ticket <a href="https://github.com/akalsi87/akmalloc/issues">here</a>.
 *
 * <h2> Usage </h2>
 * See \ref customMalloc for details on how to include and work with <tt>akmalloc</tt>.
 */

#define AKMALLOC_MAJOR_VER    1
#define AKMALLOC_MINOR_VER    0
#define AKMALLOC_PATCH_VER    1

#if !defined(AKMALLOC_INCLUDE_ONLY) && !defined(AKMALLOC_LINK_STATIC) && !defined(AKMALLOC_BUILD)
#  error "You must set one of the defined AKMALLOC_INCLUDE_ONLY (header only), AKMALLOC_LINK_STATIC (static libs), or AKMALLOC_BUILD (shared libs)"
#endif

/*
 * Revision history:
 *
 *
 * (July 16, 2016)  1.0.1: Added customized coalescing allocators to malloc state.
 *                           - Performance and memory preservation are good
 *                           - More debuggable, with diagnostic tooling
 *
 * (July 11, 2016)  0.1.2: Functioning multi threaded malloc.
 *
 * (July 10, 2016)  0.1.1: Functioning single threaded malloc.
 *
 * (March 11, 2016) 0.0.1: Initial commit
 */

/*
 * Compiler definition macros
 */
#if defined(_MSC_VER)
#  define AKMALLOC_MSVC       1
#endif

#if defined(__GNUC__) && !defined(__clang__)
#  define AKMALLOC_GCC        1
#endif

#if defined(__clang__)
#  define AKMALLOC_CLANG      1
#endif

#if !AKMALLOC_MSVC && !AKMALLOC_GCC && !AKMALLOC_CLANG
#  error "Unsupported compiler!"
#endif

/*
 * Platform definition macros
 */
#if defined(_WIN32)
#  define AKMALLOC_LINUX      0
#  define AKMALLOC_WINDOWS    1
#  define AKMALLOC_IOS        0
#  define AKMALLOC_MACOS      0
#endif

#if defined(__linux__)
#  define AKMALLOC_LINUX      1
#  define AKMALLOC_WINDOWS    0
#  define AKMALLOC_IOS        0
#  define AKMALLOC_MACOS      0
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#  if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#    define AKMALLOC_LINUX    0
#    define AKMALLOC_WINDOWS  0
#    define AKMALLOC_IOS      1
#    define AKMALLOC_MACOS    0
#  elif TARGET_OS_MAC
#    define AKMALLOC_LINUX    0
#    define AKMALLOC_WINDOWS  0
#    define AKMALLOC_IOS      0
#    define AKMALLOC_MACOS    1
#  else
#    error "Unknown Apple platform"
#  endif
#endif

#if !AKMALLOC_LINUX && !AKMALLOC_WINDOWS && !AKMALLOC_IOS && !AKMALLOC_MACOS
#  error "Invalid or unsupported platform!"
#endif

/*
 * Bit-ness definition macros
 * see: https://sourceforge.net/p/predef/wiki/Architectures/
 */
#if defined(_WIN32)
#  if defined(_WIN64)
#    define AKMALLOC_BITNESS 64
#  else
#    define AKMALLOC_BITNESS 32
#  endif
#endif

#if AKMALLOC_GCC || AKMALLOC_CLANG
#  if defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64)
#    define AKMALLOC_BITNESS 64
#  else
#    define AKMALLOC_BITNESS 32
#  endif
#endif

#if !defined(AKMALLOC_BITNESS) || (AKMALLOC_BITNESS != 32 && AKMALLOC_BITNESS != 64)
#  error "Invalid bitness or bitness undefined."
#endif

/*
 * ISA definition macros
 * see: https://sourceforge.net/p/predef/wiki/Architectures/
 */
#if defined(__arm__) || defined(_M_ARM)
#  define AKMALLOC_ARM        1
#else
#  define AKMALLOC_ARM        0
#endif


/********************** setup begin ************************/
#if !defined(AK_EXTERN_C_BEGIN)
#  if defined(__cplusplus)
#    define AK_EXTERN_C_BEGIN extern "C"  {
#    define AK_EXTERN_C_END   }/*extern C*/
#  else
#    define AK_EXTERN_C_BEGIN
#    define AK_EXTERN_C_END
#  endif
#endif

#if !defined(AKMALLOC_INCLUDE_ONLY)
#  include "akmalloc/malloc.h"
#endif

#if !AKMALLOC_WINDOWS
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#endif

typedef int ak_i32;
typedef unsigned int ak_u32;

#if AKMALLOC_MSVC
typedef __int64 ak_i64;
typedef unsigned __int64 ak_u64;
#else
typedef long long int ak_i64;
typedef unsigned long long int ak_u64;
#endif

#if AKMALLOC_BITNESS == 32
typedef ak_u32 ak_sz;
typedef ak_i32 ak_ssz;
#else
typedef ak_u64 ak_sz;
typedef ak_i64 ak_ssz;
#endif

#if AKMALLOC_MSVC
#  define ak_inline __forceinline
#else
#  define ak_inline __inline__ __attribute__((always_inline))
#endif/*AKMALLOC_MSVC*/

#if !defined(NDEBUG)
#  undef ak_inline
#  define ak_inline
#endif

#if AKMALLOC_MSVC
#  define ak_likely(x) x
#  define ak_unlikely(x) x
#else
#  define ak_likely(x) __builtin_expect(!!(x), 1)
#  define ak_unlikely(x) __builtin_expect(!!(x), 0)
#endif/*AKMALLOC_MSVC*/

#define AK_SZ_ONE ((ak_sz)1)

#define AK_SZ_MAX (~((ak_sz)0))
#define AK_U32_MAX (~((ak_u32)0))

#define AK_NULLPTR 0

#define AKMALLOC_DEFAULT_PAGE_SIZE 4096
#define AKMALLOC_DEFAULT_LARGE_BLOCK_SIZE (AK_SZ_ONE << 21)

/*
 * Cache line macros
 */
#if AKMALLOC_ARM
#  define AKMALLOC_CACHE_LINE_LENGTH 32
#else
#  define AKMALLOC_CACHE_LINE_LENGTH 64
#endif

#define ak_as_ptr(x) (&(x))

#define ak_ptr_cast(ty, expr) ((ty*)((void*)(expr)))
/********************** setup end ************************/

/********************** assert begin ************************/
#if !defined(AKMALLOC_ASSERT_IMPL)
#  include <stdlib.h>
#  include <stdio.h>
#  define AKMALLOC_ASSERT_IMPL(x)                                                                 \
    if (!(x)) {                                                                                   \
      fprintf(stderr, "%s (%d) : %s\n", __FILE__, __LINE__, "ASSERT: failed condition `" #x "'"); \
      ak_call_abort();                                                                            \
    }

    static void ak_call_abort()
    {
        abort();
    }
#endif

#if !defined(AKMALLOC_ASSERT)
#  if !defined(NDEBUG)
#    define AKMALLOC_ASSERT AKMALLOC_ASSERT_IMPL
#  else
#    define AKMALLOC_ASSERT(...) do { } while(0)
#  endif
#endif

#if !defined(AKMALLOC_ASSERT_ALWAYS)
#  define AKMALLOC_ASSERT_ALWAYS AKMALLOC_ASSERT_IMPL
#endif

#if !defined(AKMALLOC_DEBUG_PRINT)
// #  define AKMALLOC_DEBUG_PRINT
#endif

#if defined(AKMALLOC_DEBUG_PRINT)
#  define DBG_PRINTF(...) fprintf(stdout, __VA_ARGS__); fflush(stdout)
#else
#  define DBG_PRINTF(...) (void)0
#endif/*defined(AKMALLOC_DEBUG_PRINT)*/
/********************** assert end ************************/

/********************** spinlock begin ************************/
typedef struct ak_spinlock_tag ak_spinlock;

struct ak_spinlock_tag
{
    ak_u32 islocked;  
};

static void ak_os_sleep(ak_u32 micros);

static void ak_spinlock_yield();

#if !AKMALLOC_MSVC && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
#  define ak_atomic_cas(px, nx, ox) __sync_bool_compare_and_swap((px), (ox), (nx))
#  define ak_atomic_xchg(px, nx) __sync_lock_test_and_set((px), (nx))
#else/* Windows */
#  ifndef _M_AMD64
   /* These are already defined on AMD64 builds */
   AK_EXTERN_C_BEGIN
     long __cdecl _InterlockedCompareExchange(long volatile* Target, long NewValue, long OldValue);
     long __cdecl _InterlockedExchange(long volatile* Target, long NewValue);
   AK_EXTERN_C_END
#    pragma intrinsic (_InterlockedCompareExchange)
#  endif /* _M_AMD64 */
#  define ak_atomic_cas(px, nx, ox) (_InterlockedCompareExchange((volatile long*)(px), (nx), (ox)) == (ox))
#  define ak_atomic_xchg(px, nx) _InterlockedExchange((volatile long*)(px), (nx))
#endif/* Windows */

ak_inline static int ak_spinlock_is_locked(ak_spinlock* p)
{
    return *(volatile ak_u32*)(&(p->islocked));
}

ak_inline static void ak_spinlock_init(ak_spinlock* p)
{
    p->islocked = 0;
}

ak_inline static void ak_spinlock_acquire(ak_spinlock* p)
{
    ak_u32 spins = 0;

#if AKMALLOC_WINDOWS
#  define SPINS_PER_YIELD 63
#elif AKMALLOC_MACOS || AKMALLOC_IOS
#  define SPINS_PER_YIELD 15
#else
#  define SPINS_PER_YIELD 31
#endif

    if (ak_atomic_xchg(&(p->islocked), 1)) {
        while (ak_atomic_xchg(&(p->islocked), 1)) {
            if ((++spins & SPINS_PER_YIELD) == 0) {
#if AKMALLOC_MACOS || AKMALLOC_IOS
                ak_os_sleep(40);
#else
                ak_spinlock_yield();
#endif
            }
        }
    }

#undef SPINS_PER_YIELD

    AKMALLOC_ASSERT(ak_spinlock_is_locked(p));
}

ak_inline static void ak_spinlock_release(ak_spinlock* p)
{
    AKMALLOC_ASSERT(ak_spinlock_is_locked(p));
    ak_atomic_xchg(&(p->islocked), 0);
}

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
/********************** spinlock end ************************/

/********************** bitset begin ************************/
#if AKMALLOC_MSVC
#include <intrin.h>
#endif

typedef ak_u32 ak_bitset32;

ak_inline static int ak_bitset_all(const ak_bitset32* bs)
{
    return (*bs == 0xFFFFFFFF) ? 1 : 0;
}

ak_inline static int ak_bitset_any(const ak_bitset32* bs)
{
    return *(const int*)bs;
}

ak_inline static int ak_bitset_none(const ak_bitset32* bs)
{
    return (*bs == 0x00000000) ? 1 : 0;
}

ak_inline static void ak_bitset_set_all(ak_bitset32* bs)
{
    *bs = 0xFFFFFFFF;
}

ak_inline static void ak_bitset_clear_all(ak_bitset32* bs)
{
    *bs = 0x00000000; 
}

#define ak_bitset_set(bs, i) \
  (*(bs) = (*(bs) | (0x00000001 << (i))))

#define ak_bitset_clear(bs, i) \
  (*(bs) = (*(bs) & (~(0x00000001 << (i)))))

ak_inline static void ak_bitset_set_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs = *bs | (mask << i);
}

ak_inline static void ak_bitset_clear_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs = *bs & (~(mask << i));
}

#define ak_bitset_get(bs, i) \
  ((*(bs) & (0x00000001 << (i))))

ak_inline static ak_bitset32 ak_bitset_get_n(const ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    return (*bs & (mask << i)) >> i;
}

#if AKMALLOC_MSVC
#  define ak_bitset_fill_num_leading_zeros(bs, out)             \
     do {                                                       \
        DWORD ldz = 0;                                          \
        out = (_BitScanReverse(&ldz, *(bs))) ? (31 - ldz) : 32; \
     } while (0)
#else
#  define ak_bitset_fill_num_leading_zeros(bs, out)             \
     out = (*bs) ? __builtin_clz(*bs) : 32
#endif

ak_inline static int ak_bitset_num_leading_zeros(const ak_bitset32* bs)
{
    int nlz;
    ak_bitset_fill_num_leading_zeros(bs, nlz);
    return nlz;
}

#if AKMALLOC_MSVC
#  define ak_bitset_fill_num_trailing_zeros(bs, out)            \
     do {                                                       \
        DWORD trz = 0;                                          \
        out = (_BitScanForward(&trz, *(bs))) ? trz : 32;        \
     } while (0)
#else
#  define ak_bitset_fill_num_trailing_zeros(bs, out)            \
     out = (*(bs)) ? __builtin_ctz(*(bs)) : 32
#endif

ak_inline static int ak_bitset_num_trailing_zeros(const ak_bitset32* bs)
{
    int ntz;
    ak_bitset_fill_num_trailing_zeros(bs, ntz);
    return ntz;
}

ak_inline static int ak_bitset_num_leading_ones(const ak_bitset32* bs)
{
    ak_bitset32 copy = ~(*bs);
    return ak_bitset_num_leading_zeros(&copy);
}

ak_inline static int ak_bitset_num_trailing_ones(const ak_bitset32* bs)
{
    ak_bitset32 copy = ~(*bs);
    return ak_bitset_num_trailing_zeros(&copy);
}

ak_inline static void ak_bitset_flip(ak_bitset32* bs)
{
    *bs = ~(*bs);
}

/* ak_bitset512 */

struct ak_bitset512_tag
{
    ak_bitset32 a0;
    ak_bitset32 a1;
    ak_bitset32 a2;
    ak_bitset32 a3;
    ak_bitset32 a4;
    ak_bitset32 a5;
    ak_bitset32 a6;
    ak_bitset32 a7;
    ak_bitset32 a8;
    ak_bitset32 a9;
    ak_bitset32 a10;
    ak_bitset32 a11;
    ak_bitset32 a12;
    ak_bitset32 a13;
    ak_bitset32 a14;
    ak_bitset32 a15;
};

typedef struct ak_bitset512_tag ak_bitset512;

ak_inline static int ak_bitset512_all(const ak_bitset512* bs)
{
    return ak_bitset_all(&(bs->a0))  &&
           ak_bitset_all(&(bs->a1))  &&
           ak_bitset_all(&(bs->a2))  &&
           ak_bitset_all(&(bs->a3))  &&
           ak_bitset_all(&(bs->a4))  &&
           ak_bitset_all(&(bs->a5))  &&
           ak_bitset_all(&(bs->a6))  &&
           ak_bitset_all(&(bs->a7))  &&
           ak_bitset_all(&(bs->a8))  &&
           ak_bitset_all(&(bs->a9))  &&
           ak_bitset_all(&(bs->a10)) &&
           ak_bitset_all(&(bs->a11)) &&
           ak_bitset_all(&(bs->a12)) &&
           ak_bitset_all(&(bs->a13)) &&
           ak_bitset_all(&(bs->a14)) &&
           ak_bitset_all(&(bs->a15));
}

ak_inline static int ak_bitset512_any(const ak_bitset512* bs)
{
    return ak_bitset_any(&(bs->a0))  ||
           ak_bitset_any(&(bs->a1))  ||
           ak_bitset_any(&(bs->a2))  ||
           ak_bitset_any(&(bs->a3))  ||
           ak_bitset_any(&(bs->a4))  ||
           ak_bitset_any(&(bs->a5))  ||
           ak_bitset_any(&(bs->a6))  ||
           ak_bitset_any(&(bs->a7))  ||
           ak_bitset_any(&(bs->a8))  ||
           ak_bitset_any(&(bs->a9))  ||
           ak_bitset_any(&(bs->a10)) ||
           ak_bitset_any(&(bs->a11)) ||
           ak_bitset_any(&(bs->a12)) ||
           ak_bitset_any(&(bs->a13)) ||
           ak_bitset_any(&(bs->a14)) ||
           ak_bitset_any(&(bs->a15));
}

ak_inline static int ak_bitset512_none(const ak_bitset512* bs)
{
    return ak_bitset_none(&(bs->a0))  &&
           ak_bitset_none(&(bs->a1))  &&
           ak_bitset_none(&(bs->a2))  &&
           ak_bitset_none(&(bs->a3))  &&
           ak_bitset_none(&(bs->a4))  &&
           ak_bitset_none(&(bs->a5))  &&
           ak_bitset_none(&(bs->a6))  &&
           ak_bitset_none(&(bs->a7))  &&
           ak_bitset_none(&(bs->a8))  &&
           ak_bitset_none(&(bs->a9))  &&
           ak_bitset_none(&(bs->a10)) &&
           ak_bitset_none(&(bs->a11)) &&
           ak_bitset_none(&(bs->a12)) &&
           ak_bitset_none(&(bs->a13)) &&
           ak_bitset_none(&(bs->a14)) &&
           ak_bitset_none(&(bs->a15));
}

ak_inline static void ak_bitset512_set_all(ak_bitset512* bs)
{
    ak_bitset_set_all(&(bs->a0));
    ak_bitset_set_all(&(bs->a1));
    ak_bitset_set_all(&(bs->a2));
    ak_bitset_set_all(&(bs->a3));
    ak_bitset_set_all(&(bs->a4));
    ak_bitset_set_all(&(bs->a5));
    ak_bitset_set_all(&(bs->a6));
    ak_bitset_set_all(&(bs->a7));
    ak_bitset_set_all(&(bs->a8));
    ak_bitset_set_all(&(bs->a9));
    ak_bitset_set_all(&(bs->a10));
    ak_bitset_set_all(&(bs->a11));
    ak_bitset_set_all(&(bs->a12));
    ak_bitset_set_all(&(bs->a13));
    ak_bitset_set_all(&(bs->a14));
    ak_bitset_set_all(&(bs->a15));
}

ak_inline static void ak_bitset512_clear_all(ak_bitset512* bs)
{
    ak_bitset_clear_all(&(bs->a0));
    ak_bitset_clear_all(&(bs->a1));
    ak_bitset_clear_all(&(bs->a2));
    ak_bitset_clear_all(&(bs->a3));
    ak_bitset_clear_all(&(bs->a4));
    ak_bitset_clear_all(&(bs->a5));
    ak_bitset_clear_all(&(bs->a6));
    ak_bitset_clear_all(&(bs->a7));
    ak_bitset_clear_all(&(bs->a8));
    ak_bitset_clear_all(&(bs->a9));
    ak_bitset_clear_all(&(bs->a10));
    ak_bitset_clear_all(&(bs->a11));
    ak_bitset_clear_all(&(bs->a12));
    ak_bitset_clear_all(&(bs->a13));
    ak_bitset_clear_all(&(bs->a14));
    ak_bitset_clear_all(&(bs->a15));
}

ak_inline static void ak_bitset512_set(ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            ak_bitset_set(&(bs->a15), idx & 31);
            return;
        case 1:
            ak_bitset_set(&(bs->a14), idx & 31);
            return;
        case 2:
            ak_bitset_set(&(bs->a13), idx & 31);
            return;
        case 3:
            ak_bitset_set(&(bs->a12), idx & 31);
            return;
        case 4:
            ak_bitset_set(&(bs->a11), idx & 31);
            return;
        case 5:
            ak_bitset_set(&(bs->a10), idx & 31);
            return;
        case 6:
            ak_bitset_set(&(bs->a9), idx & 31);
            return;
        case 7:
            ak_bitset_set(&(bs->a8), idx & 31);
            return;
        case 8:
            ak_bitset_set(&(bs->a7), idx & 31);
            return;
        case 9:
            ak_bitset_set(&(bs->a6), idx & 31);
            return;
        case 10:
            ak_bitset_set(&(bs->a5), idx & 31);
            return;
        case 11:
            ak_bitset_set(&(bs->a4), idx & 31);
            return;
        case 12:
            ak_bitset_set(&(bs->a3), idx & 31);
            return;
        case 13:
            ak_bitset_set(&(bs->a2), idx & 31);
            return;
        case 14:
            ak_bitset_set(&(bs->a1), idx & 31);
            return;
        case 15:
            ak_bitset_set(&(bs->a0), idx & 31);
            return;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
    }
    return;
}

ak_inline static void ak_bitset512_clear(ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            ak_bitset_clear(&(bs->a15), idx & 31);
            return;
        case 1:
            ak_bitset_clear(&(bs->a14), idx & 31);
            return;
        case 2:
            ak_bitset_clear(&(bs->a13), idx & 31);
            return;
        case 3:
            ak_bitset_clear(&(bs->a12), idx & 31);
            return;
        case 4:
            ak_bitset_clear(&(bs->a11), idx & 31);
            return;
        case 5:
            ak_bitset_clear(&(bs->a10), idx & 31);
            return;
        case 6:
            ak_bitset_clear(&(bs->a9), idx & 31);
            return;
        case 7:
            ak_bitset_clear(&(bs->a8), idx & 31);
            return;
        case 8:
            ak_bitset_clear(&(bs->a7), idx & 31);
            return;
        case 9:
            ak_bitset_clear(&(bs->a6), idx & 31);
            return;
        case 10:
            ak_bitset_clear(&(bs->a5), idx & 31);
            return;
        case 11:
            ak_bitset_clear(&(bs->a4), idx & 31);
            return;
        case 12:
            ak_bitset_clear(&(bs->a3), idx & 31);
            return;
        case 13:
            ak_bitset_clear(&(bs->a2), idx & 31);
            return;
        case 14:
            ak_bitset_clear(&(bs->a1), idx & 31);
            return;
        case 15:
            ak_bitset_clear(&(bs->a0), idx & 31);
            return;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
    }
    return;
}

ak_inline static ak_bitset32 ak_bitset512_get(const ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            return ak_bitset_get(&(bs->a15), idx & 31);
        case 1:
            return ak_bitset_get(&(bs->a14), idx & 31);
        case 2:
            return ak_bitset_get(&(bs->a13), idx & 31);
        case 3:
            return ak_bitset_get(&(bs->a12), idx & 31);
        case 4:
            return ak_bitset_get(&(bs->a11), idx & 31);
        case 5:
            return ak_bitset_get(&(bs->a10), idx & 31);
        case 6:
            return ak_bitset_get(&(bs->a9), idx & 31);
        case 7:
            return ak_bitset_get(&(bs->a8), idx & 31);
        case 8:
            return ak_bitset_get(&(bs->a7), idx & 31);
        case 9:
            return ak_bitset_get(&(bs->a6), idx & 31);
        case 10:
            return ak_bitset_get(&(bs->a5), idx & 31);
        case 11:
            return ak_bitset_get(&(bs->a4), idx & 31);
        case 12:
            return ak_bitset_get(&(bs->a3), idx & 31);
        case 13:
            return ak_bitset_get(&(bs->a2), idx & 31);
        case 14:
            return ak_bitset_get(&(bs->a1), idx & 31);
        case 15:
            return ak_bitset_get(&(bs->a0), idx & 31);
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
            return 0;
    }
}

#define ak_bitset512_fill_num_leading_zeros(bs, nlz)            \
    nlz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset_fill_num_leading_zeros(&(bs->a0), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a1), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a2), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a3), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a4), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a5), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a6), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a7), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a8), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a9), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a10), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a11), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a12), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a13), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a14), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a15), cur);      \
        nlz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

#define ak_bitset512_fill_num_leading_ones(bs, nlz)             \
    nlz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset32 mybs;                                       \
        mybs = ~(bs->a0);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a1);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a2);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a3);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a4);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a5);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a6);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a7);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a8);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a9);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a10);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a11);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a12);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a13);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a14);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a15);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

ak_inline static int ak_bitset512_num_leading_zeros(const ak_bitset512* bs)
{
    int nlz = 0;
    ak_bitset512_fill_num_leading_zeros(bs, nlz);
    return nlz;
}

#define ak_bitset512_fill_num_trailing_zeros(bs, ntz)           \
    ntz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset_fill_num_trailing_zeros(&(bs->a15), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a14), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a13), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a12), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a11), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a10), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a9), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a8), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a7), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a6), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a5), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a4), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a3), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a2), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a1), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a0), cur);      \
        ntz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

#define ak_bitset512_fill_num_trailing_ones(bs, ntz)            \
    ntz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset32 mybs;                                       \
        mybs = ~(bs->a15);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a14);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a13);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a12);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a11);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a10);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a9);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a8);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a7);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a6);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a5);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a4);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a3);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a2);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a1);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a0);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }


ak_inline static int ak_bitset512_num_trailing_zeros(const ak_bitset512* bs)
{
    int ntz = 0;
    ak_bitset512_fill_num_trailing_zeros(bs, ntz);
    return ntz;
}

ak_inline static void ak_bitset512_flip(ak_bitset512* bs)
{
    ak_bitset_flip(&(bs->a0));
    ak_bitset_flip(&(bs->a1));
    ak_bitset_flip(&(bs->a2));
    ak_bitset_flip(&(bs->a3));
    ak_bitset_flip(&(bs->a4));
    ak_bitset_flip(&(bs->a5));
    ak_bitset_flip(&(bs->a6));
    ak_bitset_flip(&(bs->a7));
    ak_bitset_flip(&(bs->a8));
    ak_bitset_flip(&(bs->a9));
    ak_bitset_flip(&(bs->a10));
    ak_bitset_flip(&(bs->a11));
    ak_bitset_flip(&(bs->a12));
    ak_bitset_flip(&(bs->a13));
    ak_bitset_flip(&(bs->a14));
    ak_bitset_flip(&(bs->a15));
}

ak_inline static int ak_bitset512_num_leading_ones(const ak_bitset512* bs)
{
    int nlo;
    ak_bitset512_fill_num_leading_ones(bs, nlo);
    return nlo;
}

ak_inline static int ak_bitset512_num_trailing_ones(const ak_bitset512* bs)
{
    int nto;
    ak_bitset512_fill_num_trailing_ones(bs, nto);
    return nto;
}
/********************** bitset end ************************/

/********************** os alloc begin ********/
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

#if AKMALLOC_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
    return VirtualAlloc(0, s, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

ak_inline static void ak_munmap(void* p, ak_sz s)
{
    (void)VirtualFree(p, s, MEM_RELEASE);
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
    (void)munmap(p, s);
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
/********************** os alloc end **********/

/********************** mallocstate config begin **********/

/*!
 * Decide to use or not use locks
 */
#if !defined(AKMALLOC_USE_LOCKS) || AKMALLOC_USE_LOCKS
#  define AK_MALLOCSTATE_USE_LOCKS
#endif

#if defined(AK_MALLOCSTATE_USE_LOCKS)
#  define AK_SLAB_USE_LOCKS
#  define AK_CA_USE_LOCKS
#  define AKMALLOC_LOCK_DEFINE(nm)  ak_spinlock nm
#  define AKMALLOC_LOCK_INIT(lk)    ak_spinlock_init((lk))
#  define AKMALLOC_LOCK_ACQUIRE(lk) ak_spinlock_acquire((lk))
#  define AKMALLOC_LOCK_RELEASE(lk) ak_spinlock_release((lk))
#else
#  define AKMALLOC_LOCK_DEFINE(nm)
#  define AKMALLOC_LOCK_INIT(lk)
#  define AKMALLOC_LOCK_ACQUIRE(lk)
#  define AKMALLOC_LOCK_RELEASE(lk)
#endif

#if !defined(AK_COALESCE_SEGMENT_GRANULARITY)
#  define AK_COALESCE_SEGMENT_GRANULARITY (((size_t)1) << 18) /* 256KB */
#endif

#if !defined(AK_SEG_CBK_DEFINED)
/**
 * Gets a pointer to a memory segment and its size.
 * \param p; Pointer to segment memory.
 * \param sz; Number of bytes in the segment.
 * 
 * \return \c 0 to stop iteration, non-zero to continue.
 */
typedef int(*ak_seg_cbk)(const void*, size_t);
#endif

/********************** mallocstate config end ************/

/********************** slab begin **********************/
/*!
 *
 * \page slaballoc Slab allocator
 *
 * Slabs are a concept borrowed from <a href="https://www.usenix.org/legacy/publications/library/proceedings/bos94/full_papers/bonwick.a">Jeff Bonwick's UseNIX paper </a> and adapted
 * as a general implementation scheme.
 *
 * The major departures from the scheme presented in Jeff's paper is that we don't keep the
 * client-specified object APIs because we are using slabs to implement the plain old \p libc
 * memory allocation routines which do not have this flexibility.
 *
 * Nevertheless, slabs prove to be efficient and compact for dealing with memory.
 *
 * Slabs are all page sized, and allocate a fixed size.
 * They have a bit map at the head with some links to forward and back slabs.
 * The slab bit map knows which indexes of fixed size entries are free and which are allocated.
 * Allocating a new object is as simple as finding the first index of the unused bit in the
 * bit map and flipping it. Freeing is also fast, because we mask out the pointer to get the page
 * and in constant time we know which slab the pointer came from and we update the bit map.
 *
 * Instead of using free lists which require more memory hops, we favour a bit mask at the head.
 *
 * Slabs are held in chains with a root. This root has the meta data about the slab, like what
 * fixed size it is allocating, how often do we return slab pages to the OS etc.
 *
 * Slab roots have three lists of slabs
 *
 * -# <em>Partial</em>: List of partially filled slabs.
 *
 * -# <em>Full</em>: List of completely full slabs.
 *
 * -# <em>Empty</em>: List of completely empty slabs.
 *
 * This separation exists so that a slab root can get to an allocation in constant time by
 * only checking the partial bins. As they fill up, the slab is moved to the full bin. If
 * something in a full bin is freed, it is moved to the partial bin, and when all entries in
 * a slab are free, it is moved to the empty bin.
 *
 * When there are more than a user-settable number of empty slabs, the slab allocator will free
 * another user-settable number of them. The default is for both number to be equal, which means
 * every so often, a slab allocator will return all its free pages to the OS.
 *
 * This allocator can be made thread safe upon request.
 */

typedef struct ak_slab_tag ak_slab;

typedef struct ak_slab_root_tag ak_slab_root;

#if defined(AK_SLAB_USE_LOCKS)
#  define AK_SLAB_LOCK_DEFINE(nm)    ak_spinlock nm
#  define AK_SLAB_LOCK_INIT(root)    ak_spinlock_init(ak_as_ptr((root)->LOCKED))
#  define AK_SLAB_LOCK_ACQUIRE(root) ak_spinlock_acquire(ak_as_ptr((root)->LOCKED))
#  define AK_SLAB_LOCK_RELEASE(root) ak_spinlock_release(ak_as_ptr((root)->LOCKED))
#else
#  define AK_SLAB_LOCK_DEFINE(nm)
#  define AK_SLAB_LOCK_INIT(root)
#  define AK_SLAB_LOCK_ACQUIRE(root)
#  define AK_SLAB_LOCK_RELEASE(root)
#endif

struct ak_slab_tag
{
    ak_slab*      fd;
    ak_slab*      bk;
    ak_slab_root* root;
    ak_bitset512  avail;
    void*         _unused;
};

/*!
 * Slab allocator
 */
struct ak_slab_root_tag
{
    ak_u32 sz;                      /**< the size of elements in this slab */
    ak_u32 npages;                  /**< number of pages to obtain from the OS */
    ak_u32 navail;                  /**< max number of available bits for the slab size \p sz */
    ak_u32 nempty;                  /**< number of empty pages */
    ak_u32 release;                 /**< number of accumulated free empty pages since last release */
    ak_u32 _unused;                 /**< for alignment */

    ak_slab partial_root;           /**< root of the partially filled slab list*/
    ak_slab full_root;              /**< root of the full slab list */
    ak_slab empty_root;             /**< root of the empty slab list */

    ak_u32 RELEASE_RATE;            /**< number of pages moved to empty before a release */
    ak_u32 MAX_PAGES_TO_FREE;       /**< number of pages to free when release happens */
    AK_SLAB_LOCK_DEFINE(LOCKED);    /**< lock for this allocator if locks are enabled */
};

#if !defined(AK_SLAB_RELEASE_RATE)
#  define AK_SLAB_RELEASE_RATE 127
#endif

#if !defined(AK_SLAB_MAX_PAGES_TO_FREE)
#  define AK_SLAB_MAX_PAGES_TO_FREE AK_SLAB_RELEASE_RATE
#endif

/**************************************************************/
/* P R I V A T E                                              */
/**************************************************************/

#define ak_slab_unlink(slab)               \
  do {                                     \
    ak_slab* const sU = (slab);            \
    sU->bk->fd = (sU->fd);                 \
    sU->fd->bk = (sU->bk);                 \
    sU->fd = sU->bk = AK_NULLPTR;          \
  } while (0)

#define ak_slab_link_fd(slab, fwd)         \
  do {                                     \
    ak_slab* const sLF = (slab);           \
    ak_slab* const fLF = (fwd);            \
    sLF->fd = fLF;                         \
    fLF->bk = sLF;                         \
  } while (0)

#define ak_slab_link_bk(slab, back)        \
  do {                                     \
    ak_slab* const sLB = (slab);           \
    ak_slab* const bLB = (back);           \
    sLB->bk = bLB;                         \
    bLB->fd = sLB;                         \
  } while (0)

#define ak_slab_link(slab, fwd, back)      \
  do {                                     \
    ak_slab* const sL = (slab);            \
    ak_slab* const fL = (fwd);             \
    ak_slab* const bL = (back);            \
    ak_slab_link_bk(sL, bL);               \
    ak_slab_link_fd(sL, fL);               \
  } while (0)

ak_inline static void ak_slab_init_chain_head(ak_slab* s, ak_slab_root* rootp)
{
    s->fd = s->bk = s;
    s->root = rootp;
    ak_bitset512_clear_all(&(s->avail));
}

ak_inline static ak_sz ak_num_pages_for_sz(ak_sz sz)
{
    return (sz)/4;
}

#define ak_slab_init(m, s, av, r)                                             \
  do {                                                                        \
    void* slabmem = (m);                                                      \
    ak_sz slabsz = (s);                                                       \
    ak_sz slabnavail = (av);                                                  \
    ak_slab_root* slabroot = (r);                                             \
                                                                              \
    AKMALLOC_ASSERT(slabmem);                                                 \
    AKMALLOC_ASSERT(slabsz < (AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))); \
    AKMALLOC_ASSERT(slabsz > 0);                                              \
    AKMALLOC_ASSERT(slabsz % 2 == 0);                                         \
                                                                              \
    AKMALLOC_ASSERT(slabnavail < 512);                                        \
    AKMALLOC_ASSERT(slabnavail > 0);                                          \
                                                                              \
    ak_slab* s = (ak_slab*)slabmem;                                           \
    s->fd = s->bk = AK_NULLPTR;                                               \
    s->root = slabroot;                                                       \
    ak_bitset512_clear_all(&(s->avail));                                      \
    int inavail = (int)slabnavail;                                            \
    for (int i = 0; i < inavail; ++i) {                                       \
        ak_bitset512_set(&(s->avail), i);                                     \
    }                                                                         \
    (void)slabsz;                                                             \
  } while (0)

ak_inline static ak_slab* ak_slab_new_init(char* mem, ak_sz sz, ak_sz navail, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    ak_slab_init(mem, sz, navail, root);
    ak_slab* slab = ak_ptr_cast(ak_slab, mem);
    ak_slab_link(slab, fd, bk);
    return slab;
}

static ak_slab* ak_slab_new_alloc(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    const int NPAGES = root->npages;

    // try to acquire a page and fit as many slabs as possible in
    char* const mem = (char*)ak_os_alloc(NPAGES * AKMALLOC_DEFAULT_PAGE_SIZE);
    {// return if no mem
        if (ak_unlikely(!mem)) { return AK_NULLPTR; }
    }

    ak_sz navail = root->navail;

    char* cmem = mem;
    for (int i = 0; i < NPAGES - 1; ++i) {
        ak_slab* nextpage = ak_ptr_cast(ak_slab, (cmem + AKMALLOC_DEFAULT_PAGE_SIZE));
        ak_slab* curr = ak_slab_new_init(cmem, sz, navail, nextpage, bk, root);
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_ones(&(curr->avail)) == (int)navail);
        (void)curr;
        bk = nextpage;
        cmem += AKMALLOC_DEFAULT_PAGE_SIZE;
    }

    ak_slab_new_init(cmem, sz, navail, fd, bk, root);

    return ak_ptr_cast(ak_slab, mem);
}

static ak_slab* ak_slab_new_reuse(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    AKMALLOC_ASSERT(root->nempty >= 1);

    ak_sz navail = root->navail;
    ak_slab* const curr = root->empty_root.fd;
    ak_slab_unlink(curr);
    ak_slab_new_init((char*)curr, sz, navail, fd, bk, root);

    --(root->nempty);

    return curr;
}

ak_inline static ak_slab* ak_slab_new(ak_sz sz, ak_slab* fd, ak_slab* bk, ak_slab_root* root)
{
    return (root->nempty > 0)
                ? ak_slab_new_reuse(sz, fd, bk, root)
                : ak_slab_new_alloc(sz, fd, bk, root);
}

#define ak_slab_2_mem(s) (char*)(void*)((s) + 1)

ak_inline static int ak_slab_all_free(ak_slab* s)
{
    const ak_bitset512* pavail = &(s->avail);
    ak_u32 nto;
    ak_bitset512_fill_num_trailing_ones(pavail, nto);
    return nto == s->root->navail;
}

ak_inline static int ak_slab_none_free(ak_slab* s)
{
    const ak_bitset512* pavail = &(s->avail);
    int ntz;
    ak_bitset512_fill_num_trailing_zeros(pavail, ntz);
    return ntz == 512;
}

static void* ak_slab_search(ak_slab* s, ak_sz sz, ak_u32 navail, ak_slab** pslab, int* pntz)
{
    const ak_slab* const root = s;
    void* mem = AK_NULLPTR;
    if (ak_likely(s->fd != root)) {
        AKMALLOC_ASSERT(pslab);
        AKMALLOC_ASSERT(pntz);
        s = s->fd;

        // partial list entry must not be full
        AKMALLOC_ASSERT(ak_bitset512_num_trailing_zeros(&(s->avail)) != 512);

        const ak_bitset512* pavail = &(s->avail);
        int ntz;
        ak_bitset512_fill_num_trailing_zeros(pavail, ntz);

        AKMALLOC_ASSERT(ak_bitset512_get(&(s->avail), ntz));
        ak_bitset512_clear(&(s->avail), ntz);
        mem = ak_slab_2_mem(s) + (ntz * sz);

        *pslab = s;
        if (ntz == (int)navail - 1) {
            ntz = 512;
        } else {
            ak_bitset512_fill_num_trailing_zeros(pavail, ntz);
        }
        *pntz = ntz;
    }
    return mem;
}

static void ak_slab_release_pages(ak_slab_root* root, ak_slab* s, ak_u32 numtofree)
{
    ak_slab* const r = s;
    ak_slab* next = AK_NULLPTR;
    s = s->fd;
    for (ak_u32 ct = 0; ct < numtofree; ++ct) {
        if (s == r) {
            break;
        } else {
            next = s->fd;
        }
        ak_slab_unlink(s);
        ak_os_free(s, AKMALLOC_DEFAULT_PAGE_SIZE);
        s = next;
    }
}

ak_inline static void ak_slab_release_os_mem(ak_slab_root* root)
{
    ak_u32 numtofree = root->nempty;
    numtofree = (numtofree > root->MAX_PAGES_TO_FREE)
                    ? root->MAX_PAGES_TO_FREE
                    : numtofree;
    ak_slab_release_pages(root, &(root->empty_root), numtofree);
    root->nempty -= numtofree;
    root->release = 0;
}

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

/*!
 * Initialize a slab allocator.
 * \param s; Pointer to the allocator root to initialize (non-NULL)
 * \param sz; Size of the slab elements (maximum allowed is 4000)
 * \param npages; Number of pages to allocate from the OS at once.
 * \param relrate; Release rate, \ref akmallocDox
 * \param maxpagefree; Number of segments to free upon release, \ref akmallocDox
 */
static void ak_slab_init_root(ak_slab_root* s, ak_sz sz, ak_u32 npages, ak_u32 relrate, ak_u32 maxpagefree)
{
    s->sz = (ak_u32)sz;
    s->navail = (ak_u32)(AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/(ak_u32)sz;
    s->npages = npages;
    s->nempty = 0;
    s->release = 0;

    ak_slab_init_chain_head(&(s->partial_root), s);
    ak_slab_init_chain_head(&(s->full_root), s);
    ak_slab_init_chain_head(&(s->empty_root), s);

    s->RELEASE_RATE = relrate;
    s->MAX_PAGES_TO_FREE = maxpagefree;
    AK_SLAB_LOCK_INIT(s);
}

/*!
 * Default initialize a slab allocator.
 * \param s; Pointer to the allocator root to initialize (non-NULL)
 * \param sz; Size of the slab elements (maximum allowed is 4000)
 */
ak_inline static void ak_slab_init_root_default(ak_slab_root* s, ak_sz sz)
{
    ak_slab_init_root(s, sz, (ak_u32)ak_num_pages_for_sz(sz), (ak_u32)(AK_SLAB_RELEASE_RATE), (ak_u32)(AK_SLAB_MAX_PAGES_TO_FREE));
}

/*!
 * Attempt to allocate memory from the slab allocator root.
 * \param root; Pointer to the allocator root
 *
 * \return \c 0 on failure, else pointer to at least \p root->sz bytes of memory.
 */
ak_inline static void* ak_slab_alloc(ak_slab_root* root)
{
    int ntz = 0;
    ak_slab* slab = AK_NULLPTR;

    AK_SLAB_LOCK_ACQUIRE(root);
    const ak_sz sz = root->sz;

    void* mem = ak_slab_search(&(root->partial_root), sz, root->navail, &slab, &ntz);

    if (ak_unlikely(!mem)) {
        slab = ak_slab_new(sz, root->partial_root.fd, &(root->partial_root), root);
        if (ak_likely(slab)) {
            AKMALLOC_ASSERT(ak_bitset512_get(&(slab->avail), 0));
            ak_bitset512_clear(&(slab->avail), 0);
            mem = ak_slab_2_mem(slab);
        }
    } else if (ak_unlikely(ntz == 512)) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->full_root.fd, &(root->full_root));
    }

    AK_SLAB_LOCK_RELEASE(root);

    return mem;
}

/*!
 * Return memory to the slab allocator root.
 * \param p; Pointer to the memory to return.
 */
ak_inline static void ak_slab_free(void* p)
{
    char* mem = (char*)p;

    // round to page
    ak_slab* slab = (ak_slab*)(ak_page_start_before(p));
    AKMALLOC_ASSERT(slab->root);

    ak_slab_root* root = slab->root;
    AK_SLAB_LOCK_ACQUIRE(root);

    int movetopartial = ak_slab_none_free(slab);
    const ak_sz sz = root->sz;

    int idx = (int)(mem - (char*)ak_slab_2_mem(slab))/(int)sz;
    AKMALLOC_ASSERT(!ak_bitset512_get(&(slab->avail), idx));
    ak_bitset512_set(&(slab->avail), idx);

    if (ak_unlikely(movetopartial)) {
        // put at the back of the partial list so the full ones
        // appear at the front
        ak_slab_unlink(slab);
        ak_slab_link(slab, &(root->partial_root), root->partial_root.bk);
    } else if (ak_unlikely(ak_slab_all_free(slab))) {
        ak_slab_unlink(slab);
        ak_slab_link(slab, root->empty_root.fd, &(root->empty_root));
        ++(root->nempty); ++(root->release);
        if (root->release >= root->RELEASE_RATE) {
            ak_slab_release_os_mem(root);
        }
    }

    AK_SLAB_LOCK_RELEASE(root);
}

/*!
 * Destroy the slab allocator root and return all memory to the OS.
 * \param root; Pointer to the allocator root
 */
static void ak_slab_destroy(ak_slab_root* root)
{
    ak_slab_release_pages(root, &(root->empty_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->partial_root), AK_U32_MAX);
    ak_slab_release_pages(root, &(root->full_root), AK_U32_MAX);
    root->nempty = 0;
    root->release = 0;
}
/********************** slab end ************************/

/********************** coalescing allocator begin **********************/
/*!
 *
 * \page caalloc Coalescing allocator with a free list
 *
 * This is a popular allocator and similar to the one implemented in <tt>dlmalloc</tt> which
 * is at the heart of many allocator implementations in use today including <tt>glibc</tt>.
 *
 * It is based on the constant time coalescing scheme described by Donald Knuth in The Art of Computer Programming, Vol I, Fundamental Algorithms, Addison-Wesley, Reading, MA, 1968.
 *
 * The major departures from the scheme presented in dlmalloc is that this allocator doesn't
 * try to merge new segments and tries to free similarly to slab allocators (\ref slaballoc)
 * wherein we keep a list of empty segments that can be re-used and a user-settable number of
 * them are freed to the OS at a user-settable interval.
 *
 * Segment sizes acquired for this allocator are user-settable also.
 *
 * This allocator works by keeping a boundary tag for each allocation which holds the information
 * about the chunk to which it points.
 *
 * In actuality, each tag holds the information about the previous chunk also. This allocator
 * has a minimum alignment of 16 bytes which yields four free bits in every address.
 *
 * -# <em>0th bit</em>: Used to store whether a chunk has a chunk before it in the segment.
 * -# <em>1st bit</em>: Used to store whether a chunk has a chunk after it in the segment.
 * -# <em>2nd bit</em>: Used to store whether a chunk is allocated or free.
 * -# <em>Rest</em> of the bits store the size of the allocated chunk.
 *
 * There is a bit still unused which is always 0. This is exploited by the overall malloc
 * implementation to distinguish memory allocated by coalescing allocators from other schemes.
 *
 * In conjunction with the chunks, the 16 byte alignment also allows us to store a doubly linked
 * free list on x64 platforms within the overhead of any allocated chunk.
 *
 * Allocation is done by traversing the free list to find the first chunk of the required
 * or higher size.
 *
 * Freeing is done by marking the chunk as free, merging with neighbouring chunks if they are free
 * and if the chunk is the first and last chunk in a segment, we migrate the segment to the list
 * of free segments.
 *
 * Coalescing allocator roots have two lists of segments
 *
 * -# <em>NonEmpty</em>: List of non-empty segments.
 *
 * -# <em>Empty</em>: List of completely empty segments.
 *
 * When there are more than a user-settable number of empty segments, the slab allocator will free
 * another user-settable number of them. The default is for both number to be equal, which means
 * every so often, a slab allocator will return all its free pages to the OS.
 *
 * This allocator can be made thread safe upon request.
 */

/**
 * We choose a minimum alignment of 16. One could increase this, but not decrease.
 *
 * 16 byte alignment buys us a few things:
 * -# The 3 low-bits of an address will be 000. Therefore we can store metadata in them.
 * -# On x64, we can exactly store two pointers worth of information in any block which
 *    can be used to house an implicit free list.
 */
#define AK_COALESCE_ALIGN 16

#if !defined(AK_COALESCE_SEGMENT_GRANULARITY)
#  define AK_COALESCE_SEGMENT_GRANULARITY 65536
#endif

#if !defined(AK_COALESCE_SEGMENT_SIZE)
/* 64KB */
#  define AK_COALESCE_SEGMENT_SIZE AK_COALESCE_SEGMENT_GRANULARITY
#endif

#if defined(AK_CA_USE_LOCKS)
#  define AK_CA_LOCK_DEFINE(nm)    ak_spinlock nm
#  define AK_CA_LOCK_INIT(root)    ak_spinlock_init(ak_as_ptr((root)->LOCKED))
#  define AK_CA_LOCK_ACQUIRE(root) ak_spinlock_acquire(ak_as_ptr((root)->LOCKED))
#  define AK_CA_LOCK_RELEASE(root) ak_spinlock_release(ak_as_ptr((root)->LOCKED))
#else
#  define AK_CA_LOCK_DEFINE(nm)
#  define AK_CA_LOCK_INIT(root)
#  define AK_CA_LOCK_ACQUIRE(root)
#  define AK_CA_LOCK_RELEASE(root)
#endif

typedef ak_sz ak_alloc_info;

typedef struct ak_alloc_node_tag ak_alloc_node;

typedef struct ak_free_list_node_tag ak_free_list_node;

typedef struct ak_ca_segment_tag ak_ca_segment;

typedef struct ak_ca_root_tag ak_ca_root;

struct ak_alloc_node_tag
{
#if AKMALLOC_BITNESS == 32
    ak_alloc_info _unused0;
    ak_alloc_info _unused1;
#endif
    ak_alloc_info previnfo;
    ak_alloc_info currinfo;
};

struct ak_free_list_node_tag
{
    ak_free_list_node* bk;
    ak_free_list_node* fd;
};

struct ak_ca_segment_tag
{
    ak_ca_segment* bk;
    ak_ca_segment* fd;
    ak_sz sz;
    ak_alloc_node* head;
};

/*!
 * The root for a coalescing allocator.
 */
struct ak_ca_root_tag
{
    ak_ca_segment main_root;        /**< root of non empty segments */
    ak_ca_segment empty_root;       /**< root of empty segments */

    ak_free_list_node free_root;    /**< root of the free list */

    ak_u32 nempty;                  /**< number of empty segments */
    ak_u32 release;                 /**< number of segments freed since last release */

    ak_u32 RELEASE_RATE;            /**< release rate for this root */
    ak_u32 MAX_SEGMENTS_TO_FREE;    /**< number of segments to free when release is done */
    ak_sz MIN_SIZE_TO_SPLIT;        /**< minimum size of split node to decide whether to 
                                         split a free list node */

    AK_CA_LOCK_DEFINE(LOCKED);      /**< lock for this allocator if locks are enabled */
};

/**************************************************************/
/* P R I V A T E                                              */
/**************************************************************/

#define ak_ca_to_sz(p) (((ak_sz)(p)) & ~(AK_COALESCE_ALIGN - 1))

#define ak_ca_is_first(p) (((ak_sz)(p)) & (AK_SZ_ONE << 0))

#define ak_ca_is_last(p) (((ak_sz)(p)) & (AK_SZ_ONE << 1))

#define ak_ca_is_free(p) (((ak_sz)(p)) & (AK_SZ_ONE << 2))

ak_inline static void ak_ca_set_sz(ak_alloc_info* p, ak_sz sz)
{
    AKMALLOC_ASSERT(sz == ak_ca_to_sz((ak_alloc_info)sz));
    // 7 because there are only 3 useful bits. the fourth bit may collect garbage.
    *p = (ak_alloc_info)((((ak_sz)*p)  &  ((ak_sz)7)) |
                                 (sz   & ~(AK_COALESCE_ALIGN - 1)));
}

ak_inline static void ak_ca_set_is_first(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 0)) | (v ? (AK_SZ_ONE << 0) : 0));
}

ak_inline static void ak_ca_set_is_last(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 1)) | (v ? (AK_SZ_ONE << 1) : 0));
}

ak_inline static void ak_ca_set_is_free(ak_alloc_info* p, int v)
{
    *p = (ak_alloc_info)((((ak_sz)*p) & ~(AK_SZ_ONE << 2)) | (v ? (AK_SZ_ONE << 2) : 0));
}

ak_inline static ak_alloc_node* ak_ca_next_node(ak_alloc_node* node)
{
    return ak_ca_is_last(node->currinfo)
                ? AK_NULLPTR
                : ak_ptr_cast(ak_alloc_node,((char*)(node + 1) + ak_ca_to_sz(node->currinfo)));
}

ak_inline static ak_alloc_node* ak_ca_prev_node(ak_alloc_node* node)
{
    return ak_ca_is_first(node->currinfo)
                ? AK_NULLPTR
                : ak_ptr_cast(ak_alloc_node, ((char*)(node - 1) - ak_ca_to_sz(node->previnfo)));
}

ak_inline static void ak_ca_update_footer(ak_alloc_node* p)
{
    ak_alloc_node* n = ak_ca_next_node(p);
    if (n) {
        n->previnfo = p->currinfo;
    }
}

#define ak_free_list_node_unlink(node)               \
  do {                                               \
    ak_free_list_node* const sU = (node);            \
    sU->bk->fd = (sU->fd);                           \
    sU->fd->bk = (sU->bk);                           \
    sU->fd = sU->bk = AK_NULLPTR;                    \
  } while (0)

#define ak_free_list_node_link_fd(node, fwd)         \
  do {                                               \
    ak_free_list_node* const sLF = (node);           \
    ak_free_list_node* const fLF = (fwd);            \
    sLF->fd = fLF;                                   \
    fLF->bk = sLF;                                   \
  } while (0)

#define ak_free_list_node_link_bk(node, back)        \
  do {                                               \
    ak_free_list_node* const sLB = (node);           \
    ak_free_list_node* const bLB = (back);           \
    sLB->bk = bLB;                                   \
    bLB->fd = sLB;                                   \
  } while (0)

#define ak_free_list_node_link(node, fwd, back)      \
  do {                                               \
    ak_free_list_node* const sL = (node);            \
    ak_free_list_node* const fL = (fwd);             \
    ak_free_list_node* const bL = (back);            \
    ak_free_list_node_link_bk(sL, bL);               \
    ak_free_list_node_link_fd(sL, fL);               \
  } while (0)

#define ak_ca_segment_unlink(node)                   \
  do {                                               \
    ak_ca_segment* const sU = (node);                \
    sU->bk->fd = (sU->fd);                           \
    sU->fd->bk = (sU->bk);                           \
    sU->fd = sU->bk = AK_NULLPTR;                    \
  } while (0)

#define ak_ca_segment_link_fd(node, fwd)             \
  do {                                               \
    ak_ca_segment* const sLF = (node);               \
    ak_ca_segment* const fLF = (fwd);                \
    sLF->fd = fLF;                                   \
    fLF->bk = sLF;                                   \
  } while (0)

#define ak_ca_segment_link_bk(node, back)            \
  do {                                               \
    ak_ca_segment* const sLB = (node);               \
    ak_ca_segment* const bLB = (back);               \
    sLB->bk = bLB;                                   \
    bLB->fd = sLB;                                   \
  } while (0)

#define ak_ca_segment_link(node, fwd, back)          \
  do {                                               \
    ak_ca_segment* const sL = (node);                \
    ak_ca_segment* const fL = (fwd);                 \
    ak_ca_segment* const bL = (back);                \
    ak_ca_segment_link_bk(sL, bL);                   \
    ak_ca_segment_link_fd(sL, fL);                   \
  } while (0)

#define ak_circ_list_for_each(type, name, list)      \
    type* name = (list)->fd;                         \
    for(type* const iterroot = (list); name != iterroot; name = name->fd)

#define ak_ca_aligned_size(x) ((x) ? (((x) + AK_COALESCE_ALIGN - 1) & ~(AK_COALESCE_ALIGN - 1)) : AK_COALESCE_ALIGN)

#define ak_ca_aligned_segment_size(x) (((x) + (AK_COALESCE_SEGMENT_SIZE) - 1) & ~((AK_COALESCE_SEGMENT_SIZE) - 1))

ak_inline static void* ak_ca_search_free_list(ak_free_list_node* root, ak_sz sz, ak_sz splitsz)
{
    AKMALLOC_ASSERT(splitsz >= sizeof(ak_free_list_node));
    AKMALLOC_ASSERT(splitsz % AK_COALESCE_ALIGN == 0);

    // add the overhead per node
    splitsz += sizeof(ak_alloc_node);

    // walk through list finding the first element that fits and split if required
    ak_circ_list_for_each(ak_free_list_node, node, root) {
        ak_alloc_node* n = ((ak_alloc_node*)(node)) - 1;
        AKMALLOC_ASSERT(ak_ca_is_free(n->currinfo));
        ak_sz nodesz = ak_ca_to_sz(n->currinfo);
        if (nodesz >= sz) {
            if ((nodesz - sz) > splitsz) {
                // split and assign
                ak_alloc_node* newnode = ak_ptr_cast(ak_alloc_node, (((char*)node) + sz));
                int islast = ak_ca_is_last(n->currinfo);

                ak_ca_set_sz(ak_as_ptr(n->currinfo), sz);
                ak_ca_set_is_last(ak_as_ptr(n->currinfo), 0);
                ak_ca_set_is_free(ak_as_ptr(n->currinfo), 0);
                ak_ca_update_footer(n);

                ak_ca_set_sz(ak_as_ptr(newnode->currinfo), nodesz - sz - sizeof(ak_alloc_node));
                ak_ca_set_is_first(ak_as_ptr(newnode->currinfo), 0);
                ak_ca_set_is_last(ak_as_ptr(newnode->currinfo), islast);
                ak_ca_set_is_free(ak_as_ptr(newnode->currinfo), 1);
                ak_ca_update_footer(newnode);
                
                // copy free list node from node
                ak_free_list_node* fl = (ak_free_list_node*)(newnode + 1);
                ak_free_list_node_link(fl, node->fd, node->bk);
                AKMALLOC_ASSERT(n->currinfo == newnode->previnfo);
            } else {
                // return as is
                ak_ca_set_is_free(ak_as_ptr(n->currinfo), 0);
                ak_ca_update_footer(n);
                ak_free_list_node_unlink(node);
            }
            return node;
        }
    }
    return AK_NULLPTR;
}

static int ak_ca_add_new_segment(ak_ca_root* root, char* mem, ak_sz sz)
{
    if (ak_likely(mem)) {
        // make segment
        ak_ca_segment* seg = ak_ptr_cast(ak_ca_segment, (mem + sz - sizeof(ak_ca_segment)));
        ak_ca_segment_link(seg, root->main_root.fd, ak_as_ptr(root->main_root));
        seg->sz = sz;
        seg->head = ak_ptr_cast(ak_alloc_node, mem);
        {// add to free list
            ak_alloc_node* hd = seg->head;
            ak_sz actualsize = (sz - sizeof(ak_alloc_node) - sizeof(ak_ca_segment));
            // store actual size in previnfo
            hd->previnfo = actualsize;
            ak_ca_set_is_first(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_is_last(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_is_free(ak_as_ptr(hd->currinfo), 1);
            ak_ca_set_sz(ak_as_ptr(hd->currinfo), actualsize);
            ak_free_list_node* fl = (ak_free_list_node*)(hd + 1);
            ak_free_list_node_link(fl, root->free_root.fd, ak_as_ptr(root->free_root));
        }
        return 1;
    }
    return 0;
}

static int ak_ca_get_new_segment(ak_ca_root* root, ak_sz sz)
{
    // align to segment size multiple
    sz += sizeof(ak_ca_segment) + sizeof(ak_alloc_node) + sizeof(ak_free_list_node);
    sz = ak_ca_aligned_segment_size(sz);

    // search empty_root for a segment that is as big or more
    char* mem = AK_NULLPTR;
    ak_sz segsz = sz;
    ak_circ_list_for_each(ak_ca_segment, seg, ak_as_ptr(root->empty_root)) {
        if (seg->sz >= sz) {
            mem = (char*)(seg->head);
            segsz = seg->sz;
            ak_ca_segment_unlink(seg);
            --(root->nempty);
            break;
        }
    }

    return ak_ca_add_new_segment(root, mem ? mem : ((char*)ak_os_alloc(sz)), segsz);
}

static ak_u32 ak_ca_return_os_mem(ak_ca_segment* r, ak_u32 num)
{
    ak_u32 ct = 0;
    ak_ca_segment* next = r->fd;
    ak_ca_segment* curr = next;
    for(; curr != r; curr = next) {
        if (ct >= num) {
            break;
        }
        next = curr->fd;
        ak_ca_segment_unlink(curr);
        ak_os_free(curr->head, curr->sz);
        ++ct;
    }
    return ct;
}

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

/*!
 * Initialize a coalescing allocator.
 * \param root; Pointer to the allocator root to initialize (non-NULL)
 * \param relrate; Release rate, \ref akmallocDox
 * \param maxsegstofree; Number of segments to free upon release, \ref akmallocDox
 */
static void ak_ca_init_root(ak_ca_root* root, ak_u32 relrate, ak_u32 maxsegstofree)
{
    AKMALLOC_ASSERT_ALWAYS(AK_COALESCE_SEGMENT_SIZE % AK_COALESCE_SEGMENT_GRANULARITY == 0);
    AKMALLOC_ASSERT_ALWAYS(((AK_COALESCE_SEGMENT_SIZE & (AK_COALESCE_SEGMENT_SIZE - 1)) == 0) && "Segment size must be a power of 2");

    ak_ca_segment_link(&(root->main_root), &(root->main_root), &(root->main_root));
    ak_ca_segment_link(&(root->empty_root), &(root->empty_root), &(root->empty_root));
    ak_free_list_node_link(&(root->free_root), &(root->free_root), &(root->free_root));
    root->nempty = root->release = 0;

    root->RELEASE_RATE = relrate;
    root->MAX_SEGMENTS_TO_FREE = maxsegstofree;
    root->MIN_SIZE_TO_SPLIT = (sizeof(ak_free_list_node) >= AK_COALESCE_ALIGN) ? sizeof(ak_free_list_node) : AK_COALESCE_ALIGN;
    AK_CA_LOCK_INIT(root);
}

/*!
 * Default initialize a coalescing allocator.
 * \param root; Pointer to the allocator root to initialize (non-NULL)
 */
ak_inline static void ak_ca_init_root_default(ak_ca_root* root)
{
#if AKMALLOC_BITNESS == 32
    static const ak_u32 rate =  255;
#else
    static const ak_u32 rate = 2047;
#endif
    ak_ca_init_root(root, rate, rate);
}

/*!
 * Attempt to grow an existing allocation.
 * \param root; Pointer to the allocator root
 * \param mem; Existing memory to grow
 * \param newsz; The new size for the allocation
 *
 * \return \c 0 on failure, and \p mem on success which can hold at least \p newsz bytes.
 */
ak_inline static void* ak_ca_realloc_in_place(ak_ca_root* root, void* mem, ak_sz newsz)
{
    void* retmem = AK_NULLPTR;

    ak_alloc_node* n = ak_ptr_cast(ak_alloc_node, mem) - 1;
    AKMALLOC_ASSERT(ak_ca_is_free(n->currinfo));
    // check if there is a free next, if so, maybe merge
    ak_sz sz = ak_ca_to_sz(n->currinfo);

    ak_alloc_node* next = ak_ca_next_node(n);
    if (next && ak_ca_is_free(next->currinfo)) {
        AKMALLOC_ASSERT(n->currinfo == next->previnfo);
        ak_sz nextsz = ak_ca_to_sz(next->currinfo);
        ak_sz totalsz = nextsz + sz + sizeof(ak_alloc_node);
        if (totalsz >= newsz) {
            AK_CA_LOCK_ACQUIRE(root);

            // we could remember the prev and next free entries and link them
            // back if the freed size is larger and we split the new node
            // but we assume that reallocs are rare and that one realloc may get more
            // so we try to keep it simple here, and simply merge the two

            ak_free_list_node_unlink((ak_free_list_node*)(next + 1));
            // don't need to change attributes on next as it is going away
            if (ak_ca_is_last(next->currinfo)) {
                ak_ca_set_is_last(ak_as_ptr(n->currinfo), 1);
            }
            ak_ca_set_sz(ak_as_ptr(n->currinfo), totalsz);
            ak_ca_update_footer(n);

            if ((totalsz - newsz) > root->MIN_SIZE_TO_SPLIT) {
                // split and assign
                ak_alloc_node* newnode = ak_ptr_cast(ak_alloc_node, (((char*)(n + 1)) + newsz));
                int islast = ak_ca_is_last(n->currinfo);

                ak_ca_set_sz(ak_as_ptr(n->currinfo), newsz);
                ak_ca_set_is_last(ak_as_ptr(n->currinfo), 0);
                ak_ca_set_is_free(ak_as_ptr(n->currinfo), 0);
                ak_ca_update_footer(n);

                ak_ca_set_sz(ak_as_ptr(newnode->currinfo), totalsz - newsz - sizeof(ak_alloc_node));
                ak_ca_set_is_first(ak_as_ptr(newnode->currinfo), 0);
                ak_ca_set_is_last(ak_as_ptr(newnode->currinfo), islast);
                ak_ca_set_is_free(ak_as_ptr(newnode->currinfo), 1);
                ak_ca_update_footer(newnode);
                
                // copy free list node from node
                ak_free_list_node* fl = (ak_free_list_node*)(newnode + 1);
                ak_free_list_node_link(fl, nextcopy.fd, nextcopy.bk);
                AKMALLOC_ASSERT(n->currinfo == newnode->previnfo);
            }

            retmem = mem;

            AK_CA_LOCK_RELEASE(root);
        }
    }

    return retmem;
}

/*!
 * Attempt to allocate memory from the coalescing allocator root.
 * \param root; Pointer to the allocator root
 * \param s; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p s bytes of memory.
 */
static void* ak_ca_alloc(ak_ca_root* root, ak_sz s)
{
    // align and round size
    ak_sz sz = ak_ca_aligned_size(s);
    AK_CA_LOCK_ACQUIRE(root);
    // search free list
    ak_sz splitsz = root->MIN_SIZE_TO_SPLIT;
    void* mem = ak_ca_search_free_list(ak_as_ptr(root->free_root), sz, splitsz);
    // add new segment
    if (ak_unlikely(!mem)) {
        // NOTE: could also move segments from empty_root to main_root
        if (ak_likely(ak_ca_get_new_segment(root, sz))) {
            mem = ak_ca_search_free_list(ak_as_ptr(root->free_root), sz, splitsz);
            AKMALLOC_ASSERT(mem);
        }
    }
    AK_CA_LOCK_RELEASE(root);
    return mem;
}

/*!
 * Return memory to the coalescing allocator root.
 * \param root; Pointer to the allocator root
 * \param m; The memory to return.
 */
ak_inline static void ak_ca_free(ak_ca_root* root, void* m)
{
    // get alloc header before
    ak_alloc_node* node = ((ak_alloc_node*)m) - 1;

    AK_CA_LOCK_ACQUIRE(root);

    ak_alloc_node* nextnode = ak_ca_next_node(node);
    ak_alloc_node* prevnode = ak_ca_prev_node(node);
    int coalesce = 0;

    // mark as free
    AKMALLOC_ASSERT(!ak_ca_is_free(node->currinfo));
    AKMALLOC_ASSERT(!nextnode || (node->currinfo == nextnode->previnfo));
    ak_ca_set_is_free(ak_as_ptr(node->currinfo), 1);
    ak_ca_update_footer(node);
    
    // NOTE: maybe this should happen at a lower frequency?
    // coalesce if free before or if free after or both
    if (prevnode && ak_ca_is_free(node->previnfo)) {
        // coalesce back
        // update node and the footer
        ak_sz newsz = ak_ca_to_sz(node->previnfo) + ak_ca_to_sz(node->currinfo) + sizeof(ak_alloc_node);
        ak_ca_set_sz(ak_as_ptr(prevnode->currinfo), newsz);
        ak_ca_set_is_last(ak_as_ptr(prevnode->currinfo), nextnode == AK_NULLPTR);
        ak_ca_update_footer(prevnode);
        AKMALLOC_ASSERT(!nextnode || ak_ca_next_node(prevnode) == nextnode);
        AKMALLOC_ASSERT(!nextnode || prevnode->currinfo == nextnode->previnfo);
        coalesce += 1;
        // update free list
    }

    if (nextnode && ak_ca_is_free(nextnode->currinfo)) {
        // coalesce forward
        // update node and the footer
        ak_alloc_node* n = (coalesce) ? prevnode : node;
        ak_sz newsz = ak_ca_to_sz(n->currinfo) + ak_ca_to_sz(nextnode->currinfo) + sizeof(ak_alloc_node);
        ak_ca_set_sz(ak_as_ptr(n->currinfo), newsz);
        ak_ca_set_is_last(ak_as_ptr(n->currinfo), ak_ca_is_last(nextnode->currinfo));
        ak_ca_update_footer(n);
        AKMALLOC_ASSERT(ak_ca_is_last(n->currinfo) || (n->currinfo == ak_ca_next_node(nextnode)->previnfo));
        coalesce += 2;
    }

    // update free lists
    ak_alloc_node* tocheck = AK_NULLPTR;
    switch (coalesce) {
        case 0: {
                // thread directly
                ak_free_list_node* fl = (ak_free_list_node*)(node + 1);
                ak_free_list_node_link(fl, root->free_root.fd, ak_as_ptr(root->free_root));
            }
            break;
        case 1: {
                // prevnode already threaded through
                tocheck = prevnode;
            }
            break;
        case 2: {
                // copy free list entry from nextnode
                ak_free_list_node* fl = (ak_free_list_node*)(node + 1);
                ak_free_list_node* nextfl = (ak_free_list_node*)(nextnode + 1);
                ak_free_list_node_link(fl, nextfl->fd, nextfl->bk);
                tocheck = node;
            }
            break;
        case 3: {
                ak_free_list_node* nextfl = (ak_free_list_node*)(nextnode + 1);
                ak_free_list_node_unlink(nextfl);
                tocheck = prevnode;
            }
            break;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Should not get here!");
            break;
    }

    // move to empty if segment is empty

    if (tocheck && ak_ca_is_first(tocheck->currinfo) && ak_ca_is_last(tocheck->currinfo)) {
        // remove free list entry
        ak_free_list_node* fl = (ak_free_list_node*)(tocheck + 1);
        ak_free_list_node_unlink(fl);
        // actual size is in tocheck->previnfo
        AKMALLOC_ASSERT(tocheck->previnfo == ak_ca_to_sz(tocheck->currinfo));
        ak_ca_segment* seg = ak_ptr_cast(ak_ca_segment, ((char*)(tocheck + 1) + tocheck->previnfo));
        AKMALLOC_ASSERT(tocheck->previnfo == (seg->sz - sizeof(ak_alloc_node) - sizeof(ak_ca_segment)));
        ak_ca_segment_unlink(seg);
        ak_ca_segment_link(seg, root->empty_root.fd, ak_as_ptr(root->empty_root));
        ++(root->nempty); ++(root->release);
        // check if we should free empties
        if (root->release >= root->RELEASE_RATE) {
            // release segment
            ak_u32 nrem = ak_ca_return_os_mem(ak_as_ptr(root->empty_root), root->MAX_SEGMENTS_TO_FREE);
            root->nempty -= nrem;
            root->release = 0;
        }
    }

    AK_CA_LOCK_RELEASE(root);
}

/*!
 * Destroy the coalescing allocator root and return all memory to the OS.
 * \param root; Pointer to the allocator root
 */
static void ak_ca_destroy(ak_ca_root* root)
{
    ak_ca_return_os_mem(ak_as_ptr(root->main_root), AK_U32_MAX);
    ak_ca_return_os_mem(ak_as_ptr(root->empty_root), AK_U32_MAX);
    root->nempty = root->release = 0;
}
/********************** coalescing allocator end ************************/

/********************** mallocstate begin **********************/
/*!
 *
 * \page akmallocDox akmalloc
 *
 * <tt>akmalloc</tt> is a composite allocator, combining slabs and coalescing allocators.
 *
 * It uses a static <tt>ak_malloc_state</tt>.
 *
 * All the exported APIs are based on \p ak_malloc_state.
 *
 * It uses an array of slabs of sizes from 8B to 256B, an array of coalescing allocators ranging
 * in size from 768B to 1MB and directly uses OS calls beyond that size.
 *
 * It handles multi threading by having a lock per slab or coalescing allocator, or OS calls.
 * Multiple threads that allocate or free a size in a different size category do not contend
 * with each other. 
 *
 * By default shared and static libraries have a thread safe malloc and free.
 *
 * Every allocation has a 8B header which contains a distinct bit mask allowing the allocator
 * to classify it.
 *
 * \code{.cpp}
 * ---0      Coalescing allocator with fourth bit unset always
 * 0101      Slab allocator
 * 1001      mmap()
 * \endcode
 *
 * \see akmalloc/malloc.h
 * \see akmalloc/malloc.c
 *
 * \section customMalloc Customization
 *
 * <tt>akmalloc</tt> can be built as a shared library, static library, or included directly
 * as part of your source code.
 *
 * <h3> Include only </h3>
 * To include this akmalloc directly, simply define <tt>AKMALLOC_INCLUDE_ONLY</tt> before including
 * the <tt>malloc.h</tt> file.
 *
 * <h3> Static library </h3>
 * To build a static library, define <tt>AKMALLOC_LINK_STATIC</tt> and you may have to change
 * <tt>AKMALLOC_EXPORT</tt>.
 *
 * <h3> Shared library </h3>
 * To build a shared library, define <tt>AKMALLOC_BUILD</tt> when building the library, and
 * set <tt>AKMALLOC_EXPORT</tt> to be the right visibility symbol for your compiler/system
 * i.e., <tt>__declspec(dllexport)</tt> on Windows/MSVC and
 * <tt>__attribute__((\__visibility\__("default")))</tt> on Linux/Mac.
 *
 * When including the header for the shared library, ensure <tt>AKMALLOC_EXPORT</tt> is defined
 * correctly for the import (<tt>__declspec(dllimport)</tt> on Windows/MSVC and
 * <tt></tt> on Linux/Mac).
 *
 * <h3> Customizing <tt>akmalloc</tt> </h3>
 * The following defines are available for use while building (or including if your build is only
 * directly including the header files). They are marked as controlling one or more options.
 *
 * It is better to define these before including any <tt>akmalloc</tt> header files.
 *
 * \code{.cpp}
 * // controls whether the symbols available are malloc/free etc. or ak_malloc/ak_free...
 * // works for ak_malloc
 * #define AKMALLOC_USE_PREFIX // [0 | 1]
 *
 * // controls the symbol visibility of APIs, default chosen based on build type
 * // works for all exported APIs
 * // if building a shared library, you probably want something like '__declspec(dllexport)'
 * // on Windows/MSVC and '__attribute__((__visibility__("default")))' on Linux/Mac.
 * #define AKMALLOC_EXPORT // choose based on build type, default: extern
 *
 * // controls the assert macro to use
 * // works with all APIs when built in debug mode (NDEBUG is not defined)
 * #define AKMALLOC_ASSERT_IMPL // default: custom assert macro
 *
 * // whether to use locks for malloc()/free()
 * // works for ak_malloc
 * #define AKMALLOC_USE_LOCKS [0 | 1] // defaults to 1 for libraries
 *
 * // whether to use locks for ak_malloc_from_state() variety of APIs
 * // including malloc.h directly will set this based on AKMALLOC_USE_LOCKS
 * // works for ak_malloc_state
 * #define AK_MALLOCSTATE_USE_LOCKS // defined or undefined, default is undefined
 *
 * // whether to always align allocations at 16 byte boundaries, slabs can do 8
 * // works for ak_malloc_state and ak_malloc
 * #define AK_MIN_SLAB_ALIGN_16 // defined or undefined, default is undefined
 *
 * // number of empty pages after which to free some
 * // works for ak_slab, ak_malloc_state and ak_malloc
 * #define AK_SLAB_RELEASE_RATE // default: 127
 *
 * // number of empty pages to free after a release is reached
 * // works for ak_slab, ak_malloc_state and ak_malloc
 * #define AK_SLAB_MAX_PAGES_TO_FREE // default: AK_SLAB_RELEASE_RATE
 *
 * // multiples of this size are used to obtain memory from the OS for coalescing allocators
 * // works for ak_ca_root, ak_malloc_state and ak_malloc
 * #define AK_COALESCE_SEGMENT_GRANULARITY // default is 256KB for ak_malloc and ak_malloc_state
 *                                         // deafult is 64KB for ak_ca_root
 *
 * // number of empty segments after which to free them
 * // works for ak_ca_root, ak_malloc_state and ak_malloc
 * #define AKMALLOC_COALESCING_ALLOC_RELEASE_RATE // default: 24
 *
 * // number of segments to free after a release is required
 * // works for ak_ca_root, ak_malloc_state and ak_malloc
 * #define AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE // default: AKMALLOC_COALESCING_ALLOC_RELEASE_RATE
 *
 * // at what size to resort to using mmap() like system calls
 * // works for ak_malloc_state and ak_malloc
 * #define MMAP_SIZE // default is system determined, e.g. 65536
 *
 * // customize memory map calls
 * // works for all APIs
 * // signature for map:   void* (*map)(size_t s);              // return 0 on failure
 * // signature for unmap: void  (*unmap)(void* mem, size_t s);
 * #define AKMALLOC_MMAP   // default: system dependent
 * #define AKMALLOC_MUNMAP // default: system dependent
 * \endcode
 */

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

// Coalescing allocs give 16-byte aligned memory where the preamble
// uses three bits. The fourth bit is always free. We use that bit
// to distinguish slabs from coalesced outputs, and mmap-outputs.
//
// xxx0 - coalesce
// 0101 - slab
// 1001 - mmap
#define ak_alloc_type_bits(p) \
  ((*(((const ak_sz*)(p)) - 1)) & (AK_COALESCE_ALIGN - 1))

#define ak_alloc_type_coalesce(sz) \
  ((((ak_sz)sz) & ((ak_sz)8)) == 0)

#define ak_alloc_type_slab(sz) \
  ((((ak_sz)sz) & (AK_COALESCE_ALIGN - 1)) == 10)

#define ak_alloc_type_mmap(sz) \
  ((((ak_sz)sz) & (AK_COALESCE_ALIGN - 1)) == 9)

#define ak_alloc_mark_coalesce(p) ((void)(p))

#define ak_alloc_mark_slab(p) \
  *(((ak_sz*)(p)) - 1) = ((ak_sz)10)

#define ak_alloc_mark_mmap(p) \
  *(((ak_sz*)(p)) - 1) = ((ak_sz)9)

#if defined(AK_MIN_SLAB_ALIGN_16)
#  define ak_slab_mod_sz(x) (ak_ca_aligned_size((x)) + AK_COALESCE_ALIGN)
#  define ak_slab_alloc_2_mem(x) (((ak_sz*)x) + (AK_COALESCE_ALIGN / sizeof(ak_sz)))
#  define ak_slab_mem_2_alloc(x) (((ak_sz*)x) - (AK_COALESCE_ALIGN / sizeof(ak_sz)))
#  define ak_slab_usable_size(x) ((x) - AK_COALESCE_ALIGN)
#else
#  define ak_slab_mod_sz(x) (ak_ca_aligned_size((x) + sizeof(ak_sz)))
#  define ak_slab_alloc_2_mem(x) (((ak_sz*)x) + 1)
#  define ak_slab_mem_2_alloc(x) (((ak_sz*)x) - 1)
#  define ak_slab_usable_size(x) ((x) - sizeof(ak_sz))
#endif

/* cannot be changed. we have fixed size slabs */
#define MIN_SMALL_REQUEST 256

#if !defined(MMAP_SIZE)
#  if !AKMALLOC_WINDOWS
#    define MMAP_SIZE (AK_SZ_ONE << 20) /* 1 MB */
#  else/* Windows */
    /**
     * Memory mapping on Windows is slow. Put the entries in the large free list
     * to avoid mmap() calls.
     */
#    define MMAP_SIZE AK_SZ_MAX
#  endif
#endif


#define NSLABS 16

/*!
 * Sizes for the slabs in an \c ak_malloc_state
 */
static const ak_sz SLAB_SIZES[NSLABS] = {
    16,   32,   48,   64,   80,   96,  112,  128,
   144,  160,  176,  192,  208,  224,  240,  256
};

#define NCAROOTS 8

/*!
 * Sizes for the coalescing allocators in an \c ak_malloc_state
 *
 * Size here denotes maximum size request for each allocator.
 */
static const ak_sz CA_SIZES[NCAROOTS] = {
    768, 1408, 2048, 4096, 8192, 16384, 65536, MMAP_SIZE
};

typedef struct ak_malloc_state_tag ak_malloc_state;

/*!
 * Private malloc like allocator
 */
struct ak_malloc_state_tag
{
    ak_sz         init;             /**< whether initialized */
    ak_slab_root  slabs[NSLABS];    /**< slabs of different sizes */
    ak_ca_root    ca[NCAROOTS];     /**< coalescing allocators of different size ranges */
    ak_ca_segment map_root;         /**< root of list of mmap-ed segments */

    AKMALLOC_LOCK_DEFINE(MAP_LOCK); /**< lock for mmap-ed regions if locks are enabled */
};

#if !defined(AKMALLOC_COALESCING_ALLOC_RELEASE_RATE)
#  define AKMALLOC_COALESCING_ALLOC_RELEASE_RATE 24
#endif

#if !defined(AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE)
#  define AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE AKMALLOC_COALESCING_ALLOC_RELEASE_RATE
#endif

static void ak_try_reclaim_memory(ak_malloc_state* m)
{
    // for each slab, reclaim empty pages
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_root* s = ak_as_ptr(m->slabs[i]);
        ak_slab_release_pages(s, ak_as_ptr(s->empty_root), AK_U32_MAX);
        s->nempty = 0;
        s->release = 0;
    }
    // return unused segments in ca
    for (ak_sz i = 0; i < NCAROOTS; ++i) {
        ak_ca_root* ca = ak_as_ptr(m->ca[i]);
        ak_ca_return_os_mem(ak_as_ptr(ca->empty_root), AK_U32_MAX);
        ca->nempty = 0;
        ca->release = 0;
    }

    // all memory in mmap-ed regions is being used. we return pages immediately
    // when they are free'd.
}

ak_inline static void* ak_try_slab_alloc(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_ASSERT(sz % AK_COALESCE_ALIGN == 0);
    ak_sz idx = (sz >> 4) - 1;
    ak_sz* mem = (ak_sz*)ak_slab_alloc(ak_as_ptr(m->slabs[idx]));
    if (ak_likely(mem)) {
        ak_alloc_mark_slab(ak_slab_alloc_2_mem(mem)); // we overallocate
        AKMALLOC_ASSERT(ak_alloc_type_slab(ak_alloc_type_bits(ak_slab_alloc_2_mem(mem))));
        mem = ak_slab_alloc_2_mem(mem);
    }
    return mem;
}

ak_inline static void* ak_try_coalesce_alloc(ak_malloc_state* m, ak_ca_root* proot, size_t sz)
{
    ak_sz* mem = (ak_sz*)ak_ca_alloc(proot, sz);
    if (ak_likely(mem)) {
        ak_alloc_mark_coalesce(mem);
        AKMALLOC_ASSERT(ak_alloc_type_coalesce(ak_alloc_type_bits(mem)));
    }
    return mem;
}

ak_inline static void* ak_try_alloc_mmap(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(m->MAP_LOCK));
    ak_ca_segment* mem = (ak_ca_segment*)ak_os_alloc(sz);
    if (ak_likely(mem)) {
        ak_alloc_mark_mmap(mem + 1);
        AKMALLOC_ASSERT(ak_alloc_type_mmap(ak_alloc_type_bits(mem + 1)));
        mem->sz = sz;
        ak_ca_segment_link(mem, m->map_root.fd, ak_as_ptr(m->map_root));
        mem += 1;
    }
    AKMALLOC_LOCK_RELEASE(ak_as_ptr(m->MAP_LOCK));

    return mem;
}

ak_inline static ak_ca_root* ak_find_ca_root(ak_malloc_state* m, ak_sz sz)
{
    ak_sz i = 0;
    for (; i < NCAROOTS; ++i) {
        if (CA_SIZES[i] >= sz) {
            break;
        }
    }
    AKMALLOC_ASSERT(i < NCAROOTS);
    return ak_as_ptr(m->ca[i]);
}

ak_inline static void* ak_try_alloc(ak_malloc_state* m, size_t sz)
{
    void* retmem = AK_NULLPTR;
    ak_sz modsz = ak_slab_mod_sz(sz);
    if (modsz <= MIN_SMALL_REQUEST) {
        retmem = ak_try_slab_alloc(m, modsz);
        DBG_PRINTF("a,slab,%p,%llu\n", retmem, modsz);
    } else if (sz < MMAP_SIZE) {
        const ak_sz alnsz = ak_ca_aligned_size(sz);
        ak_ca_root* proot = ak_find_ca_root(m, sz);
        retmem = ak_try_coalesce_alloc(m, proot, alnsz);
        DBG_PRINTF("a,ca[%d],%p,%llu\n", (int)(proot-ak_as_ptr(m->ca[0])), retmem, alnsz);
    } else {
        sz += sizeof(ak_ca_segment);
        const ak_sz actsz = ak_ca_aligned_segment_size(sz);
        retmem = ak_try_alloc_mmap(m, actsz);
        DBG_PRINTF("a,mmap,%p,%llu\n", retmem, actsz);
    }

    return retmem;
}


static void* ak_aligned_alloc_from_state_no_checks(ak_malloc_state* m, size_t aln, size_t sz)
{
    ak_sz req = ak_ca_aligned_size(sz);
    req += aln + (2 * sizeof(ak_alloc_node)) + (2 * sizeof(ak_free_list_node));
    char* mem = AK_NULLPTR;
    // must request from coalesce alloc so we can return the extra piece
    ak_ca_root* ca = ak_find_ca_root(m, req);

    mem = (char*)ak_ca_alloc(ca, req);
    ak_alloc_node* node = ak_ptr_cast(ak_alloc_node, mem) - 1;
    if (ak_likely(mem)) {
        if ((((ak_sz)mem) & (aln - 1)) != 0) {
            // misaligned
            AK_CA_LOCK_ACQUIRE(ca);
            char* alnpos = (char*)(((ak_sz)(mem + aln - 1)) & ~(aln - 1));
            ak_alloc_node* alnnode = ak_ptr_cast(ak_alloc_node, alnpos) - 1;
            AKMALLOC_ASSERT((ak_sz)(alnpos - mem) >= (sizeof(ak_free_list_node) + sizeof(ak_alloc_node)));
            ak_sz actsz = ak_ca_to_sz(node->currinfo);
            int islast = ak_ca_is_last(node);

            ak_ca_set_sz(ak_as_ptr(node->currinfo), alnpos - mem - sizeof(ak_alloc_node));
            ak_ca_set_is_last(ak_as_ptr(node->currinfo), 0);
            ak_ca_set_is_free(ak_as_ptr(node->currinfo), 1);

            ak_ca_set_sz(ak_as_ptr(alnnode->currinfo), actsz - ak_ca_to_sz(node->currinfo));
            ak_ca_set_is_last(ak_as_ptr(alnnode->currinfo), islast);
            ak_ca_set_is_free(ak_as_ptr(alnnode->currinfo), 0);

            ak_ca_update_footer(node);
            ak_ca_update_footer(alnnode);
            mem = alnpos;
            AK_CA_LOCK_RELEASE(ca);
        }
        return mem;
    }
    return AK_NULLPTR;
}

ak_inline static size_t ak_malloc_usable_size_in_state(const void* mem);

/**************************************************************/
/* P U B L I C                                                */
/**************************************************************/

/*!
 * Initialize a private malloc like allocator.
 * \param s; Pointer to the allocator to initialize (non-NULL)
 */
static void ak_malloc_init_state(ak_malloc_state* s)
{
    AKMALLOC_ASSERT_ALWAYS(sizeof(ak_slab) % AK_COALESCE_ALIGN == 0);

    for (ak_sz i = 0; i != NSLABS; ++i) {
        ak_slab_init_root_default(ak_as_ptr(s->slabs[i]), SLAB_SIZES[i]);
    }

    for (ak_sz i = 0; i != NCAROOTS; ++i) {
        ak_ca_init_root(ak_as_ptr(s->ca[i]), AKMALLOC_COALESCING_ALLOC_RELEASE_RATE, AKMALLOC_COALESCING_ALLOC_MAX_PAGES_TO_FREE);
    }

    ak_ca_segment_link(ak_as_ptr(s->map_root), ak_as_ptr(s->map_root), ak_as_ptr(s->map_root));

    AKMALLOC_LOCK_INIT(ak_as_ptr(s->MAP_LOCK));
    s->init = 1;
}

/*!
 * Destroy the private malloc like allocator and return all memory to the OS.
 * \param m; Pointer to the allocator
 */
static void ak_malloc_destroy_state(ak_malloc_state* m)
{
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_destroy(ak_as_ptr(m->slabs[i]));
    }
    for (ak_sz i = 0; i < NCAROOTS; ++i) {
        ak_ca_destroy(ak_as_ptr(m->ca[i]));
    }
    {// mmaped chunks
        ak_ca_segment temp;
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->map_root)) {
            temp = *seg;
            ak_os_free(seg, seg->sz);
            seg = &temp;
        }
    }
}

/*!
 * Attempt to allocate memory containing at least \p n bytes.
 * \param m; The allocator
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory.
 */
static void* ak_malloc_from_state(ak_malloc_state* m, size_t sz)
{
    AKMALLOC_ASSERT(m->init);
    void* mem = ak_try_alloc(m, sz);
    if (ak_unlikely(!mem)) {
        ak_try_reclaim_memory(m);
        mem = ak_try_alloc(m, sz);
    }
    return mem;
}

/*!
 * Return memory to the allocator.
 * \param m; The allocator
 * \param mem; Pointer to the memory to return.
 */
ak_inline static void ak_free_to_state(ak_malloc_state* m, void* mem)
{
    if (ak_likely(mem)) {
#if defined(AKMALLOC_DEBUG_PRINT)
        ak_sz ussize = ak_malloc_usable_size_in_state(mem);
#endif/*defined(AKMALLOC_DEBUG_PRINT)*/
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            DBG_PRINTF("d,slab,%p,%llu\n", mem, ussize);
            ak_slab_free(ak_slab_mem_2_alloc(mem));
        } else if (ak_alloc_type_mmap(ty)) {
            DBG_PRINTF("d,mmap,%p,%llu\n", mem, ussize);
            AKMALLOC_LOCK_ACQUIRE(ak_as_ptr(m->MAP_LOCK));
            ak_ca_segment* seg = ((ak_ca_segment*)mem) - 1;
            ak_ca_segment_unlink(seg);
            ak_os_free(seg, seg->sz);
            AKMALLOC_LOCK_RELEASE(ak_as_ptr(m->MAP_LOCK));
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            const ak_sz alnsz = ak_ca_to_sz(n->currinfo);
            ak_ca_root* proot = ak_find_ca_root(m, alnsz);
            DBG_PRINTF("d,ca[%d],%p,%llu\n", (int)(proot-ak_as_ptr(m->ca[0])), mem, alnsz);
            ak_ca_free(proot, mem);
        }
    }
}

/*!
 * Attempt to grow memory at the region pointed to by \p p to a size \p newsz without relocation.
 * \param m; The allocator
 * \param mem; Memory to grow
 * \param newsz; New size to grow to
 *
 * \return \c NULL if no memory is available, or \p mem with at least \p newsz bytes.
 */
ak_inline static void* ak_realloc_in_place_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    const ak_sz usablesize = ak_malloc_usable_size_in_state(mem);
    if (usablesize >= newsz) {
        return mem;
    }
    if (ak_alloc_type_coalesce(ak_alloc_type_bits(mem))) {
        ak_alloc_node* n = ak_ptr_cast(ak_alloc_node, mem) - 1;
        AKMALLOC_ASSERT(ak_ca_is_free(n->currinfo));
        // check if there is a free next, if so, maybe merge
        ak_sz sz = ak_ca_to_sz(n->currinfo);
        ak_ca_root* proot = ak_find_ca_root(m, sz);
        if (ak_ca_realloc_in_place(proot, mem, newsz)) {
            return mem;
        }
    }
    return AK_NULLPTR;
}

/*!
 * Attempt to grow memory at the region pointed to by \p p to a size \p newsz.
 * \param m; The allocator
 * \param mem; Memory to grow
 * \param newsz; New size to grow to
 *
 * This function will copy the old bytes to a new memory location if the old memory cannot be
 * grown in place, and will free the old memory. If no more memory is available it will not
 * destroy the old memory.
 *
 * \return \c NULL if no memory is available, or a pointer to memory with at least \p newsz bytes.
 */
static void* ak_realloc_from_state(ak_malloc_state* m, void* mem, size_t newsz)
{
    if (ak_unlikely(!mem)) {
        return ak_malloc_from_state(m, newsz);
    }

    if (ak_realloc_in_place_from_state(m, mem, newsz)) {
        return mem;
    }

    void* newmem = ak_malloc_from_state(m, newsz);
    if (!newmem) {
        return AK_NULLPTR;
    }
    if (ak_likely(mem)) {
        ak_memcpy(newmem, mem, ak_malloc_usable_size_in_state(mem));
        ak_free_to_state(m, mem);
    }
    return newmem;
}


/*!
 * Return the usable size of the memory region pointed to by \p p.
 * \param mem; Pointer to the memory to determize size of.
 *
 * \return The number of bytes that can be written to in the region.
 */
ak_inline static size_t ak_malloc_usable_size_in_state(const void* mem)
{
    if (ak_likely(mem)) {
        ak_sz ty = ak_alloc_type_bits(mem);
        if (ak_alloc_type_slab(ty)) {
            // round to page
            const ak_slab* slab = (const ak_slab*)(ak_page_start_before_const(mem));
            return ak_slab_usable_size(slab->root->sz);
        } else if (ak_alloc_type_mmap(ty)) {
            return (((const ak_ca_segment*)mem) - 1)->sz - sizeof(ak_ca_segment);
        } else {
            AKMALLOC_ASSERT(ak_alloc_type_coalesce(ty));
            const ak_alloc_node* n = ((const ak_alloc_node*)mem) - 1;
            AKMALLOC_ASSERT(!ak_ca_is_free(n->currinfo));
            return ak_ca_to_sz(n->currinfo);
        }
    } else {
        return 0;
    }
}

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln. \p aln must be a power of two. \p sz must be a multiple of \p aln.
 * \param m; The allocator
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory at an aligned address.
 */
static void* ak_aligned_alloc_from_state(ak_malloc_state* m, size_t aln, size_t sz)
{
    if (aln <= AK_COALESCE_ALIGN) {
        return ak_malloc_from_state(m, sz);
    }
    if ((aln & AK_SZ_ONE) || (aln & (aln - 1))) {
        size_t a = AK_COALESCE_ALIGN << 1;
        while (a < aln)  {
            a  = (a << 1);
        }
        aln = a;
    }
    return ak_aligned_alloc_from_state_no_checks(m, aln, sz);
}

#define AK_EINVAL 22
#define AK_ENOMEM 12

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln and assign the address to \p *pmem. \p aln must be a power of two and
 * a multiple of \c sizeof(void*).
 * \param m; The allocator
 * \param pmem; The address where the memory address should be writted.
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on success, 12 if no more memory is available, and 22 if \p aln was not a power
 * of two and a multiple of \c sizeof(void*)
 */
static int ak_posix_memalign_from_state(ak_malloc_state* m, void** pmem, size_t aln, size_t sz)
{
    void* mem = AK_NULLPTR;
    if (aln == AK_COALESCE_ALIGN) {
        mem = ak_malloc_from_state(m, sz);
    } else {
        ak_sz div = (aln / sizeof(ak_sz));
        ak_sz rem = (aln & (sizeof(ak_sz)));
        if (rem != 0 || div == 0 || (div & (div - AK_SZ_ONE)) != 0) {
            return AK_EINVAL;
        }
        aln = (aln <= AK_COALESCE_ALIGN) ? AK_COALESCE_ALIGN : aln;
        mem = ak_aligned_alloc_from_state_no_checks(m, aln, sz);
    }

    if (!mem) {
        return AK_ENOMEM;
    }

    *pmem = mem;
    return 0;
}

/*!
 * Iterate over all memory segments allocated.
 * \param m; The allocator
 * \param cbk; Callback that is given the address of a segment and its size. \see ak_seg_cbk.
 */
static void ak_malloc_for_each_segment_in_state(ak_malloc_state* m, ak_seg_cbk cbk)
{
    // for each slab, reclaim empty pages
    for (ak_sz i = 0; i < NSLABS; ++i) {
        ak_slab_root* s = ak_as_ptr(m->slabs[i]);
        ak_circ_list_for_each(ak_slab, fslab, &(s->full_root)) {
            if (!cbk(fslab, AKMALLOC_DEFAULT_PAGE_SIZE)) {
                return;
            }
        }
        ak_circ_list_for_each(ak_slab, pslab, &(s->partial_root)) {
            if (!cbk(pslab, AKMALLOC_DEFAULT_PAGE_SIZE)) {
                return;
            }
        }
    }

    {// ca roots
        for (ak_sz i = 0; i < NCAROOTS; ++i) {
            ak_circ_list_for_each(ak_ca_segment, seg, &(m->ca[i].main_root)) {
                if (!cbk(seg->head, seg->sz)) {
                    return;
                }
            }
        }
    }

    {// mmaped chunks
        ak_circ_list_for_each(ak_ca_segment, seg, &(m->map_root)) {
            if (!cbk(seg, seg->sz)) {
                return;
            }
        }
    }
}
/********************** mallocstate end ************************/

/***********************************************
 * Exported APIs
 ***********************************************/

static int MALLOC_INIT = 0;

static ak_malloc_state MALLOC_ROOT;
static ak_malloc_state* GMSTATE = AK_NULLPTR;
static ak_spinlock MALLOC_INIT_LOCK = { 0 };

#define ak_ensure_malloc_state_init()                        \
{                                                            \
    if (ak_unlikely(!MALLOC_INIT)) {                         \
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
