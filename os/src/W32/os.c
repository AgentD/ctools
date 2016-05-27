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
#include "os.h"
#include "tl_network.h"

#ifdef _MSC_VER
    #include <intrin.h>

    #pragma intrinsic (_InterlockedIncrement)
    #pragma intrinsic (_InterlockedDecrement)

    static volatile LONG refcount = 0;
#else
    static volatile int refcount = 0;
#endif

int errno_to_fs( int code )
{
    switch( code )
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_DIRECTORY:
        return TL_ERR_NOT_EXIST;

    case ERROR_INVALID_ACCESS:
        return TL_ERR_ACCESS;

    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return TL_ERR_EXISTS;


    case ERROR_DISK_FULL:
        return TL_ERR_NO_SPACE;

    case ERROR_DIR_NOT_EMPTY:
        return TL_ERR_NOT_EMPTY;

        /*return TL_ERR_NOT_DIR;*/
    }

    return code==0 ? 0 : TL_ERR_INTERNAL;
}

WCHAR* utf8_to_utf16( const char* utf8 )
{
    unsigned int length = MultiByteToWideChar(CP_UTF8,0,utf8,-1,NULL,0);
    WCHAR* out = malloc( sizeof(WCHAR)*(length+1) );

    if( out )
    {
        MultiByteToWideChar( CP_UTF8, 0, utf8, -1, out, length );
        out[length] = '\0';
    }
    return out;
}

int winsock_acquire( void )
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

void winsock_release( void )
{
#ifdef _MSC_VER
    if( _InterlockedDecrement( &refcount ) == 0 )
#else
    if( __sync_fetch_and_sub( &refcount, 1 )==1 )
#endif
    {
        WSACleanup( );
    }
}

int wait_for_socket( SOCKET socket, unsigned long timeout, int write )
{
    struct timeval tv;
    fd_set fds;

    if( timeout > 0 )
    {
        FD_ZERO( &fds );
        FD_SET( socket, &fds );

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;

        if( select( socket+1, write ? 0 : &fds,
                    write ? &fds : 0, 0, &tv ) <= 0 )
        {
            return 0;
        }
    }

    return 1;
}

int WSAHandleFuckup( void )
{
    int status = WSAGetLastError( );

    switch( status )
    {
    case WSAENOPROTOOPT:
    case WSAEINVAL:
        return TL_ERR_NOT_SUPPORTED;
    case WSAETIMEDOUT:
    case WSAEWOULDBLOCK:
        return TL_ERR_TIMEOUT;
    case WSAEHOSTDOWN:
    case WSAEHOSTUNREACH:
        return TL_ERR_HOST_UNREACH;
    case WSAECONNRESET:
        return TL_ERR_NET_RESET;
    case WSAENETUNREACH:
        return TL_ERR_NET_UNREACH;
    case WSAENETDOWN:
        return TL_ERR_NET_DOWN;
    case WSAEAFNOSUPPORT:
        return TL_ERR_NET_ADDR;
    case WSAEMSGSIZE:
        return TL_ERR_TOO_LARGE;
    case WSAEACCES:
        return TL_ERR_ACCESS;
    case WSAECONNABORTED:
    case WSAESHUTDOWN:
    case WSAENOTSOCK:
    case WSAENOTCONN:
    case WSAENETRESET:
        return TL_ERR_CLOSED;
    }
    return TL_ERR_INTERNAL;
}

void convert_ipv6( const IN6_ADDR* v6, tl_net_addr* addr )
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

void convert_in6addr( const tl_net_addr* addr, IN6_ADDR* v6 )
{
    v6->u.Byte[ 0] = (addr->addr.ipv6[7]>>8) & 0xFF;
    v6->u.Byte[ 1] =  addr->addr.ipv6[7]     & 0xFF;
    v6->u.Byte[ 2] = (addr->addr.ipv6[6]>>8) & 0xFF;
    v6->u.Byte[ 3] =  addr->addr.ipv6[6]     & 0xFF;
    v6->u.Byte[ 4] = (addr->addr.ipv6[5]>>8) & 0xFF;
    v6->u.Byte[ 5] =  addr->addr.ipv6[5]     & 0xFF;
    v6->u.Byte[ 6] = (addr->addr.ipv6[4]>>8) & 0xFF;
    v6->u.Byte[ 7] =  addr->addr.ipv6[4]     & 0xFF;
    v6->u.Byte[ 8] = (addr->addr.ipv6[3]>>8) & 0xFF;
    v6->u.Byte[ 9] =  addr->addr.ipv6[3]     & 0xFF;
    v6->u.Byte[10] = (addr->addr.ipv6[2]>>8) & 0xFF;
    v6->u.Byte[11] =  addr->addr.ipv6[2]     & 0xFF;
    v6->u.Byte[12] = (addr->addr.ipv6[1]>>8) & 0xFF;
    v6->u.Byte[13] =  addr->addr.ipv6[1]     & 0xFF;
    v6->u.Byte[14] = (addr->addr.ipv6[0]>>8) & 0xFF;
    v6->u.Byte[15] =  addr->addr.ipv6[0]     & 0xFF;
}

