/* hashmap.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
	size_t idx;
	int used;
	tl_hashmap_entry *ent;
} entrydata;

static void get_entry_data(const tl_hashmap *this, entrydata *ent,
			   const void *key)
{
	size_t offset;

	ent->idx = this->hash(key) % this->bincount;

	offset = ent->idx * this->binsize;

	ent->ent = (tl_hashmap_entry *)((char *)this->bins + offset);
	ent->used = this->bitmap[ent->idx / (sizeof(int) * CHAR_BIT)];
	ent->used &= 1 << (ent->idx % (sizeof(int) * CHAR_BIT));
}

static void free_hashmap(tl_hashmap *this)
{
	tl_hashmap_entry *it, *old;
	char *ptr, *entry;
	size_t i;
	int used;

	ptr = (char *)this->bins;

	for (i = 0; i < this->bincount; ++i, ptr += this->binsize) {
		used = this->bitmap[i / (sizeof(int) * CHAR_BIT)];
		used = (used >> (i % (sizeof(int) * CHAR_BIT))) & 0x01;

		if (!used)
			continue;

		it = (tl_hashmap_entry *)ptr;

		while (it != NULL) {
			old = it;
			it = it->next;

			entry = (char *)it + sizeof(tl_hashmap_entry);
			tl_allocator_cleanup(this->keyalloc, entry,
					     this->keysize, 1);

			entry += this->keysize_padded;
			tl_allocator_cleanup(this->objalloc, entry,
					     this->objsize, 1);

			if (old != (tl_hashmap_entry *)ptr)
				free(old);
		}
	}
}

/****************************************************************************/

int tl_hashmap_init(tl_hashmap *this, size_t keysize, size_t objsize,
		    size_t bincount, tl_hash keyhash, tl_compare keycompare,
		    tl_allocator *keyalloc, tl_allocator *valalloc)
{
	size_t binsize, mapcount, keysize_padded;

	/* sanity check */
	assert(this && keysize && objsize && bincount);
	assert(keyhash && keycompare);

	/* allocate bins */
	keysize_padded = keysize;
	if (keysize % sizeof(void*))
		keysize_padded += sizeof(void*) - keysize % sizeof(void*);

	binsize = sizeof(tl_hashmap_entry) + keysize_padded + objsize;
	if (binsize % sizeof(void*))
		binsize += sizeof(void*) - binsize % sizeof(void*);

	this->bins = calloc(bincount, binsize);

	if (!this->bins)
		return 0;

	/* allocate usage bitmap */
	mapcount = 1 + (bincount / (sizeof(int) * CHAR_BIT));
	this->bitmap = calloc(mapcount, sizeof(int));

	if (!this->bitmap) {
		free(this->bins);
		return 0;
	}

	/* init */
	this->keysize = keysize;
	this->keysize_padded = keysize_padded;
	this->objsize = objsize;
	this->bincount = bincount;
	this->binsize = binsize;
	this->hash = keyhash;
	this->compare = keycompare;
	this->keyalloc = keyalloc;
	this->objalloc = valalloc;
	return 1;
}

void tl_hashmap_cleanup(tl_hashmap *this)
{
	assert(this);

	free_hashmap(this);
	free(this->bitmap);
	free(this->bins);

	memset(this, 0, sizeof(tl_hashmap));
}

int tl_hashmap_copy(tl_hashmap *this, const tl_hashmap *src)
{
	tl_hashmap_entry *sit, *dit;
	size_t i, mapcount;
	char *sptr, *dptr;
	tl_hashmap cpy;
	int used;

	assert(this && src);

	memcpy(&cpy, src, sizeof(cpy));

	cpy.bins = calloc(cpy.binsize, cpy.bincount);
	if (!cpy.bins)
		return 0;

	mapcount = 1 + (cpy.bincount / (sizeof(int) * CHAR_BIT));

	cpy.bitmap = calloc(mapcount, sizeof(int));
	if (!cpy.bitmap) {
		free(cpy.bins);
		return 0;
	}

	/* copy bins */
	for (i = 0; i < cpy.bincount; ++i) {
		dit = (tl_hashmap_entry *)(cpy.bins + i * cpy.binsize);
		sit = (tl_hashmap_entry *)((char *)src->bins + i*cpy.binsize);
		used = src->bitmap[i / (sizeof(int) * CHAR_BIT)];
		used &= 1 << (i % (sizeof(int) * CHAR_BIT));

		if (!used)
			continue;

		cpy.bitmap[i / (sizeof(int) * CHAR_BIT)] |= used;

		for (; sit != NULL; sit = sit->next, dit = dit->next) {
			dptr = (char *)dit + sizeof(tl_hashmap_entry);
			sptr = (char *)sit + sizeof(tl_hashmap_entry);

			tl_allocator_copy(cpy.keyalloc, dptr, sptr,
					  cpy.keysize, 1);
			sptr += this->keysize_padded;
			dptr += this->keysize_padded;

			tl_allocator_copy(cpy.objalloc, dptr, sptr,
					  cpy.objsize, 1);

			if (sit->next) {
				dit->next = calloc(1, cpy.binsize);
				if (!dit->next)
					goto fail;
			}
		}
	}

	/* set */
	tl_hashmap_cleanup(this);
	memcpy(this, &cpy, sizeof(cpy));
	return 1;
fail:
	tl_hashmap_cleanup(&cpy);
	return 0;
}

