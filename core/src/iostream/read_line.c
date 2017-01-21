/* read_line.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iostream.h"
#include "tl_string.h"

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

