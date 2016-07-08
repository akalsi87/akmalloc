/**
 * \file tMallocPerf.cpp
 * \date Jul 08, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"

#include <stdlib.h>
#include <string.h>

#define USE_MALLOC 1

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

CPP_TEST( allocRandomFreeSlab )
{
    static const ak_sz nptrs = 10000;
    static const ak_sz sizemin = 8;
    static const ak_sz sizemax = 256;

    void* p[nptrs];
    ak_sz order[nptrs];

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[order[i]] = malloc(s);
#else
        p[order[i]] = ak_malloc(s);
#endif
        memset(p[order[i]], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }
}

CPP_TEST( allocRandomFreeCoalesce )
{
    static const ak_sz nptrs = 1000;
    static const ak_sz sizemin = 257;
    static const ak_sz sizemax = (AK_SZ_ONE << 20) - 1;

    void* p[nptrs];
    ak_sz order[nptrs];

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[order[i]] = malloc(s);
#else
        p[order[i]] = ak_malloc(s);
#endif
        memset(p[order[i]], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }
}

CPP_TEST( allocRandomFreeMap )
{
#if AKMALLOC_BITNESS == 64
    static const ak_sz nptrs = 1000;
#else
    static const ak_sz nptrs =  600;
#endif
    static const ak_sz sizemin = (AK_SZ_ONE << 20);
    static const ak_sz sizemax = (AK_SZ_ONE << 22);

    void* p[nptrs];
    ak_sz order[nptrs];

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[order[i]] = malloc(s);
#else
        p[order[i]] = ak_malloc(s);
#endif
        memset(p[order[i]], 42, s);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }
}