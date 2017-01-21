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

tl_iostream* tl_network_create_client( const tl_net_addr* peer,
                                       const tl_net_addr* local,
                                       int flags )
{
    tl_iostream* stream;
    SOCKET sockfd;

    assert( peer );

    if( !winsock_acquire( ) )
        return NULL;

    sockfd = create_socket( peer->net, peer->transport );
    if( sockfd == INVALID_SOCKET )
        goto fail_release;

    if( !set_socket_flags( sockfd, peer->net, &flags ) )
        goto fail;

    if( local && !bind_socket( sockfd, local ) )
        goto fail;

    if( !connect_socket( sockfd, peer ) )
        goto fail;

    if( !(stream = sock_stream_create( sockfd, peer->transport )) )
        goto fail;

    return stream;
fail:
    closesocket( sockfd );
fail_release:
    winsock_release( );
    return NULL;
}

