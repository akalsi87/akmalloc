/**
 * \file utils.h
 * \date Mar 01, 2016
 */

#ifndef AKMALLOC_UTILS_H
#define AKMALLOC_UTILS_H

#include "akmalloc/types.h"

#define align_size(s, aln)  (((s) + (aln) - 1)/(aln)) * (aln)
#define align2_size(s, aln) ((s) + (aln) - 1) & ((aln) - 1)

#define align_off(s, aln)   ((s) % (aln))
#define align2_off(s, aln)  ((s) & ((aln) - 1))

#define is_aligned(s, aln)  (align_off(s, aln) == 0)
#define is_aligned2(s, aln) (align2_off(s, aln) == 0)

#endif/*AKMALLOC_UTILS_H*/
