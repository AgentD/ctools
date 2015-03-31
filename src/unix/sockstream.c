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
    struct timeval tv;

    tv.tv_sec  = timeout/1000;
    tv.tv_usec = timeout*1000 - tv.tv_sec*1000000;

    if( setsockopt(this->socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) < 0 )
        goto fail;

    if( setsockopt(this->socket,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv)) < 0 )
        goto fail;

    return 0;
fail:
    if( errno==EBADF || errno==ENOTSOCK )
        return TL_IO_CLOSED;
    if( errno==EINVAL || errno==ENODEV || errno==ENXIO )
        return TL_IO_NOT_SUPPORTED;
    return TL_IO_INTERNAL;
}

static int cl_stream_write_raw( tl_iostream* super, const void* buffer,
                                size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    ssize_t bytes;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( !size )
    {
        if( actual )
            *actual = 0;
        return 0;
    }

    bytes = write( ((cl_stream*)this)->socket, buffer, size );

    if( bytes<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
        {
            if( actual )
                *actual = 0;
            return TL_IO_TIMEOUT;
        }
        if( errno==EBADF || errno==EINVAL )
            return TL_IO_CLOSED;
        return TL_IO_INTERNAL;
    }

    if( actual )
        *actual = bytes;
    return 0;
}

static int cl_stream_write( tl_iostream* this, const tl_blob* blob,
                            size_t* actual )
{
    if( !blob )
        return TL_IO_INTERNAL;

    return cl_stream_write_raw( this, blob->data, blob->size, actual );
}

static int cl_stream_read_raw( tl_iostream* super, void* buffer,
                               size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    ssize_t bytes;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( !size )
    {
        if( actual )
            *actual = 0;
        return 0;
    }

    bytes = read( this->socket, buffer, size );

    if( bytes==0 )
        return TL_IO_CLOSED;

    if( bytes < 0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
        {
            if( actual )
                *actual = 0;
            return TL_IO_TIMEOUT;
        }

        return TL_IO_INTERNAL;
    }

    if( actual )
        *actual = bytes;
    return 0;
}

static int cl_stream_read( tl_iostream* this, tl_blob* blob,
                           size_t maximum )
{
    int status;

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_IO_INTERNAL;

    status = cl_stream_read_raw( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
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
    super->write_raw   = cl_stream_write_raw;
    super->write       = cl_stream_write;
    super->read_raw    = cl_stream_read_raw;
    super->read        = cl_stream_read;
    return super;
}

