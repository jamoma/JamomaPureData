/* 
 *	TTClassWrapperPd
 *	An automated class wrapper to make Jamoma objects available as objects for PureData
 *	Copyright � 2008 by Timothy Place
 *      Pd port by Antoine Villeret - 2015
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperPd.h"
#include "TTCallback.h"
#ifdef MAC_VERSION
#include <dlfcn.h>
#endif

extern "C" void wrappedClass_receiveNotificationForOutlet(WrappedInstancePtr self, TTValue& arg);


/** A hash of all wrapped clases, keyed on the Pd class name. */
//static t_hashtab*	wrappedPdClasses = NULL;
static std::map<std::string,t_object*> wrappedPdClasses;


t_eobj* wrappedClass_new(t_symbol* name, long argc, t_atom* argv)
{	
	WrappedClass*		wrappedPdClass = NULL;
    WrappedInstancePtr	x = NULL;
	TTValue				sr(sys_getsr());
	TTValue				v, none;
 	long				attrstart = attr_args_offset(argc, argv);		// support normal arguments
	TTErr				err = kTTErrNone;
	
	// Find the WrappedClass
    //hashtab_lookup(wrappedPdClasses, name, (t_object**)&wrappedPdClass);
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
        x = (WrappedInstancePtr)eobj_new(wrappedPdClass->pdClass);
    if (x) {
		x->wrappedClassDefinition = wrappedPdClass;
		x->maxNumChannels = 2;		// An initial argument to this object will set the maximum number of channels
		if (attrstart && argv)
			x->maxNumChannels = atom_getlong(argv);
		
		ttEnvironment->setAttributeValue(kTTSym_sampleRate, sr);
		
		if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("numChannelsUseFixedRatioInputsToOutputs"), v)) {
		   TTUInt16	inputs;
		   TTUInt16	outputs;
		   
		   inputs = v[0];
		   outputs = v[1];
		   x->numInputs = x->maxNumChannels * inputs;
		   x->numOutputs = x->maxNumChannels * outputs;
		}
		else if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("fixedNumChannels"), v)) {
			TTUInt16 numChannels;
			
			numChannels = v[0];
			x->numInputs = numChannels;
			x->numOutputs = numChannels;
		}
		else if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("fixedNumOutputChannels"), v)) {
			TTUInt16 numChannels;
			
			numChannels = v[0];
			x->numInputs = x->maxNumChannels;
			x->numOutputs = numChannels;
		}
		else {
		   x->numInputs = x->maxNumChannels;
		   x->numOutputs = x->maxNumChannels;
		}
		
		if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("additionalSignalInputSetsAttribute"), v)) {
			x->numControlSignals = v.size();
			x->controlSignalNames = new TTSymbol[x->numControlSignals];
			for (TTUInt16 i=0; i<x->numControlSignals; i++) {
				x->numInputs++;
				x->controlSignalNames[i] = v[i];
			}
		}
		
		x->wrappedObject = new TTAudioObject(wrappedPdClass->ttblueClassName, x->maxNumChannels);
		x->audioIn = new TTAudio(x->numInputs);
		x->audioOut = new TTAudio(x->numOutputs);
		attr_args_process(x,argc,argv);				// handle attribute args

		/*
		x->dumpOut = outlet_new((t_object*)x,NULL);

        for (short i=1; i < x->numInputs; i++)
            inlet_new(&x->obj.o_obj, &x->obj.o_obj.ob_pd, &s_signal, &s_signal);
		*/

        // eobj_dspsetup(x,x->numInputs,0);
        // dsp_setup(x, x->numInputs);			// inlets
				
		//if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("numControlOutlets"), v))
		//	v.get(0, numControlOutlets);
		if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("controlOutletFromNotification"), v)) {
            TTUInt16    outletIndex = 0;
            TTSymbol	notificationName;
            
 			outletIndex = v[0];
 			notificationName = v[1];
            
            // TODO: to support more than one notification->outlet we need see how many args are actually passed-in
            // and then we need to track them in a hashtab or something...
            
            x->controlOutlet = outlet_new((t_object*)x, NULL);
            
            x->controlCallback = new TTObject("callback");
            x->controlCallback->set("function", TTPtr(&wrappedClass_receiveNotificationForOutlet));
            x->controlCallback->set("baton", TTPtr(x));	
 
        	// dynamically add a message to the callback object so that it can handle the 'objectFreeing' notification
            x->controlCallback->instance()->registerMessage(notificationName, (TTMethod)&TTCallback::notify, kTTMessagePassValue);
            
            // tell the source that is passed in that we want to watch it
            x->wrappedObject->registerObserverForNotifications(*x->controlCallback);

        }
        
        x->outlets = new t_outlet*[x->numOutputs];
		for (short i=0; i < x->numOutputs; i++)
            x->outlets[i] = outlet_new(&x->obj.o_obj, &s_signal);

        // x->obj.z_misc = Z_NO_INPLACE;
	}
    return (t_eobj*)x;
}


