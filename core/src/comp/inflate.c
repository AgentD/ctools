/* inflate.c -- This file is part of ctools
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

int tl_inflate( tl_blob* dst, const void* data, size_t size )
{
    char out[1024];
    z_stream strm;
    tl_array temp;
    void* fit;
    int ret;

    assert( dst );

    if( !data || !size )
    {
        memset( dst, 0, sizeof(*dst) );
        return 0;
    }

    memset( &strm, 0, sizeof(strm) );

    if( inflateInit( &strm ) != Z_OK )
        return TL_ERR_INTERNAL;

    tl_array_init( &temp, 1, NULL );

    strm.next_in = (void*)data;
    strm.avail_in = size;

    do
    {
        strm.next_out = (void*)out;
        strm.avail_out = sizeof(out);

        ret = inflate( &strm, Z_NO_FLUSH );

        if( ret != Z_OK && ret != Z_STREAM_END )
        {
            ret = TL_ERR_INTERNAL;
            goto fail;
        }

        ret = tl_array_append_array( &temp, out,
                                     sizeof(out) - strm.avail_out );

        if( !ret )
        {
            ret = TL_ERR_ALLOC;
            goto fail;
        }
    }
    while( strm.avail_out == 0 );

    inflateEnd( &strm );

    dst->data = temp.data;
    dst->size = temp.used;

    if( temp.used < temp.reserved )
    {
        fit = realloc( dst->data, dst->size );
        if( fit )
            dst->data = fit;
    }
    return 0;
fail:
    tl_array_cleanup( &temp );
    inflateEnd( &strm );
    return ret;
}

