/* array_unsorted.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_array.h"

void *tl_array_search_unsorted(const tl_array *this, tl_compare cmp,
				const void *key)
{
	size_t i = 0;
	char *ptr;

	assert(this && cmp && key);

	for (ptr = this->data; i < this->used; ++i, ptr += this->unitsize) {
		if (cmp(ptr, key) == 0)
			return ptr;
	}

	return NULL;
}
