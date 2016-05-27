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

int set_socket_flags( int fd, int netlayer, int* flags )
{
    int val, level, opt;

    if( (*flags) & (~TL_ALL_NETWORK_FLAGS) )   /* unknown flags? */
        return 0;

    val = 1; setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );
    val = 1; setsockopt( fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val) );

    if( ((*flags) & TL_ALLOW_BROADCAST) && (netlayer == TL_IPV4) )
    {
        val = 1;
        if( setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(int)) < 0 )
            return 0;
    }

    if( (*flags) & TL_DONT_FRAGMENT )
    {
        switch( netlayer )
        {
#ifdef __linux__
        case TL_IPV6: opt = IPV6_MTU_DISCOVER; val = IPV6_PMTUDISC_DO; break;
        case TL_IPV4: opt = IP_MTU_DISCOVER;   val = IP_PMTUDISC_DO;   break;
#elif defined(__FreeBSD__)
        case TL_IPV6: opt = IPV6_USE_MIN_MTU;  val = 1; break;
        case TL_IPV4: opt = IP_DONTFRAG;       val = 1; break;
#endif
        default:
            *flags &= ~TL_DONT_FRAGMENT;
            goto skip;
        }
        level = netlayer==TL_IPV6 ? IPPROTO_IPV6 : IPPROTO_IP;
        if( setsockopt( fd, level, opt, &val, sizeof(int) ) < 0 )
            *flags &= ~TL_DONT_FRAGMENT;
    }
skip:
    if( netlayer == TL_IPV6 )
    {
        val = 1;
        if( setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val)) < 0 )
            *flags |= TL_ENFORCE_V6_ONLY;
    }
    return 1;
}

