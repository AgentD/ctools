/*
 * sockstream.c
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



typedef struct
{
    tl_iostream super;
    int socket;
}
cl_stream;



static void cl_stream_destroy( tl_iostream* super )
{
    cl_stream* this = (cl_stream*)super;
    close( this->socket );
    free( this );
}

static int cl_stream_set_timeout( tl_iostream* super, unsigned int timeout )
{
    cl_stream* this = (cl_stream*)super;
    unsigned long sec = timeout/1000;
    unsigned long usec = (timeout - sec*1000)*1000;
    struct timeval tv;

    tv.tv_sec = sec; tv.tv_usec = usec;
    if( setsockopt(this->socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) < 0 )
        goto fail;

    tv.tv_sec = sec; tv.tv_usec = usec;
    if( setsockopt(this->socket,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv)) < 0 )
        goto fail;

    return 0;
fail:
    if( errno==EBADF || errno==ENOTSOCK )
        return TL_ERR_CLOSED;
    if( errno==EINVAL || errno==ENODEV || errno==ENXIO )
        return TL_ERR_NOT_SUPPORTED;
    return TL_ERR_INTERNAL;
}

static int cl_stream_write_raw( tl_iostream* super, const void* buffer,
                                size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    ssize_t bytes;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_ERR_INTERNAL;

    if( !size )
        return 0;

    bytes = write( ((cl_stream*)this)->socket, buffer, size );

    if( bytes<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;
        if( errno==EBADF || errno==EINVAL )
            return TL_ERR_CLOSED;
        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = bytes;
    return 0;
}

static int cl_stream_read_raw( tl_iostream* super, void* buffer,
                               size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    ssize_t bytes;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_ERR_INTERNAL;

    if( !size )
        return 0;

    bytes = read( this->socket, buffer, size );

    if( bytes==0 )
        return TL_ERR_CLOSED;

    if( bytes < 0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;

        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = bytes;
    return 0;
}

/****************************************************************************/

tl_iostream* sock_stream_create( int sockfd )
{
    cl_stream* this = malloc( sizeof(cl_stream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    this->socket       = sockfd;
    super->destroy     = cl_stream_destroy;
    super->set_timeout = cl_stream_set_timeout;
    super->write       = cl_stream_write_raw;
    super->read        = cl_stream_read_raw;
    return super;
}

