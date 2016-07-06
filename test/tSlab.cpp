/**
 * \file tSlab.cpp
 * \date Jul 01, 2016
 */

#include "unittest.hpp"

#include "akmalloc/slab.h"

#include <string.h>

#define USE_MALLOC 0

#if !defined(NDEBUG)
#  define TEST_DESTRUCTION 1
#else
#  define TEST_DESTRUCTION 0
#endif

static const ak_sz nptrs = 100000;

CPP_TEST( slab8 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 8);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 8);
        TEST_TRUE(root.npages == 2);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(8);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 8);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab16 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 16);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 16);
        TEST_TRUE(root.npages == 4);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(16);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 16);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab28 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 28);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 28);
        TEST_TRUE(root.npages == 7);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(28);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 28);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab32 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 32);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 32);
        TEST_TRUE(root.npages == 8);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(32);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 32);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab128 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 128);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 128);
        TEST_TRUE(root.npages == 32);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(128);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 128);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab256 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 256);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 256);
        TEST_TRUE(root.npages == 64);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(256);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 256);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab512 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 512);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 512);
        TEST_TRUE(root.npages == 128);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(512);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 512);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slab1024 )
{
    ak_slab_root root;
    ak_slab_init_root_default(&root, 1024);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 1024);
        TEST_TRUE(root.npages == 256);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    void* parr[nptrs] = { 0 };

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        parr[i] = malloc(1024);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
        memset(parr[i], 42, 1024);
    }

    for (ak_sz i = 0; i != nptrs; ++i) {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slabDynamics )
{
    static const ak_sz slabsz = 128;

    ak_slab_root root;
    ak_slab_init_root(&root, slabsz, 1, 2, 1);

    static const ak_sz nmain = ((AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/slabsz);

    ASSERT_TRUE(root.empty_root.fd == &(root.empty_root));

    ak_slab* pempty = AK_NULLPTR;
    void* parr[nmain] = { 0 };

    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd == &(root.full_root));

    parr[0] = ak_slab_alloc(&root);

    ASSERT_TRUE(root.partial_root.fd != &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd == &(root.full_root));

    for (ak_sz i = 1; i != nmain - 1; ++i) {
        parr[i] = ak_slab_alloc(&root);
        memset(parr[i], 42, slabsz);
    }

    parr[nmain - 1] = ak_slab_alloc(&root);

    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd != &(root.full_root));

    ak_slab_free(parr[0]);

    ASSERT_TRUE(root.partial_root.fd != &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd == &(root.full_root));

    for (ak_sz i = 1; i != nmain - 1; ++i) {
        ak_slab_free(parr[i]);
    }

    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty == &(root.empty_root));

    ak_slab_free(parr[nmain - 1]);

    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty != &(root.empty_root));
    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));

    void* ptemp = ak_slab_alloc(&root);
    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty == &(root.empty_root));
    ASSERT_TRUE(root.partial_root.fd != &(root.partial_root));

    root.RELEASE_RATE = 1;
    ak_slab_free(ptemp);
    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty == &(root.empty_root));
    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}

CPP_TEST( slabDynamicsMultipleAllocReuse )
{
    static const ak_sz slabsz = 128;

    ak_slab_root root;
    ak_slab_init_root(&root, slabsz, 2, 3, 4);

    static const ak_sz nmain = 2 * ((AKMALLOC_DEFAULT_PAGE_SIZE - sizeof(ak_slab))/slabsz);

    ASSERT_TRUE(root.empty_root.fd == &(root.empty_root));

    ak_slab* pempty = AK_NULLPTR;
    void* parr[nmain] = { 0 };

    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd == &(root.full_root));

    parr[0] = ak_slab_alloc(&root);

    ASSERT_TRUE(root.partial_root.fd != &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd == &(root.full_root));

    for (ak_sz i = 1; i != nmain - 1; ++i) {
        parr[i] = ak_slab_alloc(&root);
        if (i < (nmain/2) - 1) {
            ASSERT_TRUE(root.full_root.fd == &(root.full_root));
        } else {
            ASSERT_TRUE(root.full_root.fd != &(root.full_root));
        }
        memset(parr[i], 42, slabsz);
    }

    parr[nmain - 1] = ak_slab_alloc(&root);

    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd != &(root.full_root));

    ak_slab_free(parr[0]);

    ASSERT_TRUE(root.partial_root.fd != &(root.partial_root));
    ASSERT_TRUE(root.full_root.fd != &(root.full_root));

    for (ak_sz i = 1; i != nmain - 1; ++i) {
        ak_slab_free(parr[i]);
    }

    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty->fd == &(root.empty_root));

    ak_slab_free(parr[nmain - 1]);

    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty != &(root.empty_root));
    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));

    void* ptemp = ak_slab_alloc(&root);
    pempty = root.empty_root.fd;

    root.RELEASE_RATE = 1;
    ak_slab_free(ptemp);
    pempty = root.empty_root.fd;
    ASSERT_TRUE(pempty == &(root.empty_root));
    ASSERT_TRUE(root.partial_root.fd == &(root.partial_root));

#if TEST_DESTRUCTION
    ak_slab_destroy(&root);

    TEST_TRUE(root.nempty == 0);
    TEST_TRUE(root.release == 0);
#endif
}
