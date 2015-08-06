/*
 * fdstream.c
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
#include "os.h"
#include "tl_iostream.h"
#include <sys/select.h>
#include <sys/time.h>
#include <assert.h>

static void fd_stream_destroy( tl_iostream* super )
{
    fd_stream* this = (fd_stream*)super;
    assert( this );
    close( this->readfd );
    close( this->writefd );
    free( this );
}

static int fd_stream_set_timeout( tl_iostream* this, unsigned int timeout )
{
    assert( this );
    ((fd_stream*)this)->timeout = timeout;
    return 0;
}

static int fd_stream_write( tl_iostream* super, const void* buffer,
                            size_t size, size_t* actual )
{
    fd_stream* this = (fd_stream*)super;
    ssize_t result;

    assert( this && buffer );

    if( actual           ) *actual = 0;
    if( this->writefd<0  ) return TL_ERR_NOT_SUPPORTED;
    if( !size            ) return 0;

    if( !wait_for_fd( this->writefd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    result = write( this->writefd, buffer, size );

    if( result<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;
        if( errno==EBADF || errno==EINVAL || errno==EPIPE )
            return TL_ERR_CLOSED;
        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = result;
    return 0;
}

static int fd_stream_read( tl_iostream* super, void* buffer,
                           size_t size, size_t* actual )
{
    fd_stream* this = (fd_stream*)super;
    ssize_t result;

    assert( this && buffer );

    if( actual           ) *actual = 0;
    if( this->readfd<0   ) return TL_ERR_NOT_SUPPORTED;
    if( !size            ) return 0;

    if( !wait_for_fd( this->readfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    result = read( this->readfd, buffer, size );

    if( result==0 )
        return TL_ERR_CLOSED;

    if( result < 0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;

        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = result;
    return 0;
}

/****************************************************************************/

tl_iostream* pipe_stream_create( int readfd, int writefd, int flags )
{
    fd_stream* this = malloc( sizeof(fd_stream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    ((unix_stream*)this)->flags = flags;

    this->readfd       = readfd;
    this->writefd      = writefd;
    super->destroy     = fd_stream_destroy;
    super->set_timeout = fd_stream_set_timeout;
    super->write       = fd_stream_write;
    super->read        = fd_stream_read;
    return super;
}
