/* array.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iterator.h"
#include "tl_array.h"

#include <stdlib.h>


typedef struct {
	tl_iterator super;
	tl_array *array;
	size_t idx;
	int forward;
} tl_array_iterator;


static void tl_array_iterator_destroy(tl_iterator *this)
{
	free(this);
}

static void tl_array_iterator_reset(tl_iterator *super)
{
	tl_array_iterator *this = (tl_array_iterator *)super;

	if (!this->forward && this->array->used) {
		this->idx = this->array->used - 1;
	} else {
		this->idx = 0;
	}
}

static int tl_array_iterator_has_data(tl_iterator *super)
{
	tl_array_iterator *this = (tl_array_iterator *)super;
	return this->idx < this->array->used;
}

static void tl_array_iterator_next(tl_iterator *super)
{
	tl_array_iterator *this = (tl_array_iterator *)super;

	if (this->idx < this->array->used) {
		if (this->forward) {
			++this->idx;
		} else {
			--this->idx; /* eventually underflows out of range */
		}
	}
}

static void *tl_array_iterator_get_key(tl_iterator *this)
{
	(void)this;
	return NULL;
}

static void *tl_array_iterator_get_value(tl_iterator *super)
{
	tl_array_iterator *this = (tl_array_iterator *)super;

	if (this->idx >= this->array->used)
		return NULL;

	return (char *)this->array->data + this->idx * this->array->unitsize;
}

static void tl_array_iterator_remove(tl_iterator *super)
{
	tl_array_iterator *this = (tl_array_iterator *)super;

	tl_array_remove(this->array, this->idx, 1);

	if (!this->forward)
		--this->idx;
}

static tl_iterator *tl_array_iterator_create(tl_array *array, int first)
{
	tl_array_iterator* this = malloc(sizeof(*this));
	tl_iterator *super = (tl_iterator *)this;

	this->array = array;
	this->idx = first ? 0 : array->used - 1;
	this->forward = first;

	super->destroy = tl_array_iterator_destroy;
	super->reset = tl_array_iterator_reset;
	super->has_data = tl_array_iterator_has_data;
	super->next = tl_array_iterator_next;
	super->get_key = tl_array_iterator_get_key;
	super->get_value = tl_array_iterator_get_value;
	super->remove = tl_array_iterator_remove;
	return super;
}

tl_iterator *tl_array_first(tl_array *this)
{
	assert(this);
	return tl_array_iterator_create(this, 1);
}

tl_iterator *tl_array_last(tl_array *this)
{
	assert(this);
	return tl_array_iterator_create(this, 0);
}
