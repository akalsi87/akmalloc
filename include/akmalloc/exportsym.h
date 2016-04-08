/*! \file exportsym.h */
/* Export symbol definitions */

#if !defined(AKMALLOC_EXPORTSYM_H)
#define AKMALLOC_EXPORTSYM_H

#if defined(AKMALLOC_LINK_STATIC)
/*! Macro to define library export symbol */
#  define AKMALLOC_API 
#elif defined(AKMALLOC_BUILD)
#  if defined(_MSC_VER)
/*! Macro to define library export symbol */
#    define AKMALLOC_API __declspec(dllexport)
#  else/*GCC-like compiler*/
/*! Macro to define library export symbol */
#    define AKMALLOC_API __attribute__((__visibility__("default")))
#  endif
#else/* import symbol */
#  if defined(_MSC_VER)
/*! Macro to define library export symbol */
#    define AKMALLOC_API __declspec(dllimport)
#  else/*GCC-like compiler*/
/*! Macro to define library export symbol */
#    define AKMALLOC_API 
#  endif
#endif/*defined(AKMALLOC_LINK_STATIC)*/

/*! Allow deprecation? */
#define AKMALLOC_ALLOW_DEPRECATION 1

#if AKMALLOC_ALLOW_DEPRECATION
/*! Deprecated macro */
#  if defined(_MSC_VER)
#    define AKMALLOC_DEPRECATED(x) __declspec(deprecated(x))
#  else/*GCC-like compiler*/
#    define AKMALLOC_DEPRECATED(x) __attribute__((deprecated(x)))
#  endif/*defined(_MSC_VER)*/
#else/* render macro useless */
#  define AKMALLOC_DEPRECATED(x) 
#endif/*!AKMALLOC_ALLOW_DEPRECATION*/

#endif/*!defined(AKMALLOC_EXPORTSYM_H)*/
