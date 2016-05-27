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
#include "tl_thread.h"
#include "tl_server.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_fs.h"

#include "../platform.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>



typedef struct fd_stream fd_stream;

struct fd_stream
{
    tl_iostream super;
    int readfd;
    int writefd;
    unsigned int timeout;
};

struct tl_monitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

typedef enum
{
    TL_ENFORCE_V6_ONLY = 0x1000
}
TL_NETWORK_INTERNAL_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate an errno value to an TL_FS_* error code */
int errno_to_fs( int code );

/**
 * \brief Wait for a file descriptor to become read or writeable
 *
 * \param fd        The file descriptor
 * \param timeoutms Timeout in milli seconds (zero for infinite)
 * \param writeable Zero to wait for the descriptor to become readable,
 *                  non-zero to wait for it to become writeable
 */
int wait_for_fd( int fd, unsigned long timeoutms, int writeable );

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
 * \brief Create a tl_iostream implementation that operates on a pipe
 *
 * \param readpipe  A file descriptor to read from
 * \param writepipe A file descriptor to write to
 * \param flags     Flags to set for the underlying tl_iostream structure
 */
tl_iostream* pipe_stream_create( int readpipe, int writepipe, int flags );

/**
 * \brief Create a tl_server implementation instance that offers a TCP server
 *
 * \param sockfd  A server socket file descriptor
 * \param backlog The connection backlog for the listen syscall
 */
tl_server* tcp_server_create( int sockfd, unsigned int backlog, int flags );

/**
 * \brief Convert a tl_net_addr structure to a sockaddr_in
 *
 * \param peer       The intput address
 * \param addrbuffer A pointer to a buffer to write the sockaddr_in structure
 *                   data to
 * \param size       Returns the number of bytes written to the addrbuffer
 *
 * \return Non-zero on success, zero on failure
 */
int encode_sockaddr( const tl_net_addr* peer, void* addrbuffer, int* size );

/**
 * \brief Create a socket
 *
 * \param peer       The address to bind to
 * \param addrbuffer A pointer to a buffer to write the sockaddr_in structure
 *                   data to
 * \param size       Returns the number of bytes written to the addrbuffer
 *
 * \return A socket file descriptor on success, -1 on failrue
 */
int create_socket( const tl_net_addr* peer, void* addrbuffer, int* size );

/**
 * \brief Bind a socket to an address
 *
 * \param sockfd   The socket file descriptor
 * \param address  A pointer to the address structure
 * \param addrsize The size of the address structure
 *
 * \return Non-zero on success, zero on failure
 */
int bind_socket( int sockfd, void* address, int addrsize );

/**
 * \brief Decode a sockaddr_in or sockaddr_in6 structure to a tl_net_addr
 *
 * \param addr A pointer to the address structure
 * \param len  The size of the address structure
 * \param out  A pointer to the tl_net_addr structure to write to
 *
 * \return Non-zero on success, zero on failure
 */
int decode_sockaddr_in( const void* addr, size_t len, tl_net_addr* out );

/** \brief Initialize a monitor object */
int tl_monitor_init( tl_monitor* monitor );

/** \brief Destruct and cleanup a monitor object */
void tl_monitor_cleanup( tl_monitor* monitor );

/**
 * \brief Convert a relative timeout to an absolute timespec value
 *
 * \param timeout A timeout in milli seconds
 * \param ts      Returns the absolute time stamp when the timeout occoures
 */
void timeout_to_abs( unsigned long timeout, struct timespec* ts );

/**
 * \brief A version of waitpid that has a millisecond timeout
 *
 * \param pid     The process ID to wait for
 * \param status  If not NULL, returns the exit status of the process
 * \param timeout Maximum number of milli seconds to wait for the process
 *
 * \return The pid of the process on success, 0 if a timeout occoured, a
 *         negative value on error.
 */
pid_t wait_pid_ms( pid_t pid, int* status, unsigned long timeout );

/**
 * \brief Apply \ref TL_NETWORK_FLAGS to a socket
 *
 * \param fd       Socket file descriptor
 * \param netlayer A \ref TL_NETWORK_PROTOCOL value
 * \param flags    Pointer to the flags to apply to the socket. Returns the
 *                 successfully set flags, possibly combined with
 *                 \ref TL_NETWORK_INTERNAL_FLAGS values.
 *
 * \return Non-zero on success, zero on failure
 */
int set_socket_flags( int fd, int netlayer, int* flags );

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

