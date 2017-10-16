/* mutex.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"

tl_mutex *tl_mutex_create(int recursive)
{
	CRITICAL_SECTION *this = calloc(1, sizeof(*this));
	(void)recursive;

	assert(this);
	InitializeCriticalSection(this);
	return (tl_mutex *)this;
}

int tl_mutex_lock(tl_mutex *this, unsigned long timeout)
{
	unsigned long dt;

	assert(this);

	if (timeout > 0) {
retry:
		if (TryEnterCriticalSection((CRITICAL_SECTION *)this))
			return 1;

		if (timeout) {
			dt = timeout < 10 ? timeout : 10;
			Sleep(dt);
			timeout -= dt;
			goto retry;
		}

		return 0;
	}

	EnterCriticalSection((CRITICAL_SECTION *)this);
	return 1;
}

void tl_mutex_unlock(tl_mutex *this)
{
	assert(this);
	LeaveCriticalSection((CRITICAL_SECTION *)this);
}

void tl_mutex_destroy(tl_mutex *this)
{
	assert(this);
	DeleteCriticalSection((CRITICAL_SECTION *)this);
	free(this);
}