void wrappedClass_free(WrappedInstancePtr x)
{
    for ( short i = 1; i < x->numInputs; i++ )
    {
        inlet_free(x->inlets[i]);
    }
    for ( short i = 0; i < x->numOutputs; i++ )
    {
        outlet_free(x->outlets[i]);
    }
    delete x->wrappedObject;
	delete x->audioIn;
	delete x->audioOut;
	delete[] x->controlSignalNames;

    eobj_free(x);
}


void wrappedClass_receiveNotificationForOutlet(WrappedInstancePtr self, TTValue& arg)
{
    TTString	string = arg[0];
    t_symbol*   s = gensym((char*)string.c_str());
    
    outlet_anything((t_outlet*)self->controlOutlet, s, 0, NULL);
}


t_max_err wrappedClass_attrGet(TTPtr self, t_object* attr, long* argc, t_atom** argv)
{
	t_symbol*	attrName = (t_symbol*)object_method(attr, _sym_getname);
	TTValue		v;
	long	i;
	WrappedInstancePtr x = (WrappedInstancePtr)self;
    TTPtr		rawpointer = NULL;
	
    rawpointer = x->wrappedClassDefinition->pdNamesToTTNames[attrName->s_name];
    // err = hashtab_lookup(x->wrappedClassDefinition->pdNamesToTTNames, attrName, (t_object**)&rawpointer);
    if (!rawpointer)
        return -2; // MAX_ERR_INVALID_PTR

	TTSymbol	ttAttrName(rawpointer);
	
	x->wrappedObject->get(ttAttrName, v);

	*argc = v.size();
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom *)sysmem_newptr(sizeof(t_atom) * v.size());

	for (i=0; i<v.size(); i++) {
		if (v[i].type() == kTypeFloat32 || v[i].type() == kTypeFloat64) {
			TTFloat64	value;
			value = v[i];
			atom_setfloat(*argv+i, value);
		}
		else if (v[i].type() == kTypeSymbol) {
			TTSymbol	value;
			value = v[i];
			atom_setsym(*argv+i, gensym((char*)value.c_str()));
		}
		else {	// assume int
			TTInt32		value;
			value = v[i];
			atom_setlong(*argv+i, value);
		}
	}	
    return 0;
}


t_max_err wrappedClass_attrSet(TTPtr self, t_object* attr, long argc, t_atom* argv)
{
	WrappedInstancePtr x = (WrappedInstancePtr)self;
	
	if (argc && argv) {
		t_symbol*	attrName = (t_symbol*)object_method(attr, _sym_getname);
		TTValue		v;
        long        i;
		TTPtr		ptr = NULL;
		
        ptr = x->wrappedClassDefinition->pdNamesToTTNames[attrName->s_name];
        if (!ptr)
            return -1;
		
		TTSymbol	ttAttrName(ptr);
		
		v.resize(argc);
		for (i=0; i<argc; i++) {
			if (atom_gettype(argv+i) == A_LONG)
				v[i] = (TTInt32)atom_getlong(argv+i);
			else if (atom_gettype(argv+i) == A_FLOAT)
				v[i] = atom_getfloat(argv+i);
			else if (atom_gettype(argv+i) == A_SYM)
				v[i] = TT(atom_getsym(argv+i)->s_name);
			else
				pd_error((t_object*)x, "bad type for attribute setter");
		}
		x->wrappedObject->set(ttAttrName, v);
        return 0; // MAX_ERR_NONE;
	}
    return -1; // MAX_ERR_GENERIC;
}


