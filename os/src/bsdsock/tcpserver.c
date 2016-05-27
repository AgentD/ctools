/*
 * tcpserver.c
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
#include "tl_server.h"
#include "sock.h"
#include "os.h"

static void tcp_destroy( tl_server* this )
{
    assert( this );
    closesocket( ((tcp_server*)this)->socket );
    free( this );
    winsock_release( );
}

static tl_iostream* tcp_wait_for_client( tl_server* super, int timeout )
{
    struct sockaddr_storage addr;
    struct sockaddr_in6* v6;
    tcp_server* this = (tcp_server*)super;
    socklen_t len = sizeof(addr);
    int x, y, flags = TL_STREAM_TYPE_SOCK|TL_STREAM_TCP;
    SOCKET peer;

    assert( this );

    if( !wait_for_fd( this->socket, timeout, 0 ) )
        return NULL;

    peer = accept( this->socket, 0, 0 );
    if( peer == INVALID_SOCKET )
        return NULL;

    if( this->flags & TL_ENFORCE_V6_ONLY )
    {
        if( getpeername( peer, (void*)&addr, &len ) != 0 )
            goto ignore;
        if( addr.ss_family != AF_INET6 )
            goto ignore;

        v6 = (struct sockaddr_in6*)&addr;

        x = v6->sin6_addr.s6_addr[0] | v6->sin6_addr.s6_addr[1] |
            v6->sin6_addr.s6_addr[2] | v6->sin6_addr.s6_addr[3] |
            v6->sin6_addr.s6_addr[4] | v6->sin6_addr.s6_addr[5] |
            v6->sin6_addr.s6_addr[6] | v6->sin6_addr.s6_addr[7] |
            v6->sin6_addr.s6_addr[8] | v6->sin6_addr.s6_addr[9];

        y = v6->sin6_addr.s6_addr[10] & v6->sin6_addr.s6_addr[11];

        if( x==0 && y==0xFF )
            goto ignore;
    }

    return peer==INVALID_SOCKET ? NULL : sock_stream_create( peer, flags );
ignore:
    closesocket( peer );
    return NULL;
}

/****************************************************************************/

tl_server* tcp_server_create( const tl_net_addr* addr, unsigned int backlog,
                              int flags )
{
    struct sockaddr_storage addrbuffer;
    tcp_server* this;
    tl_server* super;
    socklen_t size;
    SOCKET sockfd;

    if( !winsock_acquire( ) )
        return NULL;

    if( !encode_sockaddr( addr, &addrbuffer, &size ) )
        return NULL;

    sockfd = create_socket( addr->net, addr->transport );
    if( sockfd == INVALID_SOCKET )
        goto fail;

    if( !set_socket_flags( sockfd, addr->net, &flags ) )
        goto failclose;

    if( bind( sockfd, (void*)&addrbuffer, size ) < 0 )
        goto failclose;

    if( listen( sockfd, backlog ) == SOCKET_ERROR )
        goto failclose;

    this = calloc( 1, sizeof(tcp_server) );
    super = (tl_server*)this;
    if( !this )
        goto fail;

    this->flags            = flags;
    this->socket           = sockfd;
    super->destroy         = tcp_destroy;
    super->wait_for_client = tcp_wait_for_client;
    return super;
failclose:
    closesocket( sockfd );
fail:
    winsock_release( );
    return NULL;
}

