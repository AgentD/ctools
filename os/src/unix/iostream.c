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
#include "os.h"

#include <assert.h>


#ifdef __linux__
static int try_splice( tl_iostream* out, tl_iostream* in,
                       size_t count, size_t* actual )
{
    int outtype, intype, infd, outfd;
    ssize_t res;

    outtype = out->flags & TL_STREAM_TYPE_MASK;
    intype  = in->flags  & TL_STREAM_TYPE_MASK;

    if( (intype != TL_STREAM_TYPE_PIPE) && (outtype != TL_STREAM_TYPE_PIPE) )
        return TL_ERR_NOT_SUPPORTED;

    switch( intype )
    {
    case TL_STREAM_TYPE_PIPE:
    case TL_STREAM_TYPE_FILE:
    case TL_STREAM_TYPE_SOCK:
        infd = ((fd_stream*)in)->readfd;
        if( !wait_for_fd( infd, ((fd_stream*)in)->timeout, 0 ) )
            return TL_ERR_TIMEOUT;
        break;
    default:
        return TL_ERR_NOT_SUPPORTED;
    }

    switch( outtype )
    {
    case TL_STREAM_TYPE_PIPE:
    case TL_STREAM_TYPE_FILE:
    case TL_STREAM_TYPE_SOCK:
        outfd = ((fd_stream*)out)->writefd;
        if( !wait_for_fd( infd, ((fd_stream*)out)->timeout, 1 ) )
            return TL_ERR_TIMEOUT;
        break;
    default:
        return TL_ERR_NOT_SUPPORTED;
    }

    res = splice( infd, NULL, outfd, NULL, count, SPLICE_F_MOVE );

    /* let the fallback implementation retry figure that out */
    if( res <= 0 )
        return TL_ERR_NOT_SUPPORTED;

    if( actual )
        *actual = res;
    return 0;
}
#endif  /* __linux__ */

static int copy_data( tl_iostream* out, tl_iostream* in,
                      size_t count, size_t* actual )
{
    size_t indiff, outdiff, outcount = 0;
    char buffer[ 1024 ];
    int res = 0;

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

int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual )
{
    int res;

    assert( out && in );

    if( actual )
        *actual = 0;

    if( !count )
        return 0;

#ifdef __linux__
    res = try_splice( out, in, count, actual );

    if( res != TL_ERR_NOT_SUPPORTED )
        return res;
#endif

    res = copy_data( out, in, count, actual );
    return res;
}

