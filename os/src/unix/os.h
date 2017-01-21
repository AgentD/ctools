/* os.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef OS_H
#define OS_H



#include "tl_iostream.h"
#include "tl_thread.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_file.h"
#include "tl_fs.h"

#include "../platform.h"
#include "../bsdsock/bsdsock.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/mman.h>
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
typedef struct file_stream file_stream;

typedef enum
{
    STREAM_UDP = 0x0001,
    STREAM_TCP = 0x0002
}
STREAM_FLAGS;

struct file_stream
{
    tl_file super;
    unsigned int timeout;
    int flags;
    int fd;
}; 

struct fd_stream
{
    tl_iostream super;
    int flags;
    int readfd;
    int writefd;
    unsigned int timeout;
};

struct tl_monitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

#define sock_stream_create(fd, proto) \
	fdstream_create(fd,fd,TL_STREAM_TYPE_SOCK, \
			(proto)==TL_UDP ? STREAM_UDP : STREAM_TCP)

extern fd_stream tl_stdio;
extern fd_stream tl_stderr;

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
 * \brief Create a tl_iostream implementation that operates on a pipe
 *
 * \param readpipe  A file descriptor to read from
 * \param writepipe A file descriptor to write to
 * \param type      Stream type to set for the tl_iostream
 * \param flags     Flags to set for the fd_stream structure
 */
tl_iostream* fdstream_create( int readpipe, int writepipe,
                              int type, int flags );

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

