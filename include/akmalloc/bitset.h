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
 * \file bitset.h
 * \date Apr 13, 2016
 */

#ifndef AKMALLOC_DETAIL_BITSET_H
#define AKMALLOC_DETAIL_BITSET_H

#include "akmalloc/assert.h"
#include "akmalloc/inline.h"
#include "akmalloc/types.h"

#if AKMALLOC_MSVC
#include <intrin.h>
#endif

typedef ak_u32 ak_bitset32;

ak_inline static int ak_bitset_all(const ak_bitset32* bs)
{
    return (*bs == 0xFFFFFFFF) ? 1 : 0;
}

ak_inline static int ak_bitset_any(const ak_bitset32* bs)
{
    return *(const int*)bs;
}

ak_inline static int ak_bitset_none(const ak_bitset32* bs)
{
    return (*bs == 0x00000000) ? 1 : 0;
}

ak_inline static void ak_bitset_set_all(ak_bitset32* bs)
{
    *bs = 0xFFFFFFFF;
}

ak_inline static void ak_bitset_clear_all(ak_bitset32* bs)
{
    *bs = 0x00000000; 
}

#define ak_bitset_set(bs, i) \
  (*(bs) = (*(bs) | (0x00000001 << (i))))

#define ak_bitset_clear(bs, i) \
  (*(bs) = (*(bs) & (~(0x00000001 << (i)))))

ak_inline static void ak_bitset_set_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs = *bs | (mask << i);
}

ak_inline static void ak_bitset_clear_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs = *bs & (~(mask << i));
}

#define ak_bitset_get(bs, i) \
  ((*(bs) & (0x00000001 << (i))))

ak_inline static ak_bitset32 ak_bitset_get_n(const ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    return (*bs & (mask << i)) >> i;
}

#if AKMALLOC_MSVC
#  define ak_bitset_fill_num_leading_zeros(bs, out)             \
     do {                                                       \
        DWORD ldz = 0;                                          \
        out = (_BitScanReverse(&ldz, *(bs))) ? (31 - ldz) : 32; \
     } while (0)
#else
#  define ak_bitset_fill_num_leading_zeros(bs, out)             \
     out = (*bs) ? __builtin_clz(*bs) : 32
#endif

ak_inline static int ak_bitset_num_leading_zeros(const ak_bitset32* bs)
{
    int nlz;
    ak_bitset_fill_num_leading_zeros(bs, nlz);
    return nlz;
}

#if AKMALLOC_MSVC
#  define ak_bitset_fill_num_trailing_zeros(bs, out)            \
     do {                                                       \
        DWORD trz = 0;                                          \
        out = (_BitScanForward(&trz, *(bs))) ? trz : 32;        \
     } while (0)
#else
#  define ak_bitset_fill_num_trailing_zeros(bs, out)            \
     out = (*(bs)) ? __builtin_ctz(*(bs)) : 32
#endif

ak_inline static int ak_bitset_num_trailing_zeros(const ak_bitset32* bs)
{
    int ntz;
    ak_bitset_fill_num_trailing_zeros(bs, ntz);
    return ntz;
}

ak_inline static int ak_bitset_num_leading_ones(const ak_bitset32* bs)
{
    ak_bitset32 copy = ~(*bs);
    return ak_bitset_num_leading_zeros(&copy);
}

ak_inline static int ak_bitset_num_trailing_ones(const ak_bitset32* bs)
{
    ak_bitset32 copy = ~(*bs);
    return ak_bitset_num_trailing_zeros(&copy);
}

ak_inline static void ak_bitset_flip(ak_bitset32* bs)
{
    *bs = ~(*bs);
}

/* ak_bitset512 */

struct ak_bitset512_tag
{
    ak_bitset32 a0;
    ak_bitset32 a1;
    ak_bitset32 a2;
    ak_bitset32 a3;
    ak_bitset32 a4;
    ak_bitset32 a5;
    ak_bitset32 a6;
    ak_bitset32 a7;
    ak_bitset32 a8;
    ak_bitset32 a9;
    ak_bitset32 a10;
    ak_bitset32 a11;
    ak_bitset32 a12;
    ak_bitset32 a13;
    ak_bitset32 a14;
    ak_bitset32 a15;
};

typedef struct ak_bitset512_tag ak_bitset512;

