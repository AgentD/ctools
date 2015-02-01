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
#include "tl_network.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>
#include <ctype.h>



static int change_network( int proto, int family )
{
    switch( proto )
    {
    case TL_TCP_IPV6:
    case TL_TCP_IPV4:
    case TL_TCP_ANY:
        return family==AF_INET6 ? TL_TCP_IPV6 : TL_TCP_IPV4;
    case TL_UDP_IPV4:
    case TL_UDP_IPV6:
    case TL_UDP_ANY:
        return family==AF_INET6 ? TL_UDP_IPV6 : TL_UDP_IPV4;
    }
    return 0;
}

static void convert_ipv6( struct in6_addr* v6, tl_net_addr* addr )
{
    addr->addr.ipv6[7] = (v6->s6_addr[ 0]<<8) | v6->s6_addr[ 1];
    addr->addr.ipv6[6] = (v6->s6_addr[ 2]<<8) | v6->s6_addr[ 3];
    addr->addr.ipv6[5] = (v6->s6_addr[ 4]<<8) | v6->s6_addr[ 5];
    addr->addr.ipv6[4] = (v6->s6_addr[ 6]<<8) | v6->s6_addr[ 7];
    addr->addr.ipv6[3] = (v6->s6_addr[ 8]<<8) | v6->s6_addr[ 9];
    addr->addr.ipv6[2] = (v6->s6_addr[10]<<8) | v6->s6_addr[11];
    addr->addr.ipv6[1] = (v6->s6_addr[12]<<8) | v6->s6_addr[13];
    addr->addr.ipv6[0] = (v6->s6_addr[14]<<8) | v6->s6_addr[15];
}

static int resolve_hostname( const char* hostname, int ai_family,
                             tl_net_addr* addr )
{
    struct addrinfo* info;
    struct addrinfo hints;
    struct addrinfo* p;
    struct in6_addr v6;
    struct in_addr v4;

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = ai_family;

    if( getaddrinfo( hostname, NULL, &hints, &info )!=0 )
        return 0;

    for( p=info; p!=NULL; p=p->ai_next )
    {
        if( p->ai_family!=AF_INET && p->ai_family!=AF_INET6 )
            continue;

        if( ai_family!=AF_UNSPEC && p->ai_family!=ai_family )
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
            v6 = ((struct sockaddr_in6*)p->ai_addr)->sin6_addr;
            convert_ipv6( &v6, addr );
        }
        else
        {
            v4 = ((struct sockaddr_in*)p->ai_addr)->sin_addr;
            addr->addr.ipv4 = ntohl( v4.s_addr );
        }

        addr->protocol = change_network( addr->protocol, p->ai_family );
    }

    freeaddrinfo( info );
    return 1;
}

/****************************************************************************/

int tl_network_resolve_name( const char* hostname, tl_net_addr* addr )
{
    struct in6_addr addr6;
    struct in_addr addr4;
    int use_ipv6 = 0;
    int use_ipv4 = 0;

    if( !hostname )
        return 0;

    while( isspace( *hostname ) )
        ++hostname;

    if( addr )
    {
        use_ipv6 = addr->protocol==TL_TCP_IPV6 || addr->protocol==TL_UDP_IPV6;
        use_ipv4 = addr->protocol==TL_TCP_IPV4 || addr->protocol==TL_UDP_IPV4;
    }

    /* check if hostname is actually a numeric IPv4  address */
    if( inet_pton( AF_INET, hostname, &addr4 )>0 )
    {
        if( use_ipv6 )
            return 0;

        if( addr )
        {
            addr->addr.ipv4 = ntohl( addr4.s_addr );
            addr->protocol = change_network( addr->protocol, AF_INET );
        }
        return 1;
    }

    /* check if hostname is acutally a numeric IPv6 address */
    if( inet_pton( AF_INET6, hostname, &addr6 )>0 )
    {
        if( use_ipv4 )
            return 0;

        if( addr )
        {
            convert_ipv6( &addr6, addr );
            addr->protocol = change_network( addr->protocol, AF_INET6 );
        }
        return 1;
    }

    /* try to resolve hostname */
    if( use_ipv6 )
        return resolve_hostname( hostname, AF_INET6, addr );
    else if( use_ipv4 )
        return resolve_hostname( hostname, AF_INET, addr );

    return resolve_hostname( hostname, AF_UNSPEC, addr );
}

