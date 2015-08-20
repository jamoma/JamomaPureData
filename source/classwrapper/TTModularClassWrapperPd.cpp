/*
 *	TTModularClassWrapperMax
 *	An automated class wrapper to make Jamoma object's available as objects for Puredata
 *	Copyright © 2008 by Timothy Place
 *      Pd port by Antoine Villeret 2015
 *
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TTModularClassWrapperPd.h"
#include "commonsyms.h"
// #include "ext_hashtab.h"

/** A hash of all wrapped clases, keyed on the Pd class name. */
// static t_hashtab*	wrappedPdClasses = NULL;
static std::map<std::string,t_object*> wrappedPdClasses;

t_object *wrappedModularClass_new(t_symbol *name, long argc, t_atom *argv)
{
    WrappedClass*				wrappedPdClass = NULL;
    WrappedModularInstancePtr	x = NULL;
	TTErr						err = kTTErrNone;
	
	// Find the WrappedClass
    // hashtab_lookup(wrappedPdClasses, name, (t_object**)&wrappedPdClass);
    wrappedPdClass = (WrappedClass*)wrappedPdClasses[name->s_name];
	
	// If the WrappedClass has a validity check defined, then call the validity check function.
	// If it returns an error, then we won't instantiate the object.
    if (wrappedPdClass) {
        if (wrappedPdClass->validityCheck)
            err = wrappedPdClass->validityCheck(wrappedPdClass->validityCheckArgument);
		else
			err = kTTErrNone;
	}
	else
		err = kTTErrGeneric;
	
	if (!err)
        x = (WrappedModularInstancePtr)eobj_new(wrappedPdClass->pdClass);
	
    if (x) {
		
        x->wrappedClassDefinition = wrappedPdClass;
		
		x->useInternals = NO;
        x->internals = new TTHash();
		x->address = kTTAdrsEmpty;
		x->argv = NULL;
		x->iterateInternals = NO;
		
#ifdef ARRAY_EXTERNAL
		x->arrayFormatInteger = TTString();
		x->arrayFormatString = TTString();
#endif
        
        x->patcherPtr = NULL;
        x->patcherContext = kTTSymEmpty;
        x->patcherClass = kTTSymEmpty;
        x->patcherName = kTTSymEmpty;
        x->patcherAddress = kTTAdrsEmpty;

        x->dumpOut = NULL;

        // create the first inlet
        eobj_proxynew(x);
		
		// Make specific things
        ModularSpec *spec = (ModularSpec*)wrappedPdClass->specificities;
		if (spec) {
			if (spec->_new)
				spec->_new((TTPtr)x, argc, argv);
        } else {
            // handle attribute args
            attr_args_process(x, argc, argv);
        }

        if ( !x->dumpOut ) x->dumpOut = outlet_new((t_object*)x,NULL);

        // call an object loadbang method if it exists
        method _method = (method)zgetfn((t_pd*)x,_sym_loadbang);
        if (_method)
            _method(x);
	}

	return (t_object*)x;
}


void wrappedModularClass_unregister(WrappedModularInstancePtr x)
{
	TTValue		keys, v, none;
	TTSymbol	name;
	TTAddress	objectAddress;
	TTErr		err;
    
#ifndef ARRAY_EXTERNAL

	x->subscriberObject = TTObject();
    
    // check the wrappedObject is still valid because it could have been released in spec->_free method
	if (x->wrappedObject.valid()) {
        
        // don't release the local application
        if (!(x->wrappedObject.instance() == accessApplicationLocal)) {
            
            if (x->wrappedObject.instance()->getReferenceCount() > 1)
                pd_error((t_object*)x, "there are still unreleased reference of the wrappedObject (refcount = %d)", x->wrappedObject.instance()->getReferenceCount() - 1);
            
            // this line should release the last instance of the wrapped object
            // otherwise there is something wrong
            x->wrappedObject = TTObject();
        }
    }

#endif
	
    if (!x->internals->isEmpty()) {
        
        err = x->internals->getKeys(keys);
        
        if (!err) {
            
            x->iterateInternals = YES;
            
            for (int i = 0; i < keys.size(); i++) {
                
                name = keys[i];
                err = x->internals->lookup(name, v);
                
                if (!err) {
                    
                    TTObject o = v[0];
                    
                    if (o.name() == kTTSym_Sender || o.name() == kTTSym_Receiver || o.name() == kTTSym_Viewer)
                        o.set(kTTSym_address, kTTAdrsEmpty);
                    
                    // absolute registration case : remove the address
                    if (v.size() == 2) {
                        objectAddress = v[1];
                        
                        JamomaDebug logpost((t_object*)x, 0, "Remove internal %s object at : %s", name.c_str(), objectAddress.c_str());
                        JamomaApplication.send("ObjectUnregister", objectAddress, none);
                    }
                }
            }
            x->iterateInternals = NO;
        }
        x->internals->clear();
    }
}


