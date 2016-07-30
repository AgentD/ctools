/* deflate.c -- This file is part of ctools
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

#include <zlib.h>

int tl_deflate( tl_blob* dst, const void* data, size_t size, int flags )
{
    int level = Z_DEFAULT_COMPRESSION;
    z_stream strm;

    assert( dst );

    if( !data || !size )
    {
        memset( dst, 0, sizeof(*dst) );
        return 0;
    }

    if( flags & TL_COMPRESS_GOOD )
        level = Z_BEST_COMPRESSION;
    else if( flags & TL_COMPRESS_FAST )
        level = Z_BEST_SPEED;

    memset( &strm, 0, sizeof(strm) );

    if( deflateInit( &strm, level ) != Z_OK )
        return TL_ERR_INTERNAL;

    dst->size = deflateBound( &strm, size );
    dst->data = malloc( dst->size );

    if( !dst->data )
    {
        deflateEnd( &strm );
        return TL_ERR_ALLOC;
    }

    strm.next_in = (void*)data;
    strm.avail_in = size;
    strm.next_out = dst->data;
    strm.avail_out = dst->size;

    if( deflate( &strm, Z_FINISH ) != Z_STREAM_END )
    {
        deflateEnd( &strm );
        free( dst->data );
        return TL_ERR_INTERNAL;
    }

    deflateEnd( &strm );
    dst->size = (Bytef*)strm.next_out - (Bytef*)dst->data;
    return 0;
}

