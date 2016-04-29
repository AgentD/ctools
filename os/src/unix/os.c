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
    case EFBIG:
    case EMSGSIZE:
        return TL_ERR_TOO_LARGE;
    case EAGAIN:
#if EWOULDBLOCK != EAGAIN       /* for hysterical raisins */
    case EWOULDBLOCK:
#endif
        return TL_ERR_TIMEOUT;
    case EHOSTUNREACH:
        return TL_ERR_HOST_UNREACH;
    case ENETDOWN:
        return TL_ERR_NET_DOWN;
    case ENETUNREACH:
        return TL_ERR_NET_UNREACH;
    case ECONNRESET:
        return TL_ERR_NET_RESET;
    case EAFNOSUPPORT:
        return TL_ERR_NET_ADDR;
    }

    return code==0 ? 0 : TL_ERR_INTERNAL;
}

int wait_for_fd( int fd, unsigned long timeoutms, int writeable )
{
    int ret, mask = writeable ? POLLOUT : POLLIN;
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = mask;

    ret = poll( &pfd, 1, timeoutms > 0 ? (long)timeoutms : -1L );

    return (ret == 1) && (pfd.revents & mask);
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

pid_t wait_pid_ms( pid_t pid, int* status, unsigned long timeout )
{
    struct timeval before, after;
    unsigned long delta;
    struct timespec ts;
    sigset_t mask;
    int result;
    pid_t ret;

    while( timeout )
    {
        ret = waitpid( pid, status, WNOHANG );

        if( ret!=0 )
            return ret;

        /* setup timeout structure and signal mask */
        ts.tv_sec  = timeout / 1000;
        ts.tv_nsec = ((long)timeout - ts.tv_sec*1000L)*1000000L;

        sigemptyset( &mask );
        sigaddset( &mask, SIGCHLD );

        /* wait for a signal */
        gettimeofday( &before, NULL );
        result = pselect( 0, NULL, NULL, NULL, &ts, &mask );

        if( result==0 )
            return 0;
        if( result==-1 && errno!=EINTR )
            return -1;

        /* subtract elapsed time from timeout */
        gettimeofday( &after, NULL );

        delta  = (after.tv_sec  - before.tv_sec )*1000;
        delta += (after.tv_usec - before.tv_usec)/1000;
        timeout = (delta >= (unsigned long)timeout) ? 0 : (timeout - delta);
    }

    return 0;
}

int set_socket_flags( int fd, int netlayer, int flags )
{
    int val, level, opt;

    if( flags & (~TL_ALL_NETWORK_FLAGS) )   /* unknown flags? */
        return 0;

    if( (flags & TL_ALLOW_BROADCAST) && (netlayer == TL_IPV4) )
    {
        val = 1;
        setsockopt( fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(int) );
    }

    if( flags & TL_DONT_FRAGMENT )
    {
        switch( netlayer )
        {
#ifdef __linux__
        case TL_IPV6: opt = IPV6_MTU_DISCOVER; val = IPV6_PMTUDISC_DO; break;
        case TL_IPV4: opt = IP_MTU_DISCOVER;   val = IP_PMTUDISC_DO;   break;
#endif
        default:
            goto skip;
        }
        level = netlayer==TL_IPV6 ? IPPROTO_IPV6 : IPPROTO_IP;
        setsockopt( fd, level, opt, &val, sizeof(int) );
    }
skip:
    return 1;
}