void wrappedModularClass_free(WrappedModularInstancePtr x)
{
	ModularSpec* spec = (ModularSpec*)x->wrappedClassDefinition->specificities;
    
    // call specific free method before freeing internal stuff
	if (spec->_free)
		spec->_free(x);
	
	wrappedModularClass_unregister(x);
	
	if (x->argv)
        free(x->argv);
	
	x->argv = NULL;

    eobj_free(x);
    
    // delete x->internals;
    // x->internals = NULL;
}


t_max_err wrappedModularClass_notify(TTPtr self, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	ModularSpec*				spec = (ModularSpec*)x->wrappedClassDefinition->specificities;
	TTValue						v, none;
	TTAddress                   contextAddress;
    
#ifndef ARRAY_EXTERNAL
	t_object	*				context;
    
	if (x->subscriberObject.valid()) {
        
		x->subscriberObject.get("context", v);
		context = (t_object*)((TTPtr)v[0]);
		
		// if the patcher is deleted
		if (sender == context) {
			if (msg == _sym_free) {
				
				// delete the context node if it exists
				x->subscriberObject.get("contextAddress", v);
				contextAddress = v[0];
				
				JamomaApplication.send("ObjectUnregister", contextAddress, none);
				
				// delete
				x->subscriberObject = TTObject();
				
				// no more notification
                // TODO rewrite this for PD
                // object_detach_byptr((t_object*)x, context);
			}
		}
	}
#endif
	
	if (spec->_notify)
		spec->_notify(self, s, msg, sender, data);
	
    return 0; // MAX_ERR_NONE;
}


void wrappedModularClass_shareContextNode(TTPtr self, TTNodePtr *contextNode)
{
	TTValue	v;
#ifndef ARRAY_EXTERNAL
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    
	if (x->subscriberObject.valid()) {
		x->subscriberObject.get("contextNode", v);
		*contextNode = TTNodePtr((TTPtr)v[0]);
	}
	else
#endif
		*contextNode = NULL;
}


t_max_err wrappedModularClass_attrGet(TTPtr self, t_object *attr, long* argc, t_atom** argv)
{
	// t_symbol	*attrName = (t_symbol*)object_method(attr, _sym_getname);
	t_symbol	*attrName = ((t_eattr*)attr)->name;
	TTValue		v;
	WrappedModularInstancePtr x = (WrappedModularInstancePtr)self;
	TTPtr		ptr;
	
    // err = hashtab_lookup(x->wrappedClassDefinition->pdNamesToTTNames, attrName, (t_object**)&ptr);
    ptr = x->wrappedClassDefinition->pdNamesToTTNames[attrName->s_name];
    if (ptr == NULL)
        return -2;
	
	TTSymbol	ttAttrName(ptr);
	
	if (selectedObject) {
		selectedObject->getAttributeValue(ttAttrName, v);
		jamoma_ttvalue_to_Atom(v, argc, argv);
	}
	
    return 0; // MAX_ERR_NONE;
}


