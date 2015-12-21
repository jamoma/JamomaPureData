/** @file
 *
 * @ingroup implementationPdLibrary
 *
 * @brief Jamoma For Max Shared Library
 *
 * @details Functions and resources used by Max objects.
 *
 * @authors Tim Place, Théo de la Hogue, Trond Lossius, Antoine Villeret
 *
 * @copyright Copyright © 2013, Tim Place & Théo de la Hogue @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#include "JamomaForPd.h"
#if defined(TT_PLATFORM_WIN)
	#include <shlobj.h>
#else
	#include <dlfcn.h>
#endif
#include <map>

// statics and globals
static long                 initialized = false;			///< Global variabel indicating whether Jamoma has been initiated or not.

TTSymbol					kTTSym_Jamoma;
TTObject                    JamomaApplicationManager;
TTObject                    JamomaApplication;

TTRegex*					ttRegexForJmod = NULL;
TTRegex*					ttRegexForJcom = NULL;
TTRegex*					ttRegexForModel = NULL;
TTRegex*					ttRegexForModule = NULL;
TTRegex*					ttRegexForView = NULL;
TTRegex*					ttRegexForPdpat = NULL;
TTRegex*					ttRegexForPdhelp = NULL;
TTRegex*					ttRegexForBracket = NULL;

TTString*					ModelPatcherFormat = NULL;
TTString*					ModelPresetFormat = NULL;
TTString*					ViewPresetFormat = NULL;
TTString*					HelpPatcherFormat = NULL;
TTString*					RefpageFormat = NULL;
TTString*					DocumentationFormat = NULL;

/************************************************************************************/
// Init the framework

void jamoma_init(void)
{
	if (!initialized) {

        TTString    JamomaConfigurationFilePath;
		TTValue		v, out;
        TTErr       err;

        // Init the Modular library

        char		mainBundleStr[4096];
        mainBundleStr[0]='\0';

#if defined(TT_PLATFORM_WIN)
		TTString	fullpath{};
		char		temppath[4096];
		LONG		lRes;

		LPCSTR moduleName = "JamomaFoundation.dll";
		HMODULE	hmodule = GetModuleHandle(moduleName);
		// get the path
		GetModuleFileName(hmodule, (LPSTR)temppath, 4096);

		if (!FAILED(hmodule) && temppath[0])
		{
			fullpath = temppath;

			// get support folder path
			fullpath = fullpath.substr(0, fullpath.length() - (strlen(moduleName) + 1));
			lRes = SHCreateDirectory(NULL, (LPCWSTR)fullpath.c_str());
		}

		strncpy(mainBundleStr, fullpath.c_str(), 4095);
#else
		Dl_info		info;
        // Use the path of JamomaFoundation
        if (dladdr((const void*)jamoma_init, &info))
        {
            char *c = 0;

            strncpy(mainBundleStr, info.dli_fname, 4095);
            c = strrchr(mainBundleStr, '/');
            if (c)
                *c = 0; // chop the filename off of the path
            strcat(mainBundleStr,"/../support");
        }
#endif
		TTLogError("support folder : %s \n", mainBundleStr);
        TTModularInit(mainBundleStr);

        // prepare a symbol for Jamoma
        kTTSym_Jamoma = TTSymbol(JAMOMA);

        // Create an application manager
        JamomaApplicationManager = TTObject("ApplicationManager");

        // Create a local application called "Jamoma" and get it back
        err = JamomaApplicationManager.send("ApplicationInstantiateLocal", kTTSym_Jamoma, out);

        if (err) {
            TTLogError("Error : can't create Jamoma application \n");
            return;
        }
        else
            JamomaApplication = out[0];

        // Edit the path to the JamomaConfiguration.xml file
        long size = TTFoundationBinaryPath.size() - std::string("support").size();
        size = size > 0 ? size : 0;
        JamomaConfigurationFilePath = TTFoundationBinaryPath.substr(0,size);
        JamomaConfigurationFilePath += "misc/JamomaConfiguration.xml";

        // check if the JamomaConfiguration.xml file exists
        //if (locatefile_extended(name, &outvol, &outtype, &filetype, 1))
            //return error("Jamoma not loaded : can't find %s", JamomaConfigurationFilePath.data());

        // JamomaApplication have to read JamomaConfiguration.xml
        TTObject anXmlHandler(kTTSym_XmlHandler);
        anXmlHandler.set(kTTSym_object, JamomaApplication);
        err = anXmlHandler.send(kTTSym_Read, TTSymbol(JamomaConfigurationFilePath));
        if ( err )
             post("error %d : can't load configuration file  \"%s\"", err, JamomaConfigurationFilePath.c_str() );

		// Initialize common symbols
		jamomaSymbolsInit();

		// Initialize common regex
		ttRegexForJmod = new TTRegex("(jmod.)");
		ttRegexForJcom = new TTRegex("(j\\.)");
		ttRegexForModel = new TTRegex("(.model)");
		ttRegexForModule = new TTRegex("(.module)");
		ttRegexForView = new TTRegex("(.view)");
        ttRegexForPdpat = new TTRegex("(.pd)");
        ttRegexForPdhelp = new TTRegex("(-help.pd)");
		ttRegexForBracket = new TTRegex("\\[(\\d|\\d\\d|\\d\\d\\d)\\]");	// parse until 999

		ModelPatcherFormat = new TTString("%s.model.pd");
		ModelPresetFormat = new TTString("%s.model.presets.txt");
		ViewPresetFormat = new TTString("%s.view.presets.txt");
		HelpPatcherFormat = new TTString("%s.model");
		RefpageFormat = new TTString("%s.model");
		DocumentationFormat = new TTString("%s.model.html");

        jamoma_object_initclass();

        post("Jamoma for Pd version %s - %s | jamoma.org",JAMOMA_PD_VERSION, JAMOMA_PD_REV);

		initialized = true;
	}
}

