/**
 * \file tSlab.cpp
 * \date Jul 01, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"

#define USE_MALLOC 0

CPP_TEST( slab8 )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 8, 1);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 8);
        TEST_TRUE(ak_num_slabs_per_page(8) == 16);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    static const ak_sz nptrs = 10000;
    
    void* parr[nptrs] = { 0 };
    
    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        parr[i] = malloc(8);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
    }

    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }
}

CPP_TEST( slab16 )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 16, 1);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 16);
        TEST_TRUE(ak_num_slabs_per_page(16) == 8);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    static const ak_sz nptrs = 10000;
    
    void* parr[nptrs] = { 0 };
    
    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        parr[i] = malloc(16);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
    }

    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }
}

CPP_TEST( slab128 )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 128, 1);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 128);
        TEST_TRUE(ak_num_slabs_per_page(128) == 1);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.partial_root.fd != &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk != &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        ak_slab_free(p);
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
    }

    static const ak_sz nptrs = 10000;
    
    void* parr[nptrs] = { 0 };
    
    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        parr[i] = malloc(128);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
    }

    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }
}

CPP_TEST( slab256 )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 256, 8);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 256);
        TEST_TRUE(ak_num_slabs_per_page(256) == 1);
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

    static const ak_sz nptrs = 10000;
    
    void* parr[nptrs] = { 0 };
    
    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        parr[i] = malloc(256);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
    }

    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }
}

CPP_TEST( slab512 )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 512, 16);

    {// test root
        TEST_TRUE(root.partial_root.fd == &(root.partial_root));
        TEST_TRUE(root.partial_root.bk == &(root.partial_root));
        TEST_TRUE(root.full_root.fd == &(root.full_root));
        TEST_TRUE(root.full_root.bk == &(root.full_root));
        TEST_TRUE(root.sz == 512);
        TEST_TRUE(ak_num_slabs_per_page(512) == 1);
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

    static const ak_sz nptrs = 10000;
    
    void* parr[nptrs] = { 0 };
    
    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        parr[i] = malloc(512);
#else
        parr[i] = ak_slab_alloc(&root);
#endif
    }

    for (ak_sz i = 0; i != nptrs; ++i)
    {
#if USE_MALLOC
        free(parr[i]);
#else
        ak_slab_free(parr[i]);
#endif
    }
}
