#include "tl_allocator.h"
#include "tl_list.h"

#include <stdlib.h>
#include <string.h>



#ifdef TL_ALLIGN_MEMORY
    #define PADDING sizeof(void*)
    #define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
    #define PADDING 0
    #define ALLIGN( ptr )
#endif



tl_list_node* tl_list_node_create( const tl_list* this, const void* data )
{
    tl_list_node* node;
    char* ptr;

    node = malloc( sizeof(tl_list_node) + this->unitsize + PADDING );

    if( node )
    {
        ptr = (char*)node + sizeof(tl_list_node);
        ALLIGN( ptr );

        if( data )
        {
            tl_allocator_copy( this->alloc, ptr, data, this->unitsize, 1 );
        }
        else
        {
            tl_allocator_init( this->alloc, ptr, this->unitsize, 1 );
        }

        node->prev = node->next = NULL;
    }

    return node;
}

void* tl_list_node_get_data( const tl_list_node* node )
{
    char* ptr = NULL;

    if( node )
    {
        ptr = (char*)node + sizeof(tl_list_node);
        ALLIGN( ptr );
    }

    return ptr;
}

/****************************************************************************/

void tl_list_init( tl_list* this, size_t elementsize, tl_allocator* alloc )
{
    if( this )
    {
        this->first    = NULL;
        this->last     = NULL;
        this->size     = 0;
        this->unitsize = elementsize;
        this->alloc    = alloc;
    }
}

void tl_list_cleanup( tl_list* this )
{
    tl_list_clear( this );
}

tl_list_node* tl_list_node_from_index( const tl_list* this, size_t index )
{
    tl_list_node* n = NULL;
    size_t i;

    if( this && index<this->size )
    {
        if( index > this->size/2 )
        {
            for(n=this->last, i=this->size-1; n && i>index; --i, n=n->prev) {}
        }
        else
        {
            for( n=this->first, i=0; n && i<index; ++i, n=n->next ) { }
        }
    }
    return n;
}

int tl_list_from_array( tl_list* this, const void* data, size_t count )
{
    tl_list temp;
    size_t i;

    if( !this || !data )
        return 0;

    tl_list_init( &temp, this->unitsize, this->alloc );

    for( i=0; i<count; ++i )
    {
        if( !tl_list_append( &temp, data ) )
        {
            tl_list_clear( &temp );
            return 0;
        }

        data = (unsigned char*)data + this->unitsize;
    }

    tl_list_clear( this );
    this->first = temp.first;
    this->last  = temp.last;
    this->size  = temp.size;
    return 1;
}

void tl_list_to_array( const tl_list* this, void* data )
{
    tl_list_node* n;

    if( this && this->size && data )
    {
        for( n=this->first; n; n=n->next )
        {
            tl_allocator_copy( this->alloc, data,
                               tl_list_node_get_data( n ),
                               this->unitsize, 1 );
            data = (unsigned char*)data + this->unitsize;
        }
    }
}

int tl_list_copy( tl_list* this, const tl_list* src )
{
    return tl_list_copy_range( this, src, 0, src->size );
}

int tl_list_copy_range( tl_list* this, const tl_list* src,
                        size_t start, size_t count )
{
    tl_list_node* n;
    tl_list temp;
    size_t i;

    /* sanity check */
    if( !this || !src || (start+count)>src->size ) return 0;
    if( !src->size                               ) return 1;

    /* get the start node */
    if( !(n = tl_list_node_from_index( src, start )) )
        return 0;

    /* create a copy of the sublist */
    tl_list_init( &temp, src->unitsize, this->alloc );

    for( i=0; i<count; ++i, n=n->next )
    {
        if( !tl_list_append( &temp, tl_list_node_get_data( n ) ) )
        {
            tl_list_clear( &temp );
            return 0;
        }
    }

    /* free the current list and overwrite with the copy */
    tl_list_clear( this );
    this->first = temp.first;
    this->last  = temp.last;
    this->size  = temp.size;
    return 1;
}

