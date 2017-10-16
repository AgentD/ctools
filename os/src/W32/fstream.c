/* fstream.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_iostream.h"
#include "os.h"

static void fstream_destroy(tl_iostream *super)
{
	fstream *this = (fstream *)super;

	assert(this);
	if (this->rhnd)
		CloseHandle(this->rhnd);
	if (this->whnd && (this->rhnd != this->whnd))
		CloseHandle(this->whnd);
	free(this);
}

static int fstream_set_timeout(tl_iostream *super, unsigned int timeout)
{
	fstream *this = (fstream *)super;

	assert(this);

	if (this->rhnd)
		set_handle_timeout(this->rhnd, timeout);

	if (this->whnd && this->whnd != this->rhnd)
		set_handle_timeout(this->whnd, timeout);

	return 0;
}

static int fstream_write(tl_iostream *super, const void *buffer,
			 size_t size, size_t *actual)
{
	fstream *this = (fstream *)super;
	DWORD result;

	if (actual)
		*actual = 0;

	assert(this && buffer);

	if (!this->whnd)
		return TL_ERR_NOT_SUPPORTED;

	if (!WriteFile(this->whnd, buffer, size, &result, NULL))
		return errno_to_fs(GetLastError());

	if (actual)
		*actual = result;

	return 0;
}

static int fstream_read(tl_iostream *super, void *buffer, size_t size,
			size_t *actual)
{
	fstream *this = (fstream *)super;
	DWORD result;

	if (actual)
		*actual = 0;

	assert(this && buffer);

	if (!this->rhnd)
		return TL_ERR_NOT_SUPPORTED;

	if (!ReadFile(this->rhnd, buffer, size, &result, NULL))
		return errno_to_fs(GetLastError());

	if (actual)
		*actual = result;

	return result ? 0 : TL_ERR_CLOSED;
}

/****************************************************************************/

fstream tl_stdio = {
	{
	 TL_STREAM_TYPE_FILE,
	 NULL,
	 fstream_set_timeout,
	 fstream_write,
	 fstream_read},
	NULL,
	NULL
};

fstream tl_stderr = {
	{
	 TL_STREAM_TYPE_FILE,
	 NULL,
	 fstream_set_timeout,
	 fstream_write,
	 fstream_read},
	NULL,
	NULL
};

/****************************************************************************/

tl_iostream *fstream_create(HANDLE readhnd, HANDLE writehnd, int type)
{
	fstream *this = calloc(1, sizeof(*this));
	tl_iostream *super = (tl_iostream *)this;

	if (this) {
		this->rhnd = readhnd;
		this->whnd = writehnd;
		super->type = type;
		super->read = fstream_read;
		super->write = fstream_write;
		super->destroy = fstream_destroy;
		super->set_timeout = fstream_set_timeout;
	}
	return super;
}
