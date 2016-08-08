/* network.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

tl_iostream* tl_network_create_client( const tl_net_addr* peer, int flags )
{
    struct sockaddr_storage addrbuffer;
    tl_iostream* stream;
    socklen_t size;
    SOCKET sockfd;

    assert( peer );

    if( !winsock_acquire( ) )
        return NULL;

    if( !encode_sockaddr( peer, &addrbuffer, &size ) )
        goto fail_release;

    sockfd = create_socket( peer->net, peer->transport );
    if( sockfd == INVALID_SOCKET )
        goto fail_release;

    if( !set_socket_flags( sockfd, peer->net, &flags ) )
        goto fail;

    if( connect( sockfd, (void*)&addrbuffer, size ) == SOCKET_ERROR )
        goto fail;

    flags = TL_STREAM_TYPE_SOCK;
    flags |= (peer->transport==TL_UDP ? TL_STREAM_UDP : TL_STREAM_TCP);

    if( !(stream = sock_stream_create( sockfd, flags )) )
        goto fail;

    return stream;
fail:
    closesocket( sockfd );
fail_release:
    winsock_release( );
    return NULL;
}

