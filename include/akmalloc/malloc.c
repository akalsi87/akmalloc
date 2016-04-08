/**
 * \file malloc.c
 * \date Apr 08, 2016
 */

#ifndef AKMALLOC_MALLOC_C
#define AKMALLOC_MALLOC_C

#include "akmalloc/assert.h"
#include "akmalloc/constants.h"
#include "akmalloc/memmap.h"

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

#if !defined(AKMALLOC_LARGE_BLOCK_SIZE)
#  define AKMALLOC_LARGE_BLOCK_SIZE AKMALLOC_DEFAULT_LARGE_BLOCK_SIZE
#endif

#if !defined(AKMALLOC_ASSERT)
#  define AKMALLOC_ASSERT AKMALLOC_DEFAULT_ASSERT
#endif

/***********************************************
 * IMPLEMENTATION
 ***********************************************/

void* ak_malloc(size_t sz)
{
    return 0;
}

void* ak_calloc(size_t elsz, size_t numel)
{
    return 0;
}

void ak_free(void* mem)
{

}

void* ak_aligned_alloc(size_t sz, size_t aln)
{
    return 0;
}

int ak_posix_memalign(void** pmem, size_t sz, size_t aln)
{
    return 0;
}

void* ak_memalign(size_t sz, size_t aln)
{
    return 0;
}

void* ak_realloc(void* mem, size_t newsz)
{
    return 0;
}

size_t ak_malloc_usable_size(const void* mem)
{
    return 0;
}

#endif/*AKMALLOC_MALLOC_C*/
