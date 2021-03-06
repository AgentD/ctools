/* list.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_list.h"

tl_list_node *tl_list_node_from_index(const tl_list * this, size_t idx)
{
	tl_list_node *n = NULL;
	size_t i;

	assert(this);

	if (idx < this->size) {
		if (idx > this->size / 2) {
			for (n = this->last, i = this->size - 1; n && i > idx;
			     --i, n = n->prev) {
			}
		} else {
			for (n = this->first, i = 0; n && i < idx;
			     ++i, n = n->next) {
			}
		}
	}
	return n;
}

int tl_list_from_array(tl_list * this, const void *data, size_t count)
{
	tl_list temp;
	size_t i;

	assert(this && data);

	tl_list_init(&temp, this->unitsize, this->alloc);

	for (i = 0; i < count; ++i) {
		if (!tl_list_append(&temp, data)) {
			tl_list_clear(&temp);
			return 0;
		}

		data = (unsigned char *)data + this->unitsize;
	}

	tl_list_clear(this);
	this->first = temp.first;
	this->last = temp.last;
	this->size = temp.size;
	return 1;
}

void tl_list_to_array(const tl_list * this, void *data)
{
	tl_list_node *n;

	assert(this && data);

	if (this->size) {
		for (n = this->first; n; n = n->next) {
			tl_allocator_copy(this->alloc, data,
					  tl_list_node_get_data(n),
					  this->unitsize, 1);
			data = (unsigned char *)data + this->unitsize;
		}
	}
}

int tl_list_copy_range(tl_list * this, const tl_list * src,
		       size_t start, size_t count)
{
	tl_list_node *n;
	tl_list temp;
	size_t i;

	assert(this && src);

	/* sanity check */
	if ((start + count) > src->size)
		return 0;
	if (!src->size)
		return 1;

	/* get the start node */
	if (!(n = tl_list_node_from_index(src, start)))
		return 0;

	/* create a copy of the sublist */
	tl_list_init(&temp, src->unitsize, this->alloc);

	for (i = 0; i < count; ++i, n = n->next) {
		if (!tl_list_append(&temp, tl_list_node_get_data(n))) {
			tl_list_clear(&temp);
			return 0;
		}
	}

	/* free the current list and overwrite with the copy */
	tl_list_clear(this);
	this->first = temp.first;
	this->last = temp.last;
	this->size = temp.size;
	return 1;
}

int tl_list_join(tl_list * this, tl_list * other, size_t idx)
{
	tl_list_node *n;

	assert(this && other && this->unitsize == other->unitsize);

	if (idx > this->size)
		return 0;

	if (!other->size)
		return 1;

	if (!this->size) {	/* overwrite empty list */
		this->first = other->first;
		this->last = other->last;
	} else if (idx == 0) {	/* prepend to list */
		other->last->next = this->first;
		this->first->prev = other->last;

		this->first = other->first;
	} else if (idx == this->size) {	/* append to list */
		other->first->prev = this->last;
		this->last->next = other->first;

		this->last = other->last;
	} else {		/* insert somewhere in the list */

		if (!(n = tl_list_node_from_index(this, idx)))
			return 0;

		n->prev->next = other->first;
		other->first->prev = n->prev;

		n->prev = other->last;
		other->last->next = n;
	}

	this->size += other->size;

	other->last = NULL;
	other->first = NULL;
	other->size = 0;
	return 1;
}

void tl_list_reverse(tl_list * this)
{
	tl_list_node *temp;
	tl_list_node *i;

	assert(this);

	if (this->size) {
		/* swap the previous and the next pointer for every node */
		for (i = this->first; i; i = i->prev) {
			temp = i->next;
			i->next = i->prev;
			i->prev = temp;
		}

		/* swap the first and last pointer */
		temp = this->first;
		this->first = this->last;
		this->last = temp;
	}
}

int tl_list_concat(tl_list * this, const tl_list * src)
{
	tl_list temp;

	assert(this && src && src->unitsize == this->unitsize);

	if (!src->size)
		return 1;

	/* create a copy of the source list */
	tl_list_init(&temp, src->unitsize, this->alloc);

	if (!tl_list_copy(&temp, src))
		return 0;

	/* join the copy and the current list */
	return tl_list_join(this, &temp, this->size);
}

