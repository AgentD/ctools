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

#include <assert.h>



int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual )
{
    size_t indiff, outdiff, outcount = 0;
    char buffer[ 1024 ];
    int res = 0;

    assert( out && in );

    if( actual )
        *actual = 0;

    if( !count )
        return 0;

    while( count )
    {
        res = in->read( in, buffer, sizeof(buffer), &indiff );

        if( res!=0 )
            break;

        while( indiff )
        {
            res = out->write( out, buffer, indiff, &outdiff );

            if( res!=0 )
                break;

            indiff -= outdiff;
            count -= outdiff;
            outcount += outdiff;
        }
    }

    if( *actual )
        *actual = outcount;

    return res;
}