void tl_hashmap_clear(tl_hashmap *this)
{
	size_t mapcount;

	assert(this);

	mapcount = 1 + (this->bincount / (sizeof(int) * CHAR_BIT));

	free_hashmap(this);
	memset(this->bins, 0, this->bincount * this->binsize);
	memset(this->bitmap, 0, mapcount * sizeof(int));
}

tl_hashmap_entry *tl_hashmap_get_bin(const tl_hashmap *this, size_t idx)
{
	int used;

	assert(this);

	if (idx >= this->bincount)
		return NULL;

	used = this->bitmap[idx / (sizeof(int) * CHAR_BIT)];
	used = (used >> (idx % (sizeof(int) * CHAR_BIT))) & 0x01;

	if (!used)
		return NULL;

	return (tl_hashmap_entry *)(this->bins + idx * this->binsize);
}

int tl_hashmap_insert(tl_hashmap *this, const void *key, const void *object)
{
	tl_hashmap_entry *new;
	entrydata data;
	char *ptr;
	int mask;

	assert(this && key && object);

	get_entry_data(this, &data, key);

	if (data.used) {
		new = malloc(this->binsize);
		if (!new)
			return 0;

		memcpy(new, data.ent, this->binsize);
		data.ent->next = new;
	} else {
		mask = 1 << (data.idx % (sizeof(int) * CHAR_BIT));
		this->bitmap[data.idx / (sizeof(int) * CHAR_BIT)] |= mask;
	}

	/* copy key */
	ptr = (char *)data.ent + sizeof(tl_hashmap_entry);
	tl_allocator_copy(this->keyalloc, ptr, key, this->keysize, 1);

	/* copy value */
	ptr += this->keysize_padded;
	tl_allocator_copy(this->objalloc, ptr, object, this->objsize, 1);
	return 1;
}

int tl_hashmap_set(tl_hashmap *this, const void *key, const void *object)
{
	void *ptr;

	assert(this && key && object);

	ptr = tl_hashmap_at(this, key);

	if (ptr) {
		tl_allocator_cleanup(this->objalloc, ptr, this->objsize, 1);
		tl_allocator_copy(this->objalloc, ptr, object, this->objsize,
				  1);
		return 1;
	}

	return 0;
}

void *tl_hashmap_at(const tl_hashmap *this, const void *key)
{
	tl_hashmap_entry *it;
	entrydata data;
	char *ptr;

	assert(this && key);

	get_entry_data(this, &data, key);

	if (!data.used)
		return NULL;

	for (it = data.ent; it != NULL; it = it->next) {
		ptr = (char *)it + sizeof(tl_hashmap_entry);

		if (this->compare(ptr, key) == 0)
			return ptr + this->keysize_padded;
	}

	return NULL;
}

int tl_hashmap_remove(tl_hashmap *this, const void *key, void *object)
{
	tl_hashmap_entry *it, *prev;
	entrydata data;
	size_t off;
	char *ptr;
	int mask;

	assert(this && key);

	get_entry_data(this, &data, key);

	if (!data.used)
		return 0;

	prev = NULL;
	it = data.ent;

	while (it != NULL) {
		ptr = (char *)it + sizeof(tl_hashmap_entry);

		if (this->compare(ptr, key) != 0) {
			prev = it;
			it = it->next;
			continue;
		}

		tl_allocator_cleanup(this->keyalloc, ptr, this->keysize, 1);
		ptr += this->keysize_padded;

		if (object) {
			memcpy(object, ptr, this->objsize);
		} else {
			tl_allocator_cleanup(this->objalloc, ptr,
					     this->objsize, 1);
		}

		if (prev) {
			prev->next = it->next;
			free(it);
		} else if (it->next) {
			prev = it->next;
			memcpy(it, it->next, this->binsize);
			free(prev);
		} else {
			mask = 1 << (data.idx % (sizeof(int) * CHAR_BIT));
			off = data.idx / (sizeof(int) * CHAR_BIT);
			this->bitmap[off] &= ~mask;
		}
		return 1;
	}

	return 0;
}

int tl_hashmap_is_empty(const tl_hashmap *this)
{
	size_t i, mapcount;

	assert(this);

	mapcount = 1 + (this->bincount / (sizeof(int) * CHAR_BIT));

	for (i = 0; i < mapcount; ++i) {
		if (this->bitmap[i])
			return 0;
	}

	return 1;
}
