/* fdstream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "os.h"
#include "tl_iostream.h"

static void fd_stream_destroy(tl_iostream *super)
{
	fd_stream *this = (fd_stream *)super;
	assert(this);
	if (this->readfd >= 0)
		close(this->readfd);
	if ((this->readfd != this->writefd) && (this->writefd >= 0))
		close(this->writefd);
	free(this);
}

static int fd_stream_set_timeout(tl_iostream *this, unsigned int timeout)
{
	assert(this);
	((fd_stream *)this)->timeout = timeout;
	return 0;
}

static int fd_stream_write(tl_iostream *super, const void *buffer,
			   size_t size, size_t *actual)
{
	fd_stream *this = (fd_stream *)super;
	ssize_t result = 0, intr_count = 0;
	int ret = 0;

	assert(this && buffer);

	if (this->writefd < 0)
		return TL_ERR_NOT_SUPPORTED;
	if (!size)
		goto out;

retry:
	if (!wait_for_fd(this->writefd, this->timeout, 1))
		return TL_ERR_TIMEOUT;

	if (super->type == TL_STREAM_TYPE_SOCK) {
		result = sendto(this->writefd, buffer, size,
				MSG_NOSIGNAL, NULL, 0);
	} else {
		result = write(this->writefd, buffer, size);
	}

	if (result < 0) {
		if (errno == EINTR && (intr_count++) < 3)
			goto retry;
		ret = errno_to_fs(errno);
	}

out:
	if (actual)
		*actual = result;
	return ret;
}

static int fd_stream_read(tl_iostream *super, void *buffer,
			  size_t size, size_t *actual)
{
	fd_stream *this = (fd_stream *)super;
	ssize_t result = 0, intr_count = 0;
	int ret = 0;

	assert(this && buffer);

	if (this->readfd < 0) {
		ret = TL_ERR_NOT_SUPPORTED;
		goto out;
	}
	if (!size)
		goto out;

retry:
	if (!wait_for_fd(this->readfd, this->timeout, 0))
		return TL_ERR_TIMEOUT;

	if (super->type == TL_STREAM_TYPE_SOCK) {
		result = recvfrom(this->readfd, buffer, size,
				  MSG_NOSIGNAL, NULL, NULL);
	} else {
		result = read(this->readfd, buffer, size);
	}

	if (result < 0 && errno == EINTR && (intr_count++) < 3)
		goto retry;

	ret = result < 0 ? errno_to_fs(errno) :
	      (result == 0 ? TL_ERR_CLOSED : 0);
out:
	if (actual)
		*actual = result > 0 ? result : 0;
	return ret;
}

/****************************************************************************/

fd_stream tl_stdio = {
	{
	 TL_STREAM_TYPE_FILE,
	 NULL,
	 fd_stream_set_timeout,
	 fd_stream_write,
	 fd_stream_read},
	0,
	STDIN_FILENO,
	STDOUT_FILENO,
	0
};

fd_stream tl_stderr = {
	{
	 TL_STREAM_TYPE_FILE,
	 NULL,
	 fd_stream_set_timeout,
	 fd_stream_write,
	 fd_stream_read},
	0,
	-1,
	STDERR_FILENO,
	0
};

/****************************************************************************/

tl_iostream *fdstream_create(int readfd, int writefd, int type, int flags)
{
	fd_stream *this = calloc(1, sizeof(*this));
	tl_iostream *super = (tl_iostream *)this;

	if (!this)
		return NULL;

	this->readfd = readfd;
	this->writefd = writefd;
	this->flags = flags;
	super->type = type;
	super->destroy = fd_stream_destroy;
	super->set_timeout = fd_stream_set_timeout;
	super->write = fd_stream_write;
	super->read = fd_stream_read;
	return super;
}
