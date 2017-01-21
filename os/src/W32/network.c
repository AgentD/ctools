/* network.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
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

    return resolve_name( hostname, proto, addr, count );
}

int tl_network_get_peer_address( tl_iostream* stream, tl_net_addr* addr )
{
    sockstream* sock = (sockstream*)stream;
    struct sockaddr_storage buffer;
    socklen_t len = sizeof(buffer);

    assert( stream && addr );

    if( stream->type == TL_STREAM_TYPE_SOCK )
    {
        addr->transport = sock->proto;

        if( getpeername( sock->socket, (void*)&buffer, &len )==0 )
            return decode_sockaddr_in( &buffer, len, addr );
    }

    return 0;
}

int tl_network_get_local_address( tl_iostream* stream, tl_net_addr* addr )
{
    sockstream* sock = (sockstream*)stream;
    struct sockaddr_storage buffer;
    socklen_t len = sizeof(buffer);

    assert( stream && addr );

    if( stream->type == TL_STREAM_TYPE_SOCK )
    {
        addr->transport = sock->proto;

        if( getsockname( sock->socket, (void*)&buffer, &len )==0 )
            return decode_sockaddr_in( &buffer, len, addr );
    }

    return 0;
}

