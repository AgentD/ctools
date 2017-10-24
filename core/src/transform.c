/* transform.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_transform.h"

#include <string.h>
#include <stdlib.h>

tl_transform *tl_create_transform(int algo, int flags)
{
	(void)flags;

	switch (algo) {
#ifdef TL_HAVE_DEFLATE
	case TL_DEFLATE:
		return tl_deflate(flags);
	case TL_INFLATE:
		return tl_inflate(flags);
#endif
	case TL_BASE64_ENCODE:
		return tl_base64_encode(flags);
	case TL_BASE64_DECODE:
		return tl_base64_decode(flags);
	default:
		break;
	}

	return NULL;
}
