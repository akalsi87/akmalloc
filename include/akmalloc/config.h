/**
 * \file config.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_CONFIG_H
#define AKMALLOC_CONFIG_H

#define AKMALLOC_MAJOR_VER 0
#define AKMALLOC_MINOR_VER 0
#define AKMALLOC_PATCH_VER 1

/*
 * Compiler definition macros
 */
#if defined(_MSC_VER)
#  define AKMALLOC_MSVC 1
#endif

#if defined(__GNUC__) && !defined(__clang__)
#  define AKMALLOC_GCC 1
#endif

#if defined(__clang__)
#  define AKMALLOC_CLANG 1
#endif

#if !AKMALLOC_MSVC && !AKMALLOC_GCC && !AKMALLOC_CLANG
#  error "Unsupported compiler!"
#endif

/*
 * Platform definition macros
 */
#if defined(_WIN32)
#  define AKMALLOC_LINUX 0
#  define AKMALLOC_WINDOWS 1
#  define AKMALLOC_IOS 0
#  define AKMALLOC_MACOS 0
#endif

#if defined(__linux__)
#  define AKMALLOC_LINUX 1
#  define AKMALLOC_WINDOWS 0
#  define AKMALLOC_IOS 0
#  define AKMALLOC_MACOS 0
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#  if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#    define AKMALLOC_LINUX 0
#    define AKMALLOC_WINDOWS 0
#    define AKMALLOC_IOS 1
#    define AKMALLOC_MACOS 0
#  elif TARGET_OS_MAC
#    define AKMALLOC_LINUX 0
#    define AKMALLOC_WINDOWS 0
#    define AKMALLOC_IOS 0
#    define AKMALLOC_MACOS 1
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
#  define AKMALLOC_ARM 1
#else
#  define AKMALLOC_ARM 0
#endif

#endif/*AKMALLOC_CONFIG_H*/
