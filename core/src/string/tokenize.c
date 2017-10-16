/* tokenize.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iterator.h"
#include "tl_string.h"

typedef struct {
	tl_iterator super;
	tl_string *str;		/* string to search throug */
	tl_string current;	/* the last extracted token */
	tl_string seperators;	/* string of seperator characters */
	size_t offset;		/* offset after the last substring */
} tl_token_iterator;

#define STATE_NONE_FOUND 0
#define STATE_LAST_WAS_START 1
#define STATE_NONSEP_FOUND 2

static void token_iterator_destroy(tl_iterator *super)
{
	tl_token_iterator *this = (tl_token_iterator *)super;
	tl_string_cleanup(&this->current);
	tl_string_cleanup(&this->seperators);
	free(this);
}

static int token_iterator_has_data(tl_iterator *this)
{
	return !tl_string_is_empty(&((tl_token_iterator *)this)->current);
}

static void token_iterator_next(tl_iterator *super)
{
	tl_token_iterator *this = (tl_token_iterator *)super;
	size_t first;
	char *ptr;

	tl_string_clear(&this->current);

	if (this->offset >= (this->str->data.used - 1))
		return;

	/* find first non-serperator character */
	ptr = (char *)this->str->data.data + this->offset;

	for (; *ptr; ++ptr, ++this->offset) {
		if ((*ptr & 0xC0) == 0x80)
			continue;
		if (!tl_utf8_strchr(tl_string_cstr(&this->seperators), ptr))
			break;
	}

	if (!(*ptr))
		return;

	first = this->offset;

	/* find next seperator character */
	for (; *ptr; ++ptr, ++this->offset) {
		if ((*ptr & 0xC0) == 0x80)
			continue;
		if (tl_utf8_strchr(tl_string_cstr(&this->seperators), ptr))
			break;
	}

	/* isolate */
	if (*ptr) {
		tl_string_append_utf8_count(&this->current,
					(char *)this->str->data.data + first,
					this->offset - first);
	} else {
		tl_string_append_utf8(&this->current,
				      (char *)this->str->data.data + first);
	}
}

static void token_iterator_reset(tl_iterator *super)
{
	tl_token_iterator *this = (tl_token_iterator *)super;
	this->offset = 0;
	token_iterator_next(super);
}

static void* token_iterator_get_value(tl_iterator *this)
{
	return &((tl_token_iterator *)this)->current;
}

tl_iterator *tl_string_tokenize(tl_string *str, const char *seperators)
{
	tl_token_iterator *it;

	assert(str && seperators);

	it = calloc(1, sizeof(*it));
	if (!it)
		return NULL;

	if (!tl_string_init(&it->seperators))
		goto fail;

	if (!tl_string_append_utf8(&it->seperators, seperators))
		goto failsep;

	if (!tl_string_init(&it->current))
		goto failsep;

	it->str = str;
	((tl_iterator*)it)->destroy = token_iterator_destroy;
	((tl_iterator*)it)->reset = token_iterator_reset;
	((tl_iterator*)it)->has_data = token_iterator_has_data;
	((tl_iterator*)it)->next = token_iterator_next;
	((tl_iterator*)it)->get_value = token_iterator_get_value;

	token_iterator_reset((tl_iterator* )it);
	return (tl_iterator*)it;
failsep:
	tl_string_cleanup(&it->seperators);
fail:
	free(it);
	return NULL;
}