#pragma mark -
#pragma mark Utilities

// Copying all of the elements should be faster than branching and copying one element
//	(that's the theory anyway...)
void jamoma_atom_copy(t_atom *dst, t_atom *src)
{
	//	memcpy(dst, src, sizeof(t_atom));
    memcpy(src, dst, sizeof(t_atom));
}

bool jamoma_atom_compare(t_symbol *type, t_atom *a1, t_atom *a2)
{
	if (!a1 || !a2)
		return 0;

	if (type == jps_decimal) {				// float is first so that it gets process the most quickly
		if (atom_getfloat(a1) == atom_getfloat(a2))
			return 1;
	}
	else if ((type == jps_integer) || (type == jps_boolean)) {
        if (atom_getint(a1) == atom_getint(a2))
			return 1;
	}
    else if (type == jps_string) {
        if (atom_getsymbol(a1) == atom_getsymbol(a2))
			return 1;
	}
	// type array should be checked here as well.  If type == array and this function is called
	// it means we are dealing with a list of length 1, so we only need to compare one atom anyway.
	else if ((type == jps_decimalArray) || (type == jps_array)) {
		if (a1->a_w.w_float == a2->a_w.w_float)
			return 1;
	}
    /*
	else if (type == jps_integerArray) {
		if (a1->a_w.w_long == a2->a_w.w_long)
			return 1;
	}
    */
	else if (type == jps_generic){
		// note that if the two are of different types, then they are obviously not the same
        /*
        if ((a1->a_type == A_LONG) && (a2->a_type == A_LONG)) {
			if (a1->a_w.w_long == a2->a_w.w_long)
				return 1;
		}
        else */ if ((a1->a_type == A_FLOAT) && (a2->a_type == A_FLOAT)) {
			if (a1->a_w.w_float == a2->a_w.w_float)
				return 1;
		}
        else if ((a1->a_type == A_SYMBOL) && (a2->a_type == A_SYMBOL)) {
            if (a1->a_w.w_symbol == a2->a_w.w_symbol)
				return 1;
		}
	}
	else
		error("atom_compare: cannot do comparison on this data type");
	return 0;
}

