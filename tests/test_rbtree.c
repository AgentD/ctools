#include "tl_rbtree.h"

#include <stdlib.h>
#include <stdio.h>


static int compare_int( const void* a, const void* b )
{
    if( *((int*)a) < *((int*)b) ) return -1;
    if( *((int*)a) > *((int*)b) ) return 1;
    return 0;
}

static int is_bst( tl_rbtree* tree, tl_rbtree_node* n, int* min, int* max )
{
    int* key;

    if( !n )
        return 1;

    key = (int*)tl_rbtree_node_get_key( tree, n );

    if( *key < *min || *key > *max )
        return 0;

    return is_bst( tree, n->left, min, key ) &&
           is_bst( tree, n->right, key, max );
} 

static int is_23( tl_rbtree* tree, tl_rbtree_node* n )
{
    if( !n )
        return 1;
    if( n->right && n->right->is_red )
        return 0;
    if( n!=tree->root && n->is_red && n->left && n->left->is_red )
        return 0;
    return is_23( tree, n->left ) && is_23( tree, n->right );
}

static int are_subtrees_balanced( tl_rbtree_node* n, int blackcount )
{
    if( !n )
        return blackcount==0;

    if( !n->is_red )
        --blackcount;

    return are_subtrees_balanced( n->left,  blackcount ) &&
           are_subtrees_balanced( n->right, blackcount );
}

static int is_balanced( tl_rbtree* tree )
{
    tl_rbtree_node* n;
    int blackcount;

    for( blackcount=0, n=tree->root; n; n=n->left )
    {
        if( !n->is_red )
            ++blackcount;
    }

    return are_subtrees_balanced( tree->root, blackcount );
}

static int find_min_key( tl_rbtree* tree )
{
    tl_rbtree_node* n = tree->root;

    if( !n )
        return 0;

    while( n->left )
        n = n->left;

    return *((int*)tl_rbtree_node_get_key( tree, n ));
}

static int find_max_key( tl_rbtree* tree )
{
    tl_rbtree_node* n = tree->root;

    if( !n )
        return 0;

    while( n->right )
        n = n->right;

    return *((int*)tl_rbtree_node_get_key( tree, n ));
}

static int check_tree( tl_rbtree* tree )
{
    int min = find_min_key( tree );
    int max = find_max_key( tree );

    return is_bst( tree, tree->root, &min, &max ) &&
           is_23( tree, tree->root ) &&
           is_balanced( tree );
}

static int are_subtrees_equal( tl_rbtree* atree, tl_rbtree* btree,
                               tl_rbtree_node* a, tl_rbtree_node* b )
{
    if( !a && !b )
        return 1;

    if( a && b )
    {
        if( *((int*)tl_rbtree_node_get_key( atree, a )) !=
            *((int*)tl_rbtree_node_get_key( btree, b )) )
            return 0;

        if( *((int*)tl_rbtree_node_get_value( atree, a )) !=
            *((int*)tl_rbtree_node_get_value( btree, b )) )
            return 0;

        return are_subtrees_equal( atree, btree, a->left, b->left ) &&
               are_subtrees_equal( atree, btree, a->right, b->right );
    }

    return 0;
}

