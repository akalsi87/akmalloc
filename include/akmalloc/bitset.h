/**
 * \file bitset.h
 * \date Apr 13, 2016
 */

#ifndef AKMALLOC_DETAIL_BITSET_H
#define AKMALLOC_DETAIL_BITSET_H

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

ak_inline static void ak_bitset_set(ak_bitset32* bs, int i)
{
    *bs = *bs | (0x00000001 << i);
}

ak_inline static void ak_bitset_clear(ak_bitset32* bs, int i)
{
    *bs = *bs & (~(0x00000001 << i));
}

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

ak_inline static ak_bitset32 ak_bitset_get(const ak_bitset32* bs, int i)
{
    return (*bs & (0x00000001 << i));
}

ak_inline static ak_bitset32 ak_bitset_get_n(const ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    return (*bs & (mask << i)) >> i;
}

ak_inline static int ak_bitset_num_leading_zeros(const ak_bitset32* bs)
{
#if AKMALLOC_MSVC
    DWORD ldz = 0;
    return (_BitScanReverse(&ldz, *bs)) ? (31 - ldz) : 32;
#else
    return (*bs) ? __builtin_clz(*bs) : 32;
#endif
}

ak_inline static int ak_bitset_num_trailing_zeros(const ak_bitset32* bs)
{
#if AKMALLOC_MSVC
    DWORD trz = 0;
    return (_BitScanForward(&trz, *bs)) ? trz : 32;
#else
    return (*bs) ? __builtin_ctz(*bs) : 32;
#endif
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

/* ak_bitset512 */

struct ak_bitset512
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

ak_inline static ak_bitset32* ak_bitset512_get_set_num(ak_bitset512* bs, int idx)
{
    switch (idx) {
        case 0:
            return &(bs->a15);
        case 1:
            return &(bs->a14);
        case 2:
            return &(bs->a13);
        case 3:
            return &(bs->a12);
        case 4:
            return &(bs->a11);
        case 5:
            return &(bs->a10);
        case 6:
            return &(bs->a9);
        case 7:
            return &(bs->a8);
        case 8:
            return &(bs->a7);
        case 9:
            return &(bs->a6);
        case 10:
            return &(bs->a5);
        case 11:
            return &(bs->a4);
        case 12:
            return &(bs->a3);
        case 13:
            return &(bs->a2);
        case 14:
            return &(bs->a1);
        case 15:
            return &(bs->a0);
    }
    return 0;
}

ak_inline static ak_bitset32* ak_bitset512_get_set(ak_bitset512* bs, int idx)
{
    return ak_bitset512_get_set_num(bs, idx >> 5);
}

ak_inline static const ak_bitset32* ak_bitset512_get_set_num_const(const ak_bitset512* bs, int idx)
{
    switch (idx) {
        case 0:
            return &(bs->a15);
        case 1:
            return &(bs->a14);
        case 2:
            return &(bs->a13);
        case 3:
            return &(bs->a12);
        case 4:
            return &(bs->a11);
        case 5:
            return &(bs->a10);
        case 6:
            return &(bs->a9);
        case 7:
            return &(bs->a8);
        case 8:
            return &(bs->a7);
        case 9:
            return &(bs->a6);
        case 10:
            return &(bs->a5);
        case 11:
            return &(bs->a4);
        case 12:
            return &(bs->a3);
        case 13:
            return &(bs->a2);
        case 14:
            return &(bs->a1);
        case 15:
            return &(bs->a0);
    }
    return 0;
}

ak_inline static const ak_bitset32* ak_bitset512_get_set_const(const ak_bitset512* bs, int idx)
{
    return ak_bitset512_get_set_num_const(bs, idx >> 5);
}

ak_inline static void ak_bitset512_set(ak_bitset512* bs, int i)
{
    ak_bitset_set(ak_bitset512_get_set(bs, i), (i & 31));
}

ak_inline static void ak_bitset512_clear(ak_bitset512* bs, int i)
{
    ak_bitset_clear(ak_bitset512_get_set(bs, i), (i & 31));
}

ak_inline static void ak_bitset512_set_n(ak_bitset512* bs, int i, int n)
{
    /* complicated */
}

ak_inline static void ak_bitset512_clear_n(ak_bitset512* bs, int i, int n)
{
    /* complicated */
}

ak_inline static ak_bitset32 ak_bitset512_get(const ak_bitset512* bs, int i)
{
    return ak_bitset_get(ak_bitset512_get_set_const(bs, i), (i & 31));
}

ak_inline static ak_bitset512 ak_bitset512_get_n(const ak_bitset512* bs, int i, int n)
{
    /* complicated */
    ak_bitset512 tmp;
    return tmp;
}

ak_inline static int ak_bitset512_num_leading_zeros(const ak_bitset512* bs)
{
    int nlz = 0;
    int cur = 0;

    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a0)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a1)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a2)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a3)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a4)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a5)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a6)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a7)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a8)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a9)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a10)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a11)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a12)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a13)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a14)));
    if (cur != 32) { return nlz; }
    nlz += (cur = ak_bitset_num_leading_zeros(&(bs->a15)));

    return nlz;
}

ak_inline static int ak_bitset512_num_trailing_zeros(const ak_bitset512* bs)
{
    int ntz = 0;
    int cur = 0;

    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a15)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a14)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a13)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a12)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a11)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a10)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a9)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a8)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a7)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a6)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a5)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a4)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a3)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a2)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a1)));
    if (cur != 32) { return ntz; }
    ntz += (cur = ak_bitset_num_trailing_zeros(&(bs->a0)));

    return ntz;
}

ak_inline static int ak_bitset512_num_leading_ones(const ak_bitset512* bs)
{
    ak_bitset512 copy = *bs;
    for (int i = 0; i != 16; ++i) {
        ak_bitset32* sbs = ak_bitset512_get_set_num(&copy, i);
        *sbs = ~(*sbs);
    }
    return ak_bitset512_num_leading_zeros(&copy);
}

ak_inline static int ak_bitset512_num_trailing_ones(const ak_bitset512* bs)
{
    ak_bitset512 copy = *bs;
    for (int i = 0; i != 16; ++i) {
        ak_bitset32* sbs = ak_bitset512_get_set_num(&copy, i);
        *sbs = ~(*sbs);
    }
    return ak_bitset512_num_trailing_zeros(&copy);
}


#endif/*AKMALLOC_DETAIL_BITSET_H*/
