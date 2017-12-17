/* array_insert_sorted.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_array.h"
#include "tl_allocator.h"

int tl_array_insert_sorted(tl_array *this, tl_compare cmp, const void *element)
{
	size_t i = 0;
	char *ptr;

	assert(this && cmp && element);

	for (ptr = this->data; i < this->used; ++i, ptr += this->unitsize) {
		if (cmp(ptr, element) <= 0)
			continue;

		if (!tl_array_resize(this, this->used + 1, 0))
			return 0;

		ptr = (char *)this->data + i * this->unitsize;

		memmove(ptr + this->unitsize, ptr,
			(this->used - 1 - i) * this->unitsize);

		tl_allocator_copy(this->alloc, ptr, element,
				  this->unitsize, 1);
		return 1;
	}

	return tl_array_append(this, element);
}
