/* list.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iterator.h"
#include "tl_list.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
	tl_iterator super;
	tl_list_node *node;
	tl_list *list;
	int forward;
} tl_list_iterator;


static void tl_list_iterator_destroy(tl_iterator *this)
{
	free(this);
}

static void tl_list_iterator_reset(tl_iterator *super)
{
	tl_list_iterator *this = (tl_list_iterator *)super;

	this->node = this->forward ? this->list->first : this->list->last;
}

static int tl_list_iterator_has_data(tl_iterator *super)
{
	tl_list_iterator *this = (tl_list_iterator *)super;

	return this->node != NULL;
}

static void tl_list_iterator_next(tl_iterator *super)
{
	tl_list_iterator *this = (tl_list_iterator *)super;

	if (!this->node)
		return;

	this->node = this->forward ? this->node->next : this->node->prev;
}

static void *tl_list_iterator_get_key(tl_iterator *this)
{
	(void)this;
	return NULL;
}

static void* tl_list_iterator_get_value(tl_iterator *super)
{
	tl_list_iterator *this = (tl_list_iterator *)super;
	return this->node ? tl_list_node_get_data(this->node) : NULL;
}

static void tl_list_iterator_remove(tl_iterator *super)
{
	tl_list_iterator *this = (tl_list_iterator *)super;
	tl_list_node *old;

	if (!this->node)
		return;

	old = this->node;

	if (this->list->size)
		--this->list->size;

	if (this->node == this->list->first) {
		this->node = this->node->next;
		this->list->first = this->node;

		if (this->node) {
			this->node->prev = NULL;
		} else {
			this->list->last = NULL;
		}
	} else if (this->node == this->list->last) {
		this->node = this->node->prev;
		this->list->last = this->node;

		if (this->node) {
			this->node->next = NULL;
		} else {
			this->list->first = NULL;
		}
	} else {
		this->node->prev->next = this->node->next;
		this->node->next->prev = this->node->prev;
		this->node = this->forward ? this->node->next :
					     this->node->prev;
	}

	tl_list_node_destroy(old, this->list);
}

static tl_iterator* tl_list_iterator_create(tl_list *list, int first)
{
	tl_list_iterator* this = malloc(sizeof(*this));
	tl_iterator *super = (tl_iterator *)this;

	this->list = list;
	this->node = first ? list->first : list->last;
	this->forward = first;

	super->destroy = tl_list_iterator_destroy;
	super->reset = tl_list_iterator_reset;
	super->has_data = tl_list_iterator_has_data;
	super->next = tl_list_iterator_next;
	super->get_key = tl_list_iterator_get_key;
	super->get_value = tl_list_iterator_get_value;
	super->remove = tl_list_iterator_remove;
	return super;
}

tl_iterator *tl_list_first(tl_list *this)
{
	assert(this);
	return tl_list_iterator_create(this, 1);
}

tl_iterator *tl_list_last(tl_list *this)
{
	assert(this);
	return tl_list_iterator_create(this, 0);
}