t_max_err wrappedModularClass_attrSet(TTPtr self, t_object *attr, long argc, const t_atom *argv)
{
	WrappedModularInstancePtr x = (WrappedModularInstancePtr)self;
	// t_symbol	*attrName = (t_symbol*)object_method(attr, _sym_getname);
	t_symbol	*attrName = ((t_eattr*)attr)->name;
	TTValue		v;
	long        ac = 0;
	t_atom		*av = NULL;
	TTErr		err;
	TTPtr		ptr;
	
	// for an array of wrapped object
	if (x->useInternals && !x->iterateInternals) {
		
		TTValue		keys;
		
		// temporary set x->iterateInternals to YES
		x->iterateInternals = YES;
		
		// then recall this method for each element of the array
		if (!x->internals->isEmpty()) {
			err = x->internals->getKeys(keys);
			if (!err) {
				for (TTUInt32 i = 0; i < keys.size(); i++) {
					x->cursor = keys[i];
					wrappedModularClass_attrSet(self, attr, argc, argv);
				}
			}
		}
		
		// reset x->iterateInternals to NO
		x->iterateInternals = NO;
		
        return 0; // MAX_ERR_NONE;
	}
	
    // m_err = hashtab_lookup(x->wrappedClassDefinition->pdNamesToTTNames, attrName, (t_object**)&ptr);
    ptr = x->wrappedClassDefinition->pdNamesToTTNames[attrName->s_name];
    if (ptr == NULL)
        return -2; // INVALID POINTER
	
	TTSymbol	ttAttrName(ptr);
	
	// set attribute's value
	if (argc && argv) {
		
		jamoma_ttvalue_from_Atom(v, _sym_nothing, argc, argv);
		
		if (selectedObject) {
            TTErr err = selectedObject->setAttributeValue(ttAttrName, v);
            return err;  // MAX_ERR_NONE;
		}
		else
            return -1; // MAX_ERR_GENERIC;
        
	}
	// or get it and dumpout his value
	else {
		
		if (selectedObject) {
			// don't consider array case here (they should have all the same attribute value)
			selectedObject->getAttributeValue(ttAttrName, v);
			
			jamoma_ttvalue_to_Atom(v, &ac, &av);
			// object_obex_dumpout(self, attrName, ac, av);
			outlet_anything(x->dumpOut, attrName, ac, av);
            free(av);
            return 0; // MAX_ERR_NONE;
		}
		else
            return -1; // MAX_ERR_GENERIC;
	}
	
    return -1; // MAX_ERR_GENERIC;
}


void wrappedModularClass_anything(TTPtr self, t_symbol *s, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	ModularSpec*				spec = (ModularSpec*)x->wrappedClassDefinition->specificities;
	TTErr						err;
	
	// for an array of wrapped object
	if (x->useInternals && !x->iterateInternals) {
		
		// temporary set x->iterateInternals to YES
		x->iterateInternals = YES;
		
		// then recall this method for each element of the array
		if (!x->internals->isEmpty()) {
			
			TTUInt32	i = 0;
			TTValue		keys;
			TTSymbol	memoCursor;
			
			err = x->internals->getKeys(keys);
			if (!err) {
				
				memoCursor = x->cursor;
				while (i < keys.size() && !err) {
					
					x->cursor = keys[i];
                    
                    // Is it a message of the wrapped object ?
                    err = wrappedModularClass_sendMessage(self, s, argc, argv);
                    
                    // Is it an attribute of the wrapped object ?
                    if (err)
                        err = wrappedModularClass_setAttribute(self, s, argc, argv);
                    
                    // if error : stop the while because this is an array and all objects are the same
                    if (err)
                        break;
					
					i++;
				}
				x->cursor = memoCursor;
			}
			
			// don't iterate the specific anything method on each object of the array
			if (err && spec->_any)
				spec->_any(self, s, argc, argv);
		}
		
		// reset x->iterateInternals to NO
		x->iterateInternals = NO;
	}
	
	// for single wrapped object
	else {
		
		// Is it a message of the wrapped object ?
		if (!wrappedModularClass_sendMessage(self, s, argc, argv))
			return;
		
        // It could be an extended attribute (not registered in pdNamesToTTNames)
		// Is it an attribute of the wrapped object ?
		if (!wrappedModularClass_setAttribute(self, s, argc, argv))
			return;
		
		if (spec->_any)
			spec->_any(self, s, argc, argv);
	}
}


TTErr wrappedModularClass_sendMessage(TTPtr self, t_symbol *s, long argc, const t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue			inputValue, outputValue;
	TTSymbol		ttName;
	TTMessagePtr	aMessage = NULL;
	long            ac = 0;
	t_atom			*av = NULL;
	TTErr			err;
    TTPtr           ptr;
	
    // m_err = hashtab_lookup(x->wrappedClassDefinition->pdNamesToTTNames, s, (t_object**)&ptr);
    ptr = x->wrappedClassDefinition->pdNamesToTTNames[s->s_name];
    if (ptr != NULL ) {
		
		// Is it a message of the wrapped object ?
        ttName = TTSymbol(ptr);
		err = selectedObject->findMessage(ttName, &aMessage);
		if (!err) {
			// send message
			if (argc && argv) {
				
				jamoma_ttvalue_from_Atom(inputValue, _sym_nothing, argc, argv);
				selectedObject->sendMessage(ttName, inputValue, outputValue);
				
				jamoma_ttvalue_to_Atom(outputValue, &ac, &av);
//				object_obex_dumpout(self, s, ac, av);
				outlet_anything(x->dumpOut, s, ac, av);
				free(av);
			}
			else
				selectedObject->sendMessage(ttName);
		}
		
		return err;
	}
	else
		return kTTErrGeneric;
}