int tl_list_join( tl_list* this, tl_list* other, size_t index )
{
    tl_list_node* n;

    if( !this||!other||this->unitsize!=other->unitsize||index>this->size )
        return 0;

    if( !other->size )
        return 1;

    if( !this->size )                       /* overwrite empty list */
    {
        this->first = other->first;
        this->last  = other->last;
    }
    else if( index==0 )                     /* prepend to list */
    {
        other->last->next = this->first;
        this->first->prev = other->last;

        this->first = other->first;
    }
    else if( index==this->size )            /* append to list */
    {
        other->first->prev = this->last;
        this->last->next = other->first;

        this->last = other->last;
    }
    else                                    /* insert somewhere in the list */
    {
        if( !(n = tl_list_node_from_index( this, index )) )
            return 0;

        n->prev->next = other->first;
        other->first->prev = n->prev;

        n->prev = other->last;
        other->last->next = n;
    }

    this->size += other->size;

    other->last  = NULL;
    other->first = NULL;
    other->size  = 0;
    return 1;
}

void tl_list_reverse( tl_list* this )
{
    tl_list_node* temp;
    tl_list_node* i;

    if( this && this->size )
    {
        /* swap the previous and the next pointer for every node */
        for( i=this->first; i; i=i->prev )
        {
            temp    = i->next;
            i->next = i->prev;
            i->prev = temp;
        }

        /* swap the first and last pointer */
        temp        = this->first;
        this->first = this->last;
        this->last  = temp;
    }
}

int tl_list_concat( tl_list* this, const tl_list* src )
{
    tl_list temp;

    if( !this || !src || (src && src->unitsize!=this->unitsize) )
        return 0;

    if( !src->size )
        return 1;

    /* create a copy of the source list */
    tl_list_init( &temp, src->unitsize, this->alloc );

    if( !tl_list_copy( &temp, src ) )
        return 0;

    /* join the copy and the current list */
    return tl_list_join( this, &temp, this->size );
}

int tl_list_remove( tl_list* this, size_t index, size_t count )
{
    tl_list_node* old;
    tl_list_node* n;
    char* ptr;
    size_t i;

    if( !this || index>=this->size )
        return 0;

    count = (count > this->size) ? this->size : count;

    if( index==0 )
    {
        for( i=0; this->first && i<count; ++i )
        {
            n = this->first;
            this->first = this->first->next;

            ptr = (char*)n + sizeof(tl_list_node);
            ALLIGN( ptr );
            tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );

            free( n );
        }

        if( this->first )
            this->first->prev = NULL;
    }
    else if( (index+count) >= this->size )
    {
        count = this->size - index;
        for( i=0; i<count && this->last; ++i )
        {
            n = this->last;
            this->last = this->last->prev;

            ptr = (char*)n + sizeof(tl_list_node);
            ALLIGN( ptr );
            tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );

            free( n );
        }

        if( this->last )
            this->last->next = NULL;
    }
    else
    {
        if( !(n = tl_list_node_from_index( this, index )) )
            return 0;

        for( i=0; i<count && n; ++i )
        {
            old = n;

            n->prev->next = n->next;
            n->next->prev = n->prev;
            n = n->next;

            ptr = (char*)old + sizeof(tl_list_node);
            ALLIGN( ptr );
            tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );

            free( old );
        }
    }

    this->size -= count;

    if( !this->size )
    {
        this->first = NULL;
        this->last  = NULL;
    }

    return 1;
}

int tl_list_is_empty( const tl_list* this )
{
    return (!this || this->size==0);
}

void* tl_list_at( const tl_list* this, size_t index )
{
    return tl_list_node_get_data( tl_list_node_from_index( this, index ) );
}

int tl_list_set( tl_list* this, size_t index, const void* element )
{
    void* ptr;

    ptr = tl_list_at( this, index );

    if( !ptr )
        return 0;

    tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );
    tl_allocator_copy( this->alloc, ptr, element, this->unitsize, 1 );
    return 1;
}

int tl_list_append( tl_list* this, const void* element )
{
    tl_list_node* node;

    if( !this || !element )
        return 0;

    node = tl_list_node_create( this, element );

    if( !node )
        return 0;

    if( !this->size )
    {
        this->first = this->last = node;
    }
    else
    {
        this->last->next = node;
        node->prev = this->last;
        this->last = node;
    }

    ++(this->size);
    return 1;
}

int tl_list_prepend( tl_list* this, const void* element )
{
    tl_list_node* node;

    if( !this || !element )
        return 0;

    node = tl_list_node_create( this, element );

    if( !node )
        return 0;

    if( !this->size )
    {
        this->first = this->last = node;
    }
    else
    {
        this->first->prev = node;
        node->next = this->first;
        this->first = node;
    }

    ++(this->size);
    return 1;
}

