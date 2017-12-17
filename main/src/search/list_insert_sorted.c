/* list_insert_sorted.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_list.h"

int tl_list_insert_sorted(tl_list *this, tl_compare cmp, const void *element)
{
	tl_list_node *n, *t;

	assert(this && cmp && element);

	if (!(t = tl_list_node_create(this, element)))
		return 0;

	if (!this->size) {
		this->first = this->last = t;
	} else if (cmp(element, tl_list_node_get_data(this->first)) <= 0) {
		t->next = this->first;
		this->first->prev = t;
		this->first = t;
	} else {
		for (n = this->first; n != NULL; n = n->next) {
			if (cmp(tl_list_node_get_data(n), element) > 0) {
				t->next = n;
				t->prev = n->prev;

				t->next->prev = t;
				t->prev->next = t;
				goto done;
			}
		}

		t->prev = this->last;
		this->last->next = t;
		this->last = t;
	}
done:
	this->size += 1;
	return 1;
}

