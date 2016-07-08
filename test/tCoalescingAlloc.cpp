/**
 * \file tCoalescingAlloc.cpp
 * \date Jul 07, 2016
 */

#include "unittest.hpp"

#include "akmalloc/coalescingalloc.h"

#if !defined(NDEBUG)
#  define TEST_DESTRUCTION 1
#else
#  define TEST_DESTRUCTION 0
#endif

CPP_TEST( config )
{
    ASSERT_TRUE(AK_COALESCE_ALIGN >= 16);
    ASSERT_TRUE(AK_COALESCE_ALIGN % sizeof(size_t) == 0);
    ASSERT_TRUE(sizeof(ak_alloc_info) == sizeof(size_t));
}

CPP_TEST( allocInfo )
{
    ak_alloc_info i = AK_NULLPTR;
    ASSERT_TRUE(ak_ca_to_ptr(i) == AK_NULLPTR);
    ASSERT_TRUE(ak_ca_is_first(i) == 0);
    ASSERT_TRUE(ak_ca_is_last(i) == 0);
    ASSERT_TRUE(ak_ca_is_free(i) == 0);

    i = (ak_alloc_info)AK_SZ_MAX;
    ASSERT_TRUE(ak_ca_to_ptr(i) == (ak_alloc_info)(AK_SZ_MAX << 4));
    ASSERT_TRUE(ak_ca_is_first(i) == 1);
    ASSERT_TRUE(ak_ca_is_last(i) == 2);
    ASSERT_TRUE(ak_ca_is_free(i) == 4);    
}

CPP_TEST( caRootInit )
{
    ak_ca_root r;
    ak_ca_init_root_default(&r);

    ASSERT_TRUE(r.main_root.fd == &(r.main_root));
    ASSERT_TRUE(r.main_root.bk == &(r.main_root));

    ASSERT_TRUE(r.empty_root.fd == &(r.empty_root));
    ASSERT_TRUE(r.empty_root.bk == &(r.empty_root));

    ASSERT_TRUE(r.free_root.fd == &(r.free_root));
    ASSERT_TRUE(r.free_root.bk == &(r.free_root));

    ASSERT_TRUE(r.nempty == 0);
    ASSERT_TRUE(r.release == 0);

#if TEST_DESTRUCTION
    ak_ca_destroy(&r);
    ASSERT_TRUE(r.nempty == 0);
    ASSERT_TRUE(r.release == 0);
#endif
}
