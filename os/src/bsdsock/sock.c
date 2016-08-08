/* sock.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

#include <string.h>

int decode_sockaddr_in( const struct sockaddr_storage* addr, socklen_t len,
                        tl_net_addr* out )
{
    const struct sockaddr_in6* ipv6 = (const struct sockaddr_in6*)addr;
    const struct sockaddr_in* ipv4 = (const struct sockaddr_in*)addr;

    if( len==sizeof(*ipv4) && ipv4->sin_family==AF_INET )
    {
        out->net       = TL_IPV4;
        out->port      = ntohs( ipv4->sin_port );
        out->addr.ipv4 = ntohl( ipv4->sin_addr.s_addr );
        return 1;
    }

    if( len==sizeof(*ipv6) && ipv6->sin6_family==AF_INET6 )
    {
        convert_ipv6( &(ipv6->sin6_addr), out );
        out->net  = TL_IPV6;
        out->port = ntohs( ipv6->sin6_port );
        return 1;
    }

    return 0;
}

int encode_sockaddr( const tl_net_addr* peer,
                     struct sockaddr_storage* addrbuffer, socklen_t* size )
{
    struct sockaddr_in6* v6addr = (struct sockaddr_in6*)addrbuffer;
    struct sockaddr_in* v4addr = (struct sockaddr_in*)addrbuffer;

    if( !peer )
        return 0;

    if( peer->net==TL_IPV4 )
    {
        memset( v4addr, 0, sizeof(*v4addr) );
        v4addr->sin_addr.s_addr = htonl( peer->addr.ipv4 );
        v4addr->sin_port        = htons( peer->port );
        v4addr->sin_family      = AF_INET;
        *size                   = sizeof(*v4addr);
        return 1;
    }
    if( peer->net==TL_IPV6 )
    {
        memset( v6addr, 0, sizeof(*v6addr) );
        convert_in6addr( peer, &(v6addr->sin6_addr) );
        v6addr->sin6_port   = htons( peer->port );
        v6addr->sin6_family = AF_INET6;
        *size               = sizeof(*v6addr);
        return 1;
    }

    return 1;
}

SOCKET create_socket( int net, int transport )
{
    int family, type, proto;
    SOCKET fd;

    switch( net )
    {
    case TL_IPV4: family = AF_INET;  break;
    case TL_IPV6: family = AF_INET6; break;
    default:      return INVALID_SOCKET;
    }

    switch( transport )
    {
    case TL_TCP: type = SOCK_STREAM; proto = IPPROTO_TCP; break;
    case TL_UDP: type = SOCK_DGRAM;  proto = IPPROTO_UDP; break;
    default:     return INVALID_SOCKET;
    }

    fd = socket( family, type, proto );
#ifdef MACHINE_OS_UNIX
    if( fd >= 0 && fcntl( fd, F_SETFD, FD_CLOEXEC ) == -1 )
    {
        close( fd );
        return INVALID_SOCKET;
    }
#endif
    return fd;
}

