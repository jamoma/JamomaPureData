/** @file
 *
 * @ingroup implementationPdLibrary
 *
 * @brief Data types for Max objects that fit more naturally with the Jamoma coding style.
 *
 * @details
 *
 * @authors Tim Place, Antoine Villeret
 *
 * @copyright Copyright © 2006, Tim Place @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __JAMOMA_PD_OBJECT_TYPES_H__
#define __JAMOMA_PD_OBJECT_TYPES_H__

#include "JamomaForPd.h"

// typedef t_pxobject	MspObject;

// typedef t_class*	ClassPtr;
// typedef MspObject*	MspObjectPtr;
// typedef t_max_err	MaxErr;

// typedef t_linklist*	LinkedListPtr;
// typedef t_hashtab*	HashtabPtr;

// use SymbolGen() instead of gensym() so that we don't have tons of const warningss									
template <class T>
t_symbol *SymbolGen(T stringArg)
{
	return gensym((char*)stringArg);
}

template <class T>
t_symbol * SymbolGen(const char* stringArg)
{
	return gensym((char*)stringArg);
}
											
#ifndef SELF
#define SELF (t_object*(self))
#endif

#endif // __JAMOMA_PD_OBJECT_TYPES_H__
