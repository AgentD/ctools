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

#define IS_RED(n) ((n) && (n)->is_red)

static void destroy_node(tl_rbtree_node *this, const tl_rbtree *tree)
{
	unsigned char *ptr;

	ptr = (unsigned char *)this + sizeof(*this);
	tl_allocator_cleanup(tree->keyalloc, ptr, tree->keysize, 1);

	ptr += tree->keysize_padded;
	tl_allocator_cleanup(tree->valalloc, ptr, tree->valuesize, 1);
	free(this);
}

static void node_recursive_delete(tl_rbtree_node *this, const tl_rbtree *tree)
{
	tl_rbtree_node *l, *r;
recursion:
	if (!this)
		return;

	l = this->left;
	r = this->right;
	destroy_node(this, tree);

	node_recursive_delete(l, tree);
	this = r;
	goto recursion;
}

static void flip_colors(tl_rbtree_node *this)
{
	this->is_red = !this->is_red;
	this->left->is_red = !this->left->is_red;
	this->right->is_red = !this->right->is_red;
}

static tl_rbtree_node *rotate_right(tl_rbtree_node * this)
{
	tl_rbtree_node *x;

	x = this->left;
	this->left = x->right;
	x->right = this;
	x->is_red = x->right->is_red;
	x->right->is_red = 1;

	return x;
}

static tl_rbtree_node *rotate_left(tl_rbtree_node *this)
{
	tl_rbtree_node *x;

	x = this->right;
	this->right = x->left;
	x->left = this;
	x->is_red = x->left->is_red;
	x->left->is_red = 1;

	return x;
}

static tl_rbtree_node *subtree_balance(tl_rbtree_node *this)
{
	if (IS_RED(this->right) && !IS_RED(this->left))
		this = rotate_left(this);

	if (IS_RED(this->left) && IS_RED(this->left->left))
		this = rotate_right(this);

	if (IS_RED(this->left) && IS_RED(this->right))
		flip_colors(this);

	return this;
}

static tl_rbtree_node *move_red_left(tl_rbtree_node *this)
{
	flip_colors(this);

	if (IS_RED(this->right->left)) {
		this->right = rotate_right(this->right);
		this = rotate_left(this);
		flip_colors(this);
	}

	return this;
}

static tl_rbtree_node *move_red_right(tl_rbtree_node *this)
{
	flip_colors(this);

	return IS_RED(this->left->left) ? rotate_right(this) : this;
}

static tl_rbtree_node *subtree_insert(tl_rbtree *this, tl_rbtree_node *root,
				      tl_rbtree_node *newnode)
{
	void *newkey, *key;

	if (!root)
		return newnode;

	newkey = tl_rbtree_node_get_key(this, newnode);
	key = tl_rbtree_node_get_key(this, root);

	if (this->compare(newkey, key) < 0) {
		root->left = subtree_insert(this, root->left, newnode);
	} else {
		root->right = subtree_insert(this, root->right, newnode);
	}

	return subtree_balance(root);
}

static tl_rbtree_node *remove_min_from_subtree(tl_rbtree_node *this,
					       tl_rbtree *tree)
{
	if (!this->left) {
		destroy_node(this, tree);
		return NULL;
	}

	if (!IS_RED(this->left) && !IS_RED(this->left->left))
		this = move_red_left(this);

	this->left = remove_min_from_subtree(this->left, tree);
	return subtree_balance(this);
}

static tl_rbtree_node *remove_max_from_subtree(tl_rbtree_node *this,
					       tl_rbtree *tree)
{
	if (IS_RED(this->left))
		this = rotate_right(this);

	if (!this->right) {
		destroy_node(this, tree);
		return NULL;
	}

	if (!IS_RED(this->right) && !IS_RED(this->right->left))
		this = move_red_right(this);

	this->right = remove_max_from_subtree(this->right, tree);
	return subtree_balance(this);
}

