/**
 * \file constants.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_CONSTANTS_H
#define AKMALLOC_CONSTANTS_H

#include "akmalloc/config.h"
#include "akmalloc/types.h"

#define AKSIZE_ONE ((ak_sz)1)
#define AKSIZE_MAX (~(ak_sz)0)

#define AKMALLOC_DEFAULT_PAGE_SIZE 4096
#define AKMALLOC_DEFAULT_LARGE_BLOCK_SIZE ((ak_sz)1) << 21

/*
 * Cache line macros
 */
#if AKMALLOC_ARM
#  define AKMALLOC_CACHE_LINE_LENGTH 32
#else
#  define AKMALLOC_CACHE_LINE_LENGTH 64
#endif

#endif/*AKMALLOC_CONSTANTS_H*/
