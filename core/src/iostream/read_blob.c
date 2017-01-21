/* read_blob.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iostream.h"

int tl_iostream_read_blob( tl_iostream* this, tl_blob* blob,
                           size_t maximum )
{
    int status;

    assert( this && blob );

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_ERR_ALLOC;

    status = this->read( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
}