static tl_rbtree_node *remove_from_subtree(tl_rbtree *this,
					   tl_rbtree_node *root,
					   const void *key)
{
	tl_rbtree_node *min;

	if (this->compare(key, tl_rbtree_node_get_key(this, root)) < 0) {
		if (!IS_RED(root->left) && !IS_RED(root->left->left))
			root = move_red_left(root);

		root->left = remove_from_subtree(this, root->left, key);
	} else {
		if (IS_RED(root->left))
			root = rotate_right(root);

		if (!this->compare(key, tl_rbtree_node_get_key(this, root))
		    && !(root->right)) {
			free(root);
			return NULL;
		}

		if (!IS_RED(root->right) && !IS_RED(root->right->left))
			root = move_red_right(root);

		if (!this->compare(key, tl_rbtree_node_get_key(this, root))) {
			/* find minimum of right subtree */
			for (min = root->right; min->left; min = min->left)
				;

			/* "swap" minimum of right subtree with root */
			memcpy((char *)root + sizeof(*root),
			       (char *)min + sizeof(*min),
			       this->keysize_padded + this->valuesize);

			/* remove minimum of the right subtree */
			root->right =
			    remove_min_from_subtree(root->right, this);
		} else {
			root->right =
			    remove_from_subtree(this, root->right, key);
		}
	}

	return subtree_balance(root);
}

/****************************************************************************/

tl_rbtree_node *tl_rbtree_node_create(const tl_rbtree *tree,
				      const void *key, const void *value)
{
	tl_rbtree_node *node;
	unsigned char *ptr;
	size_t size;

	assert(tree);

	size = sizeof(*node) + tree->keysize_padded + tree->valuesize;
	node = calloc(1, size);
	if (!node)
		return NULL;

	node->is_red = 1;

	/* set key pointer */
	ptr = (unsigned char *)node + sizeof(*node);

	if (key) {
		tl_allocator_copy(tree->keyalloc, ptr, key, tree->keysize, 1);
	} else {
		tl_allocator_init(tree->keyalloc, ptr, tree->keysize, 1);
	}

	/* set value */
	ptr += tree->keysize_padded;

	if (value) {
		tl_allocator_copy(tree->valalloc, ptr, value,
				  tree->valuesize, 1);
	} else {
		tl_allocator_init(tree->valalloc, ptr, tree->valuesize, 1);
	}
	return node;
}

static tl_rbtree_node *copy_subtree(const tl_rbtree *this,
				    const tl_rbtree_node *src)
{
	tl_rbtree_node *copy;
	char *value, *key;

	if (!src)
		return NULL;

	key = tl_rbtree_node_get_key(this, src);
	value = tl_rbtree_node_get_value(this, src);
	copy = tl_rbtree_node_create(this, key, value);

	if (!copy)
		return NULL;

	copy->left = copy_subtree(this, src->left);
	copy->right = copy_subtree(this, src->right);

	if ((src->left && !copy->left) || (src->right && !copy->right)) {
		node_recursive_delete(copy, this);
		copy = NULL;
	}
	return copy;
}

/****************************************************************************/

void tl_rbtree_init(tl_rbtree *this, size_t keysize, size_t valuesize,
		    tl_compare comparefun, tl_allocator *keyalloc,
		    tl_allocator *valalloc)
{
	assert(this);

	this->root = NULL;
	this->size = 0;
	this->compare = comparefun;
	this->keysize = keysize;
	this->keysize_padded = keysize;
	this->valuesize = valuesize;
	this->keyalloc = keyalloc;
	this->valalloc = valalloc;

	if (keysize % sizeof(void*)) {
		this->keysize_padded += sizeof(void*);
		this->keysize_padded -= keysize % sizeof(void*);
	}
}

void tl_rbtree_cleanup(tl_rbtree *this)
{
	assert(this);

	node_recursive_delete(this->root, this);
	this->root = NULL;
	this->size = 0;
}

int tl_rbtree_copy(tl_rbtree *this, const tl_rbtree *src)
{
	tl_rbtree_node *newroot;

	assert(this && src);

	newroot = copy_subtree(src, src->root);

	if (!newroot)
		return 0;

	node_recursive_delete(this->root, this);
	this->root = newroot;
	this->size = src->size;
	this->keysize = src->keysize;
	this->valuesize = src->valuesize;
	this->compare = src->compare;
	this->valalloc = src->valalloc;
	this->keyalloc = src->keyalloc;
	return 1;
}

