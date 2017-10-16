/* sockstream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "os.h"
#include "tl_iostream.h"

static void sockstream_destroy(tl_iostream *super)
{
	sockstream *this = (sockstream *)super;

	assert(this);

	closesocket(this->socket);
	free(this);
	winsock_release();
}

static int sockstream_set_timeout(tl_iostream *this, unsigned int timeout)
{
	assert(this);

	((sockstream *)this)->timeout = timeout;
	return 0;
}

static int sockstream_write_raw(tl_iostream *super, const void *buffer,
				size_t size, size_t *actual)
{
	sockstream *this = (sockstream *)super;
	int status;

	assert(this && buffer);

	if (actual)
		*actual = 0;
	if (!size)
		return 0;

	if (!wait_for_fd(this->socket, this->timeout, 1))
		return TL_ERR_TIMEOUT;

	status = send(((sockstream *)this)->socket, buffer, size, 0);

	if (status < 0)
		return WSAHandleFuckup();
	if (actual)
		*actual = status;
	return 0;
}

static int sockstream_read_raw(tl_iostream *super, void *buffer,
			       size_t size, size_t *actual)
{
	sockstream *this = (sockstream *)super;
	int status;

	assert(this && buffer);

	if (actual)
		*actual = 0;
	if (!size)
		return 0;
	if (!wait_for_fd(this->socket, this->timeout, 0))
		return TL_ERR_TIMEOUT;

	status = recv(this->socket, buffer, size, 0);

	if (status == 0)
		return TL_ERR_CLOSED;
	if (status < 0)
		return WSAHandleFuckup();
	if (actual)
		*actual = status;
	return 0;
}

/****************************************************************************/

tl_iostream *sock_stream_create(SOCKET sockfd, int proto)
{
	sockstream *this = calloc(1, sizeof(*this));
	tl_iostream *super = (tl_iostream *)this;

	if (!this)
		return NULL;

	this->socket = sockfd;
	this->proto = proto;
	super->type = TL_STREAM_TYPE_SOCK;
	super->destroy = sockstream_destroy;
	super->set_timeout = sockstream_set_timeout;
	super->write = sockstream_write_raw;
	super->read = sockstream_read_raw;
	return super;
}
