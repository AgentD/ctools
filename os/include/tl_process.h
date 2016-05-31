/*
 * tl_process.h
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
/**
 * \file tl_process.h
 *
 * \brief Contains process utilities
 */
#ifndef TOOLS_PROCESS_H
#define TOOLS_PROCESS_H



#include "tl_predef.h"



/**
 * \page conc Concurrency
 *
 * \section proc Process utilities
 *
 * The tl_process structure encapsulates a child process and allows spawning
 * and managing child processes, including accessing the client input/output
 * streams via tl_iostream implementations. Refere to the tl_process_create
 * function on how to create a process with given command line arguments and
 * environment.
 *
 * An example program running the UNIX "cal" program in a child process:
 * \code{.c}
 * char buffer[1024];
 * tl_process* proc;
 * tl_iostream* io;
 * size_t bytes;
 *
 * static const char* args[] =
 * {
 *     "9",
 *     "1752",
 *     NULL
 * };
 *
 * proc = tl_process_create( "cal", args, NULL, TL_PIPE_STDOUT );
 * io = tl_process_get_stdio( proc );
 *
 * io->read( io, buffer, sizeof(buffer), &bytes );
 * buffer[ bytes ] = '\0';
 * puts( buffer );
 *
 * tl_process_wait( proc, NULL, 0 );
 * io->destroy( io );
 * tl_process_destroy( proc );
 * \endcode
 */

/**
 * \struct tl_process
 *
 * \brief Encapsulates a child process
 */

/**
 * \enum TL_PROCESS_FLAGS
 *
 * \brief Flags that can be used by tl_process_create
 */
typedef enum
{
    /**
     * \brief If set, redirect the input stream to a tl_iostream
     *
     * If not set, the standard input stream is connected to the standard
     * input stream of the parent process.
     */
    TL_PIPE_STDIN = 0x0001,

    /**
     * \brief If set, redirect the output stream to a tl_iostream
     *
     * If not set, the standard output stream is connected to the standard
     * output stream of the parent process.
     */
    TL_PIPE_STDOUT = 0x0002,

    /**
     * \brief If set, redirect the error stream to a tl_iostream
     *
     * If not set, the standard error stream is connected to the standard
     * error stream of the parent process.
     */
    TL_PIPE_STDERR = 0x0004,

    /** \brief If set, redirect the error stream to the output stream */
    TL_STDERR_TO_STDOUT = 0x0008
}
TL_PROCESS_FLAGS;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a child process, running a specified executable file
 *
 * \memberof tl_process
 *
 * \param filename A path to an executable file
 * \param argv     A NULL-terminated array of process arguments. Must not
 *                 be NULL.
 * \param env      A NULL-terminated array of environment strings. The system
 *                 might also append further environment strings. If NULL, the
 *                 child process inherits the environment of the parent.
 * \param flags    A combination of \ref TL_PROCESS_FLAGS
 */
TLOSAPI tl_process* tl_process_create( const char* filename,
                                       const char* const* argv,
                                       const char* const* env, int flags );

/**
 * \brief Kill a child process and free the memory used for managing it
 *
 * \memberof tl_process
 *
 * \param proc a pointer to a process object
 */
TLOSAPI void tl_process_destroy( tl_process* proc );

/**
 * \brief Get the input and output stream of a process
 *
 * \memberof tl_process
 *
 * A NULL pointer can be passed to this function to get the stdio stream of
 * the calling process. Do not attempt to destroy this stream.
 *
 * For a child process that was created with the TL_PIPE_STDIN flag, data
 * written to the stream is sent to STDIN of the child process. If the process
 * was created with the TL_PIPE_STDOUT flag, data written by the process to
 * STDOUT can be read from the iostream object. If the flag
 * TL_STDERR_TO_STDOUT was set,  STDERR data also appears on the
 * stream object. If neither TL_PIPE_STDOUT nor TL_PIPE_STDIN was set,
 * this function returns NULL.
 *
 * \param proc A pointer to the process object or NULL for the calling process
 *
 * \return A pointer to a stream encapsulating the STDOUT and STDIN streams of
 *         the child, or NULL if they are not redirected.
 */
TLOSAPI tl_iostream* tl_process_get_stdio( tl_process* proc );

/**
 * \brief Get the error stream of a process
 *
 * \memberof tl_process
 *
 * A NULL pointer can be passed to this function to get the stderr stream of
 * the calling process. Do not attempt to destroy this stream.
 *
 * For child processes created with TL_PIPE_STDERR flag set, this function
 * returns a stream object that allows reading the data a child process writes
 * to STDERR. If TL_PIPE_STDERR was not set, or TL_STDERR_TO_STDOUT is set,
 * this returns NULL.
 *
 * Data cannot be writen to the stream.
 *
 * \param proc A pointer to the process objet or NULL for the calling process
 *
 * \return A pointer to a stream encapsulating the STDERR stream or NULL if
 *         it is not redirected
 */
TLOSAPI tl_iostream* tl_process_get_stderr( tl_process* proc );

/**
 * \brief Immediately terminate a child process
 *
 * On Unix-like systems, SIGKILL is sent to the process. On Windows, the
 * TerminateProcess function is used.
 *
 * \memberof tl_process
 *
 * \param proc A pointer to a process object
 */
TLOSAPI void tl_process_kill( tl_process* proc );

/**
 * \brief Ask a child process to terminate
 *
 * On Unix-like systems, SIGTERM is sent to the process. On Windows, a WM_QUIT
 * message is sent to the process.
 *
 * \memberof tl_process
 *
 * \param proc A pointer to a process object
 */
TLOSAPI void tl_process_terminate( tl_process* proc );

/**
 * \brief Wait for a process to terminate and get its exit status
 *
 * \memberof tl_process
 *
 * This function blocks until the given chid process terminates and retrieves
 * the exit status of the process, or alternatively returns if a maximum
 * timeout passed and the process did not exit.
 *
 * \note Whether an arbitrary process ever terminates or not is undecidable.
 *
 * \param proc    A pointer to a process object
 * \param status  If not NULL, returns the exit status of the process
 * \param timeout A number of milliseconds to wait for the process to
 *                terminate, or 0 for infinite
 *
 * \return Zero if the process terminated, TL_ERR_TIMEOUT if a timeout
 *         occoured, TL_ERR_NOT_EXIST if the process does not exist and
 *         TL_ERR_INTERNAL if another kind of error occoured.
 */
TLOSAPI int tl_process_wait( tl_process* proc, int* status,
                             unsigned int timeout );

/**
 * \brief Make the calling thread block for a specified number of milliseconds
 *
 * This function waits at least for the specified time.
 *
 * \param ms A number of milliseconds to wait
 */
TLOSAPI void tl_sleep( unsigned long ms );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_PROCESS_H */

