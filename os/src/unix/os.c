/*
 * os.c
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



int errno_to_fs( int code )
{
    switch( code )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return TL_ERR_ACCESS;
    case ENOENT:
        return TL_ERR_NOT_EXIST;
    case ENOTDIR:
        return TL_ERR_NOT_DIR;
    case ENOSPC:
    case EDQUOT:
        return TL_ERR_NO_SPACE;
    case EEXIST:
        return TL_ERR_EXISTS;
    case ENOTEMPTY:
        return TL_ERR_NOT_EMPTY;
    case ENOMEM:
        return TL_ERR_ALLOC;
    }

    return code==0 ? 0 : TL_ERR_INTERNAL;
}

int wait_for_fd( int fd, unsigned long timeout, int writeable )
{
    struct timeval tv;
    fd_set fds;

    if( timeout > 0 )
    {
        FD_ZERO( &fds );
        FD_SET( fd, &fds );

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;

        if( writeable )
        {
            if( select( fd+1, 0, &fds, 0, &tv ) <= 0 )
                return 0;
        }
        else
        {
            if( select( fd+1, &fds, 0, 0, &tv ) <= 0 )
                return 0;
        }
        if( !FD_ISSET( fd, &fds ) )
            return 0;
    }

    return 1;
}

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

int encode_sockaddr( const tl_net_addr* peer, void* addrbuffer, int* size )
{
    struct sockaddr_in6* v6addr = addrbuffer;
    struct sockaddr_in* v4addr = addrbuffer;

    if( !peer )
        return 0;

    switch( peer->net )
    {
    case TL_IPV4:
        memset( v4addr, 0, sizeof(struct sockaddr_in) );
        v4addr->sin_addr.s_addr = htonl( peer->addr.ipv4 );
        v4addr->sin_port        = htons( peer->port );
        v4addr->sin_family      = AF_INET;
        *size                   = sizeof(struct sockaddr_in);
        break;
    case TL_IPV6:
        memset( v6addr, 0, sizeof(struct sockaddr_in6) );
        convert_in6addr( peer, &(v6addr->sin6_addr) );
        v6addr->sin6_port   = htons( peer->port );
        v6addr->sin6_family = AF_INET6;
        *size               = sizeof(struct sockaddr_in6);
        break;
    default:
        return 0;
    }

    return 1;
}

int create_socket( const tl_net_addr* peer, void* addrbuffer, int* size )
{
    int family, type, proto;

    if( !encode_sockaddr( peer, addrbuffer, size ) )
        return -1;

    switch( peer->net )
    {
    case TL_IPV4: family = PF_INET;  break;
    case TL_IPV6: family = PF_INET6; break;
    default:      return -1;
    }

    switch( peer->transport )
    {
    case TL_TCP: type = SOCK_STREAM; proto = IPPROTO_TCP; break;
    case TL_UDP: type = SOCK_DGRAM;  proto = IPPROTO_UDP; break;
    default:     return -1;
    }

    return socket( family, type, proto );
}

int bind_socket( int sockfd, void* address, int addrsize )
{
    int val;
    val=1; setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );
    val=1; setsockopt( sockfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val) );

    return bind( sockfd, address, addrsize ) >= 0;
}

int decode_sockaddr_in( const void* addr, size_t len, tl_net_addr* out )
{
    const struct sockaddr_in6* ipv6 = addr;
    const struct sockaddr_in* ipv4 = addr;

    if( len==sizeof(struct sockaddr_in) && ipv4->sin_family==AF_INET )
    {
        out->net       = TL_IPV4;
        out->port      = ntohs( ipv4->sin_port );
        out->addr.ipv4 = ntohl( ipv4->sin_addr.s_addr );
        return 1;
    }

    if( len==sizeof(struct sockaddr_in6) && ipv6->sin6_family==AF_INET6 )
    {
        convert_ipv6( &(ipv6->sin6_addr), out );
        out->net  = TL_IPV6;
        out->port = ntohs( ipv6->sin6_port );
        return 1;
    }

    return 0;
}

void timeout_to_abs( unsigned long timeout, struct timespec* ts )
{
    struct timespec now;

    ts->tv_sec = timeout / 1000;
    ts->tv_nsec = (timeout - ts->tv_sec*1000) * 1000000;

    clock_gettime( CLOCK_REALTIME, &now );
    ts->tv_sec += now.tv_sec;
    ts->tv_nsec += now.tv_nsec;
}

