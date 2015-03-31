/*
 * network.c
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
#include "tl_network.h"
#include "tl_iostream.h"
#include "tl_blob.h"
#include "os.h"



static int create_socket(const tl_net_addr* peer, void* addrbuffer, int* size)
{
    struct sockaddr_in6* v6addr = addrbuffer;
    struct sockaddr_in* v4addr = addrbuffer;
    int family, type, proto;

    if( !peer )
        return -1;

    if( peer->net==TL_IPV4 )
    {
        memset( v4addr, 0, sizeof(struct sockaddr_in) );
        v4addr->sin_addr.s_addr = htonl( peer->addr.ipv4 );
        v4addr->sin_port        = htons( peer->port );
        v4addr->sin_family      = AF_INET;
        family                  = PF_INET;
        *size                   = sizeof(struct sockaddr_in);
    }
    else if( peer->net==TL_IPV6 )
    {
        memset( v6addr, 0, sizeof(struct sockaddr_in6) );
        convert_in6addr( peer, &(v6addr->sin6_addr) );
        v6addr->sin6_port   = htons( peer->port );
        v6addr->sin6_family = AF_INET6;
        family              = PF_INET6;
        *size               = sizeof(struct sockaddr_in6);
    }
    else
    {
        return -1;
    }

    if( peer->transport==TL_TCP )
    {
        type = SOCK_STREAM;
        proto = IPPROTO_TCP;
    }
    else if( peer->transport==TL_UDP )
    {
        type = SOCK_DGRAM;
        proto = IPPROTO_UDP;
    }
    else
    {
        return -1;
    }

    return socket( family, type, proto );
}

/****************************************************************************/

int tl_network_resolve_name( const char* hostname, int proto,
                             tl_net_addr* addr )
{
    struct addrinfo hints, *info, *p;
    struct in6_addr addr6;
    struct in_addr addr4;

    if( !hostname )
        return 0;

    /* check if hostname is actually a numeric IPv4 address */
    if( inet_pton( AF_INET, hostname, &addr4 )>0 )
    {
        if( proto!=TL_IPV4 && proto!=TL_ANY )
            return 0;
        if( addr )
        {
            addr->addr.ipv4 = ntohl( addr4.s_addr );
            addr->net = TL_IPV4;
        }
        return 1;
    }

    /* check if hostname is acutally a numeric IPv6 address */
    if( inet_pton( AF_INET6, hostname, &addr6 )>0 )
    {
        if( proto!=TL_IPV6 && proto!=TL_ANY )
            return 0;
        if( addr )
        {
            convert_ipv6( &addr6, addr );
            addr->net = TL_IPV6;
        }
        return 1;
    }

    /* try to resolve hostname */
    proto =  (proto==TL_IPV6) ? AF_INET6 : 
            ((proto==TL_IPV4) ? AF_INET : AF_UNSPEC);

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = proto;

    if( getaddrinfo( hostname, NULL, &hints, &info )!=0 )
        return 0;

    for( p=info; p!=NULL; p=p->ai_next )
    {
        if( p->ai_family!=AF_INET && p->ai_family!=AF_INET6 )
            continue;

        if( proto!=AF_UNSPEC && p->ai_family!=proto )
            continue;

        break;
    }

    if( !p )
    {
        freeaddrinfo( info );
        return 0;
    }

    if( addr )
    {
        if( p->ai_family==AF_INET6 )
        {
            addr6 = ((struct sockaddr_in6*)p->ai_addr)->sin6_addr;
            convert_ipv6( &addr6, addr );
        }
        else
        {
            addr4 = ((struct sockaddr_in*)p->ai_addr)->sin_addr;
            addr->addr.ipv4 = ntohl( addr4.s_addr );
        }

        addr->net = p->ai_family==AF_INET6 ? TL_IPV6 : TL_IPV4;
    }

    freeaddrinfo( info );
    return 1;
}

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog )
{
    unsigned char addrbuffer[128];
    int sockfd, size, val;
    tl_server* server;

    sockfd = create_socket( addr, (void*)addrbuffer, &size );

    if( sockfd < 0 )
        return NULL;

    val=1; setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );
    val=1; setsockopt( sockfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val) );

    if( bind( sockfd, (void*)addrbuffer, size ) < 0 )
        goto fail;

    if( !(server = tcp_server_create( sockfd, backlog )) )
        goto fail;

    return server;
fail:
    close( sockfd );
    return NULL;
}

tl_iostream* tl_network_create_client( const tl_net_addr* peer )
{
    unsigned char addrbuffer[128];
    tl_iostream* stream;
    int sockfd, size;

    sockfd = create_socket( peer, addrbuffer, &size );

    if( sockfd < 0 )
        return NULL;

    if( connect( sockfd, (void*)addrbuffer, size ) < 0 )
        goto fail;

    if( !(stream = sock_stream_create( sockfd )) )
        goto fail;

    return stream;
fail:
    close( sockfd );
    return NULL;
}

int tl_network_get_special_address( tl_net_addr* addr, int type, int net )
{
    if( !addr )
        return 0;

    addr->net = net;

    if( net==TL_IPV4 )
    {
        switch( type )
        {
        case TL_LOOPBACK:   addr->addr.ipv4 = INADDR_LOOPBACK;  return 1;
        case TL_BROADCAST:  addr->addr.ipv4 = INADDR_BROADCAST; return 1;
        case TL_ALL:        addr->addr.ipv4 = INADDR_ANY;       return 1;
        }
    }
    else if( net==TL_IPV6 )
    {
        switch( type )
        {
        case TL_LOOPBACK:  convert_ipv6( &in6addr_loopback, addr ); return 1;
        case TL_ALL:       convert_ipv6( &in6addr_any,      addr ); return 1;
        }
    }

    return 0;
}

