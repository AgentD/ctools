/* monitor.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"

int tl_monitor_init(tl_monitor *this)
{
	assert(this);
	this->notify_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!this->notify_event)
		return 0;

	this->notify_all_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!this->notify_all_event) {
		CloseHandle(this->notify_event);
		return 0;
	}

	InitializeCriticalSection(&this->mutex);
	InitializeCriticalSection(&this->waiter_mutex);
	this->wait_count = 0;
	return 1;
}

void tl_monitor_cleanup(tl_monitor *this)
{
	assert(this);
	CloseHandle(this->notify_event);
	CloseHandle(this->notify_all_event);
	DeleteCriticalSection(&this->mutex);
	DeleteCriticalSection(&this->waiter_mutex);
}

tl_monitor *tl_monitor_create(void)
{
	tl_monitor *this = calloc(1, sizeof(*this));

	if (!tl_monitor_init(this)) {
		free(this);
		this = NULL;
	}

	return this;
}

void tl_monitor_destroy(tl_monitor *this)
{
	assert(this);
	tl_monitor_cleanup(this);
	free(this);
}

int tl_monitor_lock(tl_monitor *this, unsigned long timeout)
{
	assert(this);
	return tl_mutex_lock((tl_mutex *)&this->mutex, timeout);
}

void tl_monitor_unlock(tl_monitor *this)
{
	assert(this);
	tl_mutex_unlock((tl_mutex *)&this->mutex);
}

int tl_monitor_wait(tl_monitor *this, unsigned long timeout)
{
	DWORD status = WAIT_FAILED, waittime = timeout ? timeout : INFINITE;
	HANDLE events[2];

	assert(this);

	/* increment wait count */
	EnterCriticalSection(&this->waiter_mutex);
	++(this->wait_count);
	LeaveCriticalSection(&this->waiter_mutex);

	/* wait for event */
	LeaveCriticalSection(&this->mutex);
	events[0] = this->notify_event;
	events[1] = this->notify_all_event;

	status = WaitForMultipleObjects(2, events, FALSE, waittime);

	/* decrement wait count */
	EnterCriticalSection(&this->waiter_mutex);
	--(this->wait_count);

	if (this->wait_count == 0 && status == (WAIT_OBJECT_0 + 1))
		ResetEvent(this->notify_all_event);
	LeaveCriticalSection(&this->waiter_mutex);

	/* restore state */
	EnterCriticalSection(&this->mutex);
	return status != WAIT_TIMEOUT && status != WAIT_FAILED;
}

void tl_monitor_notify(tl_monitor *this)
{
	assert(this);

	EnterCriticalSection(&this->waiter_mutex);

	if (this->wait_count > 0)
		SetEvent(this->notify_event);

	LeaveCriticalSection(&this->waiter_mutex);
}

void tl_monitor_notify_all(tl_monitor *this)
{
	assert(this);

	EnterCriticalSection(&this->waiter_mutex);

	if (this->wait_count > 0)
		SetEvent(this->notify_all_event);

	LeaveCriticalSection(&this->waiter_mutex);
}