void tl_list_remove(tl_list * this, size_t idx, size_t count)
{
	tl_list_node *old;
	tl_list_node *n;
	size_t i;

	assert(this);

	if (idx >= this->size)
		return;

	count = (count > this->size) ? this->size : count;

	if (idx == 0) {
		for (i = 0; this->first && i < count; ++i) {
			n = this->first;
			this->first = this->first->next;

			tl_list_node_destroy(n, this);
		}

		if (this->first)
			this->first->prev = NULL;
	} else if ((idx + count) >= this->size) {
		count = this->size - idx;
		for (i = 0; i < count && this->last; ++i) {
			n = this->last;
			this->last = this->last->prev;

			tl_list_node_destroy(n, this);
		}

		if (this->last)
			this->last->next = NULL;
	} else {
		if (!(n = tl_list_node_from_index(this, idx)))
			return;

		for (i = 0; i < count && n; ++i) {
			old = n;

			n->prev->next = n->next;
			n->next->prev = n->prev;
			n = n->next;

			tl_list_node_destroy(old, this);
		}
	}

	this->size -= count;

	if (!this->size) {
		this->first = NULL;
		this->last = NULL;
	}
}

int tl_list_set(tl_list * this, size_t idx, const void *element)
{
	void *ptr;

	ptr = tl_list_at(this, idx);

	if (!ptr)
		return 0;

	tl_allocator_cleanup(this->alloc, ptr, this->unitsize, 1);
	tl_allocator_copy(this->alloc, ptr, element, this->unitsize, 1);
	return 1;
}

int tl_list_append(tl_list * this, const void *element)
{
	tl_list_node *node;

	assert(this && element);

	node = tl_list_node_create(this, element);

	if (!node)
		return 0;

	if (!this->size) {
		this->first = this->last = node;
	} else {
		this->last->next = node;
		node->prev = this->last;
		this->last = node;
	}

	++(this->size);
	return 1;
}

int tl_list_prepend(tl_list * this, const void *element)
{
	tl_list_node *node;

	assert(this && element);

	node = tl_list_node_create(this, element);

	if (!node)
		return 0;

	if (!this->size) {
		this->first = this->last = node;
	} else {
		this->first->prev = node;
		node->next = this->first;
		this->first = node;
	}

	++(this->size);
	return 1;
}

int tl_list_insert(tl_list * this, size_t idx,
		   const void *elements, size_t count)
{
	tl_list list;

	assert(this && elements);

	if (idx > this->size)
		return 0;

	if (!count)
		return 1;

	/* construct a list from the array */
	tl_list_init(&list, this->unitsize, this->alloc);

	if (!tl_list_from_array(&list, elements, count))
		return 0;

	/* merge the list into the array */
	if (!tl_list_join(this, &list, idx)) {
		tl_list_clear(this);
		return 0;
	}
	return 1;
}

void tl_list_remove_first(tl_list * this)
{
	tl_list_node *n;

	assert(this);

	if (this->size) {
		if (this->size == 1) {
			n = this->first;
			this->first = this->last = NULL;
		} else {
			n = this->first;
			this->first = this->first->next;
			this->first->prev = NULL;
		}

		tl_list_node_destroy(n, this);
		--(this->size);
	}
}

void tl_list_remove_last(tl_list * this)
{
	tl_list_node *n;

	assert(this);

	if (this->size) {
		if (this->size == 1) {
			n = this->first;
			this->first = this->last = NULL;
		} else {
			n = this->last;
			this->last = this->last->prev;
			this->last->next = NULL;
		}

		tl_list_node_destroy(n, this);
		--(this->size);
	}
}

void tl_list_clear(tl_list * this)
{
	tl_list_node *n, *old;

	assert(this);

	n = this->first;

	this->size = 0;
	this->first = NULL;
	this->last = NULL;

	while (n != NULL) {
		old = n;
		n = n->next;
		tl_list_node_destroy(old, this);
	}
}

tl_list_node *tl_list_drop_first(tl_list * this)
{
	tl_list_node *n = NULL;

	assert(this);

	if (this->size) {
		n = this->first;

		this->first = this->first->next;

		if (!this->first)
			this->last = NULL;

		--(this->size);
	}

	return n;
}

tl_list_node *tl_list_drop_last(tl_list * this)
{
	tl_list_node *n = NULL;

	assert(this);

	if (this->size) {
		n = this->last;

		this->last = this->last->prev;

		if (!this->last)
			this->first = NULL;

		--(this->size);
	}

	return n;
}
