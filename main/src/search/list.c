/* list.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_list.h"

tl_list_node *tl_list_search(const tl_list *this, tl_compare cmp,
			     const void *key)
{
	tl_list_node *n;

	assert(this && cmp && key);

	for (n = this->first; n != NULL; n = n->next) {
		if (cmp(tl_list_node_get_data(n), key) == 0)
			return n;
	}

	return NULL;
}
