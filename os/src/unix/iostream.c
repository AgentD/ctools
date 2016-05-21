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

#ifdef __linux__
int __tl_os_splice( tl_iostream* out, tl_iostream* in,
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

    /* let the fallback implementation retry and figure that out */
    if( res <= 0 )
        return TL_ERR_NOT_SUPPORTED;

    if( actual )
        *actual = res;
    return 0;
}
#else /* __linux__ */
int __tl_os_splice( tl_iostream* out, tl_iostream* in,
                    size_t count, size_t* actual )
{
    (void)out; (void)in; (void)count; (void)actual;
    return TL_ERR_NOT_SUPPORTED;
}
#endif

void tl_unix_iostream_fd( tl_iostream* str, int* fds )
{
    int type = str->flags & TL_STREAM_TYPE_MASK;

    fds[0] = -1;
    fds[1] = -1;

    switch( type )
    {
    case TL_STREAM_TYPE_PIPE:
    case TL_STREAM_TYPE_FILE:
    case TL_STREAM_TYPE_SOCK:
        fds[0] = ((fd_stream*)str)->readfd;
        fds[1] = ((fd_stream*)str)->writefd;
        break;
    }
}

