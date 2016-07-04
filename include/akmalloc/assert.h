/**
 * \file assert.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_ASSERT_H
#define AKMALLOC_ASSERT_H

#include <stdlib.h>
#include <stdio.h>

#define AKMALLOC_DEFAULT_ASSERT(x)                                           \
  if (!(x)) {                                                                \
      fprintf(stderr, "%s (%d) : %s\n", __FILE__, __LINE__, "AKMALLOC_ASSERT: failed condition `" #x "'"); \
      abort();                                                               \
  }

#endif/*AKMALLOC_ASSERT_H*/
