/*
 * iostream.c
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define TL_EXPORT
#include "tl_iostream.h"



int tl_iostream_write_blob( tl_iostream* this, const tl_blob* blob,
                            size_t* actual )
{
    if( !this || !blob )
        return TL_ERR_INTERNAL;

    return this->write( this, blob->data, blob->size, actual );
}

int tl_iostream_read_blob( tl_iostream* this, tl_blob* blob,
                           size_t maximum )
{
    int status;

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_ERR_INTERNAL;

    status = this->read( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
}

