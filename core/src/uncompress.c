/* compress.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_compress.h"

#include <string.h>
#include <stdlib.h>


int tl_uncompress( tl_blob* dst, const void* src, size_t size, int algo )
{
    switch( algo )
    {
#ifdef TL_HAVE_DEFLATE
    case TL_DEFLATE:
        return tl_inflate( dst, src, size );
#endif
    default:
        break;
    }

    return TL_ERR_NOT_SUPPORTED;
}

