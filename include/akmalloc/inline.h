/**
 * \file inline.h
 * \date Apr 13, 2016
 */

#ifndef AKMALLOC_INLINE_H
#define AKMALLOC_INLINE_H

#include "akmalloc/config.h"

#if AKMALLOC_MSVC
#  define ak_inline __forceinline
#else
#  define ak_inline __inline__ __attribute__((always_inline))
#endif/*AKMALLOC_MSVC*/

#if AKMALLOC_MSVC
#  define ak_likely(x) x
#  define ak_unlikely(x) x
#else
#  define ak_likely(x) __builtin_expect(!!(x), 1)
#  define ak_unlikely(x) __builtin_expect(!!(x), 0)
#endif/*AKMALLOC_MSVC*/

#endif/*AKMALLOC_INLINE_H*/
