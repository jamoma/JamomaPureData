/** @file
 *
 * @ingroup implementationPdLibrary
 *
 * @brief The global Jamoma object.
 *
 * @details Functions and resources used by Pd objects.
 *
 * @authors Tim Place, Antoine Villeret
 *
 * @copyright Copyright © 2007, Tim Place @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __JAMOMA_OBJECT_H__
#define __JAMOMA_OBJECT_H__

#include "JamomaForPd.h"

/** Data Structure for the global jamoma object.
 *
 * @details This object is instantiated once when the jamoma_init() method is first called and the Jamoma environment is loaded.
 */
typedef struct _jamoma_object
{
	t_object		obj;	
} t_jamoma_object;

#ifdef __cplusplus
extern "C" {
#endif

/** Set up the class.
 */
void		jamoma_object_initclass(void);


/** Object instantiation.
 @return		Pointer to the instantiated object.
 */
t_object	*jamoma_object_new();


/** Called when the object is freed.
 @param obj		Pointer to the object.
 */
void		jamoma_object_free(t_jamoma_object *obj);


#ifdef __cplusplus
}
#endif

#endif //__JAMOMA_OBJECT_H__

