/* merge_array.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_array.h"
#include "tl_sort.h"

void tl_array_stable_sort(tl_array *this, tl_compare cmp)
{
	assert(this && cmp);

	if (!this->data || !this->used)
		return;
	if (tl_mergesort(this->data, this->used, this->unitsize, cmp))
		return;
	tl_mergesort_inplace(this->data, this->used, this->unitsize, cmp);
}
