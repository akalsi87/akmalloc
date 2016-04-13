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

#endif/*AKMALLOC_INLINE_H*/