void wrappedClass_anything(TTPtr self, t_symbol* s, long argc, t_atom* argv)
{
	WrappedInstancePtr	x = (WrappedInstancePtr)self;
	TTSymbol			ttName;
	TTValue				v_in;
	TTValue				v_out;
	
//	err = hashtab_lookup(x->wrappedClassDefinition->pdNamesToTTNames, s, (t_object**)&ttName);
    ttName = x->wrappedClassDefinition->pdNamesToTTNames[s->s_name];
    if (ttName.string().empty()) {
        pd_error((t_object*)x, "no method found for %s", s->s_name);
		return;
	}

	if (argc && argv) {
		v_in.resize(argc);
		for (long i=0; i<argc; i++) {
			if (atom_gettype(argv+i) == A_LONG)
				v_in[i] = (TTInt32)atom_getlong(argv+i);
			else if (atom_gettype(argv+i) == A_FLOAT)
				v_in[i] = atom_getfloat(argv+i);
			else if (atom_gettype(argv+i) == A_SYM)
				v_in[i] = TT(atom_getsym(argv+i)->s_name);
			else
				pd_error((t_object*)x, "bad type for message arg");
		}
	}
	x->wrappedObject->send(ttName, v_in, v_out);
		
	// process the returned value for the dumpout outlet
	{
		long	ac = v_out.size();

		if (ac) {
			t_atom*		av = (t_atom*)malloc(sizeof(t_atom) * ac);
			
			for (long i=0; i<ac; i++) {
				if (v_out[0].type() == kTypeSymbol) {
					TTSymbol ttSym;
					ttSym = v_out[i];
					atom_setsym(av+i, gensym((char*)ttSym.c_str()));
				}
				else if (v_out[0].type() == kTypeFloat32 || v_out[0].type() == kTypeFloat64) {
					TTFloat64 f = 0.0;
					f = v_out[i];
					atom_setfloat(av+i, f);
				}
				else {
					TTInt32 l = 0;
					l = v_out[i];
					atom_setfloat(av+i, l);
				}
			}
            outlet_anything(x->dumpOut, s, ac, av);
			free(av);
		}
	}
}


// Method for Assistance Messages
void wrappedClass_assist(WrappedInstancePtr self, void *b, long msg, long arg, char *dst)
{
	if (msg==1)	{		// Inlets
		if (arg==0)
            strcpy(dst, "signal input, control messages"); //leftmost inlet
		else { 
			if (arg > self->numInputs-self->numControlSignals-1)
				//strcpy(dst, "control signal input");		
				snprintf(dst, 256, "control signal for \"%s\"", self->controlSignalNames[arg - self->numInputs+1].c_str());
			else
				strcpy(dst, "signal input");		
		}
	}
	else if (msg==2)	{	// Outlets
		if (arg < self->numOutputs)
			strcpy(dst, "signal output");
		else
			strcpy(dst, "dumpout"); //rightmost outlet
	}
}


void wrappedClass_perform64(WrappedInstancePtr self, t_object* dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	TTUInt16 i;
	//TTUInt16 numChannels = numouts;
	
	self->numChannels = numouts; // <-- this is kinda lame, but for the time being I think we can get away with this assumption...
	
	for (i=0; i < self->numControlSignals; i++) {
		int signal_index = self->numInputs - self->numControlSignals + i;
		
		if (self->signals_connected[signal_index])
			self->wrappedObject->set(self->controlSignalNames[i], *ins[signal_index]);
	}
	
	self->audioIn->setNumChannels(self->numInputs-self->numControlSignals);
	self->audioOut->setNumChannels(self->numOutputs);
	self->audioOut->allocWithVectorSize(sampleframes);
	
	for (i=0; i < self->numInputs-self->numControlSignals; i++)
		self->audioIn->setVector(i, self->vs, ins[i]);
	
	self->wrappedObject->process(self->audioIn, self->audioOut);
	
	for (i=0; i < self->numOutputs; i++) 
		self->audioOut->getVectorCopy(i, self->vs, outs[i]);
	
}


