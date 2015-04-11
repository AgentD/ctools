/*
 * pipestream.c
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
#define TL_EXPORT
#include "os.h"
#include "tl_iostream.h"
#include <sys/select.h>
#include <sys/time.h>



typedef struct
{
    tl_iostream super;
    int readpipe;
    int writepipe;
    unsigned int timeout;
}
pipe_stream;



static void pipe_stream_destroy( tl_iostream* super )
{
    pipe_stream* this = (pipe_stream*)super;
    close( this->readpipe );
    close( this->writepipe );
    free( this );
}

static int pipe_stream_set_timeout( tl_iostream* this, unsigned int timeout )
{
    if( this )
        ((pipe_stream*)this)->timeout = timeout;
    return 0;
}

static int pipe_stream_write( tl_iostream* super, const void* buffer,
                              size_t size, size_t* actual )
{
    pipe_stream* this = (pipe_stream*)super;
    struct timeval tv;
    ssize_t result;
    fd_set out_fds;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( this->writepipe<0 )
        return TL_IO_NOT_SUPPORTED;

    if( !size )
        return 0;

    if( this->timeout )
    {
        FD_ZERO( &out_fds );
        FD_SET( this->writepipe, &out_fds );

        tv.tv_sec = this->timeout/1000;
        tv.tv_usec = (this->timeout - tv.tv_sec*1000)*1000;
        result = select( this->writepipe+1, 0, &out_fds, 0, &tv );

        if( result<0 )
            return TL_IO_INTERNAL;
        if( result==0 || !FD_ISSET(this->writepipe,&out_fds) )
            return TL_IO_TIMEOUT;
    }

    result = write( this->writepipe, buffer, size );

    if( result<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_IO_TIMEOUT;
        if( errno==EBADF || errno==EINVAL || errno==EPIPE )
            return TL_IO_CLOSED;
        return TL_IO_INTERNAL;
    }

    if( actual )
        *actual = result;
    return 0;
}

static int pipe_stream_read( tl_iostream* super, void* buffer,
                             size_t size, size_t* actual )
{
    pipe_stream* this = (pipe_stream*)super;
    struct timeval tv;
    ssize_t result;
    fd_set in_fds;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( this->readpipe<0 )
        return TL_IO_NOT_SUPPORTED;

    if( !size )
        return 0;

    if( this->timeout )
    {
        FD_ZERO( &in_fds );
        FD_SET( this->readpipe, &in_fds );

        tv.tv_sec = this->timeout/1000;
        tv.tv_usec = (this->timeout - tv.tv_sec*1000)*1000;
        result = select( this->readpipe+1, &in_fds, 0, 0, &tv );

        if( result<0 )
            return TL_IO_INTERNAL;
        if( result==0 || !FD_ISSET(this->readpipe, &in_fds) )
            return TL_IO_TIMEOUT;
    }

    result = read( this->readpipe, buffer, size );

    if( result==0 )
        return TL_IO_CLOSED;

    if( result<0 )
        return TL_IO_INTERNAL;

    if( actual )
        *actual = result;
    return 0;
}

/****************************************************************************/

tl_iostream* pipe_stream_create( int readpipe, int writepipe )
{
    pipe_stream* this = malloc( sizeof(pipe_stream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    this->readpipe     = readpipe;
    this->writepipe    = writepipe;
    super->destroy     = pipe_stream_destroy;
    super->set_timeout = pipe_stream_set_timeout;
    super->write       = pipe_stream_write;
    super->read        = pipe_stream_read;
    return super;
}

