/* blob.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_blob.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CLAMP_SIZE( blob, offset, length )\
        if( (offset) >= (blob)->size ) (length) = 0;\
        if( (length) && (((offset) + (length)) >= (blob)->size) )\
            (length) = (blob)->size - (offset)

int tl_blob_init(tl_blob * this, size_t size, const void *data)
{
	assert(this);

	memset(this, 0, sizeof(tl_blob));

	if (size) {
		this->data = malloc(size);
		this->size = size;

		if (!this->data)
			return 0;

		if (data)
			memcpy(this->data, data, size);
	}

	return 1;
}

int tl_blob_copy_range(tl_blob * this, const tl_blob * src,
		       size_t offset, size_t size)
{
	assert(this && src);
	CLAMP_SIZE(src, offset, size);
	return tl_blob_init(this, size, (char *)src->data + offset);
}

int tl_blob_append_range(tl_blob * this, const tl_blob * src,
			 size_t offset, size_t size)
{
	assert(this && src);
	CLAMP_SIZE(src, offset, size);
	return tl_blob_append_raw(this, (char *)src->data + offset, size);
}

int tl_blob_append_raw(tl_blob * this, const void *src, size_t size)
{
	void *new;

	assert(this);

	if (!size)
		return 1;

	new = realloc(this->data, this->size + size);
	if (!new)
		return 0;

	if (src)
		memcpy((char *)new + this->size, src, size);

	this->data = new;
	this->size += size;
	return 1;
}

int tl_blob_split(tl_blob * this, tl_blob * src, size_t offset)
{
	assert(this && src);

	if (!offset) {
		memcpy(this, src, sizeof(tl_blob));
		memset(src, 0, sizeof(tl_blob));
	} else if (offset >= src->size) {
		memset(this, 0, sizeof(tl_blob));
	} else {
		if (!tl_blob_init
		    (this, src->size - offset, (char *)src->data + offset))
			return 0;

		tl_blob_truncate(src, offset);
	}
	return 1;
}

int tl_blob_cut_range(tl_blob * this, tl_blob * src,
		      size_t offset, size_t length)
{
	if (!tl_blob_copy_range(this, src, offset, length))
		return 0;
	tl_blob_remove(src, offset, length);
	return 1;
}

int tl_blob_insert_raw(tl_blob * this, const void *src,
		       size_t offset, size_t length)
{
	void *new;

	assert(this);

	if (!length)
		return 1;
	new = realloc(this->data, this->size + length);
	if (!new)
		return 0;

	offset = offset >= this->size ? this->size : offset;

	memmove((char *)new + offset + length, (char *)new + offset,
		this->size - offset);

	if (src)
		memcpy((char *)new + offset, src, length);

	this->data = new;
	this->size += length;
	return 1;
}

int tl_blob_insert(tl_blob * this, const tl_blob * src,
		   size_t dstoffset, size_t srcoffset, size_t length)
{
	assert(this && src);
	CLAMP_SIZE(src, srcoffset, length);
	return tl_blob_insert_raw(this, (char *)src->data + srcoffset,
				  dstoffset, length);
}

void tl_blob_remove(tl_blob * this, size_t offset, size_t length)
{
	void *new;

	assert(this);

	if (length && (offset < this->size)) {
		if ((offset + length) >= this->size) {
			tl_blob_truncate(this, offset);
		} else {
			memmove((char *)this->data + offset,
				(char *)this->data + offset + length,
				this->size - (offset + length));

			new = realloc(this->data, this->size - length);
			this->data = new ? new : this->data;
			this->size -= length;
		}
	}
}

void tl_blob_truncate(tl_blob * this, size_t offset)
{
	void *new;

	assert(this);

	if (offset < this->size) {
		if (offset) {
			new = realloc(this->data, offset);
			this->data = new ? new : this->data;
		} else {
			free(this->data);
			this->data = NULL;
		}

		this->size = offset;
	}
}
