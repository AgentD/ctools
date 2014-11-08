/**
 * \file tl_interfaces.h
 *
 * \brief Forward declarations and callback types
 */
#ifndef TOOLS_PREDEF_H
#define TOOLS_PREDEF_H



#include <stddef.h>

#ifdef _MSC_VER
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;

    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;
#else
    #include <stdint.h>
#endif




/**
 * \brief A function used to compare two objects
 *
 * \param a A pointer to the first object
 * \param b A pointer to the second object
 *
 * \return >0 if a is greater than b, <0 if a is smaller than b, 0 if both
 *         are equal
 */
typedef int(* tl_compare )( const void* a, const void* b );



#endif /* TOOLS_PREDEF_H */

