#ifndef TOOLS_INTERFACES_H
#define TOOLS_INTERFACES_H



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



#endif /* TOOLS_INTERFACES_H */

