/*
 * os.c
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
#include "os.h"



int errno_to_fs( int code )
{
    switch( code )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return TL_FS_ACCESS;
    case ENOENT:
        return TL_FS_NOT_EXIST;
    case ENOTDIR:
        return TL_FS_NOT_DIR;
    case ENOSPC:
    case EDQUOT:
        return TL_FS_NO_SPACE;
    case EEXIST:
        return TL_FS_EXISTS;
    case ENOTEMPTY:
        return TL_FS_NOT_EMPTY;
    }

    return code==0 ? 0 : TL_FS_SYS_ERROR;
}

char* to_utf8( const tl_string* in )
{
    size_t count = tl_string_utf8_len( in );
    char* str = malloc( count + 1 );

    if( str )
        tl_string_to_utf8( in, str, count+1 );

    return str;
}

