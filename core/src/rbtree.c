/* rbtree.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_rbtree.h"

#include <stdlib.h>
#include <string.h>



#define IS_RED( n ) ((n) && (n)->is_red)



#ifdef TL_ALLIGN_MEMORY
    #define PADDING sizeof(void*)
    #define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
    #define PADDING 0
    #define ALLIGN( ptr )
#endif



static void node_recursive_delete( tl_rbtree_node* this,
                                   const tl_rbtree* tree )
{
    char* ptr;

    if( this )
    {
        node_recursive_delete( this->left,  tree );
        node_recursive_delete( this->right, tree );

        ptr = (char*)this + sizeof(tl_rbtree_node);
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->keyalloc, ptr, tree->keysize, 1 );

        ptr += tree->keysize;
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->valalloc, ptr, tree->valuesize, 1 );

        free( this );
    }
}

static void flip_colors( tl_rbtree_node* this )
{
    this->is_red        = !this->is_red;
    this->left->is_red  = !this->left->is_red;
    this->right->is_red = !this->right->is_red;
}

static tl_rbtree_node* rotate_right( tl_rbtree_node* this )
{
    tl_rbtree_node* x;

    x = this->left;
    this->left = x->right;
    x->right = this;
    x->is_red = x->right->is_red;
    x->right->is_red = 1;

    return x;
}

static tl_rbtree_node* rotate_left( tl_rbtree_node* this )
{
    tl_rbtree_node* x;

    x = this->right;
    this->right = x->left;
    x->left = this;
    x->is_red = x->left->is_red;
    x->left->is_red = 1;

    return x;
}

static tl_rbtree_node* subtree_balance( tl_rbtree_node* this )
{
    if( IS_RED(this->right) && !IS_RED(this->left) )
        this = rotate_left( this );

    if( IS_RED(this->left) && IS_RED(this->left->left) )
        this = rotate_right( this );

    if( IS_RED(this->left) && IS_RED(this->right) )
        flip_colors( this );

    return this;
}

static tl_rbtree_node* move_red_left( tl_rbtree_node* this )
{
    flip_colors( this );

    if( IS_RED(this->right->left) )
    {
        this->right = rotate_right(this->right);
        this = rotate_left( this );
        flip_colors( this );
    }

    return this;
}

static tl_rbtree_node* move_red_right( tl_rbtree_node* this )
{
    flip_colors( this );

    return IS_RED(this->left->left) ? rotate_right( this ) : this;
}

static tl_rbtree_node* subtree_insert( tl_rbtree* this,
                                       tl_rbtree_node* root,
                                       tl_rbtree_node* newnode )
{ 
    void* newkey;
    void* key;

    if( !root )
        return newnode;

    newkey = tl_rbtree_node_get_key( this, newnode );
    key    = tl_rbtree_node_get_key( this, root    );

    if( this->compare( newkey, key ) < 0 )
    {
        root->left = subtree_insert( this, root->left, newnode );
    }
    else
    {
        root->right = subtree_insert( this, root->right, newnode );
    }

    return subtree_balance( root );
}

static tl_rbtree_node* remove_min_from_subtree( tl_rbtree_node* this,
                                                tl_rbtree* tree )
{
    char* ptr;

    if( !this->left )
    {
        ptr = (char*)this + sizeof(tl_rbtree_node);
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->keyalloc, ptr, tree->keysize, 1 );

        ptr += tree->keysize;
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->valalloc, ptr, tree->valuesize, 1 );

        free( this );
        return NULL;
    }

    if( !IS_RED(this->left) && !IS_RED(this->left->left) )
        this = move_red_left( this );

    this->left = remove_min_from_subtree( this->left, tree );
    return subtree_balance( this );
}

static tl_rbtree_node* remove_max_from_subtree( tl_rbtree_node* this,
                                                tl_rbtree* tree )
{
    char* ptr;

    if( IS_RED(this->left) )
        this = rotate_right( this );

    if( !this->right )
    {
        ptr = (char*)this + sizeof(tl_rbtree_node);
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->keyalloc, ptr, tree->keysize, 1 );

        ptr += tree->keysize;
        ALLIGN( ptr );
        tl_allocator_cleanup( tree->valalloc, ptr, tree->valuesize, 1 );

        free( this );
        return NULL;
    }

    if( !IS_RED(this->right) && !IS_RED(this->right->left) )
        this = move_red_right( this );

    this->right = remove_max_from_subtree( this->right, tree );
    return subtree_balance( this );
}

static tl_rbtree_node* remove_from_subtree( tl_rbtree* this,
                                            tl_rbtree_node* root,
                                            const void* key )
{
    tl_rbtree_node* min;

    if( this->compare( key, tl_rbtree_node_get_key( this, root ) ) < 0 )
    {
        if( !IS_RED(root->left) && !IS_RED(root->left->left) )
            root = move_red_left( root );

        root->left = remove_from_subtree( this, root->left, key );
    }
    else
    {
        if( IS_RED(root->left) )
            root = rotate_right( root );

        if( this->compare( key, tl_rbtree_node_get_key( this, root ) )==0 &&
            !(root->right) )
        {
            free( root );
            return NULL;
        }

        if( !IS_RED(root->right) && !IS_RED(root->right->left) )
            root = move_red_right( root );

        if( this->compare( key, tl_rbtree_node_get_key( this, root ) )==0 )
        {
            /* find minimum of right subtree */
            for( min=root->right; min->left; min=min->left ) { }

            /* "swap" minimum of right subtree with root */
            memcpy( (unsigned char*)root + sizeof(tl_rbtree_node),
                    (unsigned char*)min + sizeof(tl_rbtree_node),
                    2*PADDING+this->keysize+this->valuesize );

            /* remove minimum of the right subtree */
            root->right = remove_min_from_subtree( root->right, this );
        }
        else
        {
            root->right = remove_from_subtree( this, root->right, key );
        }
    }

    return subtree_balance( root );
}

