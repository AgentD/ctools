/*
 * tcpserver.c
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
#include "tl_server.h"
#include <sys/select.h>
#include <sys/time.h>



typedef struct
{
    tl_server super;
    int socket;
}
tcp_server;



static void tcp_destroy( tl_server* this )
{
    close( ((tcp_server*)this)->socket );
    free( this );
}

static tl_iostream* tcp_wait_for_client( tl_server* super, int timeout )
{
    tcp_server* this = (tcp_server*)super;
    struct timeval tv;
    fd_set in_fds;
    int peer;

    if( timeout > 0 )
    {
        FD_ZERO( &in_fds );
        FD_SET( this->socket, &in_fds );

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = timeout * 1000 - tv.tv_sec * 1000000;

        if( select( this->socket+1, &in_fds, 0, 0, &tv ) <= 0 )
            return NULL;
    }

    peer = accept( this->socket, NULL, 0 );

    return peer<0 ? NULL : sock_stream_create( peer, USTR_SOCK|USTR_TCP );
}

/****************************************************************************/

tl_server* tcp_server_create( int sockfd, unsigned int backlog )
{
    tcp_server* this = malloc( sizeof(tcp_server) );
    tl_server* super = (tl_server*)this;

    if( listen( sockfd, backlog ) < 0 )
    {
        free( this );
        return NULL;
    }

    this->socket           = sockfd;
    super->destroy         = tcp_destroy;
    super->wait_for_client = tcp_wait_for_client;
    return super;
}