ak_inline static int ak_bitset512_all(const ak_bitset512* bs)
{
    return ak_bitset_all(&(bs->a0))  &&
           ak_bitset_all(&(bs->a1))  &&
           ak_bitset_all(&(bs->a2))  &&
           ak_bitset_all(&(bs->a3))  &&
           ak_bitset_all(&(bs->a4))  &&
           ak_bitset_all(&(bs->a5))  &&
           ak_bitset_all(&(bs->a6))  &&
           ak_bitset_all(&(bs->a7))  &&
           ak_bitset_all(&(bs->a8))  &&
           ak_bitset_all(&(bs->a9))  &&
           ak_bitset_all(&(bs->a10)) &&
           ak_bitset_all(&(bs->a11)) &&
           ak_bitset_all(&(bs->a12)) &&
           ak_bitset_all(&(bs->a13)) &&
           ak_bitset_all(&(bs->a14)) &&
           ak_bitset_all(&(bs->a15));
}

ak_inline static int ak_bitset512_any(const ak_bitset512* bs)
{
    return ak_bitset_any(&(bs->a0))  ||
           ak_bitset_any(&(bs->a1))  ||
           ak_bitset_any(&(bs->a2))  ||
           ak_bitset_any(&(bs->a3))  ||
           ak_bitset_any(&(bs->a4))  ||
           ak_bitset_any(&(bs->a5))  ||
           ak_bitset_any(&(bs->a6))  ||
           ak_bitset_any(&(bs->a7))  ||
           ak_bitset_any(&(bs->a8))  ||
           ak_bitset_any(&(bs->a9))  ||
           ak_bitset_any(&(bs->a10)) ||
           ak_bitset_any(&(bs->a11)) ||
           ak_bitset_any(&(bs->a12)) ||
           ak_bitset_any(&(bs->a13)) ||
           ak_bitset_any(&(bs->a14)) ||
           ak_bitset_any(&(bs->a15));
}

ak_inline static int ak_bitset512_none(const ak_bitset512* bs)
{
    return ak_bitset_none(&(bs->a0))  &&
           ak_bitset_none(&(bs->a1))  &&
           ak_bitset_none(&(bs->a2))  &&
           ak_bitset_none(&(bs->a3))  &&
           ak_bitset_none(&(bs->a4))  &&
           ak_bitset_none(&(bs->a5))  &&
           ak_bitset_none(&(bs->a6))  &&
           ak_bitset_none(&(bs->a7))  &&
           ak_bitset_none(&(bs->a8))  &&
           ak_bitset_none(&(bs->a9))  &&
           ak_bitset_none(&(bs->a10)) &&
           ak_bitset_none(&(bs->a11)) &&
           ak_bitset_none(&(bs->a12)) &&
           ak_bitset_none(&(bs->a13)) &&
           ak_bitset_none(&(bs->a14)) &&
           ak_bitset_none(&(bs->a15));
}

ak_inline static void ak_bitset512_set_all(ak_bitset512* bs)
{
    ak_bitset_set_all(&(bs->a0));
    ak_bitset_set_all(&(bs->a1));
    ak_bitset_set_all(&(bs->a2));
    ak_bitset_set_all(&(bs->a3));
    ak_bitset_set_all(&(bs->a4));
    ak_bitset_set_all(&(bs->a5));
    ak_bitset_set_all(&(bs->a6));
    ak_bitset_set_all(&(bs->a7));
    ak_bitset_set_all(&(bs->a8));
    ak_bitset_set_all(&(bs->a9));
    ak_bitset_set_all(&(bs->a10));
    ak_bitset_set_all(&(bs->a11));
    ak_bitset_set_all(&(bs->a12));
    ak_bitset_set_all(&(bs->a13));
    ak_bitset_set_all(&(bs->a14));
    ak_bitset_set_all(&(bs->a15));
}

ak_inline static void ak_bitset512_clear_all(ak_bitset512* bs)
{
    ak_bitset_clear_all(&(bs->a0));
    ak_bitset_clear_all(&(bs->a1));
    ak_bitset_clear_all(&(bs->a2));
    ak_bitset_clear_all(&(bs->a3));
    ak_bitset_clear_all(&(bs->a4));
    ak_bitset_clear_all(&(bs->a5));
    ak_bitset_clear_all(&(bs->a6));
    ak_bitset_clear_all(&(bs->a7));
    ak_bitset_clear_all(&(bs->a8));
    ak_bitset_clear_all(&(bs->a9));
    ak_bitset_clear_all(&(bs->a10));
    ak_bitset_clear_all(&(bs->a11));
    ak_bitset_clear_all(&(bs->a12));
    ak_bitset_clear_all(&(bs->a13));
    ak_bitset_clear_all(&(bs->a14));
    ak_bitset_clear_all(&(bs->a15));
}

