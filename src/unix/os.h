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