int encode_sockaddr( const tl_net_addr* peer, void* addrbuffer, int* size )
{
    struct sockaddr_in6* v6addr = addrbuffer;
    struct sockaddr_in* v4addr = addrbuffer;

    if( !peer )
        return 0;

    if( peer->net==TL_IPV4 )
    {
        memset( v4addr, 0, sizeof(struct sockaddr_in) );
        v4addr->sin_addr.s_addr = htonl( peer->addr.ipv4 );
        v4addr->sin_port        = htons( peer->port );
        v4addr->sin_family      = AF_INET;
        *size                   = sizeof(struct sockaddr_in);
        return 1;
    }
    if( peer->net==TL_IPV6 )
    {
        memset( v6addr, 0, sizeof(struct sockaddr_in6) );
        convert_in6addr( peer, &(v6addr->sin6_addr) );
        v6addr->sin6_port   = htons( peer->port );
        v6addr->sin6_family = AF_INET6;
        *size               = sizeof(struct sockaddr_in6);
        return 1;
    }

    return 1;
}

SOCKET create_socket( const tl_net_addr* peer, void* addrbuffer,
                      int* size )
{
    int family, type, proto;

    if( !peer )
        return INVALID_SOCKET;

    if( !encode_sockaddr( peer, addrbuffer, size ) )
        return INVALID_SOCKET;

    switch( peer->net )
    {
    case TL_IPV4: family = PF_INET;  break;
    case TL_IPV6: family = PF_INET6; break;
    default:      return INVALID_SOCKET;
    }

    switch( peer->transport )
    {
    case TL_TCP: type = SOCK_STREAM; proto = IPPROTO_TCP; break;
    case TL_UDP: type = SOCK_DGRAM;  proto = IPPROTO_UDP; break;
    default:     return INVALID_SOCKET;
    }

    return socket( family, type, proto );
}

int decode_sockaddr_in( const void* addr, size_t len, tl_net_addr* out )
{
    const SOCKADDR_IN6* ipv6 = addr;
    const SOCKADDR_IN* ipv4 = addr;

    if( len==sizeof(SOCKADDR_IN) && ipv4->sin_family==AF_INET )
    {
        out->net       = TL_IPV4;
        out->port      = ntohs( ipv4->sin_port );
        out->addr.ipv4 = ntohl( ipv4->sin_addr.s_addr );
        return 1;
    }

    if( len==sizeof(SOCKADDR_IN6) && ipv6->sin6_family==AF_INET6 )
    {
        convert_ipv6( &(ipv6->sin6_addr), out );
        out->net  = TL_IPV6;
        out->port = ntohs( ipv6->sin6_port );
        return 1;
    }

    return 0;
}

int bind_socket( SOCKET sockfd, void* addrbuffer, int size )
{
    BOOL val = TRUE;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val) );
    return bind( sockfd, addrbuffer, size ) >= 0;
}

int set_socket_flags( SOCKET fd, int netlayer, int flags )
{
    if( flags & (~TL_ALL_NETWORK_FLAGS) )   /* unknown flags */
        return 0;

    if( (flags & TL_ALLOW_BROADCAST) && (netlayer == TL_IPV4) )
    {
        BOOL val = TRUE;
        if( setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(void*)&val,sizeof(BOOL)) )
            return 0;
    }

    if( (flags & TL_DONT_FRAGMENT) && (netlayer == TL_IPV4) )
    {
        DWORD opt = 1;
        setsockopt( fd, IPPROTO_IP, IP_DONTFRAGMENT,
                    (void*)&opt, sizeof(DWORD) );
    }
    return 1;
}

