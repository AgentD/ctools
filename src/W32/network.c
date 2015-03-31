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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <in6addr.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
    #include <intrin.h>

    #pragma intrinsic (_InterlockedIncrement)
    #pragma intrinsic (_InterlockedDecrement)

    static volatile LONG refcount = 0;
#else
    static volatile int refcount = 0;
#endif



static int winsock_acquire( void )
{
    WORD version = MAKEWORD(2, 2);
    WSADATA data;

#ifdef _MSC_VER
    if( _InterlockedIncrement( &refcount )>1 )
        return 1;
#else
    if( __sync_fetch_and_add( &refcount, 1 )>0 )
        return 1;
#endif

    return WSAStartup( version, &data )==0;
}

static void winsock_release( void )
{
#ifdef _MSC_VER
    if( _InterlockedDecrement( &refcount ) == 0 )
        WSACleanup( );
#else
    if( __sync_fetch_and_sub( &refcount, 1 )==1 )
        WSACleanup( );
#endif
}

/****************************************************************************/

static void convert_ipv6( const IN6_ADDR* v6, tl_net_addr* addr )
{
    addr->addr.ipv6[7] = (v6->u.Byte[ 0]<<8) | v6->u.Byte[ 1];
    addr->addr.ipv6[6] = (v6->u.Byte[ 2]<<8) | v6->u.Byte[ 3];
    addr->addr.ipv6[5] = (v6->u.Byte[ 4]<<8) | v6->u.Byte[ 5];
    addr->addr.ipv6[4] = (v6->u.Byte[ 6]<<8) | v6->u.Byte[ 7];
    addr->addr.ipv6[3] = (v6->u.Byte[ 8]<<8) | v6->u.Byte[ 9];
    addr->addr.ipv6[2] = (v6->u.Byte[10]<<8) | v6->u.Byte[11];
    addr->addr.ipv6[1] = (v6->u.Byte[12]<<8) | v6->u.Byte[13];
    addr->addr.ipv6[0] = (v6->u.Byte[14]<<8) | v6->u.Byte[15];
}

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
                             tl_net_addr* addr )
{
    ADDRINFOA hints, *info, *p;
    IN6_ADDR addr6;
    IN_ADDR addr4;

    if( !hostname )
        return 0;

    /* check if hostname is actually a numeric IPv4 address */
    if( parse_ipv4( hostname, &addr4 )>0 )
    {
        if( proto!=TL_IPV4 && proto!=TL_ANY )
            return 0;

        if( addr )
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

    if( !winsock_acquire( ) )
        return 0;

    if( getaddrinfo( hostname, NULL, &hints, &info )!=0 )
        goto fail;

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
        goto fail;
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
    winsock_release( );
    return 1;
fail:
    winsock_release( );
    return 0;
}

tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog )
{
    if( !addr )
        return NULL;

    (void)backlog;
    return NULL;
}

tl_iostream* tl_network_create_client( const tl_net_addr* peer )
{
    if( !peer )
        return NULL;

    if( peer->net!=TL_IPV4 && peer->net!=TL_IPV6 )
        return NULL;

    if( peer->transport!=TL_TCP && peer->transport!=TL_UDP )
        return NULL;

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

