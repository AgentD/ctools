/* xfrm.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef XFRM_H
#define XFRM_H

#include "tl_transform.h"

typedef struct base_transform {
	tl_transform super;
	unsigned char *buffer;
	size_t total;
	size_t used;
	unsigned int eof : 1;

	int (*read)(struct base_transform *xfrm, void *buffer,
		    size_t size, size_t *actual);
} base_transform;

void base_transform_init(base_transform *cmp);

void base_transform_remove(base_transform *cmp, size_t count);

int base_transform_write(tl_iostream *stream, const void *buffer,
			 size_t size, size_t *actual);

#endif /* COMPRESSOR_H */

