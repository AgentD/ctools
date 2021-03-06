/*
 * tl_thread.h
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
 * \file tl_thread.h
 *
 * \brief Contains prtotypes for threading utilities
 */
#ifndef TOOLS_THREAD_H
#define TOOLS_THREAD_H

/**
 * \page conc Concurrency
 *
 * \section thread Thread utilities
 *
 * There are a number of primitives for creating, managing and
 * synchronizing threads.
 *
 * The following data structurs and functions are currently available:
 * \li tl_thread for creating and managing threads
 * \li tl_mutex for mutually exclusive access to resources
 * \li tl_rwlock for managing parallel read and exclusive write access
 * \li tl_monitor combining a mutex and waiting/signifying condition mechanics
 * \li \ref tl_sleep for pausing the calling thread for a specified time
 * \li \ref tl_thread_get_id for getting an integer ID of either the calling
 *     thread or a specific thread object.
 *
 * \subsection createthread Creating Threads
 *
 * Here is a small example on how to create a thread:
 * \code{.c}
 * void* thread_function( void* arg )
 * {
 *     printf( "Thread argument: %d\n", *((int*)arg) );
 *     printf( "ID of thread: %d\n", tl_thread_get_id(NULL) );
 *     return NULL;
 * }
 *
 * int main( void )
 * {
 *     tl_thread* thread;
 *     int i = 42;
 *
 *     printf( "ID of main thread: %d\n", tl_thread_get_id(NULL) );
 *
 *     thread = tl_thread_create( thread_function, &i );
 *
 *     printf( "ID of thread (from main): %d\n", tl_thread_get_id(thread) );
 *
 *     tl_thread_join( thread, 0 );
 *     tl_thread_destroy( thread );
 *     return 0;
 * }
 * \endcode
 */

#include "tl_predef.h"

/**
 * \struct tl_mutex
 *
 * \brief A mutual exclusion lock
 *
 * A mutex is the simplest synchronization data structure available. A thread
 * can acquire exlusive access to a mutex, excluding all other threads that
 * then have to wait for the original thread to release the mutex before they
 * can acquire it. A mutex is basically a semaphore with a maximum count of
 * one.
 *
 * A mutex can be recursive. A recursive mutex counts "nested" lock calls and
 * requires at least as many unlock calls to actually unlock the mutex.
 * Calling lock multiple times on a non-recursive mutex has no effect and a
 * single unlock is enough to unlock the mutex.
 */

/**
 * \struct tl_rwlock
 *
 * \brief A read write lock
 *
 * A read write lock distingueshes between read and write access to a shared
 * resource. It allows multiple threads to gain read access at the same time,
 * assuming that parallel reading is possible without interference. However,
 * only one thread can gain write access at a time. A thread wanting to gain
 * write access has to wait until all readers release their read locks, or
 * if another writer is holding the write lock, until the other writer
 * releases its write access. Readers can only get read access if no thread
 * is holding the write lock.
 */

/**
 * \struct tl_monitor
 *
 * \brief A monitor object
 *
 * A monitor allows exclusive locking like a mutex. A monitor can atomically
 * release its lock, wait for notifications from other threads and regain
 * the lock if a notification was received. Other threads that hold a monitor
 * lock can signal the monitor to either wake up a single thread waiting for
 * an event or to wake up all threads waiting for an event.
 *
 * If a monitor is signaled, the threads that are woken up can only regain
 * the lock once the signalling thread releases its lock. If multiple threads
 * are woken up, only one at a time is successfull at regaining the lock. The
 * others have to wait until the lock is released again.
 */

/**
 * \struct tl_thread
 *
 * \brief Encapsulates and controls a thread
 */


/**
 * \brief A function executed in a thread
 *
 * \param arg An arbitrary argument passed to the function
 *            via tl_thread_create
 *
 * \return An arbitrary value that can be retrieved
 *         via tl_thread_get_return_value 
 */
typedef void *(*tl_thread_function)(void *arg);

/**
 * \enum TL_THREAD_STATE
 *
 * \brief Encapsultes the state that a thread is in
 */
