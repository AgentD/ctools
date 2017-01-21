#ifndef TOOLS_SPLICE_H
#define TOOLS_SPLICE_H

#include "tl_predef.h"
#include "tl_iostream.h"

/**
 * \enum TL_SPLICE_FLAG
 *
 * \brief Flags for tl_iostream_splice
 */
typedef enum
{
    /**
     * \brief If the splice operation is not supported, don't use the fallback
     *
     * By default, if the underlying OS implementation does no support splice
     * for the supplied streams, tl_iostream_splice will use a fallback
     * implementation that reads and writes data in a loop. However, this
     * fallback is no longer atomic. If a read succeeds but a subsequent write
     * fails, the fallback cannot put the data back into the stream and the
     * data is lost. So in some cases, you might prefere tl_iostream_splice
     * to fail immediately and handle the case in a different way.
     */
    TL_SPLICE_NO_FALLBACK = 0x01,

    TL_SPLICE_ALL_FLAGS = 0x01
}
TL_SPLICE_FLAG;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read data from an input steam and write it to an output stream
 *
 * If the underlying streams wrap OS specific objects and the system supports
 * it, this function attempts to read transfer data from one stream and write
 * it to another using zero-copy functions.
 *
 * Should a direct transfer not be possible, a fallback read-write-loop
 * is used, unless \ref TL_SPLICE_NO_FALLBACK is specified.
 *
 * \param out    The stream to write to
 * \param in     The stream to read from
 * \param count  The number of bytes to copy
 * \param actual If not NULL, returns the number of bytes actually copied
 * \param flags  A combination of \ref TL_SPLICE_FLAG flags
 *
 * \return Zero on success, a negative value (\ref TL_ERROR_CODE) on failure
 */
TLOSAPI int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                                size_t count, size_t* actual, int flags );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_SPLICE_H */

