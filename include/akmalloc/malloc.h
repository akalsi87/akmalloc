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
 * \file malloc.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_MALLOC_H
#define AKMALLOC_MALLOC_H

/* We only include this to get size_t */
#include <stddef.h>

#include "akmalloc/rc.h"
#include "akmalloc/types.h"

#if !defined(AKMALLOC_USE_PREFIX) || !AKMALLOC_USE_PREFIX
#  define ak_malloc              malloc
#  define ak_calloc              calloc
#  define ak_free                free
#  define ak_aligned_alloc       aligned_alloc
#  define ak_posix_memalign      posix_memalign
#  define ak_memalign            memalign
#  define ak_realloc             realloc
#  define ak_malloc_usable_size  malloc_usable_size
#endif

#if !defined(AKMALLOC_EXPORT)
#  if !defined(AKMALLOC_INCLUDE_ONLY)
#    include "akmalloc/exportsym.h"
#    define AKMALLOC_EXPORT AKMALLOC_API
#  else
#    define AKMALLOC_EXPORT
#  endif
#endif

AK_EXTERN_C_BEGIN

AKMALLOC_EXPORT void*  ak_malloc(size_t);
AKMALLOC_EXPORT void*  ak_calloc(size_t, size_t);
AKMALLOC_EXPORT void   ak_free(void*);
AKMALLOC_EXPORT void*  ak_memalign(size_t, size_t);
AKMALLOC_EXPORT void*  ak_realloc(void*, size_t);
AKMALLOC_EXPORT size_t ak_malloc_usable_size(const void*);

AKMALLOC_EXPORT void*  ak_aligned_alloc(size_t, size_t);
AKMALLOC_EXPORT int    ak_posix_memalign(void**, size_t, size_t);

AK_EXTERN_C_END

#if defined(AKMALLOC_LINK_STATIC) || defined(AKMALLOC_INCLUDE_ONLY)
#  include "akmalloc/malloc.c"
#endif

#endif/*AKMALLOC_MALLOC_H*/
