/* printf.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iostream.h"

#include <stdarg.h>
#include <stdio.h>

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

