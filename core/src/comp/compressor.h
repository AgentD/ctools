/* compressor.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "tl_compress.h"

typedef struct {
	tl_compressor super;
	unsigned char *buffer;
	size_t total;
	size_t used;
} base_compressor;

void base_compressor_init(base_compressor *cmp);

void base_compressor_remove(base_compressor *cmp, size_t count);

int base_compressor_write(tl_iostream *stream, const void *buffer,
			  size_t size, size_t *actual);

#endif /* COMPRESSOR_H */

