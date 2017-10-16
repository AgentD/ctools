/* rwlock.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"

tl_rwlock *tl_rwlock_create(void)
{
	pthread_rwlock_t *this = calloc(1, sizeof(*this));

	if (this && pthread_rwlock_init(this, NULL) != 0) {
		free(this);
		this = NULL;
	}

	return (tl_rwlock *)this;
}

int tl_rwlock_lock_read(tl_rwlock *this, unsigned long timeout)
{
	struct timespec ts;

	assert(this);

	if (timeout > 0) {
		timeout_to_abs(timeout, &ts);
		return pthread_rwlock_timedrdlock((pthread_rwlock_t *)this,
						  &ts) == 0;
	}

	return pthread_rwlock_rdlock((pthread_rwlock_t *)this) == 0;
}

int tl_rwlock_lock_write(tl_rwlock *this, unsigned long timeout)
{
	struct timespec ts;

	assert(this);

	if (timeout > 0) {
		timeout_to_abs(timeout, &ts);
		return pthread_rwlock_timedwrlock((pthread_rwlock_t *) this,
						  &ts) == 0;
	}

	return pthread_rwlock_wrlock((pthread_rwlock_t *)this) == 0;
}

void tl_rwlock_unlock_read(tl_rwlock *this)
{
	assert(this);
	pthread_rwlock_unlock((pthread_rwlock_t *)this);
}

void tl_rwlock_unlock_write(tl_rwlock *this)
{
	assert(this);
	pthread_rwlock_unlock((pthread_rwlock_t *)this);
}

void tl_rwlock_destroy(tl_rwlock *this)
{
	assert(this);
	pthread_rwlock_destroy((pthread_rwlock_t *)this);
	free(this);
}