ak_inline static void ak_bitset512_set(ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            ak_bitset_set(&(bs->a15), idx & 31);
            return;
        case 1:
            ak_bitset_set(&(bs->a14), idx & 31);
            return;
        case 2:
            ak_bitset_set(&(bs->a13), idx & 31);
            return;
        case 3:
            ak_bitset_set(&(bs->a12), idx & 31);
            return;
        case 4:
            ak_bitset_set(&(bs->a11), idx & 31);
            return;
        case 5:
            ak_bitset_set(&(bs->a10), idx & 31);
            return;
        case 6:
            ak_bitset_set(&(bs->a9), idx & 31);
            return;
        case 7:
            ak_bitset_set(&(bs->a8), idx & 31);
            return;
        case 8:
            ak_bitset_set(&(bs->a7), idx & 31);
            return;
        case 9:
            ak_bitset_set(&(bs->a6), idx & 31);
            return;
        case 10:
            ak_bitset_set(&(bs->a5), idx & 31);
            return;
        case 11:
            ak_bitset_set(&(bs->a4), idx & 31);
            return;
        case 12:
            ak_bitset_set(&(bs->a3), idx & 31);
            return;
        case 13:
            ak_bitset_set(&(bs->a2), idx & 31);
            return;
        case 14:
            ak_bitset_set(&(bs->a1), idx & 31);
            return;
        case 15:
            ak_bitset_set(&(bs->a0), idx & 31);
            return;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
    }
    return;
}

ak_inline static void ak_bitset512_clear(ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            ak_bitset_clear(&(bs->a15), idx & 31);
            return;
        case 1:
            ak_bitset_clear(&(bs->a14), idx & 31);
            return;
        case 2:
            ak_bitset_clear(&(bs->a13), idx & 31);
            return;
        case 3:
            ak_bitset_clear(&(bs->a12), idx & 31);
            return;
        case 4:
            ak_bitset_clear(&(bs->a11), idx & 31);
            return;
        case 5:
            ak_bitset_clear(&(bs->a10), idx & 31);
            return;
        case 6:
            ak_bitset_clear(&(bs->a9), idx & 31);
            return;
        case 7:
            ak_bitset_clear(&(bs->a8), idx & 31);
            return;
        case 8:
            ak_bitset_clear(&(bs->a7), idx & 31);
            return;
        case 9:
            ak_bitset_clear(&(bs->a6), idx & 31);
            return;
        case 10:
            ak_bitset_clear(&(bs->a5), idx & 31);
            return;
        case 11:
            ak_bitset_clear(&(bs->a4), idx & 31);
            return;
        case 12:
            ak_bitset_clear(&(bs->a3), idx & 31);
            return;
        case 13:
            ak_bitset_clear(&(bs->a2), idx & 31);
            return;
        case 14:
            ak_bitset_clear(&(bs->a1), idx & 31);
            return;
        case 15:
            ak_bitset_clear(&(bs->a0), idx & 31);
            return;
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
    }
    return;
}

ak_inline static ak_bitset32 ak_bitset512_get(const ak_bitset512* bs, int idx)
{
    switch (idx >> 5) {
        case 0:
            return ak_bitset_get(&(bs->a15), idx & 31);
        case 1:
            return ak_bitset_get(&(bs->a14), idx & 31);
        case 2:
            return ak_bitset_get(&(bs->a13), idx & 31);
        case 3:
            return ak_bitset_get(&(bs->a12), idx & 31);
        case 4:
            return ak_bitset_get(&(bs->a11), idx & 31);
        case 5:
            return ak_bitset_get(&(bs->a10), idx & 31);
        case 6:
            return ak_bitset_get(&(bs->a9), idx & 31);
        case 7:
            return ak_bitset_get(&(bs->a8), idx & 31);
        case 8:
            return ak_bitset_get(&(bs->a7), idx & 31);
        case 9:
            return ak_bitset_get(&(bs->a6), idx & 31);
        case 10:
            return ak_bitset_get(&(bs->a5), idx & 31);
        case 11:
            return ak_bitset_get(&(bs->a4), idx & 31);
        case 12:
            return ak_bitset_get(&(bs->a3), idx & 31);
        case 13:
            return ak_bitset_get(&(bs->a2), idx & 31);
        case 14:
            return ak_bitset_get(&(bs->a1), idx & 31);
        case 15:
            return ak_bitset_get(&(bs->a0), idx & 31);
        default:
            AKMALLOC_ASSERT_ALWAYS(0 && "Invalid bitset index");
            return 0;
    }
}

