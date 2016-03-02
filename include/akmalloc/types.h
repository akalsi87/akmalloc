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
