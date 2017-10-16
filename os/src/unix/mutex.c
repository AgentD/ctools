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
	pthread_mutex_t *this = calloc(1, sizeof(*this));
	pthread_mutexattr_t attr;

	if (!this)
		return NULL;

	if (recursive) {
		if (pthread_mutexattr_init(&attr) != 0)
			goto fail;
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
			goto failattr;
		if (pthread_mutex_init(this, &attr) != 0)
			goto failattr;
		pthread_mutexattr_destroy(&attr);
	} else if (pthread_mutex_init(this, NULL) != 0) {
		goto fail;
	}

	return (tl_mutex *)this;
 failattr:
	pthread_mutexattr_destroy(&attr);
 fail:
	free(this);
	return NULL;
}

int tl_mutex_lock(tl_mutex *this, unsigned long timeout)
{
	struct timespec ts;

	assert(this);

	if (timeout > 0) {
		timeout_to_abs(timeout, &ts);
		return pthread_mutex_timedlock((pthread_mutex_t *) this,
					       &ts) == 0;
	}

	return pthread_mutex_lock((pthread_mutex_t *)this) == 0;
}

void tl_mutex_unlock(tl_mutex * this)
{
	assert(this);
	pthread_mutex_unlock((pthread_mutex_t *)this);
}

void tl_mutex_destroy(tl_mutex * this)
{
	assert(this);
	pthread_mutex_destroy((pthread_mutex_t *)this);
	free(this);
}
