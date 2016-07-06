/**
 * \file setup.h
 * \date Jul 04, 2016
 */

#ifndef AKMALLOC_SETUP_H
#define AKMALLOC_SETUP_H

#include "akmalloc/assert.h"
#include "akmalloc/constants.h"
#include "akmalloc/inline.h"
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

/***********************************************
 * IMPLEMENTATION
 ***********************************************/

// #define ak_offsetof(ty, field) (ak_sz)(((ty *)0)->field)
// #define ak_pagealign(sz) (ak_sz)(((sz) + AKMALLOC_DEFAULT_PAGE_SIZE - 1) & (AKMALLOC_DEFAULT_PAGE_SIZE - 1))

/***********************************************
 * OS Allocation
 ***********************************************/

static void* ak_os_alloc(size_t sz)
{
    static const ak_sz pgsz = ak_page_size();
    static_cast<void>(pgsz);
    AKMALLOC_ASSERT_ALWAYS(pgsz == AKMALLOC_DEFAULT_PAGE_SIZE);
    return AKMALLOC_MMAP(sz);
}

static void ak_os_free(void* p, size_t sz)
{
    AKMALLOC_MUNMAP(p, sz);
}

ak_inline static void* ak_page_start_before(void* p)
{
    return (void*)((ak_sz)p & (~(ak_sz)(AKMALLOC_DEFAULT_PAGE_SIZE - 1)));
}

#endif/*AKMALLOC_SETUP_H*/