TTErr wrappedModularClass_setAttribute(TTPtr self, t_symbol *s, long argc, const t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue			inputValue, outputValue;
	TTSymbol		ttName;
	TTAttributePtr	anAttribute= NULL;
	long            ac = 0;
	t_atom			*av = NULL;
	TTErr			err;
	
	err = selectedObject->findAttribute(TTSymbol(s->s_name), &anAttribute);
	if (!err) {
		
		// set attribute's value
		if (argc && argv) {
			jamoma_ttvalue_from_Atom(inputValue, _sym_nothing, argc, argv);
			selectedObject->setAttributeValue(TTSymbol(s->s_name), inputValue);
		}
		// or get it and dumpout his value
		else {
			selectedObject->getAttributeValue(TTSymbol(s->s_name), outputValue);
			
			jamoma_ttvalue_to_Atom(outputValue, &ac, &av);
//			object_obex_dumpout(self, s, ac, av);
			outlet_anything(x->dumpOut, s, ac, av);
			free(av);
        }
	}
	
	return err;
}


void wrappedModularClass_dump(TTPtr self)
{
    WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    TTValue			names, v;
    TTUInt32		i;
    TTSymbol		aName, address;
    t_symbol		*s;
    long            ac;
    t_atom			*av;
	
#ifndef ARRAY_EXTERNAL
    t_atom			a;
	
    if (x->subscriberObject.valid())
    {
    	// send out the absolute address of the subscriber
        x->subscriberObject.get("nodeAddress", v);
        address = v[0];
        atom_setsym(&a, gensym((char *) address.c_str()));
//        object_obex_dumpout(self, gensym("address"), 1, &a);
		outlet_anything(x->dumpOut, gensym("address"), 1, &a);
    }
#endif
    
    selectedObject->getAttributeNames(names);
	
    for (i = 0; i < names.size(); i++) {
		
        aName = names[i];
		
		selectedObject->getAttributeValue(aName, v);
		
        s = jamoma_TTName_To_PdName(aName);
		
		ac = 0;
		av = NULL;
		jamoma_ttvalue_to_Atom(v, &ac, &av);
//		object_obex_dumpout(self, s, ac, av);
		outlet_anything(x->dumpOut, s, ac, av);
		free(av);
    }
}

