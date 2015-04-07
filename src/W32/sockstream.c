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
    SOCKET socket;
}
cl_stream;



static int WSAHandleFuckup( void )
{
    int status = WSAGetLastError( );

    if( status==WSAETIMEDOUT || status==WSAEWOULDBLOCK )
        return TL_IO_TIMEOUT;
    if( status==WSAECONNRESET || status==WSAECONNABORTED ||
        status==WSAESHUTDOWN || status==WSAENOTSOCK ||
        status==WSAENOTCONN || status==WSAENETRESET )
    {
        return TL_IO_CLOSED;
    }
    return TL_IO_INTERNAL;
}



static void cl_stream_destroy( tl_iostream* super )
{
    cl_stream* this = (cl_stream*)super;
    closesocket( this->socket );
    free( this );
    winsock_release( );
}

static int cl_stream_set_timeout( tl_iostream* super, unsigned int timeout )
{
    cl_stream* this = (cl_stream*)super;
    int status;
    DWORD ms;

    ms = timeout;
    status = setsockopt(this->socket,SOL_SOCKET,SO_RCVTIMEO,
                        (const char*)&ms,sizeof(ms));
    if( status != 0 )
        goto fail;

    ms = timeout;
    status = setsockopt(this->socket,SOL_SOCKET,SO_SNDTIMEO,
                        (const char*)&ms,sizeof(ms));
    if( status != 0 )
        goto fail;

    return 0;
fail:
    status = WSAGetLastError( );
    if( status==WSAENOTCONN || status==WSAENOTSOCK || status==WSAENETRESET )
        return TL_IO_CLOSED;
    if( status==WSAENOPROTOOPT || status==WSAEINVAL )
        return TL_IO_NOT_SUPPORTED;
    return TL_IO_INTERNAL;
}

static int cl_stream_write_raw( tl_iostream* super, const void* buffer,
                                size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    int status;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( !size )
        return 0;

    status = send( ((cl_stream*)this)->socket, buffer, size, 0 );

    if( status<0 )
        return WSAHandleFuckup( );

    if( actual )
        *actual = status;
    return 0;
}

static int cl_stream_read_raw( tl_iostream* super, void* buffer,
                               size_t size, size_t* actual )
{
    cl_stream* this = (cl_stream*)super;
    int status;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( !size )
        return 0;

    status = recv( this->socket, buffer, size, 0 );

    if( status==0 )
        return TL_IO_CLOSED;

    if( status<0 )
        return WSAHandleFuckup( );

    if( actual )
        *actual = status;
    return 0;
}

/****************************************************************************/

tl_iostream* sock_stream_create( SOCKET sockfd )
{
    cl_stream* this = malloc( sizeof(cl_stream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    this->socket       = sockfd;
    super->destroy     = cl_stream_destroy;
    super->set_timeout = cl_stream_set_timeout;
    super->write_raw   = cl_stream_write_raw;
    super->read_raw    = cl_stream_read_raw;
    super->write       = stream_write_blob;
    super->read        = stream_read_blob;
    return super;
}

