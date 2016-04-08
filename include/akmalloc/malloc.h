/**
 * \file malloc.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_MALLOC_H
#define AKMALLOC_MALLOC_H

#if defined(AKMALLOC_USE_PREFIX) && !AKMALLOC_USE_PREFIX
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

AKMALLOC_EXPORT void*  ak_malloc(size_t);
AKMALLOC_EXPORT void*  ak_calloc(size_t, size_t);
AKMALLOC_EXPORT void   ak_free(void*);
AKMALLOC_EXPORT void*  ak_aligned_alloc(size_t, size_t);
AKMALLOC_EXPORT int    ak_posix_memalign(void**, size_t, size_t);
AKMALLOC_EXPORT void*  ak_memalign(size_t, size_t);
AKMALLOC_EXPORT void*  ak_realloc(void*, size_t);
AKMALLOC_EXPORT size_t ak_malloc_usable_size(const void*);

#if !defined(AKMALLOC_BUILD)
#  include "akmalloc/malloc.c"
#endif

#endif/*AKMALLOC_MALLOC_H*/
