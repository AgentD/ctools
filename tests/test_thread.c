#include "tl_process.h"
#include "tl_thread.h"
#include <stdlib.h>
#include <stdio.h>



static volatile int value = 0;
static tl_mutex* mutex;
static tl_monitor* mon;



static void* mutex_thread( void* arg )
{
    tl_sleep( 100 );

    if( !tl_mutex_lock( mutex, 1000 ) )
        exit( EXIT_FAILURE );
    if( !tl_mutex_lock( mutex, 1000 ) )
        exit( EXIT_FAILURE );

    value += 5;

    tl_mutex_unlock( mutex );
    tl_mutex_unlock( mutex );
    return arg;
}

static void* mon_thread( void* arg )
{
    if( !tl_monitor_lock( mon, 5000 ) )
        exit( EXIT_FAILURE );
    if( !tl_monitor_wait( mon, 5000 ) )
        exit( EXIT_FAILURE );

    value += 5;
    tl_monitor_unlock( mon );
    return arg;
}


int main( void )
{
    void *a = (void*)0xDEADBEEF, *b = (void*)0xCAFEBABE;
    tl_thread *t0, *t1;

    /* mutex & thread */
    mutex = tl_mutex_create( 1 );

    t0 = tl_thread_create( mutex_thread, a );
    t1 = tl_thread_create( mutex_thread, b );

    if( !tl_thread_join( t0, 5000 ) ) return EXIT_FAILURE;
    if( !tl_thread_join( t1, 5000 ) ) return EXIT_FAILURE;

    if( tl_thread_get_return_value( t0 ) != a ) return EXIT_FAILURE;
    if( tl_thread_get_return_value( t1 ) != b ) return EXIT_FAILURE;
    if( value!=10                             ) return EXIT_FAILURE;

    tl_thread_destroy( t0 );
    tl_thread_destroy( t1 );
    tl_mutex_destroy( mutex );

    /* monitor & thread */
    mon = tl_monitor_create( );

    value = 0;
    t0 = tl_thread_create( mon_thread, a );
    t1 = tl_thread_create( mon_thread, b );

    tl_sleep( 100 );
    tl_monitor_notify_all( mon );

    if( !tl_thread_join( t0, 5000 ) ) return EXIT_FAILURE;
    if( !tl_thread_join( t1, 5000 ) ) return EXIT_FAILURE;

    if( tl_thread_get_return_value( t0 ) != a ) return EXIT_FAILURE;
    if( tl_thread_get_return_value( t1 ) != b ) return EXIT_FAILURE;
    if( value!=10                             ) return EXIT_FAILURE;

    tl_monitor_destroy( mon );
    tl_thread_destroy( t0 );
    tl_thread_destroy( t1 );

    /* monitor & thread */
    mon = tl_monitor_create( );

    value = 0;
    t0 = tl_thread_create( mon_thread, a );
    t1 = tl_thread_create( mon_thread, b );

    tl_sleep( 100 );
    tl_monitor_notify( mon );
    tl_monitor_notify( mon );

    if( !tl_thread_join( t0, 5000 ) ) return EXIT_FAILURE;
    if( !tl_thread_join( t1, 5000 ) ) return EXIT_FAILURE;

    if( tl_thread_get_return_value( t0 ) != a ) return EXIT_FAILURE;
    if( tl_thread_get_return_value( t1 ) != b ) return EXIT_FAILURE;
    if( value!=10                             ) return EXIT_FAILURE;

    tl_monitor_destroy( mon );
    tl_thread_destroy( t0 );
    tl_thread_destroy( t1 );
    return EXIT_SUCCESS;
}

