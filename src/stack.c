#include "stack.h"

#include <string.h>



void tl_stack_init( tl_stack* this, size_t element_size )
{
    if( this )
        tl_vector_init( &(this->vec), element_size );
}

void tl_stack_cleanup( tl_stack* this )
{
    if( this )
        tl_vector_cleanup( &(this->vec) );
}

int tl_stack_push( tl_stack* this, const void* element )
{
    if( !this || !element )
        return 0;

    return tl_vector_append( &(this->vec), element );
}

void* tl_stack_top( const tl_stack* this )
{
    if( !this || !this->vec.used )
        return NULL;

    return tl_vector_at( &(this->vec), this->vec.used-1 );
}

void tl_stack_pop( tl_stack* this, void* data )
{
    if( !this || !this->vec.used )
        return;

    if( data )
    {
        memcpy( data,
                tl_vector_at( &(this->vec), this->vec.used-1 ),
                this->vec.unitsize );
    }

    tl_vector_resize( &(this->vec), this->vec.used-1 );
}

int tl_stack_is_empty( tl_stack* this )
{
    return (!this || this->vec.used==0);
}

size_t tl_stack_size( tl_stack* this )
{
    return this ? this->vec.used : 0;
}

