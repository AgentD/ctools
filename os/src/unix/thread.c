/* thread.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"

struct tl_thread {
	pthread_t thread;
	tl_monitor monitor;
	int state;
	void *retval;
	tl_thread_function function;
	void *argument;
};

static void *pthread_wrapper(void *arg)
{
	void (*cleanup_fun) (void *) = (void (*)(void *))tl_monitor_unlock;
	tl_thread *this = arg;
	void *retval;

	pthread_cleanup_push(cleanup_fun, &this->monitor);

	tl_monitor_lock(&this->monitor, 0);
	this->state = TL_RUNNING;
	tl_monitor_unlock(&this->monitor);

	retval = this->function(this->argument);

	tl_monitor_lock(&this->monitor, 0);
	this->retval = retval;
	this->state = TL_TERMINATED;
	tl_monitor_notify(&this->monitor);
	tl_monitor_unlock(&this->monitor);

	pthread_cleanup_pop(0);
	return NULL;
}

tl_thread *tl_thread_create(tl_thread_function function, void *arg)
{
	tl_thread *this;

	assert(function);

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	this->state = TL_PENDING;
	this->retval = NULL;
	this->function = function;
	this->argument = arg;

	if (!tl_monitor_init(&this->monitor))
		goto fail;
	if (pthread_create(&this->thread, NULL, pthread_wrapper, this) != 0)
		goto failthread;

	return this;
failthread:
	tl_monitor_cleanup(&this->monitor);
fail:
	free(this);
	return NULL;
}

int tl_thread_join(tl_thread *this, unsigned long timeout)
{
	int status = 1;

	assert(this);

	if (timeout > 0) {
		tl_monitor_lock(&this->monitor, 0);
		if (this->state != TL_TERMINATED) {
			tl_monitor_wait(&this->monitor, timeout);
			status = (this->state == TL_TERMINATED);
		}
		tl_monitor_unlock(&this->monitor);
	} else {
		pthread_join(this->thread, NULL);
	}

	return status;
}

void *tl_thread_get_return_value(tl_thread *this)
{
	void *retval;

	assert(this);

	tl_monitor_lock(&this->monitor, 0);
	retval = this->retval;
	tl_monitor_unlock(&this->monitor);

	return retval;
}

int tl_thread_get_state(tl_thread *this)
{
	int state;

	assert(this);

	tl_monitor_lock(&this->monitor, 0);
	state = this->state;
	tl_monitor_unlock(&this->monitor);

	return state;
}

void tl_thread_destroy(tl_thread *this)
{
	assert(this);

	if (this->state != TL_TERMINATED) {
		pthread_cancel(this->thread);
		pthread_join(this->thread, NULL);
	}

	tl_monitor_cleanup(&this->monitor);
	free(this);
}

int tl_thread_get_id(tl_thread *this)
{
	return this ? this->thread : pthread_self();
}
