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
 * \section server The tl_server interface
 *
 * The tl_server interface abstracts stream based communication of a single
 * end-point with an arbitrary number of clients (e.g. a TCP server). The
 * client communications are abstracted in the form of tl_iostream instances
 * that the server creates when a new client connects.
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
 *
 * For "raw" packet based, non-connection oriented communications (e.g. UDP),
 * the \ref tl_packetserver interface is used.
 */



#include "tl_predef.h"



/**
 * \interface tl_server
 *
 * \brief An interface that abstracts byte stream based one-to-many
 *        communication on the "one" end.
 *
 * \see \ref server
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
     * \brief Wait until a new client connects
     *
     * \param server  A pointer to a server object
     * \param timeout A timeout in milliseconds, or zero (or a negative value)
     *                for infinite timeout.
     */
    tl_iostream* (* wait_for_client )( tl_server* server, int timeout );
};



#endif /* TOOLS_SERVER_H */