TTErr wrapTTModularClassAsPdClass(TTSymbol& ttblueClassName, const char* pdClassName, WrappedClassPtr* c, ModularSpec* specificities)
{
	TTObject        o;
	TTValue			v, args;
    WrappedClass*	wrappedPdClass = NULL;
	TTSymbol		TTName;
    t_symbol		*PdName = NULL;
    TTUInt16        i;
    t_symbol* s_pdClassName = gensym(pdClassName);
	
	// AV : why do we need to do initialize this each time a new class is instanciated ?
    // jamoma_init();
    // common_symbols_init();
	
    wrappedPdClass = new WrappedClass;
    wrappedPdClass->pdClassName = gensym(pdClassName);
    wrappedPdClass->pdClass = eclass_new( s_pdClassName->s_name,
										  (method)wrappedModularClass_new,
										  (method)wrappedModularClass_free,
										  sizeof(WrappedModularInstance),
                                          CLASS_NOINLET,
                                          A_GIMME,
										  0);
    wrappedPdClass->ttblueClassName = ttblueClassName;
    wrappedPdClass->validityCheck = NULL;
    wrappedPdClass->validityCheckArgument = NULL;
    wrappedPdClass->options = NULL;
    // wrappedPdClass->pdNamesToTTNames = hashtab_new(0);

    wrappedPdClass->specificities = specificities;

    if ( ! wrappedPdClass->pdClass) {
        post("can't instantiate object !!");
        return kTTErrGeneric;
    }
    
#ifdef AUDIO_EXTERNAL
    // Setup our class to work with MSP
    class_dspinit(wrappedPdClass->pdClass);
#endif
	
	// Create a temporary instance of the class so that we can query it.
    try {
      o = TTObject(ttblueClassName);
    } catch (TTException e){
        error("can't create Jamoma %s object instance. The reason invoked is : %s",ttblueClassName.c_str(),e.getReason());
        return (TTErr) -1; // generic error
    }

	
    // Register Messages as Pd method
	o.messages(v);
	for (i = 0; i < v.size(); i++) {
		TTName = v[i];
        
            if (TTName == TTSymbol("test")                      ||
                TTName == TTSymbol("getProcessingBenchmark")    ||
                TTName == TTSymbol("resetBenchmarking"))
                continue;
            else if ((PdName = jamoma_TTName_To_PdName(TTName))) {
                // hashtab_store(wrappedPdClass->pdNamesToTTNames, PdName, (t_object*)(TTName.rawpointer()));
                wrappedPdClass->pdNamesToTTNames[PdName->s_name]=(t_object*)(TTName.rawpointer());
                eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_anything, PdName->s_name, A_GIMME, 0);
            }
	}
	
    // Register Attributes as Pd attr
	o.attributes(v);
	for (i = 0; i < v.size(); i++) {
		TTAttributePtr	attr = NULL;
		
		TTName = v[i];
        
#ifdef AUDIO_EXTERNAL
        // the enable word is already used by a message declared in the dsp_init method
        if (TTName == TTSymbol("enable"))
            continue;
#endif
        
        // we want to hide service attribute for Pd external
        if (TTName == TTSymbol("service"))
            continue;
		
        if ((PdName = jamoma_TTName_To_PdName(TTName))) {
            
            if (TTName == kTTSym_bypass && wrappedPdClass->pdClassName != gensym("j.in") && wrappedPdClass->pdClassName != gensym("j.in~"))
                continue;
            
			o.instance()->findAttribute(TTName, &attr);
            wrappedPdClass->pdNamesToTTNames[PdName->s_name] = (t_object*)(TTName.rawpointer());
            // hashtab_store(wrappedPdClass->pdNamesToTTNames, PdName, (t_object*)(TTName.rawpointer()));

            if (attr->type == kTypeFloat32 )
                CLASS_ATTR_FLOAT(wrappedPdClass->pdClass,PdName->s_name,0,t_eclass,c_attr);
            else if (attr->type == kTypeFloat64)
                CLASS_ATTR_DOUBLE(wrappedPdClass->pdClass,PdName->s_name,0,t_eclass,c_attr);
            else if (attr->type == kTypeSymbol || attr->type == kTypeString)
                CLASS_ATTR_SYMBOL(wrappedPdClass->pdClass,PdName->s_name,0,t_eclass,c_attr);
            else if (attr->type == kTypeLocalValue)
                CLASS_ATTR_ATOM(wrappedPdClass->pdClass,PdName->s_name,0,t_eclass,c_attr);

            // eclass_new_attr_typed(wrappedPdClass->pdClass,PdName->s_name,pdType->s_name,1,0,0,0);
			CLASS_ATTR_ACCESSORS(wrappedPdClass->pdClass,PdName->s_name,wrappedModularClass_attrGet,wrappedModularClass_attrSet);

			// class_addattr(wrappedPdClass->pdClass, attr_offset_new(PdName->s_name, pdType, 0, (method)wrappedModularClass_attrGet, (method)wrappedModularClass_attrSet, 0));
			
			// Add display styles for the Max 5 inspector
			if (attr->type == kTypeBoolean)
                CLASS_ATTR_STYLE(wrappedPdClass->pdClass, (char*)TTName.c_str(), 0, "onoff");
			if (TTName == TTSymbol("fontFace"))
                CLASS_ATTR_STYLE(wrappedPdClass->pdClass,	"fontFace", 0, "font");
		}
	}
	
    // eclass_addmethod(wrappedPdClass->pdClass, (method)stdinletinfo,							"inletinfo",			A_CANT, 0);
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_notify,			"notify",				A_CANT, 0);
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_shareContextNode,	"share_context_node",	A_CANT,	0);
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_anything,			"anything",				A_GIMME, 0);
	
	// Register specific methods and do specific things
	if (specificities) {
		if (specificities->_wrap)
            specificities->_wrap(wrappedPdClass);
	}
	
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_dump,				"dump",					A_GIMME, 0);
	
