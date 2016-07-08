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
 * \file config.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_CONFIG_H
#define AKMALLOC_CONFIG_H

#include "akmalloc/rc.h"

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

#if defined(__cplusplus)
#  define AK_EXTERN_C_BEGIN extern "C"  {
#  define AK_EXTERN_C_END   }
#else
#  define AK_EXTERN_C_BEGIN
#  define AK_EXTERN_C_END  
#endif

#endif/*AKMALLOC_CONFIG_H*/
