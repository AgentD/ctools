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
#include "os.h"



static int parse_ipv4( const char* s, void* a0 )
{
    unsigned char* a = a0;
    int i, j, v;

    for( i=0; i<4; ++i, ++s )
    {
        for( v=j=0; j<3 && isdigit(s[j]); ++j )
            v = 10*v + s[j] - '0';

        if( v>255 || !j || (j>1 && s[0]=='0') )
            return 0;

        a[i] = v;
        s += j;

        if( !(*s) && i==3 )
            return 1;

        if( *s!='.' )
            return 0;
    }

    return 0;
}

static int xdigit( int c )
{
    return isdigit(c) ? (c-'0') : (tolower(c)-'a'+10);
}

/* based on musl libc inet_pton by Rich Felker, et al. */
static int parse_ipv6( const char* s, void* a0 )
{
    int i, j, brk=-1, need_v4=0;
    unsigned char* a = a0;
    WORD ip[8];

    if( *s==':' && *(++s)!=':' )
        return 0;

    memset( ip, 0, sizeof(ip) );

    for( i=0; ; ++i )
    {
        if( *s==':' && brk<0 )
        {
            brk = i;
            if( !*(++s) )
                break;
        }
        else
        {
            for( j=0; j<4 && isxdigit(s[j]); ++j )
                ip[i] = 16*ip[i] + xdigit(s[j]);

            if( !j )
                return 0;

            if( !s[j] && (brk>=0 || i==7) )
                break;

            if( i==7 )
                return 0;

            if( s[j]!=':' )
            {
                if( s[j]!='.' || (i<6 && brk<0) )
                    return 0;

                need_v4 = 1;
                ++i;
                break;
            }

            s += j+1;
        }
    }

    if( brk>=0 )
    {
        memmove( ip+brk+7-i, ip+brk, 2*(i+1-brk) );
        memset( ip+brk, 0, 2*(7-i) );
    }

    for( i=0; i<8; ++i )
    {
        *(a++) = ip[i]>>8;
        *(a++) = ip[i];
    }

    return need_v4 ? parse_ipv4( s, a-4 ) : 1;
}

/****************************************************************************/

int tl_network_resolve_name( const char* hostname, int proto,
                             tl_net_addr* addr, size_t count )
{
    ADDRINFOA hints, *info, *p;
    IN6_ADDR addr6;
    IN_ADDR addr4;
    size_t i = 0;

    if( !hostname )
        return 0;

    /* check if hostname is actually a numeric IPv4 address */
    if( parse_ipv4( hostname, &addr4 )>0 )
    {
        if( proto!=TL_IPV4 && proto!=TL_ANY )
            return 0;

        if( addr && count>0 )
        {
            addr->addr.ipv4 = ntohl( addr4.S_un.S_addr );
            addr->net = TL_IPV4;
        }
        return 1;
    }

    /* check if hostname is acutally a numeric IPv6 address */
    if( parse_ipv6( hostname, &addr6 )>0 )
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
    }

    freeaddrinfo( info );
out:
    winsock_release( );
    return i;
}

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog )
{
    unsigned char addrbuffer[128];
    tl_server* server;
    SOCKET sockfd;
    int size;

    winsock_acquire( );

    sockfd = create_socket( addr, (void*)addrbuffer, &size );
    if( sockfd == INVALID_SOCKET )
        goto fail;

    if( !bind_socket( sockfd, addrbuffer, size ) )
        goto failclose;

    switch( addr->transport )
    {
    case TL_TCP: server = tcp_server_create( sockfd, backlog ); break;
    case TL_UDP: server = udp_server_create( sockfd );          break;
    default:     server = NULL;                                 break;
    }

    if( server )
        return server;
failclose:
    closesocket( sockfd );
fail:
    winsock_release( );
    return NULL;
}

tl_iostream* tl_network_create_client( const tl_net_addr* peer )
{
    unsigned char addrbuffer[128];
    tl_iostream* stream;
    int size, flags;
    SOCKET sockfd;

    winsock_acquire( );

    sockfd = create_socket( peer, addrbuffer, &size );

    if( sockfd == INVALID_SOCKET )
    {
        winsock_release( );
        return NULL;
    }

    if( connect( sockfd, (void*)addrbuffer, size ) == SOCKET_ERROR )
        goto fail;

    flags = WSTR_SOCK | (peer->transport==TL_UDP ? WSTR_UDP : WSTR_TCP);

    if( !(stream = sock_stream_create( sockfd, flags )) )
        goto fail;

    return stream;
fail:
    closesocket( sockfd );
    winsock_release( );
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

int tl_network_get_peer_address( tl_iostream* stream, tl_net_addr* addr )
{
    sockstream* sock = (sockstream*)stream;
    udp_stream* udp = (udp_stream*)stream;
    w32stream* w32 = (w32stream*)stream;
    unsigned char buffer[ 64 ];
    int len;

    if( !stream || !addr )
        return 0;

    if( (w32->flags & WSTR_TYPE_MASK) == WSTR_UDPBUF )
    {
        addr->transport = TL_UDP;
        return decode_sockaddr_in( udp->address, udp->addrlen, addr );
    }
    else if( (w32->flags & WSTR_TYPE_MASK) == WSTR_SOCK )
    {
        addr->transport = (w32->flags & WSTR_UDP) ? TL_UDP : TL_TCP;
        len = sizeof(buffer);

        if( getpeername( sock->socket, (void*)buffer, &len )==0 )
            return decode_sockaddr_in( buffer, len, addr );
    }

    return 0;
}

int tl_network_get_local_address( tl_iostream* stream, tl_net_addr* addr )
{
    sockstream* sock = (sockstream*)stream;
    udp_stream* udp = (udp_stream*)stream;
    w32stream* w32 = (w32stream*)stream;
    unsigned char buffer[ 64 ];
    int len = sizeof(buffer);
    int status;

    if( !stream || !addr )
        return 0;

    if( (w32->flags & WSTR_TYPE_MASK) == WSTR_UDPBUF )
    {
        addr->transport = TL_UDP;

        tl_monitor_lock( &(udp->parent->monitor), 0 );
        status = getsockname( udp->parent->socket, (void*)buffer, &len );
        tl_monitor_unlock( &(udp->parent->monitor) );

        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }
    else if( (w32->flags & WSTR_TYPE_MASK) == WSTR_SOCK )
    {
        addr->transport = (w32->flags & WSTR_UDP) ? TL_UDP : TL_TCP;
        status = getsockname( sock->socket, (void*)buffer, &len );
        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }

    return 0;
}