#ifdef ARRAY_EXTERNAL
	
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_ArraySelect,				"array/select",			A_GIMME,0);
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedModularClass_ArrayResize,				"array/resize",			A_LONG,0);
	
    CLASS_ATTR_SYMBOL(wrappedPdClass->pdClass,			"format",	0,		WrappedModularInstance,	arrayAttrFormat);
    CLASS_ATTR_ACCESSORS(wrappedPdClass->pdClass,		"format",			wrappedModularClass_FormatGet,	wrappedModularClass_FormatSet);
    // CLASS_ATTR_ENUM(wrappedPdClass->pdClass,			"format",	0,		"single array");
#endif
	
    eclass_register(_sym_box, wrappedPdClass->pdClass);
	if (c)
        *c = wrappedPdClass;
	
    // hashtab_store(wrappedPdClasses, wrappedPdClass->pdClassName, (t_object*)(wrappedPdClass));
    wrappedPdClasses[wrappedPdClass->pdClassName->s_name] = (t_object*)(wrappedPdClass);
	return kTTErrNone;
}


TTErr makeInternals_data(TTPtr self, TTAddress address, TTSymbol name, t_symbol *callbackMethod, TTPtr context, TTSymbol service, TTObject& returnedData, TTBoolean deferlow)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue			baton, v, out;
	TTAddress       dataAddress;
    TTSymbol        dataName;
    
	returnedData = TTObject(kTTSym_Data, service);
    
    baton = TTValue(TTPtr(x), TTPtr(callbackMethod), deferlow);
	
    returnedData.set(kTTSym_baton, baton);
	returnedData.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	
	// absolute registration
	dataAddress = address.appendAddress(TTAddress(name));
    v = TTValue(dataAddress, returnedData, context);
	JamomaApplication.send("ObjectRegister", v, out);
	
	dataAddress = out[0];
	dataName = dataAddress.getNameInstance();
    
	// absolute registration case : set the address in second position (see in unregister method)
	v = TTValue(returnedData, dataAddress);
	x->internals->append(dataName, v);
	
    JamomaDebug logpost((t_object*)x, 0, "makes internal \"%s\" %s at : %s", dataName.c_str(), service.c_str(), dataAddress.c_str());
	
	return kTTErrNone;
}


TTErr makeInternals_explorer(TTPtr self, TTSymbol name, t_symbol *callbackMethod, TTObject& returnedExplorer, TTBoolean deferlow)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue		v, args, baton;
	TTObject    returnValueCallback;
    
    // check the internals do not exist yet
    if (!x->internals->lookup(name, v)) {
        returnedExplorer = v[0];
        JamomaDebug logpost((t_object*)x, 0, "makeInternals_explorer : \"%s\" internal already exists", name.c_str());
        return kTTErrNone;
    }
	
	// prepare arguments
	returnValueCallback = TTObject("callback");
    
	baton = TTValue(TTPtr(x), TTPtr(callbackMethod), deferlow);
    
	returnValueCallback.set(kTTSym_baton, baton);
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnValueCallback);
	
	args.append((TTPtr)jamoma_explorer_default_filter_bank());
	
	returnedExplorer = TTObject(kTTSym_Explorer, args);
	
	// default registration case : store object only (see in unregister method)
	x->internals->append(name, returnedExplorer);
    
    JamomaDebug logpost((t_object*)x, 0, "makes internal \"%s\" explorer", name.c_str());
    
	return kTTErrNone;
}


TTErr makeInternals_viewer(TTPtr self, TTAddress address, TTSymbol name, t_symbol *callbackMethod, TTObject& returnedViewer, TTBoolean deferlow)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue			v, baton;
	TTAddress       adrs;
    
    // check the internals do not exist yet
    if (!x->internals->lookup(name, v)) {
        returnedViewer = v[0];
        JamomaDebug logpost((t_object*)x, 0, "makeInternals_viewer : \"%s\" internal already exists", name.c_str());
        return kTTErrNone;
    }
	
	returnedViewer = TTObject(kTTSym_Viewer);
    
    baton = TTValue(TTPtr(x), TTPtr(callbackMethod), deferlow);
    returnedViewer.set(kTTSym_baton, baton);
	returnedViewer.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	
	// edit address
	adrs = address.appendAddress(TTAddress(name));
	
	// default registration case : store object only (see in unregister method)
	x->internals->append(name, returnedViewer);
    
    // set address attribute (after registration as the value can be updated in the same time)
	returnedViewer.set(kTTSym_address, adrs);
    
    JamomaDebug logpost((t_object*)x, 0, "makes internal \"%s\" viewer to bind on : %s", name.c_str(), adrs.c_str());
    
	return kTTErrNone;
}