/****************************************************************************/

tl_rbtree_node* tl_rbtree_node_create( const tl_rbtree* tree,
                                       const void* key,
                                       const void* value )
{
    tl_rbtree_node* node;
    unsigned char* ptr;

    assert( tree );

    node = malloc( sizeof(tl_rbtree_node) + 2*PADDING +
                   tree->keysize + tree->valuesize );

    if( !node )
        return NULL;

    node->left   = NULL;
    node->right  = NULL;
    node->is_red = 1;

    /* set key pointer */
    ptr = (unsigned char*)node + sizeof(tl_rbtree_node);
    ALLIGN( ptr );

    if( key )
        tl_allocator_copy( tree->keyalloc, ptr, key, tree->keysize, 1 );
    else
        tl_allocator_init( tree->keyalloc, ptr, tree->keysize, 1 );

    /* set value */
    ptr += tree->keysize;
    ALLIGN( ptr );

    if( value )
        tl_allocator_copy( tree->valalloc, ptr, value, tree->valuesize, 1 );
    else
        tl_allocator_init( tree->valalloc, ptr, tree->valuesize, 1 );

    return node;
}

void* tl_rbtree_node_get_key( const tl_rbtree* tree,
                              const tl_rbtree_node* node )
{
    char* ptr;
    (void)tree;

    assert( tree && node );

    ptr = (char*)node + sizeof(tl_rbtree_node);
    ALLIGN( ptr );

    return ptr;
}

void* tl_rbtree_node_get_value( const tl_rbtree* tree,
                                const tl_rbtree_node* node )
{
    char* ptr;

    assert( tree && node );

    ptr = (char*)node + sizeof(tl_rbtree_node);
    ALLIGN( ptr );

    ptr += tree->keysize;
    ALLIGN( ptr );

    return ptr;
}

static tl_rbtree_node* copy_subtree( const tl_rbtree* this,
                                     const tl_rbtree_node* src )
{
    tl_rbtree_node* copy;
    char* value;
    char* key;

    if( !src )
        return NULL;

    /* create a copy node */
    key   = tl_rbtree_node_get_key( this, src );
    value = tl_rbtree_node_get_value( this, src );
    copy  = tl_rbtree_node_create( this, key, value );

    /* copy subtrees */
    if( copy )
    {
        copy->left  = copy_subtree( this, src->left  );
        copy->right = copy_subtree( this, src->right );

        if( (src->left && !copy->left) || (src->right && !copy->right) )
        {
            node_recursive_delete( copy, this );
            copy = NULL;
        }
    }
    return copy;
}

/****************************************************************************/

void tl_rbtree_init( tl_rbtree* this, size_t keysize, size_t valuesize,
                     tl_compare comparefun, tl_allocator* keyalloc,
                     tl_allocator* valalloc )
{
    assert( this );

    this->root = NULL;
    this->size = 0;
    this->compare = comparefun;
    this->keysize = keysize;
    this->valuesize = valuesize;
    this->keyalloc = keyalloc;
    this->valalloc = valalloc;
}

void tl_rbtree_cleanup( tl_rbtree* this )
{
    assert( this );

    node_recursive_delete( this->root, this );
    this->root = NULL;
    this->size = 0;
}