typedef enum {
	/** \brief The thread was created but is not running yet */
	TL_PENDING = 0,

	/**
	 * \brief The thread function is being executed
	 *
	 * This value only means that the thread exists and it is executing
	 * the thread function. The thread itself may or may not be scheduled
	 * to a processor.
	 */
	TL_RUNNING = 1,

	/** \brief The thread function returned and the thread terminated. */
	TL_TERMINATED = 2
} TL_THREAD_STATE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a monitor object
 *
 * \memberof tl_monitor
 *
 * \return A pointer to a monitor on success, NULL on failure
 */
TLOSAPI tl_monitor *tl_monitor_create(void);

/**
 * \brief Lock a monitor object to gain exclusive access
 *
 * \memberof tl_monitor
 *
 * \param monitor A pointer to a monitor object
 * \param timeout The maximum number of milliseconds to wait, 0 for infinite
 *
 * \return Non-zero on success, zero if a timeout occoured
 */
TLOSAPI int tl_monitor_lock(tl_monitor *monitor, unsigned long timeout);

/**
 * \brief Unlock a monitor object, dropping exclusive access
 *
 * \memberof tl_monitor
 *
 * \param monitor A pointer to a monitor object
 */
TLOSAPI void tl_monitor_unlock(tl_monitor *monitor);

/**
 * \brief Wait for an other thread to send a notification
 *
 * \memberof tl_monitor
 *
 * This function blocks until another thread sends anotification using either
 * tl_monitor_notify or tl_monitor_notify_all. The calling thread must have
 * exclusive access to the monitor before calling this function. The function
 * atomically drops the lock and acquires it again before returning.
 *
 * \param monitor A pointer to a monitor object
 * \param timeout The maximum number of milliseconds to wait, 0 for infinite
 *
 * \return Non-zero if a notification was received, zero if a timeout occoured
 */
TLOSAPI int tl_monitor_wait(tl_monitor *monitor, unsigned long timeout);

/**
 * \brief Wake another thread waiting for a notification
 *
 * \memberof tl_monitor
 *
 * If one or more threads are waiting for a notification using
 * tl_monitor_wait, one of the threads is woken up and returns with
 * success status from tl_monitor_wait.
 *
 * \param monitor A pointer to a monitor object
 */
TLOSAPI void tl_monitor_notify(tl_monitor *monitor);

/**
 * \brief Wake all threads waiting for a notification
 *
 * \memberof tl_monitor
 *
 * If one or more threads are waiting for a notification using
 * tl_monitor_wait, all threads are woken up simultanously. Since
 * only one thread at a time can hold the lock on the monitor, only
 * one thread at a time can regain the lock and return from
 * tl_monitor_wait with success status.
 *
 * \param monitor A pointer to a monitor object
 */
TLOSAPI void tl_monitor_notify_all(tl_monitor *monitor);

/**
 * \brief Destroy a monitor object
 *
 * \memberof tl_monitor
 *
 * \param monitor A pointer to a monitor object
 */
TLOSAPI void tl_monitor_destroy(tl_monitor *monitor);


/**
 * \brief Create a read-write lock object
 *
 * \memberof tl_rwlock
 *
 * \return A pointer to a read-write lock on success, NULL on failure
 */
TLOSAPI tl_rwlock *tl_rwlock_create(void);

/**
 * \brief Gain read access to a tl_rwlock
 *
 * \memberof tl_rwlock
 *
 * \param lock    A pointer to the lock object
 * \param timeout A number of milliseconds to wait for a thread holding
 *                the write lock to release the lock. Zero for infinite.
 *
 * \return Non-zero on success, zero if a timeout occoured before gaining
 *         the lock
 */
TLOSAPI int tl_rwlock_lock_read(tl_rwlock *lock, unsigned long timeout);

/**
 * \brief Gain write access to a tl_rwlock
 *
 * \memberof tl_rwlock
 *
 * \param lock    A pointer to the lock object
 * \param timeout A number of milliseconds to wait for all threads holding
 *                a read lock to release their locks. Zero for infinite.
 *
 * \return Non-zero on success, zero if a timeout occoured before gaining
 *         the lock
 */
TLOSAPI int tl_rwlock_lock_write(tl_rwlock *lock, unsigned long timeout);