TTErr makeInternals_receiver(TTPtr self, TTAddress address, TTSymbol name, t_symbol *callbackMethod, TTObject& returnedReceiver, TTBoolean deferlow, TTBoolean appendNameAsAttribute)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue			v, args, baton;
	TTObject        returnValueCallback, empty;
	TTAddress       adrs;
    
    // check the internals do not exist yet
    if (!x->internals->lookup(name, v)) {
        returnedReceiver = v[0];
        JamomaDebug logpost((t_object*)x, 0, "makeInternals_receiver : \"%s\" internal already exists", name.c_str());
        returnedReceiver.send("Get");
        return kTTErrNone;
    }
	
	// prepare arguments
	
	// we don't want the address back
	args.append(empty);
	
	returnValueCallback = TTObject("callback");
    
	baton = TTValue(TTPtr(x), TTPtr(callbackMethod), deferlow);
    
	returnValueCallback.set(kTTSym_baton, baton);
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnValueCallback);
	
	returnedReceiver = TTObject(kTTSym_Receiver, args);
	
	// edit address
	if (appendNameAsAttribute)
        adrs = address.appendAttribute(name);
    else
        adrs = address.appendAddress(TTAddress(name.c_str()));
	
	// default registration case : store object only (see in unregister method)
	x->internals->append(name, returnedReceiver);
    
    // set address attribute (after registration as the value can be updated in the same time)
    returnedReceiver.set(kTTSym_address, adrs);
    
    JamomaDebug logpost((t_object*)x, 0, "makes internal \"%s\" receiver to bind on : %s", name.c_str(), adrs.c_str());
    
	return kTTErrNone;
}

TTErr makeInternals_sender(TTPtr self, TTAddress address, TTSymbol name, TTObject& returnedSender, TTBoolean appendNameAsAttribute)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    TTValue     v;
	TTAddress   adrs;
    
    // check the internals do not exist yet
    if (!x->internals->lookup(name, v)) {
        returnedSender = v[0];
        JamomaDebug logpost((t_object*)x, 0, "makeInternals_sender : \"%s\" internal already exists", name.c_str());
        return kTTErrNone;
    }
	
	returnedSender = TTObject(kTTSym_Sender);
	
	// edit address
	if (appendNameAsAttribute)
        adrs = address.appendAttribute(name);
    else
        adrs = address.appendAddress(TTAddress(name.c_str()));
	
	// default registration case : store object only (see in unregister method)
	x->internals->append(name, returnedSender);
    
    // set address attribute
	returnedSender.set(kTTSym_address, adrs);
    
    JamomaDebug logpost((t_object*)x, 0, "makes internal \"%s\" sender to bind on : %s", name.c_str(), adrs.c_str());
    
	return kTTErrNone;
}

TTErr removeInternals_data(TTPtr self, TTAddress address, TTAddress name)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue		v, none;
	TTAddress   dataAddress;
	TTErr		err;
	
	err = x->internals->lookup(name, v);
	
	if (!err) {
		
		dataAddress = v[1];
		
        JamomaDebug logpost((t_object*)x, 0, "Remove internal %s object at : %s", name.c_str(), dataAddress.c_str());
		JamomaApplication.send("ObjectUnregister", dataAddress, none);

		x->internals->remove(name);
	}
	
	return kTTErrNone;
}


TTObjectBasePtr	getSelectedObject(WrappedModularInstancePtr x)
{
	if (x->useInternals) {
		TTValue     v;
		TTObject    o;
		TTErr       err;
        
		err = x->internals->lookup(x->cursor, v);
		if (!err)
			o = v[0];
		
		return o.instance();
	}
#ifndef ARRAY_EXTERNAL
	else
		return x->wrappedObject.instance();
#else
	else
		return NULL;
#endif
}