int tl_list_insert( tl_list* this, size_t index,
                    const void* elements, size_t count )
{
    tl_list list;

    if( !this || index>this->size || !elements )
        return 0;

    if( !count )
        return 1;

    /* construct a list from the array */
    tl_list_init( &list, this->unitsize, this->alloc );

    if( !tl_list_from_array( &list, elements, count ) )
        return 0;

    /* merge the list into the array */
    if( !tl_list_join( this, &list, index ) )
    {
        tl_list_clear( this );
        return 0;
    }
    return 1;
}

int tl_list_insert_sorted( tl_list* this, tl_compare cmp,
                           const void* element )
{
    tl_list_node *n, *t;

    if( !this || !cmp || !element )
        return 0;

    if( !(t = tl_list_node_create( this, element )) )
        return 0;

    if( !this->size )
    {
        this->first = this->last = t;
    }
    else if( cmp( element, tl_list_node_get_data(this->first) )<=0 )
    {
        t->next = this->first;
        this->first->prev = t;
        this->first = t;
    }
    else
    {
        for( n=this->first; n; n=n->next )
        {
            if( cmp( tl_list_node_get_data(n), element )>0 )
            {
                t->next = n;
                t->prev = n->prev;

                t->next->prev = t;
                t->prev->next = t;
                goto done;
            }
        }

        t->prev = this->last;
        this->last->next = t;
        this->last = t;
    }
done:
    this->size += 1;
    return 1;
}

void tl_list_remove_first( tl_list* this )
{
    tl_list_node* n;
    char* ptr;

    if( this && this->size )
    {
        if( this->size==1 )
        {
            n = this->first;
            this->first = this->last = NULL;
        }
        else
        {
            n = this->first;
            this->first = this->first->next;
            this->first->prev = NULL;
        }

        ptr = (char*)n + sizeof(tl_list_node);
        ALLIGN( ptr );
        tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );

        free( n );
        --(this->size);
    }
}

void tl_list_remove_last( tl_list* this )
{
    tl_list_node* n;
    char* ptr;

    if( this && this->size )
    {
        if( this->size==1 )
        {
            n = this->first;
            this->first = this->last = NULL;
        }
        else
        {
            n = this->last;
            this->last = this->last->prev;
            this->last->next = NULL;
        }

        ptr = (char*)n + sizeof(tl_list_node);
        ALLIGN( ptr );
        tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );

        free( n );
        --(this->size);
    }
}

void tl_list_clear( tl_list* this )
{
    tl_list_remove( this, 0, this->size );
}

/****************************************************************************/

static tl_list_node* merge( tl_list_node* a, tl_list_node* b,
                            tl_compare cmp )
{
    tl_list_node dummy, *tail;

    if( !a ) return b;
    if( !b ) return a;

    for( tail=&dummy; a && b; tail=tail->next )
    {
        if( cmp( tl_list_node_get_data(a), tl_list_node_get_data(b) ) <= 0 )
        {
            a->prev = tail;
            tail->next = a;
            a = a->next;
        }
        else
        {
            b->prev = tail;
            tail->next = b;
            b = b->next;
        }
    }

    tail->next = a ? a : b;
    tail->next->prev = tail;
    dummy.next->prev = NULL;
    return dummy.next;
}

static tl_list_node* msort( tl_list_node* list, tl_compare cmp )
{
    tl_list_node *a, *b;

    if( !list || !list->next )
        return list;

    /* find center of the list */
    a = b = list;

    while( (b = b->next) && (b = b->next) )
        a = a->next;

    /* split list in half, sort sublists, merge back */
    b = a->next;
    a->next = NULL;
    return merge( msort(list, cmp), msort(b,cmp), cmp );
}

void tl_list_sort( tl_list* this, tl_compare cmp )
{
    tl_list_node* n;

    if( this && cmp && this->size>1 )
    {
        this->first = msort( this->first, cmp );

        for( n=this->first; n->next; n=n->next ) { }
        this->last = n;
    }
}

tl_list_node* tl_list_search( const tl_list* this, tl_compare cmp,
                              const void* key )
{
    tl_list_node* n;

    if( this && cmp && key )
    {
        for( n=this->first; n; n=n->next )
        {
            if( cmp( tl_list_node_get_data( n ), key )==0 )
                return n;
        }
    }

    return NULL;
}

