/* resolve_name.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

int resolve_name( const char* hostname, int proto,
                  tl_net_addr* addr, size_t count )
{
    struct addrinfo hints, *info, *p;
    struct in6_addr addr6;
    struct in_addr addr4;
    size_t i = 0;

    proto =  (proto==TL_IPV6) ? AF_INET6 : 
            ((proto==TL_IPV4) ? AF_INET : AF_UNSPEC);

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = proto;

    if( !winsock_acquire( ) )
        return 0;

    if( getaddrinfo( hostname, NULL, &hints, &info )!=0 )
        goto out;

    for( p=info; p!=NULL; p=p->ai_next )
    {
        if( p->ai_family!=AF_INET && p->ai_family!=AF_INET6 )
            continue;

        if( proto!=AF_UNSPEC && p->ai_family!=proto )
            continue;

        if( addr && i<count )
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
            ++i;
            ++addr;
        }
        else if( !addr )
        {
            ++i;
        }
    }

    freeaddrinfo( info );
out:
    winsock_release( );
    return i;
}

