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
 * \file rc.h
 * \date Mar 11, 2016
 */

#ifndef AKMALLOC_RC_H
#define AKMALLOC_RC_H

#define AKMALLOC_MAJOR_VER    1
#define AKMALLOC_MINOR_VER    0
#define AKMALLOC_PATCH_VER    1

#if !defined(AKMALLOC_LINK_STATIC)
#  if 1
#    define AKMALLOC_LINK_STATIC
#  endif
#endif

#if !defined(AKMALLOC_INCLUDE_ONLY)
#  if 0
#    define AKMALLOC_INCLUDE_ONLY
#  endif
#endif

/*
 * Revision history:
 *
 *
 * (July 16, 2016)  1.0.1: Added customized coalescing allocators to malloc state.
 *                           - Performance and memory preservation are good
 *                           - More debuggable, with diagnostic tooling
 *
 * (July 11, 2016)  0.1.2: Functioning multi threaded malloc.
 *
 * (July 10, 2016)  0.1.1: Functioning single threaded malloc.
 *
 * (March 11, 2016) 0.0.1: Initial commit
 */

#endif/*AKMALLOC_RC_H*/
