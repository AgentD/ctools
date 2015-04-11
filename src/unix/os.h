/*
 * os.h
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
#ifndef OS_H
#define OS_H



#include "tl_iostream.h"
#include "tl_server.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_fs.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>



typedef struct udp_stream udp_stream;
typedef struct udp_server udp_server;
typedef struct pt_monitor pt_monitor;



struct pt_monitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct timespec timeout;
};

struct udp_stream
{
    tl_iostream super;
    pt_monitor monitor;
    udp_stream* next;           /* linked list pointer */
    tl_array buffer;            /* incoming data waiting to be read */
    int addrlen;                /* size of peer address */
    udp_server* parent;         /* parent server */
    unsigned char address[1];   /* peer address */
};

struct udp_server
{
    tl_server super;
    pt_monitor monitor;
    int socket;                 /* udp socket */
    int pending;                /* number of streams not yet accepted */
    udp_stream* streams;        /* list of server-to-client */
    udp_server* next;           /* linked list pointer */
};




#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate an errno value to an TL_FS_* error code */
int errno_to_fs( int code );

/**
 * \brief Convert an in6_addr structure to a tl_net_addr
 *
 * \param v6   Pointer to input data
 * \param addr Pointer to output structure
 */
void convert_ipv6( const struct in6_addr* v6, tl_net_addr* addr );

/**
 * \brief Convert a tl_net_addr structure to an in6_addr
 *
 * \param addr Pointer to input data
 * \param v6   Pointer to output structure
 */
void convert_in6addr( const tl_net_addr* addr, struct in6_addr* v6 );

/**
 * \brief Create a tl_iostream implementation that manages a connected socket
 *
 * \param sockfd A socket file descriptor
 */
tl_iostream* sock_stream_create( int sockfd );

/**
 * \brief Create a tl_iostream implementation that operates on a pipe
 *
 * \param readpipe  A file descriptor to read from
 * \param writepipe A file descriptor to write to
 */
tl_iostream* pipe_stream_create( int readpipe, int writepipe );

/**
 * \brief Create a tl_server implementation instance that offers a TCP server
 *
 * \param sockfd  A server socket file descriptor
 * \param backlog The connection backlog for the listen syscall
 */
tl_server* tcp_server_create( int sockfd, unsigned int backlog );

/**
 * \brief Create a server to client UDP stream implementation
 *
 * \param parent  The server that the stream belongs to
 * \param addr    The peer address
 * \param addrlen The size of the peer address
 */
udp_stream* udp_stream_create( udp_server* parent, void* addr, int addrlen );

/**
 * \brief Append received data to a UDP stream
 *
 * \param stream A pointer to the stream object
 * \param buffer A pointer to the buffer to append
 * \param size   The number of bytes to copy
 */
void udp_stream_add_data( udp_stream* stream, void* buffer, size_t size );

/**
 * \brief Create a UDP tl_server implementation
 *
 * \param sockfd The server socket to use
 */
tl_server* udp_server_create( int sockfd );

/** \brief Initialize a monitor object */
int pt_monitor_init( pt_monitor* monitor );

/** \brief Destruct and cleanup a monitor object */
void pt_monitor_cleanup( pt_monitor* monitor );

/** \brief Set the timeout in milliseconds on a monitor */
void pt_monitor_set_timeout( pt_monitor* monitor, unsigned int ms );

/**
 * \brief Wait for a condition on a montitor
 *
 * \param monitor A pointer to the monitor
 *
 * \return Non-zero on success, zero if interrupted or timed out
 */
int pt_monitor_wait( pt_monitor* monitor );

#define pt_monitor_lock( m ) pthread_mutex_lock( &((m)->mutex) )

#define pt_monitor_unlock( m ) pthread_mutex_unlock( &((m)->mutex) )

#define pt_monitor_notify( m ) pthread_cond_signal( &((m)->cond) )

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

