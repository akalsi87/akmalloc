/**
 * \file malloc.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_MALLOC_H
#define AKMALLOC_MALLOC_H

#include "akmalloc/constants.h"
#include "akmalloc/memmap.h"

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

#if !defined(AKMALLOC_EXPORT)
#  define AKMALLOC_EXPORT extern
#endif

#if !defined(AKMALLOC_GETPAGESIZE)
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

#if !defined(AKMALLOC_LARGE_BLOCK_SIZE)
#  define AKMALLOC_LARGE_BLOCK_SIZE AKMALLOC_DEFAULT_LARGE_BLOCK_SIZE
#endif

AKMALLOC_EXPORT void*  ak_malloc(size_t);
AKMALLOC_EXPORT void*  ak_calloc(size_t, size_t);
AKMALLOC_EXPORT void   ak_free(void*);
AKMALLOC_EXPORT void*  ak_aligned_alloc(size_t, size_t);
AKMALLOC_EXPORT int    ak_posix_memalign(void**, size_t, size_t);
AKMALLOC_EXPORT void*  ak_memalign(size_t, size_t);
AKMALLOC_EXPORT void*  ak_realloc(void*, size_t);
AKMALLOC_EXPORT size_t ak_malloc_usable_size(const void*);

/***********************************************
 * IMPLEMENTATION
 ***********************************************/

#endif/*AKMALLOC_MALLOC_H*/
