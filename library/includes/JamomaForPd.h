/** @file
 *
 * @ingroup implementationPdLibrary
 *
 * @brief Various utilities for interfacing with Pd that are not specific to JamomaModular as such.
 *
 * @details Functions and resources used by Pd objects.
 *
 * @authors Tim Place, Théo de la Hogue, Trond Lossius, Antoine Villeret
 *
 * @copyright Copyright © 2013, Tim Place & Théo de la Hogue @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __JAMOMA_FOR_PD_H__
#define __JAMOMA_FOR_PD_H__

typedef unsigned int t_fourcc; // Max specific type

// define Pd log level
#define PD_LOG_WARN 2
#define MAX_FILENAME_CHARS MAXPDSTRING


#ifdef WIN_VERSION
#pragma warning(disable:4083) //warning C4083: expected 'newline'; found identifier 's'
#endif // WIN_VERSION

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef TT_PLATFORM_WIN

    #include "windows.h"
    #define JAMOMA_EXPORT_MAXOBJ __declspec(dllexport)

    #ifdef JAMOMA_EXPORTS
        #define JAMOMA_EXPORT __declspec(dllexport)

    #else
    #ifdef JAMOMA_STATIC
        #define JAMOMA_EXPORT

    #else
        #define JAMOMA_EXPORT __declspec(dllimport)

    #endif
    #endif

#else // TT_PLATFORM_MAC

    #define JAMOMA_EXPORT_MAXOBJ __attribute__((visibility("default")))

    #ifdef JAMOMA_EXPORTS
        #define JAMOMA_EXPORT __attribute__((visibility("default")))
    #else
        #define JAMOMA_EXPORT
    #endif

#endif

// Jamoma Core includes
#include "TTFoundationAPI.h"
#include "TTModular.h"

// CicmWrapper
#ifdef __cplusplus
extern "C"
{
#endif
    #include "cicm_wrapper.h"
#ifdef __cplusplus
}
#endif

// standard includes
#include <math.h>
#include <stdlib.h>

// Jamoma Pd includes
#include "JamomaPdVersion.h"
#include "JamomaObject.h"
#include "JamomaPdObjectTypes.h"
#include "JamomaSymbols.h"
#include "JamomaModularForPd.h"

#define JAMOMA "Jamoma"
#define JAMOMA_UNIT_HEIGHT 35.0
#define JAMOMA_UNIT_WIDTH 150.0
#define JAMOMA_MENU_FONT "Arial"
#define JAMOMA_MENU_FONTSIZE 11.0
#define JAMOMA_DEFAULT_FONT "Verdana"
#define JAMOMA_DEFAULT_FONTSIZE 9.0
#define JAMOMA_BUTTON_FONT JAMOMA_DEFAULT_FONT

// some more max to pd translation
#define sysmem_freeptr free
#define sysmem_newptr malloc
#define object_classname eobj_getclassname
#define MAX_PATH_CHARS		2048
#define attr_args_offset atoms_get_attributes_offset
#define attr_args_process ebox_attrprocess_viatoms

void JAMOMA_EXPORT jamoma_init(void);


/** Utility function to perform an atom copy.
 @param dst				The destination t_atom.
 @param src				The t_atom to be copied.
 */
void JAMOMA_EXPORT jamoma_atom_copy(t_atom *dst, t_atom *src);


/** Utility function to compare two t_atoms.
 @param type			The	atom type of a1.
 @param a1				A t_atom.
 @param a2				The t_atom to compare against.
 @return				true if the t_atom's are the same.
 */
bool JAMOMA_EXPORT jamoma_atom_compare(t_symbol *type, t_atom *a1, t_atom *a2);

/** Compare two strings.
 @param s1				Pointer to the first of the two strings to compare-
 @param s2				Pointer to the second of the two strings to compare.
 @return				true if the strings are the same.
 */
bool JAMOMA_EXPORT jamoma_string_compare(char *s1, char *s2);


/** Load obex externals for use within other externals.
 @param objectname		The object name (i.e. SymbolGen("j.send")).
 @param argc			Number of arguments to the external to be loaded.
 @param argv			Pointer to the first of an array of atom arguments to the external that is to be loaded.
 @param object			If the object is loaded successfully, this will be a pointer to the object pointer.
 @return				true if successfully loaded, otherwise false.
 */
bool JAMOMA_EXPORT jamoma_loadextern(t_symbol *objectname, long argc, t_atom *argv, t_object **object);

// prototype for some Max specific functions wrapped
t_symbol* JAMOMA_EXPORT object_attr_getsym(void *x, t_symbol *s); // Retrieves the value of an attribute, given its parent object and name.
void* JAMOMA_EXPORT object_attr_getobj(void *x, t_symbol *s);
method JAMOMA_EXPORT object_getmethod(void* x, t_symbol* s);
void* JAMOMA_EXPORT object_method(void *x, t_symbol *s);
void* JAMOMA_EXPORT object_method_typed(void* x, t_symbol* method, t_symbol* s, long argc, t_atom* argv);
// void* object_method_dsp64(void* x, t_symbol* method, void* s, void method2, long argc, void* argv);
short JAMOMA_EXPORT locatefile_extended(char *name, short *outvol, t_fourcc *outtype, const t_fourcc *filetypelist, short numtypes);
short JAMOMA_EXPORT path_topathname(const short path, const char *file, char *name);

#endif //__JAMOMA_FOR_PD__

