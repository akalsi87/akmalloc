/**
 * \file tMallocPerf.cpp
 * \date Jul 08, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"
#include "akmalloc/constants.h"

#include <stdlib.h>
#include <string.h>

#define USE_MALLOC 0

template <class T>
void shuffle(T* array, ak_sz n)
{
    if (n > 1) {
        ak_sz i;
        for (i = 0; i < n - 1; i++) {
            ak_sz j = i + rand() / (RAND_MAX / (n - i) + 1);
            T t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// CPP_TEST( allocRandomFreeSlab )
// {
//     static const ak_sz nptrs = 10000;
//     static const ak_sz sizemin = 8;
//     static const ak_sz sizemax = 256;

//     void* p[nptrs];
//     ak_sz order[nptrs];

//     ASSERT_TRUE(RAND_MAX >= nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         order[i] = i;
//     }
//     shuffle(order, nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[i] = malloc(s);
// #else
//         p[i] = ak_malloc(s);
// #endif
//         memset(p[i], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
// #if USE_MALLOC
//         free(p[order[i]]);
// #else
//         ak_free(p[order[i]]);
// #endif
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[order[i]] = malloc(s);
// #else
//         p[order[i]] = ak_malloc(s);
// #endif
//         memset(p[order[i]], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs; ++i) {
// #if USE_MALLOC
//         free(p[i]);
// #else
//         ak_free(p[i]);
// #endif
//     }
// }

// CPP_TEST( allocRandomFreeCoalesceSmall )
// {
//     static const ak_sz nptrs = 1000;
//     static const ak_sz sizemin = 257;
//     static const ak_sz sizemax = 16383;

//     void* p[nptrs];
//     ak_sz order[nptrs];

//     ASSERT_TRUE(RAND_MAX >= nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         order[i] = i;
//     }
//     shuffle(order, nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[i] = malloc(s);
// #else
//         p[i] = ak_malloc(s);
// #endif
//         memset(p[i], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
// #if USE_MALLOC
//         free(p[order[i]]);
// #else
//         ak_free(p[order[i]]);
// #endif
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[order[i]] = malloc(s);
// #else
//         p[order[i]] = ak_malloc(s);
// #endif
//         memset(p[order[i]], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs; ++i) {
// #if USE_MALLOC
//         free(p[i]);
// #else
//         ak_free(p[i]);
// #endif
//     }
// }

// CPP_TEST( allocRandomFreeCoalesceMedium )
// {
//     static const ak_sz nptrs = 1000;
//     static const ak_sz sizemin = 16384;
//     static const ak_sz sizemax = 65535;

//     void* p[nptrs];
//     ak_sz order[nptrs];

//     ASSERT_TRUE(RAND_MAX >= nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         order[i] = i;
//     }
//     shuffle(order, nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[i] = malloc(s);
// #else
//         p[i] = ak_malloc(s);
// #endif
//         memset(p[i], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
// #if USE_MALLOC
//         free(p[order[i]]);
// #else
//         ak_free(p[order[i]]);
// #endif
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[order[i]] = malloc(s);
// #else
//         p[order[i]] = ak_malloc(s);
// #endif
//         memset(p[order[i]], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs; ++i) {
// #if USE_MALLOC
//         free(p[i]);
// #else
//         ak_free(p[i]);
// #endif
//     }
// }

// CPP_TEST( allocRandomFreeCoalesceLarge )
// {
//     static const ak_sz nptrs = 1000;
//     static const ak_sz sizemin = 65537;
//     static const ak_sz sizemax = (AK_SZ_ONE << 20) - 1;

//     void* p[nptrs];
//     ak_sz order[nptrs];

//     ASSERT_TRUE(RAND_MAX >= nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         order[i] = i;
//     }
//     shuffle(order, nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[i] = malloc(s);
// #else
//         p[i] = ak_malloc(s);
// #endif
//         memset(p[i], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
// #if USE_MALLOC
//         free(p[order[i]]);
// #else
//         ak_free(p[order[i]]);
// #endif
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[order[i]] = malloc(s);
// #else
//         p[order[i]] = ak_malloc(s);
// #endif
//         memset(p[order[i]], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs; ++i) {
// #if USE_MALLOC
//         free(p[i]);
// #else
//         ak_free(p[i]);
// #endif
//     }
// }

// CPP_TEST( allocRandomFreeMap )
// {
// #if AKMALLOC_BITNESS == 64
//     static const ak_sz nptrs = 1000;
// #else
//     static const ak_sz nptrs =  600;
// #endif
//     static const ak_sz sizemin = (AK_SZ_ONE << 20);
//     static const ak_sz sizemax = (AK_SZ_ONE << 22);

//     void* p[nptrs];
//     ak_sz order[nptrs];

//     ASSERT_TRUE(RAND_MAX >= nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         order[i] = i;
//     }
//     shuffle(order, nptrs);

//     for (ak_sz i = 0; i < nptrs; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[i] = malloc(s);
// #else
//         p[i] = ak_malloc(s);
// #endif
//         memset(p[i], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
// #if USE_MALLOC
//         free(p[order[i]]);
// #else
//         ak_free(p[order[i]]);
// #endif
//     }

//     for (ak_sz i = 0; i < nptrs/2; ++i) {
//         ak_sz s = (rand() % (sizemax - sizemin + 1)) + sizemin;
// #if USE_MALLOC
//         p[order[i]] = malloc(s);
// #else
//         p[order[i]] = ak_malloc(s);
// #endif
//         memset(p[order[i]], 42, s);
//     }

//     for (ak_sz i = 0; i < nptrs; ++i) {
// #if USE_MALLOC
//         free(p[i]);
// #else
//         ak_free(p[i]);
// #endif
//     }
// }

#include "akmalloc/atomic.h"

#include <thread>

std::thread::id val;
void* PTR = AK_NULLPTR;

void AllocDeallocTask()
{
    static const std::thread::id id = std::this_thread::get_id();
    static const ak_sz nptrs = 10000;
    // static const ak_sz nptrs = 2;
    void* p[nptrs] = { 0 };
    size_t sizes[nptrs] = { 0 };

    (void)id;

    size_t sizemin =   0;
    size_t sizemax =   0;

    sizemin =   8;
    sizemax = 256;

    for (ak_sz i = 0; i < (nptrs*7)/10; ++i) {
        sizes[i] = (rand() % (sizemax - sizemin + 1)) + sizemin;
    }

    // sizemin =               257;
    // sizemax = (AK_SZ_ONE << 21);
    for (ak_sz i = (nptrs*7)/10; i < nptrs; ++i) {
        sizes[i] = (rand() % (sizemax - sizemin + 1)) + sizemin;
    }

    // randomize sizes
    shuffle(sizes, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        p[i] = malloc(sizes[i]);
#else
        p[i] = ak_malloc(sizes[i]);
#endif
        memset(p[i], 42, sizes[i]);
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        p[i] = malloc(sizes[(nptrs/2) - i]);
#else
        p[i] = ak_malloc(sizes[(nptrs/2) - i]);
#endif
        memset(p[i], 42, sizes[(nptrs/2) - i]);
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }
}

#include <vector>

CPP_TEST( multiThreadedAllocTask )
{
    const size_t nthreads = std::thread::hardware_concurrency();

    std::vector<std::thread> threads(nthreads);
    for (size_t i = 0; i < nthreads; ++i) {
        std::thread newt(AllocDeallocTask);
        threads[i].swap(newt);
    }

    for (size_t i = 0; i < nthreads; ++i) {
        threads[i].join();
    }
}
