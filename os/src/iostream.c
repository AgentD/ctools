/*
 * iostream.c
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
#define TL_OS_EXPORT
#include "tl_iostream.h"
#include "tl_string.h"

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
    if( status==TL_ERR_CLOSED && !(flags & TL_LINE_READ_FAIL_ON_EOF) )
    {
        if( !tl_string_is_empty( line ) )
            return 0;
    }
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

