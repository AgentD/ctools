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

#ifdef TL_ALLIGN_MEMORY
#define PADDING sizeof(void*)
#define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
#define PADDING 0
#define ALLIGN( ptr )
#endif

typedef struct {
	size_t binsize;
	size_t idx;
	int used;
	tl_hashmap_entry *ent;
} entrydata;

static void get_entry_data(const tl_hashmap *this, entrydata *ent,
			   const void *key)
{
	ent->binsize = sizeof(ent->ent[0]) + this->keysize + this->objsize;
	ent->binsize += 2 * PADDING;

	ent->idx = this->hash(key) % this->bincount;

	ent->ent =
	    (tl_hashmap_entry *) ((char *)this->bins + ent->idx * ent->binsize);
	ent->used = this->bitmap[ent->idx / (sizeof(int) * CHAR_BIT)];
	ent->used = (ent->used >> (ent->idx % (sizeof(int) * CHAR_BIT))) & 0x01;
}

static void free_hashmap(size_t keysize, size_t objsize, void *bins,
			 int *bitmap, size_t bincount,
			 tl_allocator *keyalloc, tl_allocator *objalloc)
{
	tl_hashmap_entry *it, *old;
	char *ptr, *entry;
	size_t i, binsize;
	int used;

	binsize = sizeof(tl_hashmap_entry) + keysize + objsize;
	binsize += 2 * PADDING;
	ptr = (char *)bins;

	for (i = 0; i < bincount; ++i, ptr += binsize) {
		used = bitmap[i / (sizeof(int) * CHAR_BIT)];
		used = (used >> (i % (sizeof(int) * CHAR_BIT))) & 0x01;

		if (!used)
			continue;

		it = (tl_hashmap_entry *)ptr;

		while (it != NULL) {
			old = it;
			it = it->next;

			entry = (char *)it + sizeof(tl_hashmap_entry);
			ALLIGN(entry);
			tl_allocator_cleanup(keyalloc, entry, keysize, 1);

			entry += keysize;
			ALLIGN(entry);
			tl_allocator_cleanup(objalloc, entry, objsize, 1);

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
	size_t binsize, mapcount;

	/* sanity check */
	assert(this && keysize && objsize && bincount);
	assert(keyhash && keycompare);

	/* allocate bins */
	binsize = sizeof(tl_hashmap_entry) + keysize + objsize + 2 * PADDING;
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
	this->objsize = objsize;
	this->bincount = bincount;
	this->hash = keyhash;
	this->compare = keycompare;
	this->keyalloc = keyalloc;
	this->objalloc = valalloc;
	return 1;
}

void tl_hashmap_cleanup(tl_hashmap *this)
{
	assert(this);

	free_hashmap(this->keysize, this->objsize, this->bins, this->bitmap,
		     this->bincount, this->keyalloc, this->objalloc);

	free(this->bitmap);
	free(this->bins);

	memset(this, 0, sizeof(tl_hashmap));
}

int tl_hashmap_copy(tl_hashmap *this, const tl_hashmap *src)
{
	tl_hashmap_entry *sit, *dit;
	size_t i, binsize, mapcount;
	char *bins, *sptr, *dptr;
	int *bitmap;
	int used;

	assert(this && src);

	/* allocate bins */
	binsize = sizeof(tl_hashmap_entry) + src->keysize + src->objsize;
	binsize += 2 * PADDING;
	bins = calloc(binsize, src->bincount);

	if (!bins)
		return 0;

	/* copy usage bitmap */
	mapcount = 1 + (src->bincount / (sizeof(int) * CHAR_BIT));
	bitmap = malloc(mapcount * sizeof(int));

	if (!bitmap) {
		free(bins);
		return 0;
	}

	memcpy(bitmap, src->bitmap, mapcount * sizeof(int));

	/* copy bin contents */
	for (i = 0; i < src->bincount; ++i) {
		dit = (tl_hashmap_entry *) (bins + i * binsize);
		sit = (tl_hashmap_entry *) ((char *)src->bins + i * binsize);
		used = src->bitmap[i / (sizeof(int) * CHAR_BIT)];
		used = (used >> (i % (sizeof(int) * CHAR_BIT))) & 0x01;

		if (!used)
			continue;

		for (; sit != NULL; sit = sit->next, dit = dit->next) {
			dptr = (char *)dit + sizeof(tl_hashmap_entry);
			sptr = (char *)sit + sizeof(tl_hashmap_entry);
			ALLIGN(dptr);
			ALLIGN(sptr);

			tl_allocator_copy(this->keyalloc, dptr, sptr,
					  this->keysize, 1);
			sptr += this->keysize;
			dptr += this->keysize;
			ALLIGN(sptr);
			ALLIGN(dptr);

			tl_allocator_copy(this->objalloc, dptr, sptr,
					  this->objsize, 1);

			if (sit->next) {
				dit->next = calloc(1, binsize);
				if (!dit->next)
					goto fail;
			}
		}
	}

	/* set */
	free(this->bins);
	free(this->bitmap);

	this->keysize = src->keysize;
	this->objsize = src->objsize;
	this->bincount = src->bincount;
	this->hash = src->hash;
	this->compare = src->compare;
	this->bins = bins;
	this->bitmap = bitmap;
	return 1;
fail:
	free_hashmap(src->keysize, src->objsize, bins, bitmap,
			src->bincount, src->keyalloc, src->objalloc);
	free(bins);
	free(bitmap);
	return 0;
}

void tl_hashmap_clear(tl_hashmap *this)
{
	size_t binsize, mapcount;

	assert(this);

	free_hashmap(this->keysize, this->objsize, this->bins, this->bitmap,
		     this->bincount, this->keyalloc, this->objalloc);

	binsize = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
	binsize += 2 * PADDING;

	mapcount = 1 + (this->bincount / (sizeof(int) * CHAR_BIT));

	memset(this->bins, 0, this->bincount * binsize);
	memset(this->bitmap, 0, mapcount * sizeof(int));
}

tl_hashmap_entry *tl_hashmap_get_bin(const tl_hashmap *this, size_t idx)
{
	size_t binsize;
	int used;

	assert(this);

	if (idx >= this->bincount)
		return NULL;

	used = this->bitmap[idx / (sizeof(int) * CHAR_BIT)];
	used = (used >> (idx % (sizeof(int) * CHAR_BIT))) & 0x01;

	if (!used)
		return NULL;

	binsize = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
	binsize += 2 * PADDING;

	return (tl_hashmap_entry *)(this->bins + idx * binsize);
}

void *tl_hashmap_entry_get_key(const tl_hashmap *this,
			       const tl_hashmap_entry *ent)
{
	char *ptr;
	(void)this;

	assert(this && ent);

	ptr = (char *)ent + sizeof(tl_hashmap_entry);
	ALLIGN(ptr);

	return ptr;
}

void *tl_hashmap_entry_get_value(const tl_hashmap *this,
				 const tl_hashmap_entry *ent)
{
	char *ptr;

	assert(this && ent);

	ptr = (char *)ent + sizeof(tl_hashmap_entry);
	ALLIGN(ptr);
	ptr += this->keysize;
	ALLIGN(ptr);

	return ptr;
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
		new = malloc(data.binsize);
		if (!new)
			return 0;

		memcpy(new, data.ent, data.binsize);
		data.ent->next = new;
	} else {
		mask = 1 << (data.idx % (sizeof(int) * CHAR_BIT));
		this->bitmap[data.idx / (sizeof(int) * CHAR_BIT)] |= mask;
	}

	/* copy key */
	ptr = (char *)data.ent + sizeof(tl_hashmap_entry);
	ALLIGN(ptr);
	tl_allocator_copy(this->keyalloc, ptr, key, this->keysize, 1);

	/* copy value */
	ptr += this->keysize;
	ALLIGN(ptr);
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
		ALLIGN(ptr);

		if (this->compare(ptr, key) == 0) {
			ptr += this->keysize;
			ALLIGN(ptr);
			return ptr;
		}
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
		ALLIGN(ptr);

		if (this->compare(ptr, key) != 0) {
			prev = it;
			it = it->next;
			continue;
		}

		tl_allocator_cleanup(this->keyalloc, ptr, this->keysize, 1);
		ptr += this->keysize;
		ALLIGN(ptr);

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
			memcpy(it, it->next, data.binsize);
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
