/**
 * \file assert.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_ASSERT_H
#define AKMALLOC_ASSERT_H

#include <stdlib.h>
#include <stdio.h>

#define AKMALLOC_DEFAULT_ASSERT(x)                                       \
  if (!(x)) {                                                            \
      fprintf(stderr, "AKMALLOC_ASSERT: failed condition `" #x "'\n");   \
      abort();                                                           \
  }

#endif/*AKMALLOC_ASSERT_H*/
