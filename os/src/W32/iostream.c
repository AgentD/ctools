/* iostream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_iostream.h"
#include "os.h"

int __tl_os_splice(tl_iostream *out, tl_iostream *in,
		   size_t count, size_t *actual)
{
	(void)out;
	(void)in;
	(void)count;
	(void)actual;
	return TL_ERR_NOT_SUPPORTED;
}
