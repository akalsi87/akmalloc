/**
 * \file tSlab.cpp
 * \date Jul 01, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"

CPP_TEST( slab )
{
    ak_slab_root root;
    ak_slab_init_root(&root, 8);

    {// test root
        TEST_TRUE(root.bk == (ak_slab*)&root);
        TEST_TRUE(root.fd == (ak_slab*)&root);
        TEST_TRUE(root.sz == 8);
        TEST_TRUE(ak_num_slabs_per_page(8) == 16);
        TEST_TRUE(root.recent == root.fd);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root);
        TEST_TRUE(root.recent == ak_page_start_before(p));
        TEST_TRUE(root.fd != (ak_slab*)&root);
        ak_slab_free(p);
        TEST_TRUE(root.fd == (ak_slab*)&root);
    }
}
