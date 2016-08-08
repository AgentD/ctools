/* iostream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_iostream.h"
#include "tl_string.h"
#include "platform.h"

#include <stdarg.h>
#include <stdio.h>



int tl_iostream_read_blob( tl_iostream* this, tl_blob* blob,
                           size_t maximum )
{
    int status;

    assert( this && blob );

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_ERR_ALLOC;

    status = this->read( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
}

int tl_iostream_read_line( tl_iostream* this, tl_string* line, int flags )
{
    unsigned char c[4];
    size_t actual, len;
    int status;

    assert( this && line );

    if( !tl_string_init( line ) )
        return TL_ERR_ALLOC;

    while( 1 )
    {
        status = this->read( this, c, 1, &actual );

        if( status!=0 )
            goto statfail;

        if( actual!=1 )
        {
            status = TL_ERR_INTERNAL;
            goto fail;
        }

        if( c[0]=='\n' )
            break;

        if( flags & TL_LINE_READ_UTF8 )
        {
                 if( (c[0] & 0x80)==0x00 ) { len = 0;  }
            else if( (c[0] & 0xE0)==0xC0 ) { len = 1;  }
            else if( (c[0] & 0xF0)==0xE0 ) { len = 2;  }
            else if( (c[0] & 0xF8)==0xF0 ) { len = 3;  }
            else                           { continue; }

            if( len )
            {
                status = this->read( this, c+1, len, &actual );
                if( status!=0 )
                    goto statfail;
                if( actual!=len )
                {
                    status = TL_ERR_INTERNAL;
                    goto fail;
                }
            }

            if( !tl_string_append_utf8_count( line, (char*)c, len+1 ) )
            {
                status = TL_ERR_ALLOC;
                goto fail;
            }
        }
        else if( !tl_string_append_latin1_count( line, (char*)c, 1 ) )
        {
            status = TL_ERR_ALLOC;
            goto fail;
        }
    }

    return status;
statfail:
    if( (status==TL_ERR_CLOSED||status==TL_EOF) && !tl_string_is_empty(line) )
        return 0;
fail:
    tl_string_cleanup( line );
    return status;
}

int tl_iostream_printf( tl_iostream* this, const char* format, ... )
{
    char* buffer;
    va_list ap;
    int size, status;
    size_t actual;

    assert( this && format );

    /* determine required buffer size and allocate */
    va_start( ap, format );
    size = vsnprintf( NULL, 0, format, ap );
    va_end( ap );

    if( size<0 )
        return TL_ERR_INTERNAL;

    if( !(buffer = malloc(size+1)) )
        return TL_ERR_ALLOC;

    /* write to buffer */
    va_start( ap, format );
    vsnprintf( buffer, size+1, format, ap );
    va_end( ap );

    /* send buffer and cleanup */
    status = this->write( this, buffer, size, &actual );

    if( actual!=(size_t)size && status==0 )
        status = TL_ERR_INTERNAL;

    free( buffer );
    return status;
}

/* fallback for tl_iostream_splice */
static int splice_copy( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual )
{
    size_t indiff, outdiff, outcount = 0;
    char buffer[ 1024 ];
    int res = 0;

    while( count )
    {
        indiff = count > sizeof(buffer) ? sizeof(buffer) : count;
        res = in->read( in, buffer, indiff, &indiff );

        if( res!=0 )
            break;

        while( indiff )
        {
            res = out->write( out, buffer, indiff, &outdiff );

            if( res!=0 )
                goto out;

            indiff -= outdiff;
            count -= outdiff;
            outcount += outdiff;
        }
    }
out:
    if( *actual )
        *actual = outcount;

    return res;
}

int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual, int flags )
{
    int res;

    assert( out && in );

    if( actual )
        *actual = 0;

    if( !count )
        return 0;

    if( flags & (~TL_SPLICE_ALL_FLAGS) )
        return TL_ERR_ARG;

    res = __tl_os_splice( out, in, count, actual );

    if( (res == TL_ERR_NOT_SUPPORTED) && !(flags & TL_SPLICE_NO_FALLBACK) )
        res = splice_copy( out, in, count, actual );

    return res;
}

