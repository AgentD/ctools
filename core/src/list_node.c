/* list_node.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_list.h"

#include <stdlib.h>

tl_list_node *tl_list_node_create(const tl_list * this, const void *data)
{
	tl_list_node *node;
	void *ptr;

	assert(this && data);

	node = malloc(sizeof(*node) - 1 + this->unitsize);
	ptr = &node->data[0];

	if (data) {
		tl_allocator_copy(this->alloc, ptr, data, this->unitsize, 1);
	} else {
		tl_allocator_init(this->alloc, ptr, this->unitsize, 1);
	}

	node->prev = node->next = NULL;
	return node;
}

void tl_list_node_destroy(tl_list_node * node, tl_list * list)
{
	assert(node && list);

	tl_allocator_cleanup(list->alloc, &node->data[0], list->unitsize, 1);
	free(node);
}

void *tl_list_node_get_data(const tl_list_node * node)
{
	assert(node);

	return (void *)&node->data[0];
}
