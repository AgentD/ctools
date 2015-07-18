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

#include <assert.h>



int tl_network_resolve_name( const char* hostname, int proto,
                             tl_net_addr* addr, size_t count )
{
    struct addrinfo hints, *info, *p;
    struct in6_addr addr6;
    struct in_addr addr4;
    size_t i = 0;

    assert( hostname );

    /* check if hostname is actually a numeric IPv4 address */
    if( inet_pton( AF_INET, hostname, &addr4 )>0 )
    {
        if( proto!=TL_IPV4 && proto!=TL_ANY )
            return 0;
        if( addr && count>0 )
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
        if( addr && count>0 )
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
    }

    freeaddrinfo( info );
    return i;
}

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog )
{
    unsigned char addrbuffer[128];
    tl_server* server;
    int sockfd, size;

    assert( addr );

    sockfd = create_socket( addr, (void*)addrbuffer, &size );
    if( sockfd < 0 )
        return NULL;

    if( !bind_socket( sockfd, addrbuffer, size ) )
        goto fail;

    switch( addr->transport )
    {
    case TL_TCP: server = tcp_server_create( sockfd, backlog ); break;
    case TL_UDP: server = udp_server_create( sockfd );          break;
    default:     server = NULL;                                 break;
    }

    if( !server )
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
    int sockfd, size, flags;

    assert( peer );

    sockfd = create_socket( peer, (void*)addrbuffer, &size );
    if( sockfd < 0 )
        return NULL;

    if( connect( sockfd, (void*)addrbuffer, size ) < 0 )
        goto fail;

    flags = USTR_SOCK | (peer->transport==TL_UDP ? USTR_UDP : USTR_TCP);

    if( !(stream = pipe_stream_create( sockfd, sockfd, flags )) )
        goto fail;

    return stream;
fail:
    close( sockfd );
    return NULL;
}

int tl_network_get_special_address( tl_net_addr* addr, int type, int net )
{
    assert( addr );

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

int tl_network_get_peer_address( tl_iostream* stream, tl_net_addr* addr )
{
    unix_stream* unix = (unix_stream*)stream;
    udp_stream* udp = (udp_stream*)stream;
    fd_stream* fd = (fd_stream*)stream;
    unsigned char buffer[ 64 ];
    socklen_t len;

    assert( stream && addr );

    if( (unix->flags & USTR_TYPE_MASK) == USTR_UDPBUF )
    {
        addr->transport = TL_UDP;
        return decode_sockaddr_in( udp->address, udp->addrlen, addr );
    }
    else if( (unix->flags & USTR_TYPE_MASK) == USTR_SOCK )
    {
        addr->transport = (unix->flags & USTR_UDP) ? TL_UDP : TL_TCP;
        len = sizeof(buffer);

        if( getpeername( fd->writefd, (void*)buffer, &len )==0 )
            return decode_sockaddr_in( buffer, len, addr );
    }

    return 0;
}

int tl_network_get_local_address( tl_iostream* stream, tl_net_addr* addr )
{
    unix_stream* unix = (unix_stream*)stream;
    udp_stream* udp = (udp_stream*)stream;
    fd_stream* fd = (fd_stream*)stream;
    unsigned char buffer[ 64 ];
    socklen_t len = sizeof(buffer);
    int status;

    assert( stream && addr );

    if( (unix->flags & USTR_TYPE_MASK) == USTR_UDPBUF )
    {
        addr->transport = TL_UDP;

        tl_monitor_lock( &(udp->parent->monitor), 0 );
        status = getsockname( udp->parent->socket, (void*)buffer, &len );
        tl_monitor_unlock( &(udp->parent->monitor) );

        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }
    else if( (unix->flags & USTR_TYPE_MASK) == USTR_SOCK )
    {
        addr->transport = (unix->flags & USTR_UDP) ? TL_UDP : TL_TCP;
        status = getsockname( fd->writefd, (void*)buffer, &len );
        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }

    return 0;
}

