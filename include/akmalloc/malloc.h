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

#if !defined(AKMALLOC_BUILD)
#  if !defined(AKMALLOC_EXPORT)
#    define AKMALLOC_EXPORT extern
#  endif
#else
#  include "akmalloc/exportsym.h"
#  if !defined(AKMALLOC_EXPORT)
#    define AKMALLOC_EXPORT AKMALLOC_API
#  endif
#endif

AKMALLOC_EXPORT void*  ak_malloc(ak_sz);
AKMALLOC_EXPORT void*  ak_calloc(ak_sz, ak_sz);
AKMALLOC_EXPORT void   ak_free(void*);
AKMALLOC_EXPORT void*  ak_aligned_alloc(ak_sz, ak_sz);
AKMALLOC_EXPORT int    ak_posix_memalign(void**, ak_sz, ak_sz);
AKMALLOC_EXPORT void*  ak_memalign(ak_sz, ak_sz);
AKMALLOC_EXPORT void*  ak_realloc(void*, ak_sz);
AKMALLOC_EXPORT ak_sz ak_malloc_usable_size(const void*);

#if !defined(AKMALLOC_BUILD)
#  include "akmalloc/malloc.c"
#endif

#endif/*AKMALLOC_MALLOC_H*/