int main( void )
{
    tl_rbtree t0, t1;
    int *p0, *p1;
    int i, j;

    tl_rbtree_init( &t0, sizeof(int), sizeof(int), compare_int, NULL, NULL );

    /* insert nodes */
    for( i=0; i<1000; ++i )
    {
        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;

        /* insert node, check size before and after */
        j = i*10 + 5;
        if( t0.size!=(size_t)i ) return EXIT_FAILURE;
        tl_rbtree_insert( &t0, &i, &j );
        if( t0.size!=(size_t)(i+1) ) return EXIT_FAILURE;

        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;

        /* check if min an max are correct */
        tl_rbtree_get_min( &t0, (void**)&p0, (void**)&p1 );
        if( !p0 || !p1 || *p0!=0 || *p1!=5 ) return EXIT_FAILURE;
        tl_rbtree_get_max( &t0, (void**)&p0, (void**)&p1 );
        if( !p0 || !p1 || *p0!=i || *p1!=j ) return EXIT_FAILURE;
    }

    /* read back nodes */
    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;
        if( !tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( *((int*)tl_rbtree_at( &t0, &i )) != j ) return EXIT_FAILURE;
    }

    /* try accessing nodes with non existent indices */
    for( i=-10000; i<0; ++i )
    {
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
    }

    for( i=1000; i<10000; ++i )
    {
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
    }

    /* remove minima */
    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;

        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        if( t0.size != 1000-(size_t)i ) return EXIT_FAILURE;

        /* check if min an max are correct */
        if( t0.size )
        {
            tl_rbtree_get_min( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=i || *p1!=(i*10+5) ) return EXIT_FAILURE;
            tl_rbtree_get_max( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=999 || *p1!=9995 ) return EXIT_FAILURE;
        }

        /* remove minimum */
        if( !tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        tl_rbtree_remove_min( &t0 );
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;

        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        if( t0.size != 1000-(size_t)i-1 ) return EXIT_FAILURE;

        /* check if min an max are correct */
        if( t0.size )
        {
            tl_rbtree_get_min( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=(i+1) || *p1!=((i+1)*10+5) )
                return EXIT_FAILURE;
            tl_rbtree_get_max( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=999 || *p1!=9995 ) return EXIT_FAILURE;
        }
    }

    if( t0.size ) return EXIT_FAILURE;

    /* remove maxima */
    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;
        tl_rbtree_insert( &t0, &i, &j );
    }

    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;

        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        if( t0.size != 1000-(size_t)i ) return EXIT_FAILURE;

        /* check if min an max are correct */
        if( t0.size )
        {
            tl_rbtree_get_min( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=0 || *p1!=5 ) return EXIT_FAILURE;
            tl_rbtree_get_max( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=(999-i) || *p1!=((999-i)*10+5) )
                return EXIT_FAILURE;
        }

        /* remove maximum */
        j = 999-i;
        if( !tl_rbtree_at( &t0, &j ) ) return EXIT_FAILURE;
        tl_rbtree_remove_max( &t0 );
        if( tl_rbtree_at( &t0, &j ) ) return EXIT_FAILURE;

        /* check if tree is valid */
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        if( t0.size != 1000-(size_t)i-1 ) return EXIT_FAILURE;

        /* check if min an max are correct */
        if( t0.size )
        {
            tl_rbtree_get_min( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=0 || *p1!=5 ) return EXIT_FAILURE;
            tl_rbtree_get_max( &t0, (void**)&p0, (void**)&p1 );
            if( !p0 || !p1 || *p0!=(999-i-1) || *p1!=((999-i-1)*10+5) )
                return EXIT_FAILURE;
        }
    }

    if( t0.size ) return EXIT_FAILURE;

    /* remove arbitrary nodes */
    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;
        tl_rbtree_insert( &t0, &i, &j );
    }

    if( t0.size != 1000 ) return EXIT_FAILURE;

    for( i=250; i<750; ++i )
    {
        if( !tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 1000-((size_t)i-250) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        tl_rbtree_remove( &t0, &i, NULL );
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 1000-((size_t)(i+1)-250) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
    }

    if( t0.size != 500 ) return EXIT_FAILURE;

    for( i=249; i>=0; --i )
    {
        if( !tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 500-(249-(size_t)i) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        tl_rbtree_remove( &t0, &i, NULL );
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 500-(249-(size_t)i+1) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
    }

    if( t0.size != 250 ) return EXIT_FAILURE;

    for( i=750; i<1000; ++i )
    {
        if( !tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 250-((size_t)i-750) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
        tl_rbtree_remove( &t0, &i, NULL );
        if( tl_rbtree_at( &t0, &i ) ) return EXIT_FAILURE;
        if( t0.size != 250-((size_t)i+1-750) ) return EXIT_FAILURE;
        if( !check_tree( &t0 ) ) return EXIT_FAILURE;
    }

    if( t0.size ) return EXIT_FAILURE;

    /* copy */
    tl_rbtree_init( &t1, sizeof(int), sizeof(int), compare_int, NULL, NULL );

    for( i=0; i<1000; ++i )
    {
        j = i*10 + 5;
        tl_rbtree_insert( &t0, &i, &j );
    }

    if( are_subtrees_equal( &t0, &t1, t0.root, t1.root ) )
        return EXIT_FAILURE;

    tl_rbtree_copy( &t1, &t0 );

    if( t1.size     !=t0.size      ) return EXIT_FAILURE;
    if( t1.keysize  !=t0.keysize   ) return EXIT_FAILURE;
    if( t1.valuesize!=t0.valuesize ) return EXIT_FAILURE;

    if( !are_subtrees_equal( &t0, &t1, t0.root, t1.root ) )
        return EXIT_FAILURE;

    tl_rbtree_cleanup( &t0 );
    tl_rbtree_cleanup( &t1 );
    return EXIT_SUCCESS;
}

