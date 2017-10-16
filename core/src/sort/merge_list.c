/* merge_ip.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_list.h"

static tl_list_node *merge(tl_list_node *a, tl_list_node *b, tl_compare cmp)
{
	tl_list_node *head, *tail;

	if (!a)
		return b;
	if (!b)
		return a;

	if (cmp(tl_list_node_get_data(a), tl_list_node_get_data(b)) <= 0) {
		head = a;
		a = a->next;
	} else {
		head = b;
		b = b->next;
	}

	head->prev = head->next = NULL;
	tail = head;

	while (a && b) {
		if (cmp(tl_list_node_get_data(a),
			tl_list_node_get_data(b)) <= 0) {
			a->prev = tail;
			tail->next = a;
			tail = a;
			a = a->next;
		} else {
			b->prev = tail;
			tail->next = b;
			tail = b;
			b = b->next;
		}
	}

	tail->next = a ? a : b;
	tail->next->prev = tail;
	return head;
}

tl_list_node *tl_mergesort_list(tl_list_node *list, size_t count,
				tl_compare cmp)
{
	tl_list_node *lo, *hi;
	size_t i;

	if (!list || !list->next || count < 1)
		return list;

	hi = list;
	for (i = 0; i < count / 2; ++i)
		hi = hi->next;

	hi->prev->next = NULL;
	hi->prev = NULL;

	lo = tl_mergesort_list(list, count / 2, cmp);
	hi = tl_mergesort_list(hi, count - count / 2, cmp);

	return merge(lo, hi, cmp);
}

void tl_list_sort(tl_list *this, tl_compare cmp)
{
	tl_list_node *n;

	assert(this && cmp);

	if (this->size > 1) {
		this->first = tl_mergesort_list(this->first, this->size, cmp);

		for (n = this->first; n->next != NULL; n = n->next)
			;
		this->last = n;
	}
}