/**
 * \brief Release the currently held read lock of a tl_rwlock
 *
 * \memberof tl_rwlock
 *
 * \param lock A pointer to the lock object
 */
TLOSAPI void tl_rwlock_unlock_read(tl_rwlock *lock);

/**
 * \brief Release the currently held write lock of a tl_rwlock
 *
 * \memberof tl_rwlock
 *
 * \param lock A pointer to the lock object
 */
TLOSAPI void tl_rwlock_unlock_write(tl_rwlock *lock);

/**
 * \brief Destroy a lock object
 *
 * \memberof tl_rwlock
 *
 * \param lock A pointer to a lock object
 */
TLOSAPI void tl_rwlock_destroy(tl_rwlock *lock);



/**
 * \brief Create a mutex object
 *
 * \memberof tl_mutex
 *
 * \param recursive Non-zero to create a recursive mutex, zero otherwise.
 *
 * \return A pointer to a mutex object on success, zero on failure
 */
TLOSAPI tl_mutex *tl_mutex_create(int recursive);

/**
 * \brief Gain exclusive access to a mutex
 *
 * \memberof tl_mutex
 *
 * \param mutex   A pointer to the mutex object
 * \param timeout A number of milliseconds to wait for an other thread to
 *                release the lock. Zero for infinite.
 *
 * \return Non-zero on success, zero if a timeout occoured before gaining
 *         the lock
 */
TLOSAPI int tl_mutex_lock(tl_mutex *mutex, unsigned long timeout);

/**
 * \brief Release a currently held lock
 *
 * \memberof tl_mutex
 *
 * \param mutex A pointer to the mutex object
 */
TLOSAPI void tl_mutex_unlock(tl_mutex *mutex);

/**
 * \brief Destroy a mutex object
 *
 * \memberof tl_mutex
 *
 * \param mutex A pointer to the mutex object
 */
TLOSAPI void tl_mutex_destroy(tl_mutex *mutex);

/**
 * \brief Create a thread
 *
 * \memberof tl_thread
 *
 * This function creates a new thread and immediately runs the given function
 * with the supplied argument in the thread.
 *
 * \param function The function to run in the thread
 * \param arg      A pointer argument to pass to the function in the thread
 *
 * \return A pointer to a thread object on success, NULL on failure
 */
TLOSAPI tl_thread *tl_thread_create(tl_thread_function function, void *arg);

/**
 * \brief Wait for a thread to terminate
 *
 * \memberof tl_thread
 *
 * \param thread  A pointer to a thread object
 * \param timeout The maximum number of milliseconds to wait,
 *                zero for infinite
 *
 * \return Non-zero on success, zero if a timeout occoured
 */
TLOSAPI int tl_thread_join(tl_thread *thread, unsigned long timeout);

/**
 * \brief Get the value returned from the thread functon
 *
 * \memberof tl_thread
 *
 * \param thread A pointer to a thread object
 *
 * \return The value returned by the thread function, NULL if the thread is
 *         still running.
 */
TLOSAPI void* tl_thread_get_return_value(tl_thread *thread);

/**
 * \brief Get the state that a thread is currently in
 *
 * \memberof tl_thread
 *
 * \param thread A pointer to a thread object
 *
 * \return A \ref TL_THREAD_STATE object
 */
TLOSAPI int tl_thread_get_state(tl_thread *thread);

/**
 * \brief Destroy a thread object and free all its resources
 *
 * \memberof tl_thread
 *
 * If the thread is still running, the ownership of the thread object is
 * transfered to the thread and the thread cleans itself up when it
 * terminates.
 *
 * \param thread A pointer ot a thread object
 */
TLOSAPI void tl_thread_destroy(tl_thread *thread);

/**
 * \brief Return a unique ID of a specific thread, or the calling thread
 *
 * \memberof tl_thread
 *
 * \static
 *
 * \note Once a thread terminates, its id can be reused
 *
 * \param thread A pointer to a thread to get the ID from, or NULL to get the
 *               ID of the calling thread.
 *
 * \return A unique thread ID
 */
TLOSAPI int tl_thread_get_id(tl_thread *thread);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_THREAD_H */