int tl_rbtree_copy( tl_rbtree* this, const tl_rbtree* src )
{
    tl_rbtree_node* newroot;

    assert( this && src );

    /* create a copy of the source tree */
    newroot = copy_subtree( src, src->root );

    if( !newroot )
        return 0;

    /* copy over new tree and data of source tree */
    node_recursive_delete( this->root, this );
    this->root      = newroot;
    this->size      = src->size;
    this->keysize   = src->keysize;
    this->valuesize = src->valuesize;
    this->compare   = src->compare;
    this->valalloc  = src->valalloc;
    this->keyalloc  = src->keyalloc;
    return 1;
}

int tl_rbtree_insert( tl_rbtree* this, const void* key, const void* value )
{
    tl_rbtree_node* node;

    assert( this && key );

    node = tl_rbtree_node_create( this, key, value );

    if( !node )
        return 0;

    this->root = subtree_insert( this, this->root, node );
    this->root->is_red = 0;

    ++(this->size);
    return 1;
}

void* tl_rbtree_at( const tl_rbtree* this, const void* key )
{
    tl_rbtree_node* node;
    void* nodekey;

    assert( this && key );

    node = this->root;

    while( node )
    {
        nodekey = tl_rbtree_node_get_key( this, node );

        if( this->compare( key, nodekey ) == 0 )
        {
            return tl_rbtree_node_get_value( this, node );
        }

        node = (this->compare( key, nodekey ) < 0) ? node->left : node->right;
    }

    return NULL;
}

int tl_rbtree_set( tl_rbtree* this, const void* key, const void* value )
{
    void* ptr;

    assert( this && key && value );

    if( !(ptr = tl_rbtree_at( this, key )) )
        return 0;

    tl_allocator_cleanup( this->valalloc, ptr, this->valuesize, 1 );
    tl_allocator_copy( this->valalloc, ptr, value, this->valuesize, 1 );
    return 1;
}

int tl_rbtree_get_min( const tl_rbtree* this, void** key, void** value )
{
    tl_rbtree_node* n;

    assert( this );

    if( !this->root )
        return 0;

    if( !key && !value )
        return 1;

    for( n=this->root; n->left; n=n->left ) { }

    if( key )
        *key = tl_rbtree_node_get_key( this, n );

    if( value )
        *value = tl_rbtree_node_get_value( this, n );

    return 1;
}

int tl_rbtree_get_max( const tl_rbtree* this, void** key, void** value )
{
    tl_rbtree_node* n;

    assert( this );

    if( !this->root )
        return 0;

    if( !key && !value )
        return 1;

    for( n=this->root; n->right; n=n->right ) { }

    if( key )
        *key = tl_rbtree_node_get_key( this, n );

    if( value )
        *value = tl_rbtree_node_get_value( this, n );

    return 1;
}

void tl_rbtree_remove_min( tl_rbtree* this )
{
    assert( this );

    if( !(this->size) )
        return;

    /* hack for tree balancing algorithm */
    if( !IS_RED(this->root->left) && !IS_RED(this->root->right) )
        this->root->is_red = 1;

    this->root = remove_min_from_subtree( this->root, this );

    if( this->root )
        this->root->is_red = 0;

    --(this->size);
}

void tl_rbtree_remove_max( tl_rbtree* this )
{
    assert( this );

    if( !(this->size) )
        return;

    /* hack for tree balancing algorithm */
    if( !IS_RED(this->root->left) && !IS_RED(this->root->right) )
        this->root->is_red = 1;

    this->root = remove_max_from_subtree( this->root, this );

    if( this->root )
        this->root->is_red = 0;

    --(this->size);
}

int tl_rbtree_remove( tl_rbtree* this, const void* key, void* value )
{ 

    tl_rbtree_node* node;
    char* ptr;

    assert( this && key );

    if( !this->size )
        return 0;

    /* find node */
    node = this->root;

    while( node )
    {
        ptr = tl_rbtree_node_get_key( this, node );

        if( this->compare( key, ptr ) == 0 )
            break;

        node = (this->compare( key, ptr ) < 0) ? node->left : node->right;
    }

    if( !node )
        return 0;

    /* cleanup */
    tl_allocator_cleanup( this->keyalloc, ptr, this->keysize, 1 );
    ptr += this->keysize;
    ALLIGN(ptr);

    if( value )
        memcpy( value, ptr, this->valuesize );
    else
        tl_allocator_cleanup( this->valalloc, ptr, this->valuesize, 1 );

    /* remove node */
    this->root = remove_from_subtree( this, this->root, key );

    if( this->root )
        this->root->is_red = 0;

    --(this->size);
    return 1;
}

void tl_rbtree_clear( tl_rbtree* this )
{
    assert( this );

    node_recursive_delete( this->root, this );
    this->root = NULL;
    this->size = 0;
}

