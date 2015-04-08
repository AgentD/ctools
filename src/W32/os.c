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
#define TL_EXPORT
#include "os.h"

#ifdef _MSC_VER
    #include <intrin.h>

    #pragma intrinsic (_InterlockedIncrement)
    #pragma intrinsic (_InterlockedDecrement)

    static volatile LONG refcount = 0;
#else
    static volatile int refcount = 0;
#endif

CRITICAL_SECTION udp_server_mutex;



int errno_to_fs( int code )
{
    switch( code )
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_DIRECTORY:
        return TL_FS_NOT_EXIST;

    case ERROR_INVALID_ACCESS:
        return TL_FS_ACCESS;

    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return TL_FS_EXISTS;


    case ERROR_DISK_FULL:
        return TL_FS_NO_SPACE;

    case ERROR_DIR_NOT_EMPTY:
        return TL_FS_NOT_EMPTY;

        /*return TL_FS_NOT_DIR;*/
    }

    return code==0 ? 0 : TL_FS_SYS_ERROR;
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

    InitializeCriticalSection( &udp_server_mutex );

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
        DeleteCriticalSection( &udp_server_mutex );
        WSACleanup( );
    }
}

int stream_write_blob( tl_iostream* this, const tl_blob* blob,
                       size_t* actual )
{
    if( !blob )
        return TL_IO_INTERNAL;

    return this->write_raw( this, blob->data, blob->size, actual );
}

int stream_read_blob( tl_iostream* this, tl_blob* blob, size_t maximum )
{
    int status;

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_IO_INTERNAL;

    status = this->read_raw( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
}

/****************************************************************************/

int monitor_init( monitor_t* this )
{
    this->cond = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( !this->cond )
        return 0;

    InitializeCriticalSection( &(this->mutex) );
    this->timeout = INFINITE;
    return 1;
}

void monitor_cleanup( monitor_t* this )
{
    CloseHandle( this->cond );
    DeleteCriticalSection( &(this->mutex) );
}

int monitor_wait( monitor_t* this )
{
    DWORD status, timeout;

    timeout = this->timeout ? this->timeout : INFINITE;

    LeaveCriticalSection( &(this->mutex) );
    status = WaitForMultipleObjects( 1, &(this->cond), FALSE, timeout );
    EnterCriticalSection( &(this->mutex) );

    return status!=WAIT_TIMEOUT && status!=WAIT_FAILED;
}