void wrappedClass_dsp64(WrappedInstancePtr self, t_object* dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	for (int i=0; i < (self->numInputs + self->numOutputs); i++)
		self->signals_connected[i] = count[i];
	
	ttEnvironment->setAttributeValue(kTTSym_sampleRate, samplerate);
	self->wrappedObject->set(TT("sampleRate"), samplerate);
	
	self->vs = maxvectorsize;
	
	self->audioIn->setVectorSizeWithInt(self->vs);
	self->audioOut->setVectorSizeWithInt(self->vs);
	
	// object_method(dsp64, gensym("dsp_add64"), self, wrappedClass_perform64, 0, NULL);
}


TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c)
{
	return wrapTTClassAsPdClass(ttblueClassName, pdClassName, c, (WrappedClassOptionsPtr)NULL);
}

void object_obex_dumpout(void *x, t_symbol *s, long argc, t_atom *argv)
{
    try {
        outlet_anything(((WrappedInstancePtr)x)->dumpOut,s,argc,argv);
    } catch (  const std::exception & e ){
        ;
    }
}

TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c, WrappedClassOptionsPtr options)
{
	TTObject		o(ttblueClassName, 1);	// Create a temporary instance of the class so that we can query it.
	TTValue			v;
	WrappedClass*	wrappedPdClass = NULL;
	TTSymbol		name;
	TTCString		nameCString = NULL;
	t_symbol*		namePdSymbol = NULL;
	TTUInt32		nameSize = 0;
	
	common_symbols_init();
	TTDSPInit();
	
	wrappedPdClass = new WrappedClass;
	wrappedPdClass->pdClassName = gensym((char*)pdClassName);
    wrappedPdClass->pdClass = eclass_new(	(char*)pdClassName,
                                            (method)wrappedClass_new,
                                            (method)wrappedClass_free,
											sizeof(WrappedInstance), 
                                            CLASS_NOINLET,
											A_GIMME, 
											0);
	wrappedPdClass->ttblueClassName = ttblueClassName;
	wrappedPdClass->validityCheck = NULL;
	wrappedPdClass->validityCheckArgument = NULL;
    wrappedPdClass->options = options;

	if (!o.valid()) {
		error("Jamoma ClassWrapper failed to load %s", ttblueClassName.c_str());
		return kTTErrAllocFailed;
	}

	o.messages(v);
	for (TTUInt16 i=0; i<v.size(); i++) {
		name = v[i];
        //nameSize = name->getString().length();	// TODO -- this crash on Windows...
		nameSize = strlen(name.c_str());
		nameCString = new char[nameSize+1];
        strncpy(nameCString, name.c_str(), nameSize+1);

		namePdSymbol = gensym(nameCString);
        wrappedPdClass->pdNamesToTTNames[namePdSymbol->s_name] = (t_object*)name.rawpointer();
        eclass_addmethod((t_eclass*)wrappedPdClass->pdClass, (method)wrappedClass_anything, nameCString, A_GIMME, 0);
		
        delete[] nameCString;
		nameCString = NULL;
	}
	
	o.attributes(v);
	for (TTUInt16 i=0; i<v.size(); i++) {
		TTAttributePtr	attr = NULL;
		
		name = v[i];
        //nameSize = name->getString().length();	// TODO -- this crash on Windows...
		nameSize = strlen(name.c_str());
		nameCString = new char[nameSize+1];
        strncpy(nameCString, name.c_str(), nameSize+1);
		namePdSymbol = gensym(nameCString);
				
		if (name == TT("maxNumChannels"))
			continue;						// don't expose these attributes to Pd users
		if (name == TT("bypass")) {
			if (wrappedPdClass->options && !wrappedPdClass->options->lookup(TT("generator"), v))
				continue;					// generators don't have inputs, and so don't really provide a bypass
		}
		
		o.instance()->findAttribute(name, &attr);
        wrappedPdClass->pdNamesToTTNames[namePdSymbol->s_name] = (t_object*)name.rawpointer();

		if (attr->type == kTypeFloat32)
            CLASS_ATTR_FLOAT(wrappedPdClass->pdClass,nameCString,0,t_eclass,c_attr);
		else if (attr->type == kTypeFloat64)
            CLASS_ATTR_DOUBLE(wrappedPdClass->pdClass,nameCString,0,t_eclass,c_attr);
		else if (attr->type == kTypeSymbol || attr->type == kTypeString)
            CLASS_ATTR_SYMBOL(wrappedPdClass->pdClass,nameCString,0,t_eclass,c_attr);
        else
            CLASS_ATTR_LONG(wrappedPdClass->pdClass,nameCString,0,t_eclass,c_attr);
		
        //class_addattr(wrappedPdClass->pdClass, attr_offset_new(nameCString, pdType, 0, (method)wrappedClass_attrGet, (method)wrappedClass_attrSet, 0));
		
    /*
		// Add display styles for the Max 5 inspector
		if (attr->type == kTypeBoolean)
			CLASS_ATTR_STYLE(wrappedPdClass->pdClass, (char*)name.c_str(), 0, (char*)"onoff");
		if (name == TT("fontFace"))
			CLASS_ATTR_STYLE(wrappedPdClass->pdClass,	(char*)"fontFace", 0, (char*)"font");
		*/
    
        delete[] nameCString;
		nameCString = NULL;
	}
			
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedClass_dsp64, 	"dsp64",		A_CANT, 0L);
    eclass_addmethod(wrappedPdClass->pdClass, (method)object_obex_dumpout, 	"dumpout",		A_CANT, 0);
    eclass_addmethod(wrappedPdClass->pdClass, (method)wrappedClass_assist, 	"assist",		A_CANT, 0L);
    // eclass_addmethod(wrappedPdClass->pdClass, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);
	
	eclass_dspinit(wrappedPdClass->pdClass);
    eclass_register(CLASS_BOX, wrappedPdClass->pdClass);
	if (c)
		*c = wrappedPdClass;
	
    wrappedPdClasses[wrappedPdClass->pdClassName->s_name] = (t_object*)wrappedPdClass;
	return kTTErrNone;
}

TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c, TTValidityCheckFunction validityCheck)
{
	TTErr err = wrapTTClassAsPdClass(ttblueClassName, pdClassName, c);
	
	if (!err) {
		(*c)->validityCheck = validityCheck;
		(*c)->validityCheckArgument = (*c)->pdClass;
	}
	return err;
}

TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c, TTValidityCheckFunction validityCheck, WrappedClassOptionsPtr options)
{
	TTErr err = wrapTTClassAsPdClass(ttblueClassName, pdClassName, c, options);
	
	if (!err) {
		(*c)->validityCheck = validityCheck;
		(*c)->validityCheckArgument = (*c)->pdClass;
	}
	return err;
}


TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c, TTValidityCheckFunction validityCheck, TTPtr validityCheckArgument)
{
	TTErr err = wrapTTClassAsPdClass(ttblueClassName, pdClassName, c);
	
	if (!err) {
		(*c)->validityCheck = validityCheck;
		(*c)->validityCheckArgument = validityCheckArgument;
	}
	return err;
}

TTErr wrapTTClassAsPdClass(TTSymbol ttblueClassName, const char* pdClassName, WrappedClassPtr* c, TTValidityCheckFunction validityCheck, TTPtr validityCheckArgument, WrappedClassOptionsPtr options)
{
	TTErr err = wrapTTClassAsPdClass(ttblueClassName, pdClassName, c, options);
	
	if (!err) {
		(*c)->validityCheck = validityCheck;
		(*c)->validityCheckArgument = validityCheckArgument;
	}
	return err;
}

TTErr TTValueFromAtoms(TTValue& v, long ac, t_atom* av)
{
	v.clear();
	
	// For now we assume floats for speed (e.g. in the performance sensitive j.unit object)
	for (int i=0; i<ac; i++)
		v.append((TTFloat64)atom_getfloat(av+i));
	return kTTErrNone;
}

TTErr TTAtomsFromValue(const TTValue& v, long* ac, t_atom** av)
{
	int	size = v.size();
	
	if (*ac && *av)
		; // memory was passed-in from the calling function -- use it
	else {
		*ac = size;
		*av = new t_atom[size];// (t_atom*)sysmem_newptr(sizeof(t_atom) * size);
	}

	for (int i=0; i<size; i++) {
		atom_setfloat((*av)+i, v[i]);
	}
	return kTTErrNone;
}
