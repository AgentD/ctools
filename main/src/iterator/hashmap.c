/* hashmap.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_iterator.h"
#include "tl_hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
	tl_iterator super;
	tl_hashmap *map;
	tl_hashmap_entry *ent;
	tl_hashmap_entry *prev;
	size_t idx;
} tl_hashmap_iterator;


static void find_next_bin(tl_hashmap_iterator *this)
{
	this->prev = this->ent = NULL;

	while (!this->ent && (this->idx < this->map->bincount)) {
		this->ent = tl_hashmap_get_bin(this->map, this->idx);

		if (this->ent)
			break;

		++this->idx;
	}
}


static void tl_hashmap_iterator_destroy(tl_iterator *this)
{
	free(this);
}

static void tl_hashmap_iterator_reset(tl_iterator *super)
{
	tl_hashmap_iterator *this = (tl_hashmap_iterator *)super;

	this->idx = 0;
	find_next_bin(this);
}

static int tl_hashmap_iterator_has_data(tl_iterator *this)
{
	return ((tl_hashmap_iterator *)this)->ent != NULL;
}

static void tl_hashmap_iterator_next(tl_iterator *super)
{
	tl_hashmap_iterator *this = (tl_hashmap_iterator *)super;

	if (this->ent) {
		this->prev = this->ent;
		this->ent = this->ent->next;

		if (!this->ent) {
			this->idx += 1;
			find_next_bin(this);
		}
	}
}

static void *tl_hashmap_iterator_get_key(tl_iterator *super)
{
	tl_hashmap_iterator *this = (tl_hashmap_iterator *)super;

	if (!this->ent)
		return NULL;

	return tl_hashmap_entry_get_key(this->map, this->ent);
}

static void *tl_hashmap_iterator_get_value(tl_iterator *super)
{
	tl_hashmap_iterator *this = (tl_hashmap_iterator *)super;

	if (!this->ent)
		return NULL;

	return tl_hashmap_entry_get_value(this->map, this->ent);
}

static void tl_hashmap_iterator_remove(tl_iterator *super)
{
	tl_hashmap_iterator *this = (tl_hashmap_iterator *)super;
	tl_hashmap_entry *old;
	void *key, *val;
	int used;

	if (!this->ent)
		return;

	key = tl_hashmap_entry_get_key(this->map, this->ent);
	val = tl_hashmap_entry_get_value(this->map, this->ent);

	tl_allocator_cleanup(this->map->keyalloc, key, this->map->keysize, 1);
	tl_allocator_cleanup(this->map->objalloc, val, this->map->objsize, 1);

	if (this->prev) {
		this->prev->next = this->ent->next;
		free(this->ent);

		this->ent = this->prev->next;

		if (this->ent)
			return;
	} else {
		if (this->ent->next) {
			old = this->ent->next;
			memcpy(this->ent, this->ent->next, this->map->binsize);
			free(old);
			return;
		}

		used = ~(1 << (this->idx % (sizeof(int) * CHAR_BIT)));
		this->map->bitmap[this->idx / (sizeof(int) * CHAR_BIT)]&=used;
	}

	this->idx += 1;
	find_next_bin(this);
}

tl_iterator *tl_hashmap_get_iterator(tl_hashmap *this)
{
	tl_hashmap_iterator *it = NULL;

	assert(this);

	it = calloc(1, sizeof(*it));
	if (!it)
		return NULL;

	it->map = this;
	it->super.destroy = tl_hashmap_iterator_destroy;
	it->super.reset = tl_hashmap_iterator_reset;
	it->super.has_data = tl_hashmap_iterator_has_data;
	it->super.next = tl_hashmap_iterator_next;
	it->super.get_key = tl_hashmap_iterator_get_key;
	it->super.get_value = tl_hashmap_iterator_get_value;
	it->super.remove = tl_hashmap_iterator_remove;

	tl_hashmap_iterator_reset((tl_iterator *)it);
	return (tl_iterator *)it;
}
