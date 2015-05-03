#include <stdio.h>
#include <stdlib.h>

#include "tl_thread.h"
#include "tl_process.h"

static tl_rwlock* rwlock;
static int readers = 0;
static int writers = 0;

void* read_thread( void* arg )
{
    if( !tl_rwlock_lock_read( rwlock, 0 ) )
        exit( EXIT_FAILURE );

    if( writers!=0 )
        exit( EXIT_FAILURE );

    ++readers;
    tl_sleep( 300 );
    --readers;

    tl_rwlock_unlock_read( rwlock );
    return arg;
}

void* write_thread( void* arg )
{
    if( !tl_rwlock_lock_write( rwlock, 0 ) )
        exit( EXIT_FAILURE );

    if( writers!=0 || readers!=0 )
        exit( EXIT_FAILURE );

    ++writers;
    tl_sleep( 100 );
    --writers;

    tl_rwlock_unlock_write( rwlock );
    return arg;
}

int main( void )
{
    tl_thread *t0, *t1, *t2, *t3;

    rwlock = tl_rwlock_create( );

    tl_rwlock_lock_read( rwlock, 0 );
    tl_rwlock_lock_read( rwlock, 0 );

    t0 = tl_thread_create( read_thread, NULL );
    t1 = tl_thread_create( read_thread, NULL );
    tl_rwlock_unlock_read( rwlock );

    t2 = tl_thread_create( write_thread, NULL );
    t3 = tl_thread_create( write_thread, NULL );
    tl_sleep( 300 );
    tl_rwlock_unlock_read( rwlock );

    tl_thread_join( t0, 0 );
    tl_thread_join( t1, 0 );
    tl_thread_join( t2, 0 );
    tl_thread_join( t3, 0 );

    tl_rwlock_destroy( rwlock );
    tl_thread_destroy( t0 );
    tl_thread_destroy( t1 );
    tl_thread_destroy( t2 );
    tl_thread_destroy( t3 );
    return EXIT_SUCCESS;
}

