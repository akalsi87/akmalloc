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
