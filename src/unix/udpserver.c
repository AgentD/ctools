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
#define TL_EXPORT
#include "os.h"



static volatile int run = 0;
static volatile udp_server* servers = NULL;
static pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t listen_thread;
static int selfpipe[2];



static void* udp_server_listen_thread( void* arg )
{
    unsigned char buffer[ 512 ], addrbuffer[ 64 ];
    int maxfd = 0, isnew;
    struct timeval tv;
    socklen_t addrlen;
    udp_server* srv;
    udp_stream* str;
    fd_set in_fds;
    ssize_t len;

    while( run )
    {
        /* add all server sockets to file descriptor set */
        pthread_mutex_lock( &server_mutex );
        FD_ZERO( &in_fds );

        for( srv=(udp_server*)servers; srv!=NULL; srv=srv->next )
        {
            FD_SET( srv->socket, &in_fds );
            maxfd = srv->socket > maxfd ? srv->socket : maxfd;
        }

        pthread_mutex_unlock( &server_mutex );

        FD_SET( selfpipe[0], &in_fds );
        maxfd = selfpipe[0] > maxfd ? selfpipe[0] : maxfd;

        /* wait for data to arive on any socket */
        tv.tv_sec = 100;
        tv.tv_usec = 0;

        if( select( maxfd+1, &in_fds, 0, 0, &tv )<=0 )
            continue;

        if( FD_ISSET(selfpipe[0], &in_fds) )
            break;

        /* route received data to propper server/stream instances */
        pthread_mutex_lock( &server_mutex );

        for( srv=(udp_server*)servers; srv!=NULL; srv=srv->next )
        {
            if( !FD_ISSET( srv->socket, &in_fds ) )
                continue;

            addrlen = sizeof(addrbuffer);
            len = recvfrom( srv->socket, buffer, sizeof(buffer), 0,
                            (void*)addrbuffer, &addrlen );

            if( len<=0 )
                continue;

            pt_monitor_lock( &(srv->monitor) );
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
                pt_monitor_notify( &(srv->monitor) );
            pt_monitor_unlock( &(srv->monitor) );
        }

        pthread_mutex_unlock( &server_mutex );
    }

    close( selfpipe[0] );
    return arg;
}

static void add_server( udp_server* server )
{
    pthread_mutex_lock( &server_mutex );
    server->next = (udp_server*)servers;
    servers = server;

    if( !(run++) )
    {
        pipe( selfpipe );
        pthread_create( &listen_thread, NULL,
                        udp_server_listen_thread, NULL );
    }
    pthread_mutex_unlock( &server_mutex );
}

static void remove_server( udp_server* server )
{
    udp_server* i;
    int stop;

    pthread_mutex_lock( &server_mutex );
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
    pthread_mutex_unlock( &server_mutex );

    if( stop )
    {
        write( selfpipe[1], &stop, sizeof(stop) );
        pthread_join( listen_thread, NULL );
        close( selfpipe[1] );
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
    pt_monitor_cleanup( &(this->monitor) );
    close( this->socket );
    free( this );
}

static tl_iostream* udp_wait_for_client( tl_server* super, int timeout )
{
    udp_server* this = (udp_server*)super;
    udp_stream* str = NULL;
    int i;

    pt_monitor_lock( &(this->monitor) );
    pt_monitor_set_timeout( &(this->monitor), timeout );
    if( this->pending == 0 )
        pt_monitor_wait( &(this->monitor) );

    if( this->pending != 0 )
    {
        str = this->streams;
        i = 1;
        for( ; i<this->pending && str; ++i, str=str->next ) { }
        --this->pending;
    }
    pt_monitor_unlock( &(this->monitor) );
    return (tl_iostream*)str;
}

tl_server* udp_server_create( int sockfd )
{
    udp_server* this = malloc( sizeof(udp_server) );
    tl_server* super = (tl_server*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(udp_server) );

    if( !pt_monitor_init( &(this->monitor) ) )
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

