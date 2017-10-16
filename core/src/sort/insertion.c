/* insertion.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_sort.h"

#include <string.h>
#include <stdlib.h>

static TL_INLINE void swap(char *a, char *b, size_t n)
{
	size_t i;
	char t;

	for (i = 0; i < n; ++i) {
		t = *a;
		*(a++) = *b;
		*(b++) = t;
	}
}

void tl_insertionsort(void *data, size_t n, size_t size, tl_compare cmp)
{
	char *first = (char *)data + size;
	char *limit = (char *)data + n * size;
	char *ptr, *pl;

	for (ptr = first; ptr < limit; ptr += size) {
		for (pl = ptr; pl > (char *)data; pl -= size) {
			if (cmp(pl-size, pl) <= 0)
				break;

			swap(pl, pl - size, size);
		}
	}
}
