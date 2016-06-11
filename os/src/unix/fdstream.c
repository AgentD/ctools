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

static void fd_stream_destroy( tl_iostream* super )
{
    fd_stream* this = (fd_stream*)super;
    assert( this );
    if( this->readfd >= 0 )
        close( this->readfd );
    if( (this->readfd != this->writefd) && (this->writefd >= 0) )
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
    ssize_t result, intr_count = 0;
    off_t old = 0;

    assert( this && buffer );

    if( actual           ) *actual = 0;
    if( this->writefd<0  ) return TL_ERR_NOT_SUPPORTED;
    if( !size            ) return 0;

retry:
    if( !wait_for_fd( this->writefd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    if( super->flags & TL_STREAM_APPEND )
    {
        old = lseek( this->writefd, 0, SEEK_END );
        if( old == (off_t)-1 )
            return TL_ERR_INTERNAL;
    }

    if( (super->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_SOCK )
        result = sendto( this->writefd, buffer, size, MSG_NOSIGNAL, NULL, 0 );
    else
        result = write( this->writefd, buffer, size );

    if( result<0 && errno==EINTR && (intr_count++)<3 )
        goto retry;

    if( super->flags & TL_STREAM_APPEND )
        lseek( this->writefd, old, SEEK_SET );

    if( result<0 )
    {
        if( errno==EBADF || errno==EINVAL || errno==EPIPE )
            return TL_ERR_CLOSED;
        return errno_to_fs( errno );
    }

    if( actual )
        *actual = result;
    return 0;
}

static int fd_stream_read( tl_iostream* super, void* buffer,
                           size_t size, size_t* actual )
{
    fd_stream* this = (fd_stream*)super;
    ssize_t result, intr_count = 0;

    assert( this && buffer );

    if( actual           ) *actual = 0;
    if( this->readfd<0   ) return TL_ERR_NOT_SUPPORTED;
    if( !size            ) return 0;

retry:
    if( !wait_for_fd( this->readfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    if( (super->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_SOCK )
        result = recvfrom(this->readfd, buffer,size,MSG_NOSIGNAL,NULL,NULL);
    else
        result = read( this->readfd, buffer, size );

    if( result<0 && errno==EINTR && (intr_count++)<3 )
        goto retry;

    if( result==0 )
    {
        if( (super->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_FILE )
            return TL_EOF;
        return TL_ERR_CLOSED;
    }

    if( result < 0 )
        return errno_to_fs( errno );

    if( actual )
        *actual = result;
    return 0;
}

/****************************************************************************/

fd_stream tl_stdio =
{
    {
        TL_STREAM_TYPE_FILE,
        NULL,
        fd_stream_set_timeout,
        fd_stream_write,
        fd_stream_read
    },
    STDIN_FILENO,
    STDOUT_FILENO,
    0
};

fd_stream tl_stderr =
{
    {
        TL_STREAM_TYPE_FILE,
        NULL,
        fd_stream_set_timeout,
        fd_stream_write,
        fd_stream_read
    },
    -1,
    STDERR_FILENO,
    0
};

/****************************************************************************/

tl_iostream* pipe_stream_create( int readfd, int writefd, int flags )
{
    fd_stream* this = calloc( 1, sizeof(fd_stream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    this->readfd       = readfd;
    this->writefd      = writefd;
    super->flags       = flags;
    super->destroy     = fd_stream_destroy;
    super->set_timeout = fd_stream_set_timeout;
    super->write       = fd_stream_write;
    super->read        = fd_stream_read;
    return super;
}

