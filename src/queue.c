#include "tl_queue.h"

#include <string.h>



void tl_queue_init( tl_queue* this, size_t element_size,
                    tl_allocator* alloc )
{
    if( this )
        tl_list_init( &(this->list), element_size, alloc );
}

void tl_queue_cleanup( tl_queue* this )
{
    if( this )
        tl_list_cleanup( &(this->list) );
}

int tl_queue_insert_front( tl_queue* this, const void* element )
{
    return (this && element) ? tl_list_prepend(&(this->list), element) : 0;
}

int tl_queue_insert_back( tl_queue* this, const void* element )
{
    return (this && element) ? tl_list_append(&(this->list), element) : 0;
}

void* tl_queue_peek_front( const tl_queue* this )
{
    return this ? tl_list_node_get_data( this->list.first ) : NULL;
}

void* tl_queue_peek_back( const tl_queue* this )
{
    return this ? tl_list_node_get_data( this->list.last ) : NULL;
}

void tl_queue_remove_front( tl_queue* this, void* data )
{
    if( this )
    {
        if( data )
        {
            memcpy( data, tl_list_node_get_data( this->list.first ),
                    this->list.unitsize );
        }

        tl_list_remove_first( &(this->list) );
    }
}

void tl_queue_remove_back( tl_queue* this, void* data )
{
    if( this )
    {
        if( data )
        {
            memcpy( data, tl_list_node_get_data( this->list.last ),
                    this->list.unitsize );
        }

        tl_list_remove_last( &(this->list) );
    }
}

int tl_queue_is_empty( const tl_queue* this )
{
    return (!this || this->list.size==0);
}

size_t tl_queue_size( const tl_queue* this )
{
    return this ? this->list.size : 0;
}

void tl_queue_flush( tl_queue* this )
{
    if( this )
        tl_list_clear( &(this->list) );
}

