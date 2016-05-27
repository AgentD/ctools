/*
 * sock.c
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
#include "sock.h"
#include "os.h"

#include <string.h>

#if defined(MACHINE_OS_WINDOWS) && !defined(s6_addr)
    #define s6_addr u.Byte
#endif


void convert_ipv6( const struct in6_addr* v6, tl_net_addr* addr )
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

void convert_in6addr( const tl_net_addr* addr, struct in6_addr* v6 )
{
    v6->s6_addr[ 0] = (addr->addr.ipv6[7]>>8) & 0xFF;
    v6->s6_addr[ 1] =  addr->addr.ipv6[7]     & 0xFF;
    v6->s6_addr[ 2] = (addr->addr.ipv6[6]>>8) & 0xFF;
    v6->s6_addr[ 3] =  addr->addr.ipv6[6]     & 0xFF;
    v6->s6_addr[ 4] = (addr->addr.ipv6[5]>>8) & 0xFF;
    v6->s6_addr[ 5] =  addr->addr.ipv6[5]     & 0xFF;
    v6->s6_addr[ 6] = (addr->addr.ipv6[4]>>8) & 0xFF;
    v6->s6_addr[ 7] =  addr->addr.ipv6[4]     & 0xFF;
    v6->s6_addr[ 8] = (addr->addr.ipv6[3]>>8) & 0xFF;
    v6->s6_addr[ 9] =  addr->addr.ipv6[3]     & 0xFF;
    v6->s6_addr[10] = (addr->addr.ipv6[2]>>8) & 0xFF;
    v6->s6_addr[11] =  addr->addr.ipv6[2]     & 0xFF;
    v6->s6_addr[12] = (addr->addr.ipv6[1]>>8) & 0xFF;
    v6->s6_addr[13] =  addr->addr.ipv6[1]     & 0xFF;
    v6->s6_addr[14] = (addr->addr.ipv6[0]>>8) & 0xFF;
    v6->s6_addr[15] =  addr->addr.ipv6[0]     & 0xFF;
}

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

    winsock_acquire( );

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

