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
