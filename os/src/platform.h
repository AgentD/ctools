/* platform.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef TL_INTERNAL_PLATFORM_H
#define TL_INTERNAL_PLATFORM_H

#if defined(MACHINE_OS_WINDOWS)
    #include "W32/os.h"
#elif defined(MACHINE_OS_UNIX)
    #include "unix/os.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Possible implementation of tl_iostream_splice. Pointers and count are
   already sanity checked. If this returns TL_ERR_NOT_SUPPORTED, a fallback
   is used that copies chunks manually. */
int __tl_os_splice( tl_iostream* out, tl_iostream* in,
                    size_t count, size_t* actual );

#ifdef __cplusplus
}
#endif

#endif /* TL_INTERNAL_PLATFORM_H */

