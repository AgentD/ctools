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




static void sockstream_destroy( tl_iostream* super )
{
    sockstream* this = (sockstream*)super;
    if( this )
    {
        closesocket( this->socket );
        free( this );
        winsock_release( );
    }
}

static int sockstream_set_timeout( tl_iostream* super, unsigned int timeout )
{
    sockstream* this = (sockstream*)super;
    int status;
    DWORD ms;

    if( !this )
        return TL_ERR_ARG;

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
        return TL_ERR_CLOSED;
    if( status==WSAENOPROTOOPT || status==WSAEINVAL )
        return TL_ERR_NOT_SUPPORTED;
    return TL_ERR_INTERNAL;
}

static int sockstream_write_raw( tl_iostream* super, const void* buffer,
                                 size_t size, size_t* actual )
{
    sockstream* this = (sockstream*)super;
    int status;

    if( actual )
        *actual = 0;
    if( !this || !buffer )
        return TL_ERR_ARG;
    if( !size )
        return 0;

    status = send( ((sockstream*)this)->socket, buffer, size, 0 );

    if( status<0 )
        return WSAHandleFuckup( );

    if( actual )
        *actual = status;
    return 0;
}

static int sockstream_read_raw( tl_iostream* super, void* buffer,
                                size_t size, size_t* actual )
{
    sockstream* this = (sockstream*)super;
    int status;

    if( actual )
        *actual = 0;
    if( !this || !buffer )
        return TL_ERR_ARG;
    if( !size )
        return 0;

    status = recv( this->socket, buffer, size, 0 );

    if( status==0 )
        return TL_ERR_CLOSED;

    if( status<0 )
        return WSAHandleFuckup( );

    if( actual )
        *actual = status;
    return 0;
}

/****************************************************************************/

tl_iostream* sock_stream_create( SOCKET sockfd, int flags )
{
    sockstream* this = malloc( sizeof(sockstream) );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    ((w32stream*)this)->flags = flags;

    this->socket       = sockfd;
    super->destroy     = sockstream_destroy;
    super->set_timeout = sockstream_set_timeout;
    super->write       = sockstream_write_raw;
    super->read        = sockstream_read_raw;
    return super;
}

