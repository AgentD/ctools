/*
 * tl_packetserver.h
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
 * \file tl_packetserver.h
 *
 * \brief Contains the tl_packetserver structure
 */
#ifndef TL_PACKETSERVER_H
#define TL_PACKETSERVER_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_packetserver The tl_packetserver interface
 *
 * The tl_packetserver interface abstracts stateless, one-to-many
 * communication over an input/output device that sends and receives discrete
 * packets of data and does not have a conecpt of connections.
 *
 * Examples for an implentation could be a UDP server, an I2C bus, et cetera.
 *
 * In contrast to the tl_server interface, the tl_packetserver does neither
 * handle packet demultiplexing, nor does it keep track of connections. The
 * tl_server interface internally demultiplexes incoming data into individual
 * end-to-end byte streams, whereas the tl_packetserver has methods for
 * sending and receiving packets and exposes the peer addresses, expecting the
 * code using it to handle packets and addresses propperly.
 *
 * Note that both tl_packetserver and tl_server have implementations that may
 * not be typicall for the given description. On the one hand, there is a
 * tl_server that handles UDP communications, demultiplexing received packets
 * into byte streams, emulating connections. On the other hand, there is a
 * tl_packetserver implementation that emulates reliable, stateles, TCP based
 * packet I/O by shortly opening a connection, transmitting data and closing
 * the connection again.
 *
 * A tl_packetserver is prefereable for applications like sending broadcast
 * packets and receiving answers to broadcasts.
 */



/**
 * \interface tl_packetserver
 *
 * \brief Abstracts stateless, one-to-many commication via a purely packet
 *        based I/O device.
 */
struct tl_packetserver
{
    /**
     * \brief Set a maximum timeout before giving up receiving or transmitting
     *
     * \param server  A pointer to a server object
     * \param timeout The maximum number of milliseconds to keep waiting. Zero
     *                for infinite.
     */
    void(* set_timeout )( tl_packetserver* server, unsigned int timeout );

    /**
     * \brief Receive a packet
     *
     * \param server  A pointer to the server object
     * \param buffer  A pointer to the buffer to write data to
     * \param address A pointer to an implementation specific structure to
     *                write the sender address to.
     * \param size    The size of the buffer, i.e. maximum bytes to receive
     * \param actual  Returns the number of bytes actually received
     *
     * \return Zero on success, a TL_ERROR_CODE value if an error occoured.
     */
    int (* receive )( tl_packetserver* server, void* buffer, void* address,
                      size_t size, size_t* actual );

    /**
     * \brief Send a packet
     *
     * \param server  A pointer to the server object
     * \param buffer  A pointer to the data to send
     * \param address A pointer to an implementation specific structure
     *                holding the destination address.
     * \param size    The number of bytes to send
     * \param actual  Returns the number of bytes actually sent
     *
     * \return Zero on success, a TL_ERROR_CODE value if an error occoured.
     */
    int (* send )( tl_packetserver* server, const void* buffer,
                   const void* address, size_t size, size_t* actual );

    /**
     * \brief Destroy a tl_packetserver object and release all its memory
     *
     * \param server A pointer to the server object
     */
    void (* destroy )( tl_packetserver* server );
};



#endif /* TL_PACKETSERVER_H */

