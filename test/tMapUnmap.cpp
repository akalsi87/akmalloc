/**
 * \file tMapUnmap.cpp
 * \date Mar 02, 2016
 */

#include "unittest.hpp"

#include "akmalloc/memmap.h"

CPP_TEST( pageSize )
{
    TEST_TRUE(ak_page_size() == 4096);
}

CPP_TEST( mapUnmap )
{
    void* addr = 0;
    addr = ak_mmap(ak_page_size());
    TEST_TRUE(addr);
    *(int*)addr = 42;
    TEST_TRUE(*(int*)addr == 42);
    ak_munmap(addr, ak_page_size());
}