int tl_rbtree_insert(tl_rbtree *this, const void *key, const void *value)
{
	tl_rbtree_node *node;

	assert(this && key);

	node = tl_rbtree_node_create(this, key, value);

	if (!node)
		return 0;

	this->root = subtree_insert(this, this->root, node);
	this->root->is_red = 0;

	++(this->size);
	return 1;
}

void *tl_rbtree_at(const tl_rbtree *this, const void *key)
{
	tl_rbtree_node *node;
	void *nodekey;
	int ret;

	assert(this && key);

	for (node = this->root; node != NULL; ) {
		nodekey = tl_rbtree_node_get_key(this, node);
		ret = this->compare(key, nodekey);

		if (ret == 0)
			return tl_rbtree_node_get_value(this, node);

		node = ret < 0 ? node->left : node->right;
	}

	return NULL;
}

int tl_rbtree_set(tl_rbtree *this, const void *key, const void *value)
{
	void *ptr;

	assert(this && key && value);

	if (!(ptr = tl_rbtree_at(this, key)))
		return 0;

	tl_allocator_cleanup(this->valalloc, ptr, this->valuesize, 1);
	tl_allocator_copy(this->valalloc, ptr, value, this->valuesize, 1);
	return 1;
}

int tl_rbtree_get_min(const tl_rbtree *this, void **key, void **value)
{
	tl_rbtree_node *n;

	assert(this);

	for (n = this->root; n->left; n = n->left)
		;

	if (!n)
		return 0;

	if (key)
		*key = tl_rbtree_node_get_key(this, n);

	if (value)
		*value = tl_rbtree_node_get_value(this, n);

	return 1;
}

int tl_rbtree_get_max(const tl_rbtree *this, void **key, void **value)
{
	tl_rbtree_node *n;

	assert(this);

	for (n = this->root; n->right; n = n->right)
		;

	if (!n)
		return 0;

	if (key)
		*key = tl_rbtree_node_get_key(this, n);

	if (value)
		*value = tl_rbtree_node_get_value(this, n);

	return 1;
}

void tl_rbtree_remove_min(tl_rbtree *this)
{
	assert(this);

	if (!(this->size))
		return;

	/* hack for tree balancing algorithm */
	if (!IS_RED(this->root->left) && !IS_RED(this->root->right))
		this->root->is_red = 1;

	this->root = remove_min_from_subtree(this->root, this);

	if (this->root)
		this->root->is_red = 0;

	--(this->size);
}

void tl_rbtree_remove_max(tl_rbtree *this)
{
	assert(this);

	if (!(this->size))
		return;

	/* hack for tree balancing algorithm */
	if (!IS_RED(this->root->left) && !IS_RED(this->root->right))
		this->root->is_red = 1;

	this->root = remove_max_from_subtree(this->root, this);

	if (this->root)
		this->root->is_red = 0;

	--(this->size);
}

int tl_rbtree_remove(tl_rbtree *this, const void *key, void *value)
{

	tl_rbtree_node *node;
	char *ptr;
	int ret;

	assert(this && key);

	/* find node */
	for (node = this->root; node != NULL; ) {
		ptr = tl_rbtree_node_get_key(this, node);
		ret = this->compare(key, ptr);

		if (ret == 0)
			break;

		node = ret < 0 ? node->left : node->right;
	}

	if (!node)
		return 0;

	/* cleanup */
	tl_allocator_cleanup(this->keyalloc, ptr, this->keysize, 1);

	ptr = tl_rbtree_node_get_value(this, node);

	if (value) {
		memcpy(value, ptr, this->valuesize);
	} else {
		tl_allocator_cleanup(this->valalloc, ptr, this->valuesize, 1);
	}

	/* remove node */
	this->root = remove_from_subtree(this, this->root, key);

	if (this->root)
		this->root->is_red = 0;

	--(this->size);
	return 1;
}

void tl_rbtree_clear(tl_rbtree *this)
{
	assert(this);

	node_recursive_delete(this->root, this);
	this->root = NULL;
	this->size = 0;
}
