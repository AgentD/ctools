/* dir_it.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_iterator.h"
#include "tl_dir.h"
#include "os.h"

#include <dirent.h>

typedef struct {
	tl_iterator super;	/* inherits iterator interface */
	tl_string current;	/* current directory name */
	struct dirent *ent;	/* current directory entry */
	DIR *dir;		/* pointer to directory */
} dir_iterator;

static void find_next(dir_iterator *this)
{
retry:
	this->ent = readdir(this->dir);

	if (!this->ent)
		return;
	if (!strcmp(this->ent->d_name, "."))
		goto retry;
	if (!strcmp(this->ent->d_name, ".."))
		goto retry;

	tl_string_init_local(&this->current, this->ent->d_name);
}

static void dir_iterator_destroy(tl_iterator *super)
{
	dir_iterator *this = (dir_iterator *)super;

	closedir(this->dir);
	free(this);
}

static void dir_iterator_reset(tl_iterator *super)
{
	dir_iterator *this = (dir_iterator *)super;
	rewinddir(this->dir);
	find_next(this);
}

static int dir_iterator_has_data(tl_iterator *this)
{
	return (((dir_iterator *)this)->ent != NULL);
}

static void dir_iterator_next(tl_iterator *super)
{
	dir_iterator *this = (dir_iterator *)super;

	if (this->ent)
		find_next(this);
}

static void *dir_iterator_get_value(tl_iterator *super)
{
	dir_iterator *this = (dir_iterator *)super;

	return this->ent ? &this->current : NULL;
}

static void dir_iterator_remove(tl_iterator *this)
{
	(void)this;
}

tl_iterator *tl_dir_iterate(const char *path)
{
	dir_iterator *it;
	DIR *dir;

	if (!(dir = opendir(path)))
		return NULL;

	if (!(it = calloc(1, sizeof(*it)))) {
		closedir(dir);
		return NULL;
	}

	it->dir = dir;
	find_next(it);

	it->super.destroy = dir_iterator_destroy;
	it->super.reset = dir_iterator_reset;
	it->super.has_data = dir_iterator_has_data;
	it->super.next = dir_iterator_next;
	it->super.get_value = dir_iterator_get_value;
	it->super.remove = dir_iterator_remove;
	return (tl_iterator *)it;
}