// Compare Strings: Is s2 after s1 in alphabetical order?
bool jamoma_string_compare(char *s1, char *s2)
{
	short i;
	short len1 = strlen(s1);
	short len2 = strlen(s2);
	bool result = false;
	bool keepgoing = true;

	if (len2 < len1)
		len1 = len2;	// only compare the characters of the short string

	for (i =0 ; i < len1 && keepgoing; i++) {
		if (s1[i] < s2[i]) {
			result = true;
			keepgoing = false;
		}
		else if (s1[i] > s2[i])
			keepgoing = false;
	}
	return result;
}

// Load an external for internal use
// returns true if successful
/*
bool jamoma_loadextern(t_symbol *objectname, long argc, t_atom *argv, t_object **object)
{
	t_class 	*c = NULL;
	t_object	*p = NULL;

	c = class_findbyname(jps_box, objectname);
	if (!c) {
		p = (t_object *)newinstance(objectname, 0, NULL);
		if (p) {
			c = class_findbyname(jps_box, objectname);
			freeobject(p);
			p = NULL;
		}
		else {
			error("jamoma: could not load extern (%s) within the core", objectname->s_name);
			return false;
		}
	}

	if (*object != NULL) {			// if there was an object set previously, free it first...
		object_free(*object);
		*object = NULL;
	}

	*object = (t_object *)object_new_typed(CLASS_BOX, objectname, argc, argv);
	return true;
}
*/

t_symbol *object_attr_getsym(void *x, t_symbol *s){
    int argc=0;
    t_atom *argv=NULL;
    t_symbol*retsym = _sym_nothing;
    eclass_attr_getter((t_object *) x, s, &argc, &argv);
    if ( argv && argv[0].a_type == A_SYMBOL){
        retsym = argv[0].a_w.w_symbol;
    }

    if (argv) {
        free(argv);
        argv=NULL;
    }

    return retsym;
}

void* object_attr_getobj(void *x, t_symbol *attrname){

    if ( attrname == _sym_parentpatcher ){
        return ((t_canvas*)x)->gl_owner;
    }
    else if ( attrname == _sym_firstobject )
    {
        return ((t_canvas*)x)->gl_list;
    }

    return NULL;
}

/*
void attr_args_process(void *x, short ac, t_atom *av)
{
    long index = atoms_get_attributes_offset(ac,av);
    if (index == ac) return; // no argument
    long next = atoms_get_attributes_offset(ac-index,av+index);

    object_attr_setvalueof((t_object*)x,atom_getsym(av+index),next-index,av+index+1);

    //loop to process other arguments
    attr_args_process(x,ac-next,av+next);

}
*/

method object_getmethod(void* x, t_symbol* s)
{
    return (method)zgetfn((t_pd *)x, s);
}

void* object_method_typed(void* x, t_symbol* method, t_symbol* s, long argc, t_atom* argv)
{
    t_ret_method nrmethod = (t_ret_method)zgetfn((t_pd *)x, method);
	return nrmethod(x, s, argc, argv);
}

void* object_method(void *x, t_symbol *s)
{
    t_ret_method nrmethod = (t_ret_method)zgetfn((t_pd *)x, s);
	return nrmethod(x);
}

short locatefile_extended(char *name, short *outvol, t_fourcc *outtype, const t_fourcc *filetypelist, short numtypes)
{
	int fd = -1;
	char dirbuf[MAXPDSTRING], *basename;
	fd = open_via_path("", name, "", dirbuf, &basename, MAXPDSTRING, 0);
	return fd < 0;
}

short path_topathname(const short path, const char *file, char *name)
{
	int fd = -1;
	char *basename;
	fd =	open_via_path(NULL, file, "", name, &basename, MAXPDSTRING, 0);
	strcat(name,basename);
	return fd < 0;
}
