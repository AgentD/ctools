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

tl_server* tl_network_create_server( int net, int proto, tl_u16 port )
{
    if( net!=TL_IPV4 && net!=TL_IPV6 )
        return NULL;

    if( proto!=TL_TCP && proto!=TL_UDP )
        return NULL;

    (void)port;
    return NULL;
}

tl_iostream* tl_network_create_client( const tl_net_addr* peer )
{
    int sockfd, family, type, size, proto;
    struct sockaddr_in v4addr;
    struct sockaddr_in6 v6addr;
    struct sockaddr* addr;
    tl_iostream* stream;

    if( !peer )
        return NULL;

    /* translate transport protocol data */
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
        return NULL;
    }

    /* translate network protocol data */
    if( peer->net==TL_IPV4 )
    {
        memset( &v4addr, 0, sizeof(v4addr) );
        v4addr.sin_family      = AF_INET;
        v4addr.sin_addr.s_addr = htonl(peer->addr.ipv4);
        v4addr.sin_port        = htons(peer->port);
        family                 = PF_INET;
        addr                   = (struct sockaddr*)&v4addr;
        size                   = sizeof(v4addr);
    }
    else if( peer->net==TL_IPV6 )
    {
        memset( &v6addr, 0, sizeof(v6addr) );
        convert_in6addr( peer, &(v6addr.sin6_addr) );
        v6addr.sin6_family = AF_INET6;
        v6addr.sin6_port   = htons(peer->port);
        family             = PF_INET6;
        addr               = (struct sockaddr*)&v6addr;
        size               = sizeof(v6addr);
    }
    else
    {
        return NULL;
    }

    /* create and connect socket */
    sockfd = socket( family, type, proto );

    if( sockfd < 0 )
        return NULL;

    if( connect( sockfd, addr, size ) < 0 )
    {
        close( sockfd );
        return NULL;
    }

    /* create stream object */
    stream = sock_stream_create( sockfd );

    if( !stream )
        close( sockfd );

    return stream;
}

