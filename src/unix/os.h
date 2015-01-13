/*
 * os.h
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
#ifndef OS_H
#define OS_H



#include "tl_string.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>



#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate an errno value to an TL_FS_* error code */
int errno_to_fs( int code );

/**
 * \brief Convert a tl_string to an UTF-8 string
 *
 * \param in A pointer to convert
 *
 * \return A buffer holding an UTF-8 version. Must be freed using free( ).
 */
char* to_utf8( const tl_string* in );

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

