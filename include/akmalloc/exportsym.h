/**
 * \file exportsym.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_EXPORTSYM_H
#define AKMALLOC_EXPORTSYM_H

#include "akmalloc/config.h"

#if AKMALLOC_MSVC
#  define AKMALLOC_PLATFORM_EXPORT __declspec(dllexport)
#else
#  define AKMALLOC_PLATFORM_EXPORT __attribute__((visibility("default")))
#endif

#endif/*AKMALLOC_EXPORTSYM_H*/
