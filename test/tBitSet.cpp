/**
 * \file tBitSet.cpp
 * \date Apr 13, 2016
 */

#include "unittest.hpp"

#include "akmalloc/bitset.h"

CPP_TEST( bitSetIncludeWorks )
{
    static_cast<void>(0);
}

static const int NUM_BITS = sizeof(ak_bitset32) * 8;

CPP_TEST( bitSetClearSetGet )
{
    ak_bitset32 bs;

    ak_bitset_clear_all(&bs);
    TEST_TRUE(bs == 0);
    for (int i = 0; i != NUM_BITS; ++i) {
        TEST_FALSE(ak_bitset_get(&bs, i));
    }
    TEST_TRUE(ak_bitset_num_leading_zeros(&bs) == NUM_BITS);
    TEST_TRUE(ak_bitset_num_trailing_zeros(&bs) == NUM_BITS);

    ak_bitset_set_all(&bs);
    TEST_TRUE(bs == 0xFFFFFFFF);
    for (int i = 0; i != NUM_BITS; ++i) {
        TEST_TRUE(ak_bitset_get(&bs, i));
    }
    TEST_TRUE(ak_bitset_num_leading_zeros(&bs) == 0);
    TEST_TRUE(ak_bitset_num_trailing_zeros(&bs) == 0);

    ak_bitset_clear_all(&bs);
    for (int i = 0; i != NUM_BITS; ++i) {
        ak_bitset_set(&bs, i);
        TEST_TRUE(ak_bitset_get(&bs, i));
        TEST_TRUE(ak_bitset_num_leading_zeros(&bs) == (NUM_BITS-(i+1)));
        TEST_TRUE(ak_bitset_num_trailing_zeros(&bs) == i);
        ak_bitset_clear(&bs, i);
    }

    ak_bitset_set_n(&bs, 0, 2);
    TEST_TRUE(bs == 3);
    TEST_TRUE(ak_bitset_get_n(&bs, 0, 2) == 3);
    ak_bitset_clear_n(&bs, 0, 2);
    TEST_TRUE(bs == 0);

    ak_bitset_set_n(&bs, 12, 3);
    TEST_TRUE(bs == (7 << 12));
    TEST_TRUE(ak_bitset_get_n(&bs, 12, 3) == 7);
    ak_bitset_clear_n(&bs, 12, 3);
    TEST_TRUE(bs == 0);

    ak_bitset_set_n(&bs, 29, 3);
    TEST_TRUE(bs == (ak_u32)(7 << 29));
    TEST_TRUE(ak_bitset_get_n(&bs, 29, 3) == 7);
    ak_bitset_clear_n(&bs, 29, 3);
    TEST_TRUE(bs == 0);
}

CPP_TEST( bs512 )
{
    ak_bitset512 bs;
    ak_bitset512_clear_all(&bs);

    TEST_TRUE(ak_bitset512_num_trailing_zeros(&bs) == 512);
    TEST_TRUE(ak_bitset512_num_leading_zeros(&bs) == 512);

    ak_bitset512_set(&bs, 100);
    int tz = ak_bitset512_num_trailing_zeros(&bs);
    int lz = ak_bitset512_num_leading_zeros(&bs);
    TEST_TRUE(tz == 100);
    TEST_TRUE(lz == 411);

    ak_bitset512_set_all(&bs);

    TEST_TRUE(ak_bitset512_num_trailing_ones(&bs) == 512);
    TEST_TRUE(ak_bitset512_num_leading_ones(&bs) == 512);

    ak_bitset512_clear(&bs, 100);
    int to = ak_bitset512_num_trailing_ones(&bs);
    int lo = ak_bitset512_num_leading_ones(&bs);
    TEST_TRUE(to == 100);
    TEST_TRUE(lo == 411);
}

