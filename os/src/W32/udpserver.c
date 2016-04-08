/*
 * udpserver.c
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


static volatile int run = 0;
static volatile udp_server* servers = NULL;
static HANDLE listen_thread;


static DWORD WINAPI udp_server_listen_thread( LPVOID arg )
{
    unsigned char buffer[ 512 ], addrbuffer[ 64 ];
    int maxfd = 0, isnew;
    struct timeval tv;
    socklen_t addrlen;
    udp_server* srv;
    udp_stream* str;
    fd_set in_fds;
    int len;

    while( run )
    {
        /* add all server sockets to file descriptor set */
        FD_ZERO( &in_fds );
        EnterCriticalSection( &udp_server_mutex );

        for( srv=(udp_server*)servers; srv!=NULL; srv=srv->next )
        {
            FD_SET( srv->socket, &in_fds );
            maxfd = srv->socket > maxfd ? srv->socket : maxfd;
        }

        LeaveCriticalSection( &udp_server_mutex );

        /* wait for data to arive on any socket */
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        if( select( maxfd+1, &in_fds, 0, 0, &tv )<=0 )
            continue;

        /* route received data to propper server/stream instances */
        EnterCriticalSection( &udp_server_mutex );

        for( srv=(udp_server*)servers; srv!=NULL; srv=srv->next )
        {
            if( !FD_ISSET( srv->socket, &in_fds ) )
                continue;

            addrlen = sizeof(addrbuffer);
            len = recvfrom( srv->socket, (char*)buffer, sizeof(buffer), 0,
                            (void*)addrbuffer, &addrlen );

            if( len<=0 )
                continue;

            tl_monitor_lock( &(srv->monitor), 0 );
            isnew = 0;

            for( str=srv->streams; str!=NULL; str=str->next )
            {
                if( str->addrlen==(int)addrlen &&
                    !memcmp( str->address, addrbuffer, addrlen ) )
                {
                    break;
                }
            }

            if( !str )
            {
                if( !(str = udp_stream_create( srv, addrbuffer, addrlen )) )
                    continue;

                isnew = 1;
                srv->pending += 1;
                str->next = srv->streams;
                srv->streams = str;
            }

            udp_stream_add_data( str, buffer, len );

            if( isnew )
                tl_monitor_notify( &(srv->monitor) );
            tl_monitor_unlock( &(srv->monitor) );
        }

        LeaveCriticalSection( &udp_server_mutex );
    }

    return arg!=NULL;
}

static void add_server( udp_server* server )
{
    EnterCriticalSection( &udp_server_mutex );
    server->next = (udp_server*)servers;
    servers = server;

    if( !(run++) )
    {
        listen_thread = CreateThread( NULL, 0, udp_server_listen_thread,
                                      NULL, 0, NULL );
    }

    LeaveCriticalSection( &udp_server_mutex );
}

static void remove_server( udp_server* server )
{
    udp_server* i;
    int stop;

    EnterCriticalSection( &udp_server_mutex );
    if( server==(udp_server*)servers )
    {
        servers = servers->next;
    }
    else
    {
        for( i=(udp_server*)servers; i!=NULL; i=i->next )
        {
            if( i->next == server )
            {
                i->next = server->next;
                break;
            }
        }
    }

    stop = (--run)==0;
    LeaveCriticalSection( &udp_server_mutex );

    if( stop )
    {
        WaitForSingleObject( listen_thread, INFINITE );
        CloseHandle( listen_thread );
    }
}

/****************************************************************************/

static void udp_server_destroy( tl_server* super )
{
    udp_server* this = (udp_server*)super;
    udp_stream* i;

    for( i=this->streams; i!=NULL; i=i->next )
        i->parent = NULL;

    remove_server( this );
    tl_monitor_cleanup( &(this->monitor) );
    closesocket( this->socket );
    free( this );

    winsock_release( );
}

static tl_iostream* udp_wait_for_client( tl_server* super, int timeout )
{
    udp_server* this = (udp_server*)super;
    udp_stream* str = NULL;
    int i;

    tl_monitor_lock( &(this->monitor), timeout );
    if( this->pending == 0 )
        tl_monitor_wait( &(this->monitor), timeout );

    if( this->pending != 0 )
    {
        str = this->streams;
        i = 1;
        for( ; i<this->pending && str; ++i, str=str->next ) { }
        --this->pending;
    }
    tl_monitor_unlock( &(this->monitor) );
    return (tl_iostream*)str;
}

tl_server* udp_server_create( SOCKET sockfd )
{
    udp_server* this = calloc( 1, sizeof(udp_server) );
    tl_server* super = (tl_server*)this;

    if( !this )
        return NULL;

    if( !tl_monitor_init( &(this->monitor) ) )
    {
        free( this );
        return NULL;
    }

    this->socket           = sockfd;
    super->destroy         = udp_server_destroy;
    super->wait_for_client = udp_wait_for_client;

    add_server( this );
    return super;
}

