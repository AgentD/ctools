/* comp_blob.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_compress.h"

int tl_compress_blob(tl_blob * dst, const tl_blob * src, int algo, int flags)
{
	tl_compressor *comp = tl_create_compressor(algo, flags);
	const unsigned char *ptr;
	size_t actual, total;
	char buffer[1024];
	int ret;

	if (!comp)
		return TL_ERR_NOT_SUPPORTED;

	total = src->size;
	ptr = (const unsigned char *)src->data;

	tl_blob_init(dst, 0, NULL);

	while (total) {
		/* stuff into compressor */
		ret = ((tl_iostream *) comp)->write((tl_iostream *) comp, ptr,
						    total > 4096 ? 4096 : total,
						    &actual);

		if (ret)
			goto fail;

		ptr += actual;
		total -= actual;

		if (total == 0) {
			ret = comp->flush(comp, TL_COMPRESS_FLUSH_EOF);
			if (ret && ret != TL_ERR_NOT_SUPPORTED)
				goto fail;
		}

		/* pull out of compressor */
		do {
			ret =
			    ((tl_iostream *) comp)->read((tl_iostream *) comp,
							 buffer, sizeof(buffer),
							 &actual);

			if (ret < 0 && ret != TL_EOF)
				goto fail;

			if (!tl_blob_append_raw(dst, buffer, actual)) {
				ret = TL_ERR_ALLOC;
				goto fail;
			}
		}
		while (ret != TL_EOF && actual > 0);
	}

	/* cleanup */
	((tl_iostream *) comp)->destroy((tl_iostream *) comp);
	return 0;
 fail:
	tl_blob_cleanup(dst);
	((tl_iostream *) comp)->destroy((tl_iostream *) comp);
	return ret;
}
