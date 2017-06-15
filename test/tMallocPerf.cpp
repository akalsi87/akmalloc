/**
 * \file tMallocPerf.cpp
 * \date Jul 08, 2016
 */

#include "unittest.hpp"

#include "akmalloc/malloc.h"
#include "akmalloc/constants.h"

#include <stdlib.h>
#include <string.h>

#if !defined(USE_MALLOC)
#  define USE_MALLOC 0
#endif

// #define PRINT_MEM_USAGE

#ifdef PRINT_MEM_USAGE
#include "mem.h"
#  define memtestsz printmemusg
#  define memprintf printf
#else
#  define memtestsz(...) (void)0
#  define memprintf(...) (void)0
#endif

static int rand_num()
{
    int const r = rand();
    return (r << 15) | (r >> 17);
}

template <class T>
void printmemusg(T const* array, ak_sz n)
{
    ak_sz mem = 0;
    for (ak_sz i = 0; i < n; ++i) {
        mem += array[i];
    }
    if (mem > (1024*1024)) {
        printf(" -- Mem usage in test: %f MB\n", double(mem)/(1024*1024));
    } else {
        printf(" -- Mem usage in test: %f KB\n", double(mem)/1024);
    }
}

template <class T>
void shuffle(T* array, ak_sz n)
{
    if (n > 1) {
        ak_sz i;
        for (i = 0; i < n - 1; i++) {
            ak_sz j = i + (rand_num() % (n - i));
            T t = array[j];
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
    ak_sz sizes[nptrs];
    (void)sizes;

    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
        sizes[i] = s;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    memtestsz(sizes, nptrs);
    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
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

CPP_TEST( allocRandomFreeCoalesceSmall )
{
    static const ak_sz nptrs = 1000;
    static const ak_sz sizemin = 257;
    static const ak_sz sizemax = 16383;

    void* p[nptrs];
    ak_sz order[nptrs];
    ak_sz sizes[nptrs];
    (void)sizes;
    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);
    
    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
        sizes[i] = s;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    memtestsz(sizes, nptrs);
    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
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

CPP_TEST( allocRandomFreeCoalesceMedium )
{
    static const ak_sz nptrs = 1000;
    static const ak_sz sizemin = 16384;
    static const ak_sz sizemax = 65535;

    void* p[nptrs];
    ak_sz order[nptrs];
    ak_sz sizes[nptrs];
    (void)sizes;
    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
        sizes[i] = s;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    memtestsz(sizes, nptrs);
    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
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

CPP_TEST( allocRandomFreeCoalesceLarge )
{
    static const ak_sz nptrs = 1000;
    static const ak_sz sizemin = 65537;
    static const ak_sz sizemax = (AK_SZ_ONE << 20) - 1;

    void* p[nptrs];
    ak_sz order[nptrs];
    ak_sz sizes[nptrs];
    (void)sizes;
    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
        sizes[i] = s;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        memset(p[i], 42, s);
    }

    memtestsz(sizes, nptrs);
    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
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
    static const ak_sz nptrs =  500;
    static int runalready = 0;
    if (!runalready) {
        runalready = 1;
    } else {
        return;
    }
#endif
    static const ak_sz sizemin = (AK_SZ_ONE << 20);
    static const ak_sz sizemax = (AK_SZ_ONE << 22);

    void* p[nptrs];
    ak_sz order[nptrs];
    ak_sz sizes[nptrs];
    (void)sizes;
    ASSERT_TRUE(RAND_MAX >= nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        order[i] = i;
    }
    shuffle(order, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
        sizes[i] = s;
#if USE_MALLOC
        p[i] = malloc(s);
#else
        p[i] = ak_malloc(s);
#endif
        // memset(p[i], 42, s);
        {
            char* pc = (char*)p[i];
            pc[0] = 42;
            pc[s-1] = 42;
        }
    }

    memtestsz(sizes, nptrs);
    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

    for (ak_sz i = 0; i < nptrs/2; ++i) {
#if USE_MALLOC
        free(p[order[i]]);
#else
        ak_free(p[order[i]]);
#endif
    }

    for (ak_sz i = 0; i < nptrs/2; ++i) {
        ak_sz s = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
#if USE_MALLOC
        p[order[i]] = malloc(s);
#else
        p[order[i]] = ak_malloc(s);
#endif
        // memset(p[i], 42, s);
        {
            char* pc = (char*)p[order[i]];
            pc[0] = 42;
            pc[s-1] = 42;
        }
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }
}

#include <thread>

static const size_t nthreads = std::thread::hardware_concurrency();

void AllocDeallocTask()
{
    static const ak_sz nptrs = (10000 * nthreads) > 20000 ? 20000/nthreads : 10000;
 
    void* p[20000] = { 0 };
    size_t sizes[20000] = { 0 };

    size_t sizemin =   0;
    size_t sizemax =   0;

    sizemin =   8;
    sizemax = 256;

    for (ak_sz i = 0; i < (nptrs*7)/10; ++i) {
        sizes[i] = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
    }

    sizemin =               257;
    sizemax = (AK_SZ_ONE << 21);
    for (ak_sz i = (nptrs*7)/10; i < nptrs; ++i) {
        sizes[i] = (rand_num() % (sizemax - sizemin + 1)) + sizemin;
    }

    // randomize sizes
    shuffle(sizes, nptrs);
    memtestsz(sizes, nptrs);

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        p[i] = malloc(sizes[i]);
#else
        p[i] = ak_malloc(sizes[i]);
#endif
        // memset(p[i], 42, sizes[i]);
        {
            char* pc = (char*)p[i];
            pc[0] = 42;
            pc[sizes[i]-1] = 42;
        }
    }

    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));

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
        // memset(p[i], 42, sizes[(nptrs/2) - i]);
        {
            char* pc = (char*)p[i];
            pc[0] = 42;
            pc[sizes[(nptrs/2) - i]-1] = 42;
        }
    }

    for (ak_sz i = 0; i < nptrs; ++i) {
#if USE_MALLOC
        free(p[i]);
#else
        ak_free(p[i]);
#endif
    }

    memprintf(" -- Current RSS: %zu MB\n", getCurrentRSS()/(1024*1024));
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
