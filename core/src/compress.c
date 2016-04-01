/*
 * compress.c
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define TL_EXPORT
#include "tl_compress.h"

#include <string.h>
#include <stdlib.h>

#ifdef TL_HAVE_DEFLATE
    #include <zlib.h>
#endif



#ifdef TL_HAVE_DEFLATE
static int do_deflate(tl_blob* dst, const void* data, size_t size, int level)
{
    z_stream strm;

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

static int do_inflate( tl_blob* dst, const void* data, size_t size )
{
    char out[1024];
    z_stream strm;
    tl_array temp;
    void* fit;
    int ret;

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
#endif

/****************************************************************************/

int tl_compress( tl_blob* dst, const void* src, size_t size,
                 int algo, int flags )
{
    assert( dst );

    if( !src || !size )
    {
        memset( dst, 0, sizeof(*dst) );
        return 0;
    }

    switch( algo )
    {
#ifdef TL_HAVE_DEFLATE
    case TL_DEFLATE:
        if( flags & TL_COMPRESS_GOOD )
            return do_deflate( dst, src, size, Z_BEST_COMPRESSION );

        if( flags & TL_COMPRESS_FAST )
            return do_deflate( dst, src, size, Z_BEST_SPEED );

        return do_deflate( dst, src, size, Z_DEFAULT_COMPRESSION );
#endif
    default:
        break;
    }

    return TL_ERR_NOT_SUPPORTED;
}

int tl_uncompress( tl_blob* dst, const void* src, size_t size, int algo )
{
    assert( dst );

    if( !src || !size )
    {
        memset( dst, 0, sizeof(*dst) );
        return 0;
    }

    switch( algo )
    {
#ifdef TL_HAVE_DEFLATE
    case TL_DEFLATE:
        return do_inflate( dst, src, size );
#endif
    default:
        break;
    }

    return TL_ERR_NOT_SUPPORTED;
}

int tl_uncompress_string( tl_string* dst, const void* src,
                          size_t size, int algo )
{
    char null = 0;
    tl_blob temp;
    size_t i;
    int ret;

    /* uncompress data block */
    ret = tl_uncompress( &temp, src, size, algo );

    if( ret != 0 )
        return ret;

    /* "convert" blob to array, append null terminator */
    memset( dst, 0, sizeof(*dst) );

    dst->data.reserved = dst->data.used = temp.size;
    dst->data.unitsize = 1;
    dst->data.data = temp.data;

    if( !tl_array_append( &dst->data, &null ) )
    {
        tl_array_cleanup( &dst->data );
        return TL_ERR_ALLOC;
    }

    /* fix string */
    for( i = 0; i < dst->data.used; ++i )
    {
        if( ((unsigned char*)dst->data.data)[i] & 0x80 )
            break;
    }

    dst->mbseq = i;
    dst->charcount = tl_utf8_charcount( dst->data.data );
    return 0;
}

