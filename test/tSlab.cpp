/**
 * \file tSlab.cpp
 * \date Jul 01, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"

CPP_TEST( slab )
{
    ak_slab root;
    ak_slab_init_root(&root, 8);

    {// test root
        TEST_TRUE(root.bk == &root);
        TEST_TRUE(root.fd == &root);
        TEST_TRUE(root.sz == 8);
        TEST_TRUE(ak_num_slabs_per_page(8) == 16);
    }

    {// single alloc and free
        void* p = ak_slab_alloc(&root, 8);
        ak_slab_free(p);
    }
}
