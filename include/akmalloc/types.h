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
 * \file types.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_TYPES_H
#define AKMALLOC_TYPES_H

#include "akmalloc/config.h"

typedef char ak_char;
typedef signed char ak_i8;
typedef unsigned char ak_u8;

typedef short ak_i16;
typedef unsigned short ak_u16;

typedef int ak_i32;
typedef unsigned int ak_u32;

#if AKMALLOC_MSVC
typedef __int64 ak_i64;
typedef unsigned __int64 ak_u64;
#else
typedef long long int ak_i64;
typedef unsigned long long int ak_u64;
#endif

typedef float ak_f32;
typedef double ak_f64;

#if AKMALLOC_BITNESS == 32
typedef ak_u32 ak_sz;
typedef ak_i32 ak_ssz;
#else
typedef ak_u64 ak_sz;
typedef ak_i64 ak_ssz;
#endif

#endif/*AKMALLOC_TYPES_H*/
