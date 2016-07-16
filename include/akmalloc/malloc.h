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
#  define ak_malloc                   malloc
#  define ak_calloc                   calloc
#  define ak_free                     free
#  define ak_aligned_alloc            aligned_alloc
#  define ak_posix_memalign           posix_memalign
#  define ak_memalign                 memalign
#  define ak_realloc                  realloc
#  define ak_malloc_usable_size       malloc_usable_size
#  define ak_malloc_for_each_segment  malloc_for_each_segment
#endif

#if !defined(AKMALLOC_EXPORT)
#  if !defined(AKMALLOC_INCLUDE_ONLY)
#    include "akmalloc/exportsym.h"
#    define AKMALLOC_EXPORT AKMALLOC_API
#  else
#    define AKMALLOC_EXPORT
#  endif
#endif

/**
 * Gets a pointer to a memory segment and its size.
 * \param p; Pointer to segment memory.
 * \param sz; Number of bytes in the segment.
 * 
 * \return \c 0 to stop iteration, non-zero to continue.
 */
typedef int(*ak_seg_cbk)(const void* p, size_t sz);
#define AK_SEG_CBK_DEFINED

AK_EXTERN_C_BEGIN

/*!
 * Attempt to allocate memory containing at least \p n bytes.
 * \param n; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory.
 */
AKMALLOC_EXPORT void*  ak_malloc(size_t n);

/*!
 * Attempt to allocate zeroed memory, containing at least \p n x \p s bytes.
 * \param n; Number of objects to zero.
 * \param s; The size for each object.
 *
 * \return \c 0 on failure, else pointer to at least \p s bytes of memory.
 */
AKMALLOC_EXPORT void*  ak_calloc(size_t n, size_t s);

/*!
 * Return memory to the allocator.
 * \param p; Pointer to the memory to return.
 */
AKMALLOC_EXPORT void   ak_free(void* p);

/*!
 * Return the usable size of the memory region pointed to by \p p.
 * \param p; Pointer to the memory to determize size of.
 *
 * \return The number of bytes that can be written to in the region.
 */
AKMALLOC_EXPORT size_t ak_malloc_usable_size(const void* p);

/*!
 * Attempt to grow memory at the region pointed to by \p p to a size \p newsz.
 * \param p; Memory to grow
 * \param newsz; New size to grow to
 *
 * This function will copy the old bytes to a new memory location if the old memory cannot be
 * grown in place, and will free the old memory. If no more memory is available it will not
 * destroy the old memory.
 *
 * \return \c NULL if no memory is available, or a pointer to memory with at least \p newsz bytes.
 */
AKMALLOC_EXPORT void*  ak_realloc(void* p, size_t newsz);

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln. \p aln must be a power of two.
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory at an aligned address.
 */
AKMALLOC_EXPORT void*  ak_memalign(size_t aln, size_t sz);

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln. \p aln must be a power of two. \p sz must be a multiple of \p aln.
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on failure, else pointer to at least \p n bytes of memory at an aligned address.
 */
AKMALLOC_EXPORT void*  ak_aligned_alloc(size_t aln, size_t sz);

/*!
 * Attempt to allocate memory containing at least \p n bytes at an address which is
 * a multiple of \p aln and assign the address to \p *pptr. \p aln must be a power of two and
 * a multiple of \c sizeof(void*).
 * \param pptr; The address where the memory address should be writted.
 * \param aln; The alignment
 * \param sz; The size for the allocation
 *
 * \return \c 0 on success, 12 if no more memory is available, and 22 if \p aln was not a power
 * of two and a multiple of \c sizeof(void*)
 */
AKMALLOC_EXPORT int    ak_posix_memalign(void** pptr, size_t aln, size_t sz);

/*!
 * Iterate over all memory segments allocated.
 * \param cbk; Callback that is given the address of a segment and its size. \see ak_seg_cbk.
 */
AKMALLOC_EXPORT void   ak_malloc_for_each_segment(ak_seg_cbk cbk);

AK_EXTERN_C_END

#if defined(AKMALLOC_INCLUDE_ONLY)
#  include "akmalloc/malloc.c"
#endif

#endif/*AKMALLOC_MALLOC_H*/
