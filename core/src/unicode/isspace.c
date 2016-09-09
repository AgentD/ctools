/* isspace.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_unicode.h"

int tl_isspace(int cp)
{
    if( cp>=0x2000 && cp<=0x200A )
    {
        if( cp==0x2007 )
            return TL_SPACE|TL_NB_SPACE;

        return TL_SPACE;
    }

    if( cp==0x2028 || cp==0x2029 )
        return TL_SPACE|TL_SPACE;

    if( cp==0x00A0 || cp==0x202F )
        return TL_SPACE|TL_NB_SPACE;

    if( cp==0x0020 || cp==0x1680 || cp==0x205F || cp==0x3000 )
        return TL_SPACE;

    if( cp>=0x09 && cp<=0x0D )
        return TL_SPACE;

    if( cp>=0x1C && cp<=0x1F )
        return TL_SPACE;

    return 0;
}
