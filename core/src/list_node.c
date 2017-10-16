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

#ifdef TL_ALLIGN_MEMORY
#define PADDING sizeof(void*)
#define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
#define PADDING 0
#define ALLIGN( ptr )
#endif

tl_list_node *tl_list_node_create(const tl_list * this, const void *data)
{
	tl_list_node *node;
	char *ptr;

	assert(this && data);

	node = malloc(sizeof(tl_list_node) + this->unitsize + PADDING);

	ptr = (char *)node + sizeof(tl_list_node);
	ALLIGN(ptr);

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
	char *ptr = NULL;

	assert(node && list);

	ptr = (char *)node + sizeof(tl_list_node);
	ALLIGN(ptr);

	tl_allocator_cleanup(list->alloc, ptr, list->unitsize, 1);
	free(node);
}

void *tl_list_node_get_data(const tl_list_node * node)
{
	char *ptr = NULL;

	assert(node);

	ptr = (char *)node + sizeof(tl_list_node);
	ALLIGN(ptr);

	return ptr;
}
