/* iostream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_iostream.h"
#include "tl_unix.h"
#include "os.h"

#ifdef __linux__
#include <sys/sendfile.h>

int __tl_os_splice( tl_iostream* out, tl_iostream* in,
                    size_t count, size_t* actual )
{
    int infd, outfd, fds[2];
    ssize_t res = -1;
    off_t old = 0;

    /* get fds */
    tl_unix_iostream_fd( in, fds );
    infd = fds[0];

    tl_unix_iostream_fd( out, fds );
    outfd = fds[1];

    if( infd==-1 || outfd==-1 )
        return TL_ERR_NOT_SUPPORTED;

    if( !wait_for_fd( infd, ((fd_stream*)in)->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    if( !wait_for_fd( outfd, ((fd_stream*)out)->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    /* splice */
    if( out->type == TL_STREAM_TYPE_FILE &&
        (((file_stream*)out)->flags & TL_APPEND) )
    {
        old = lseek( outfd, 0, SEEK_END );
        if( old == (off_t)-1 )
            return TL_ERR_INTERNAL;
    }

    if( (in->type==TL_STREAM_TYPE_PIPE) || (out->type==TL_STREAM_TYPE_PIPE) )
        res = splice( infd, NULL, outfd, NULL, count, SPLICE_F_MOVE );
    else if( in->type == TL_STREAM_TYPE_FILE )
        res = sendfile( outfd, infd, NULL, count );

    if( out->type == TL_STREAM_TYPE_FILE &&
        (((file_stream*)out)->flags & TL_APPEND) )
    {
        lseek( outfd, old, SEEK_SET );
    }

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
    fds[0] = -1;
    fds[1] = -1;

    switch( str->type )
    {
    case TL_STREAM_TYPE_PIPE:
    case TL_STREAM_TYPE_FILE:
        fds[0] = (((file_stream*)str)->flags & TL_READ) ?
                 ((file_stream*)str)->fd : -1;
        fds[1] = (((file_stream*)str)->flags & TL_WRITE) ?
        	 ((file_stream*)str)->fd : -1;
        break;
    case TL_STREAM_TYPE_SOCK:
        fds[0] = ((fd_stream*)str)->readfd;
        fds[1] = ((fd_stream*)str)->writefd;
        break;
    }
}

