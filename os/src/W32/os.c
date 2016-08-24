/* os.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
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

    case ERROR_BROKEN_PIPE:
        return TL_ERR_CLOSED;
    case ERROR_HANDLE_EOF:
        return TL_EOF;

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

int wait_for_fd( SOCKET socket, unsigned long timeout, int write )
{
    struct timeval tv;
    fd_set fds;

    FD_ZERO( &fds );
    FD_SET( socket, &fds );

    tv.tv_sec = timeout / 1000L;
    tv.tv_usec = (timeout - tv.tv_sec * 1000L) * 1000L;

    if( select( socket+1, write ? 0 : &fds,
                write ? &fds : 0, 0, timeout > 0 ? &tv : NULL ) <= 0 )
    {
        return 0;
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

int set_socket_flags( SOCKET fd, int netlayer, int* flags )
{
    BOOL bval = TRUE;

    if( (*flags) & (~TL_ALL_NETWORK_FLAGS) )   /* unknown flags */
        return 0;

    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*)&bval, sizeof(bval) );

    if( ((*flags) & TL_ALLOW_BROADCAST) && (netlayer == TL_IPV4) )
    {
        bval = TRUE;
        if( setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(void*)&bval,sizeof(bval)) )
            return 0;
    }

    if( ((*flags) & TL_DONT_FRAGMENT) && (netlayer == TL_IPV4) )
    {
        DWORD opt = 1;
        setsockopt( fd, IPPROTO_IP, IP_DONTFRAGMENT,
                    (void*)&opt, sizeof(DWORD) );
    }
    return 1;
}

tl_s64 w32_lseek(HANDLE hf, tl_s64 pos, DWORD MoveMethod)
{
    LARGE_INTEGER li;

    li.QuadPart = pos;
    li.LowPart = SetFilePointer( hf, li.LowPart, &li.HighPart, MoveMethod );

    if( li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
        li.QuadPart = -1;

    return li.QuadPart;
}

