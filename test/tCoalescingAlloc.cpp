/**
 * \file tCoalescingAlloc.cpp
 * \date Jul 07, 2016
 */

#include "unittest.hpp"

#include <stdlib.h>
#include <string.h>

#include "akmalloc/coalescingalloc.h"

#if !defined(USE_MALLOC)
#  define USE_MALLOC 0
#endif

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
    ASSERT_TRUE(ak_ca_to_sz(i) == 0);
    ASSERT_TRUE(ak_ca_is_first(i) == 0);
    ASSERT_TRUE(ak_ca_is_last(i) == 0);
    ASSERT_TRUE(ak_ca_is_free(i) == 0);

    i = (ak_alloc_info)AK_SZ_MAX;
    ASSERT_TRUE(ak_ca_to_sz(i) == (AK_SZ_MAX << 4));
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

    {// test circ list for each
        ak_u32 ct = 0;
        ak_circ_list_for_each(ak_free_list_node, flnode, &(r.free_root)) {
            (void)flnode;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, mseg, &(r.main_root)) {
            (void)mseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, eseg, &(r.empty_root)) {
            (void)eseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);
    }
#endif
}

void shuffle(ak_sz* array, ak_sz n)
{
    if (n > 1) {
        ak_sz i;
        for (i = 0; i < n - 1; i++) {
            ak_sz j = i + rand() / (RAND_MAX / (n - i) + 1);
            ak_sz t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

CPP_TEST( reallocInPlace )
{
    ak_ca_root r;
    ak_ca_init_root(&r, 1, 1);
    void* ptr = ak_ca_alloc(&r, 100);
    TEST_TRUE(ptr);
    memset(ptr, 42, 100);
    void* new_ptr = ak_ca_realloc_in_place(&r, ptr, 140);
    TEST_TRUE(new_ptr == ptr);
    memset(ptr, 42, 140);
    void* newer_ptr = ak_ca_realloc_in_place(&r, new_ptr, (ak_sz)10000000000);
    TEST_TRUE(newer_ptr == 0);
    ak_ca_free(&r, ptr);
    ak_ca_destroy(&r);
}

CPP_TEST( allocRandomFree )
{
    static const ak_sz nptrs = 10000;
    static const ak_sz sizemin = 8;
    static const ak_sz sizemax = 16384;

    void* p[nptrs];
    ak_sz order[nptrs];

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    ak_ca_root r;
    ak_ca_init_root(&r, 100, 1);

    void* za = ak_ca_alloc(&r, 0);
    ASSERT_TRUE(za != AK_NULLPTR);
    ak_ca_free(&r, za);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_ca_alloc(&r, s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_ca_free(&r, p[order[i]]);
#endif
    }
 
 #if TEST_DESTRUCTION
    ak_ca_destroy(&r);
    ASSERT_TRUE(r.nempty == 0);
    ASSERT_TRUE(r.release == 0);

    {// test circ list for each
        ak_u32 ct = 0;
        ak_circ_list_for_each(ak_free_list_node, flnode, &(r.free_root)) {
            (void)flnode;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, mseg, &(r.main_root)) {
            (void)mseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, eseg, &(r.empty_root)) {
            (void)eseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);
    }
#endif
}

CPP_TEST( allocRandomFreeDefault )
{
    static const ak_sz nptrs = 10000;
    static const ak_sz sizemin = 8;
    static const ak_sz sizemax = 16384;

    void* p[nptrs];
    ak_sz order[nptrs];

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    ak_ca_root r;
    ak_ca_init_root_default(&r);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_ca_alloc(&r, s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_ca_free(&r, p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_ca_alloc(&r, s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_ca_free(&r, p[order[i]]);
#endif
    }
 
 #if TEST_DESTRUCTION
    ak_ca_destroy(&r);
    ASSERT_TRUE(r.nempty == 0);
    ASSERT_TRUE(r.release == 0);

    {// test circ list for each
        ak_u32 ct = 0;
        ak_circ_list_for_each(ak_free_list_node, flnode, &(r.free_root)) {
            (void)flnode;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, mseg, &(r.main_root)) {
            (void)mseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);

        ak_circ_list_for_each(ak_ca_segment, eseg, &(r.empty_root)) {
            (void)eseg;
            ++ct;
        }
        ASSERT_TRUE(ct == 0);
    }
#endif
}
