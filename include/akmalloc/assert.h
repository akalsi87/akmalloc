/**
 * \file assert.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_ASSERT_H
#define AKMALLOC_ASSERT_H

#include <stdlib.h>
#include <stdio.h>

#define AKMALLOC_DEFAULT_ASSERT(x)                                                                         \
  if (!(x)) {                                                                                              \
      fprintf(stderr, "%s (%d) : %s\n", __FILE__, __LINE__, "AKMALLOC_ASSERT: failed condition `" #x "'"); \
      abort();                                                                                             \
  }

#if !defined(AKMALLOC_ASSERT_IMPL)
#  define AKMALLOC_ASSERT_IMPL AKMALLOC_DEFAULT_ASSERT
#endif

#if !defined(AKMALLOC_ASSERT)
#  if !defined(NDEBUG)
#    define AKMALLOC_ASSERT AKMALLOC_ASSERT_IMPL
#  else
#    define AKMALLOC_ASSERT(...) do { } while(0)
#  endif
#endif

#if !defined(AKMALLOC_ASSERT_ALWAYS)
#  define AKMALLOC_ASSERT_ALWAYS AKMALLOC_ASSERT_IMPL
#endif

#endif/*AKMALLOC_ASSERT_H*/
