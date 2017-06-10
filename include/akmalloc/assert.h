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
 * \file assert.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_ASSERT_H
#define AKMALLOC_ASSERT_H


#if !defined(AKMALLOC_ASSERT_IMPL)
#  include <stdlib.h>
#  include <stdio.h>
#  define AKMALLOC_ASSERT_IMPL(x)                                                                 \
    if (!(x)) {                                                                                   \
      fprintf(stderr, "%s (%d) : %s\n", __FILE__, __LINE__, "ASSERT: failed condition `" #x "'"); \
      ak_call_abort();                                                                            \
    }

    static void ak_call_abort()
    {
        abort();
    }
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

#if !defined(AKMALLOC_DEBUG_PRINT)
// #  define AKMALLOC_DEBUG_PRINT
#endif

#if defined(AKMALLOC_DEBUG_PRINT)
#  define DBG_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#  define DBG_PRINTF(...) (void)0
#endif/*defined(AKMALLOC_DEBUG_PRINT)*/

#endif/*AKMALLOC_ASSERT_H*/
