/* tcpserver.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "tl_server.h"
#include "bsdsock.h"

static void tcp_destroy( tl_server* this )
{
    assert( this );
    closesocket( ((tcp_server*)this)->socket );
    free( this );
    winsock_release( );
}

static int is_v6( SOCKET peer )
{
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    struct sockaddr_in6* v6;
    int x, y;

    if( getpeername( peer, (void*)&addr, &len ) != 0 )
        return 0;
    if( addr.ss_family != AF_INET6 )
        return 0;

    v6 = (struct sockaddr_in6*)&addr;

    x = v6->sin6_addr.s6_addr[0] | v6->sin6_addr.s6_addr[1] |
        v6->sin6_addr.s6_addr[2] | v6->sin6_addr.s6_addr[3] |
        v6->sin6_addr.s6_addr[4] | v6->sin6_addr.s6_addr[5] |
        v6->sin6_addr.s6_addr[6] | v6->sin6_addr.s6_addr[7] |
        v6->sin6_addr.s6_addr[8] | v6->sin6_addr.s6_addr[9];

    y = v6->sin6_addr.s6_addr[10] & v6->sin6_addr.s6_addr[11];

    if( x==0 && y==0xFF )
        return 0;

    return 1;
}

static tl_iostream* tcp_wait_for_client( tl_server* super, int timeout )
{
    tcp_server* this = (tcp_server*)super;
    tl_iostream* client;
    SOCKET peer;

    assert( this );

    if( !wait_for_fd( this->socket, timeout, 0 ) )
        return NULL;

    peer = accept( this->socket, 0, 0 );
    if( peer == INVALID_SOCKET )
        return NULL;

    if( this->flags & TL_ENFORCE_V6_ONLY && !is_v6( peer ) )
        goto ignore;

    if( set_cloexec( peer ) == -1 )
    {
        closesocket( peer );
        return NULL;
    }

    client = sock_stream_create( peer, TL_TCP );
    if( !client )
        goto ignore;
    return client;
ignore:
    closesocket( peer );
    return NULL;
}

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog, int flags )
{
    tcp_server* this;
    tl_server* super;
    SOCKET sockfd;

    assert( addr );

    if( addr->transport != TL_TCP )
        return NULL;

    if( !winsock_acquire( ) )
        return NULL;

    sockfd = create_socket( addr->net, addr->transport );
    if( sockfd == INVALID_SOCKET )
        goto fail;

    if( !set_socket_flags( sockfd, addr->net, &flags ) )
        goto failclose;

    if( !bind_socket( sockfd, addr ) )
        goto failclose;

    if( listen( sockfd, backlog ) == SOCKET_ERROR )
        goto failclose;

    this = calloc( 1, sizeof(tcp_server) );
    super = (tl_server*)this;
    if( !this )
        goto failclose;

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