void copy_msg_argc_argv(TTPtr self, t_symbol *msg, long argc, const t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTBoolean	copyMsg = false;
    TTUInt32	i;
	
	if (msg != _sym_nothing && msg != _sym_int && msg != _sym_float && msg != _sym_symbol && msg != _sym_list)
		copyMsg = true;
	
	x->msg = msg;
    
    // prepare memory if needed
    if (x->argv == NULL) {
        x->argc = argc + copyMsg;
        x->argv = (t_atom*)sysmem_newptr(sizeof(t_atom) * x->argc);
    }
    // or resize memory if needed
    else if (x->argc != argc + copyMsg) {
        x->argc = argc + copyMsg;
        free(x->argv);
        x->argv = (t_atom*)sysmem_newptr(sizeof(t_atom) * x->argc);
    }
    
    // copy
	if (x->argc && x->argv) {
        
		if (copyMsg) {
			atom_setsym(&x->argv[0], msg);
			for (i = 1; i < x->argc; i++)
				x->argv[i] = argv[i-1];
		}
		else
			for (i = 0; i < x->argc; i++)
				x->argv[i] = argv[i];
	}
}


#ifdef ARRAY_EXTERNAL
t_max_err wrappedModularClass_FormatGet(TTPtr self, TTPtr attr, long *ac, t_atom **av)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	
	if ((*ac)&&(*av)) {
		//memory passed in, use it
	} else {
		//otherwise allocate memory
		*ac = 1;
		if (!(*av = (t_atom*)getbytes(sizeof(t_atom)*(*ac)))) {
			*ac = 0;
            return -4; // MAX_ERR_OUT_OF_MEM;
		}
	}
	
	atom_setsym(*av, x->arrayAttrFormat);
	
    return 0; // MAX_ERR_NONE;
}


t_max_err wrappedModularClass_FormatSet(TTPtr self, TTPtr attr, long ac, t_atom *av)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	
	if (ac&&av) {
		x->arrayAttrFormat = atom_getsym(av);
	} else {
		// no args, set to single
		x->arrayAttrFormat = gensym("single");
	}
    return 0; // MAX_ERR_NONE;
}


void wrappedModularClass_ArraySelect(TTPtr self, t_symbol *msg, long ac, t_atom *av)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	t_symbol					*instanceAddress;
	TTUInt8						i;
	TTValue						v;
	
	if (!x->internals->isEmpty()) {
		
		if (ac && av) {
			if (atom_gettype(av) == A_LONG) {
				i = atom_getlong(av);
				if (i > 0 && i <= x->arraySize) {
					x->arrayIndex = i;
					jamoma_edit_numeric_instance(x->arrayFormatInteger, &instanceAddress, i);
					x->cursor = TTSymbol(instanceAddress->s_name);
				}
				else
                    pd_error((t_object*)x, "array/select : %d is not a valid index", i);
			}
			else if (atom_gettype(av) == A_SYM) {
				
				if (atom_getsym(av) == gensym("*")) {
					x->arrayIndex = 0;
					jamoma_edit_numeric_instance(x->arrayFormatInteger, &instanceAddress, 1);
					x->cursor = TTSymbol(instanceAddress->s_name);
				}
				else
					pd_error((t_object*)x, "array/select : %s is not a valid index", atom_getsym(av)->s_name);
			}
		}
		else {
			if (msg == gensym("*")) {
				x->arrayIndex = 0;
				jamoma_edit_numeric_instance(x->arrayFormatInteger, &instanceAddress, 1);
				x->cursor = TTSymbol(instanceAddress->s_name);
			}
			else
				pd_error((t_object*)x, "array/select : %s is not a valid index", msg->s_name);
		}
	}
	else
		pd_error((t_object*)x, "array/select : the array is empty");
}


void wrappedModularClass_ArrayResize(TTPtr self, long newSize)
{
    WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    t_symbol	*instanceAddress;
    TTString    s_bracket;
    TTValue     v;
    
    if (newSize >= 0) {
        
        v = TTInt64(newSize);
        v.toString();
        
        s_bracket = "[";
        s_bracket += TTString(v[0]);
        s_bracket += "]";
        
        jamoma_edit_string_instance(x->arrayFormatString, &instanceAddress, s_bracket.c_str());
        
        getfn((t_pd*)x,gensym("address"))((t_object*)x,instanceAddress,0,NULL);
//        object_method((t_object*)x, gensym("address"), instanceAddress, 0, NULL);
        JamomaDebug logpost((t_object*)x, 3,"array/resize : to %s address", instanceAddress->s_name);
    }
    else
        pd_error((t_object*)x, "array/resize : %ld is not a valid size", newSize);
}
#endif

void object_obex_dumpout(TTPtr self, t_symbol* msg, int argc, t_atom* argv)
{
    outlet_anything(((WrappedModularInstancePtr)self)->dumpOut,msg,argc,argv);
}
