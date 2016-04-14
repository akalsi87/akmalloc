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
    *bs |= (0x00000001 << i);
}

ak_inline static void ak_bitset_clear(ak_bitset32* bs, int i)
{
    *bs &= ~(0x00000001 << i);
}

ak_inline static void ak_bitset_set_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs |= (mask << i);
}

ak_inline static void ak_bitset_clear_n(ak_bitset32* bs, int i, int n)
{
    ak_bitset32 mask = ~(0xFFFFFFFF << n);
    *bs &= ~(mask << i);
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

#endif/*AKMALLOC_DETAIL_BITSET_H*/
