/*
 * tl_server.h
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
 * \file tl_server.h
 *
 * \brief Contains an abstract server interface (tl_server)
 */
#ifndef TOOLS_SERVER_H
#define TOOLS_SERVER_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_server The tl_server interface
 *
 * The tl_server interface abstracts stream or packet based communication of
 * a single process with an arbitrary number client processes. The client
 * communications are abstracted in the form of tl_iostream instances that the
 * server creates when a new client connects or (for non-connection based
 * communications) when the first data from a particular client is received.
 *
 * Usage example:
 * \code{.c}
 * tl_server* srv = function_that_creates_a_server( );
 * int run = 1;
 *
 * while( run )
 * {
 *     tl_iostream* client = srv->wait_for_client( srv, 0 );
 *
 *     handle_client( client );
 * }
 *
 * srv->destroy( destroy );
 * \endcode
 */



#include "tl_predef.h"



/**
 * \interface tl_server
 *
 * \brief An interface that abstracts stream or packet based one-to-many
 *        communication on the "one" end.
 */
struct tl_server
{
    /**
     * \brief Destroy a server object shutting down all connections and,
     *        freeing all its resources
     *
     * \param server A pointer to the server object
     */
    void(* destroy )( tl_server* server );

    /**
     * \brife Wait until a client connects
     *
     * For connection based network protocols, this function simply waits for
     * a new incomming connection and creates a tl_iostream implementation
     * object for the connection.
     *
     * For protocols that are not connection based, or other cases where the
     * API used by the implementation does not support seperate client
     * connections, this function is responsible for handling client
     * demultiplexing internally and the returned stream is a demultiplexed
     * stream that only handles communication with a specific client.
     *
     * \param server  A pointer to a server object
     * \param timeout A timeout in milliseconds, or zero (or a negative value)
     *                for infinite timeout.
     */
    tl_iostream* (* wait_for_client )( tl_server* server, int timeout );
};



#endif /* TOOLS_SERVER_H */

