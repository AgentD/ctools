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
#define TL_OS_EXPORT
#include "tl_network.h"
#include "os.h"

static int parse_ipv4( const char* s, tl_u32* addr )
{
    int i, j, v;
    tl_u8 a[4];

    for( i=0; i<4; ++i )
    {
        for( v=j=0; j<3 && isdigit(*s); ++j )
            v = 10*v + *(s++) - '0';

        if( v>255 || !j )
            return 0;

        a[i] = v;

        if( i<3 && *(s++)!='.' )
            return 0;
    }

    *addr = (a[0]<<24) | (a[1]<<16) | (a[2]<<8) | a[3];
    return *s == '\0';
}

static int xdigit( int c )
{
    return isdigit(c) ? (c-'0') : (tolower(c)-'a'+10);
}

/* based on musl libc inet_pton by Rich Felker, et al. */
static int parse_ipv6( const char* s, tl_net_addr* addr )
{
    int i, j, v, brk=-1, need_v4=0;
    tl_u16 ip[16];
    tl_u32 v4;

    if( *s==':' && *(++s)!=':' )
        return 0;

    for( i=0; ; ++i )
    {
        if( *s==':' && brk<0 )
        {
            brk = i;
            ip[i & 7] = 0;
            if( !(*(++s)) )
                break;
            if( i==7 )
                return 0;
        }
        else
        {
            for( v=j=0; j<4 && isxdigit(s[j]); ++j )
                v = (v<<4) | xdigit(s[j]);

            if( !j )
                return 0;

            ip[i & 7] = v;

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
        memmove( ip + brk + 7 - i, ip + brk, 2 * (i + 1 - brk) );
        for( j=0; j<7-i; ++j )
            ip[brk + j] = 0;
    }

    if( need_v4 )
    {
        if( !parse_ipv4( s, &v4 ) )
            return 0;
        ip[7] =  v4      & 0xFFFF;
        ip[6] = (v4>>16) & 0xFFFF;
    }

    if( addr )
    {
        addr->net = TL_IPV6;
        for( j=0; j<8; ++j )
            addr->addr.ipv6[j] = ip[(~j) & 7];
    }
    return 1;
}

/****************************************************************************/

int tl_network_resolve_name( const char* hostname, int proto,
                             tl_net_addr* addr, size_t count )
{
    ADDRINFOA hints, *info, *p;
    IN6_ADDR addr6;
    size_t i = 0;
    tl_u32 v4;

    assert( hostname );

    if( parse_ipv4( hostname, &v4 ) )
    {
        if( addr && count>0 )
        {
            addr->addr.ipv4 = v4;
            addr->net = TL_IPV4;
        }
        return proto==TL_IPV4 || proto==TL_ANY;
    }

    if( parse_ipv6( hostname, count > 0 ? addr : NULL ) )
        return proto==TL_IPV6 || proto==TL_ANY;

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
                v4 = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
                addr->addr.ipv4 = ntohl( v4 );
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

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog )
{
    unsigned char addrbuffer[128];
    tl_server* server;
    SOCKET sockfd;
    int size;

    assert( addr );

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

    assert( peer );

    winsock_acquire( );

    sockfd = create_socket( peer, addrbuffer, &size );

    if( sockfd == INVALID_SOCKET )
    {
        winsock_release( );
        return NULL;
    }

    if( connect( sockfd, (void*)addrbuffer, size ) == SOCKET_ERROR )
        goto fail;

    flags = TL_STREAM_TYPE_SOCK;
    flags |= (peer->transport==TL_UDP ? TL_STREAM_UDP : TL_STREAM_TCP);

    if( !(stream = sock_stream_create( sockfd, flags )) )
        goto fail;

    return stream;
fail:
    closesocket( sockfd );
    winsock_release( );
    return NULL;
}

int tl_network_get_peer_address( tl_iostream* stream, tl_net_addr* addr )
{
    sockstream* sock = (sockstream*)stream;
    udp_stream* udp = (udp_stream*)stream;
    unsigned char buffer[ 64 ];
    int len;

    assert( stream && addr );

    if( (stream->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_UDPBUF )
    {
        addr->transport = TL_UDP;
        return decode_sockaddr_in( udp->address, udp->addrlen, addr );
    }
    else if( (stream->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_SOCK )
    {
        addr->transport = (stream->flags & TL_STREAM_UDP) ? TL_UDP : TL_TCP;
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
    unsigned char buffer[ 64 ];
    int len = sizeof(buffer);
    int status;

    assert( stream && addr );

    if( (stream->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_UDPBUF )
    {
        addr->transport = TL_UDP;

        tl_monitor_lock( &(udp->parent->monitor), 0 );
        status = getsockname( udp->parent->socket, (void*)buffer, &len );
        tl_monitor_unlock( &(udp->parent->monitor) );

        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }
    else if( (stream->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_SOCK )
    {
        addr->transport = (stream->flags & TL_STREAM_UDP) ? TL_UDP : TL_TCP;
        status = getsockname( sock->socket, (void*)buffer, &len );
        return status==0 && decode_sockaddr_in( buffer, len, addr );
    }

    return 0;
}

