/**
 * \file malloc.c
 * \date Apr 08, 2016
 */

#ifndef AKMALLOC_MALLOC_C
#define AKMALLOC_MALLOC_C

#include "akmalloc/setup.h"
#include "akmalloc/slab.h"

/***********************************************
 * Coalescing allocator
 ***********************************************/



/***********************************************
 * Exported APIs
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