#define ak_bitset512_fill_num_leading_zeros(bs, nlz)            \
    nlz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset_fill_num_leading_zeros(&(bs->a0), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a1), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a2), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a3), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a4), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a5), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a6), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a7), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a8), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a9), cur);       \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a10), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a11), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a12), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a13), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a14), cur);      \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_leading_zeros(&(bs->a15), cur);      \
        nlz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

#define ak_bitset512_fill_num_leading_ones(bs, nlz)             \
    nlz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset32 mybs;                                       \
        mybs = ~(bs->a0);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a1);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a2);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a3);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a4);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a5);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a6);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a7);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a8);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a9);                                       \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a10);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a11);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a12);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a13);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a14);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a15);                                      \
        ak_bitset_fill_num_leading_zeros(&mybs, cur);           \
        nlz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

ak_inline static int ak_bitset512_num_leading_zeros(const ak_bitset512* bs)
{
    int nlz = 0;
    ak_bitset512_fill_num_leading_zeros(bs, nlz);
    return nlz;
}

#define ak_bitset512_fill_num_trailing_zeros(bs, ntz)           \
    ntz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset_fill_num_trailing_zeros(&(bs->a15), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a14), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a13), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a12), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a11), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a10), cur);     \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a9), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a8), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a7), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a6), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a5), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a4), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a3), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a2), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a1), cur);      \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        ak_bitset_fill_num_trailing_zeros(&(bs->a0), cur);      \
        ntz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }

#define ak_bitset512_fill_num_trailing_ones(bs, ntz)            \
    ntz = 0;                                                    \
    {                                                           \
        int cur = 0;                                            \
        ak_bitset32 mybs;                                       \
        mybs = ~(bs->a15);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a14);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a13);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a12);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a11);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a10);                                      \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a9);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a8);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a7);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a6);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a5);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a4);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a3);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a2);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a1);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        if (cur == 32) {                                        \
        mybs = ~(bs->a0);                                       \
        ak_bitset_fill_num_trailing_zeros(&mybs, cur);          \
        ntz += cur;                                             \
        } } } } } } } } } } } } } } }                           \
    }


ak_inline static int ak_bitset512_num_trailing_zeros(const ak_bitset512* bs)
{
    int ntz = 0;
    ak_bitset512_fill_num_trailing_zeros(bs, ntz);
    return ntz;
}

ak_inline static void ak_bitset512_flip(ak_bitset512* bs)
{
    ak_bitset_flip(&(bs->a0));
    ak_bitset_flip(&(bs->a1));
    ak_bitset_flip(&(bs->a2));
    ak_bitset_flip(&(bs->a3));
    ak_bitset_flip(&(bs->a4));
    ak_bitset_flip(&(bs->a5));
    ak_bitset_flip(&(bs->a6));
    ak_bitset_flip(&(bs->a7));
    ak_bitset_flip(&(bs->a8));
    ak_bitset_flip(&(bs->a9));
    ak_bitset_flip(&(bs->a10));
    ak_bitset_flip(&(bs->a11));
    ak_bitset_flip(&(bs->a12));
    ak_bitset_flip(&(bs->a13));
    ak_bitset_flip(&(bs->a14));
    ak_bitset_flip(&(bs->a15));
}

ak_inline static int ak_bitset512_num_leading_ones(const ak_bitset512* bs)
{
    int nlo;
    ak_bitset512_fill_num_leading_ones(bs, nlo);
    return nlo;
}

ak_inline static int ak_bitset512_num_trailing_ones(const ak_bitset512* bs)
{
    int nto;
    ak_bitset512_fill_num_trailing_ones(bs, nto);
    return nto;
}

#endif/*AKMALLOC_DETAIL_BITSET_H*/
