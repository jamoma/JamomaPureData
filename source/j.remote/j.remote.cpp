/** @file
 *
 * @ingroup implementationPdExternals
 *
 * @brief j.remote : Bind to a parameter value
 *
 * @details Useful when designing views and more.
 *
 * @authors Théo de la Hogue, Trond Lossius, Antoine Villeret
 *
 * @copyright © 2010 by Théo de la Hogue @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */


#include "TTModularClassWrapperPd.h"

// those stuffes are needed for handling patchers without using the pcontrol object
// #include "jpatcher_api.h"

typedef struct dll {
	t_object d_ob;
	struct dll *d_next;
	struct dll *d_prev;
	void *d_x1;
} t_dll;
/*
typedef struct outlet {
	struct tinyobject o_ob;
	struct dll *o_dll;
} t_outlet;
*/

// This is used to store extra data
typedef struct extra {
    TTAddress   name;           ///< the name to use for subscription
	t_object*	connected;		// our ui object
	long		x;				// our ui object x presentation
	long		y;				// our ui object y presentation
	long		w;				// our ui object width presentation
	long		h;				// our ui object heigth presentation
	t_object*	label;			// label to display selection state
	t_atom*		color0;			// label color for selection state == 0
	t_atom*		color1;			// label color for selection state == 1
    TTBoolean   setting;        // a flag to know if the remote is updated by a set message
} t_extra;
#define EXTRA ((t_extra*)x->extra)

// Definitions
void        WrapTTViewerClass(WrappedClassPtr c);
void        WrappedViewerClass_new(TTPtr self, long argc, t_atom *argv);
void        WrappedViewerClass_free(TTPtr self);
void        WrappedViewerClass_anything(TTPtr self, t_symbol *msg, long argc, t_atom *argv);

void        remote_assist(TTPtr self, void *b, long msg, long arg, char *dst);

void        remote_return_value(TTPtr self, t_symbol *msg, long argc, t_atom *argv);
void        remote_return_model_address(TTPtr self, t_symbol *msg, long argc, t_atom *argv);
void        remote_return_description(TTPtr self, t_symbol *msg, long argc, t_atom *argv);

void        remote_bang(TTPtr self);
void        remote_float(TTPtr self, t_float value);
TTErr       remote_list(TTPtr self, t_symbol *msg, long argc, t_atom *argv);

void        remote_set(TTPtr self, t_symbol *msg, long argc, t_atom *argv);

void        remote_address(TTPtr self, t_symbol *address);
void        remote_loadbang(TTPtr self);

void        remote_mousemove(TTPtr self, t_object *patcherview, t_pt pt, long modifiers);
void        remote_mouseleave(TTPtr self, t_object *patcherview, t_pt pt, long modifiers);
void        remote_mousedown(TTPtr self, t_object *patcherview, t_pt pt, long modifiers);

void        remote_subscribe(TTPtr self);

extern "C" void JAMOMA_EXPORT_MAXOBJ setup_j0x2eremote(void)
{
	ModularSpec *spec = new ModularSpec;
	spec->_wrap = &WrapTTViewerClass;
	spec->_new = &WrappedViewerClass_new;
	spec->_free = &WrappedViewerClass_free;
	spec->_any = &WrappedViewerClass_anything;
    spec->_notify = NULL;
	
	return (void)wrapTTModularClassAsPdClass(kTTSym_Viewer, "j.remote", NULL, spec);
}

void WrapTTViewerClass(WrappedClassPtr c)
{
	eclass_addmethod(c->pdClass, (method)remote_return_value,			"return_value",			A_CANT, 0);
	eclass_addmethod(c->pdClass, (method)remote_return_model_address,	"return_model_address",	A_CANT, 0);
	eclass_addmethod(c->pdClass, (method)remote_return_description,     "return_description",	A_CANT, 0);
    
	eclass_addmethod(c->pdClass, (method)remote_bang,					"bang",					A_NULL, 0L);
	eclass_addmethod(c->pdClass, (method)remote_float,					"float",				A_FLOAT, 0L);
	eclass_addmethod(c->pdClass, (method)remote_list,					"list",					A_GIMME, 0L);
    
	eclass_addmethod(c->pdClass, (method)remote_set,					"set",					A_GIMME, 0L);
    
	eclass_addmethod(c->pdClass, (method)remote_address,				"address",				A_SYM, 0);
    eclass_addmethod(c->pdClass, (method)remote_loadbang,				"loadbang",				A_NULL, 0);
}

void WrappedViewerClass_new(TTPtr self, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	t_symbol					*address;
 	long						attrstart = attr_args_offset(argc, argv);			// support normal arguments
	
	// read the address to bind from the first argument
	if (attrstart > 0 && argv)
		address = atom_getsym(argv);
	else
		address = _sym_nothing;
	
	// x->address = TTAddress(jamoma_parse_dieze((t_object*)x, address)->s_name);
    x->address = TTAddress(address->s_name);
	x->index = 0; // the index member is usefull to count how many time the external tries to bind
	
	// Prepare extra data
	x->extra = (t_extra*)malloc(sizeof(t_extra));
    
    // read the name to use for subscription from the first argument
	if (attrstart == 2 && argv)
		EXTRA->name = TTAddress(atom_getsym(argv+1)->s_name);
	else
		EXTRA->name = kTTAdrsEmpty;
    
    EXTRA->connected = NULL;
	EXTRA->label = NULL;
	
	EXTRA->color0 = (t_atom*)sysmem_newptr(sizeof(t_atom) * 4);
	atom_setfloat(EXTRA->color0, 0);
	atom_setfloat(EXTRA->color0+1, 0.);
	atom_setfloat(EXTRA->color0+2, 0.);
	atom_setfloat(EXTRA->color0+3, 1.);
	
	EXTRA->color1 = (t_atom*)sysmem_newptr(sizeof(t_atom) * 4);
	atom_setfloat(EXTRA->color1, 0.62);
	atom_setfloat(EXTRA->color1+1, 0.);
	atom_setfloat(EXTRA->color1+2, 0.36);
	atom_setfloat(EXTRA->color1+3, 0.70);
    
    EXTRA->setting = NO;
	
	jamoma_viewer_create((t_object*)x, x->wrappedObject);
	
	// Make two outlets
	x->outlets = (TTHandle)sysmem_newptr(sizeof(TTPtr) * 3);
    x->outlets[0] = outlet_new((t_object*)x, NULL);						// anything outlet to output qlim data
	
    // clear support for qelem value
    x->argc = 0;
    x->argv = NULL;

	// handle attribute args
	attr_args_process(x, argc, argv);

	// The following must be deferred because we have to interrogate our box,
	// and our box is not yet valid until we have finished instantiating the object.
	// Trying to use a loadbang method instead is also not fully successful (as of Max 5.0.6)
//	defer_low((t_object*)x, (method)remote_subscribe, NULL, 0, 0);
    // remote_subscribe(x);
}

void WrappedViewerClass_free(TTPtr self)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    
    x->wrappedObject.set(kTTSym_address, kTTAdrsEmpty);
    
	free(EXTRA);
}

void remote_subscribe(TTPtr self)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue						v;
	t_atom						a[1];
	TTAddress                   contextAddress = kTTAdrsEmpty;
	TTAddress                   absoluteAddress, returnedAddress;
    TTNodePtr                   returnedNode = NULL;
    TTNodePtr                   returnedContextNode = NULL;
	TTObject                    anObject, empty;
    TTErr                       err;
    
    if (x->address == kTTAdrsEmpty)
		return;
    
	// for absolute address we only bind the given address but we don't subscribe the remote into the namespace
	if (x->address.getType() == kAddressAbsolute) {
		
		x->wrappedObject.set(kTTSym_address, x->address);
		
        // observe :description attribute
        if (x->internals->lookup(TTSymbol(":description"), v))
            
            makeInternals_receiver(x, absoluteAddress, TTSymbol(":description"), gensym("return_description"), anObject, YES);
        
        else {
            anObject = v[0];
            anObject.set(kTTSym_address, absoluteAddress.appendAttribute(kTTSym_description));
        }

        return;
	}
	
	// for relative address
    jamoma_patcher_get_info(((t_eobj*)x)->o_canvas, &x->patcherPtr, x->patcherContext, x->patcherClass, x->patcherName);
    
    // if no name is provided or can be edited in remote_attach() : use the address
    if (EXTRA->name == kTTAdrsEmpty)
        EXTRA->name = x->address;
	
    // if there is a context
    if (x->patcherContext != kTTSymEmpty) {
        
        // Do we subscribe the Viewer ?
        // View patcher case :
        if (x->patcherContext == kTTSym_view) {
            
            // if the address refer to the j.model (only :attributeName) don't subscribe the Viewer
            if (x->address.getParent() == NO_PARENT &&
                x->address.getName() == NO_NAME &&
                x->address.getInstance() == NO_INSTANCE &&
                x->address.getAttribute() != NO_ATTRIBUTE)
				err = jamoma_subscriber_create((t_eobj*)x, empty, x->address, x->subscriberObject, returnedAddress, &returnedNode, &returnedContextNode);
            
             // if the address refer to the "model" node don't subscribe the Viewer (to not have model.1)
            else if (x->address.getName() == TTSymbol("model"))
				err = jamoma_subscriber_create((t_eobj*)x, empty, x->address, x->subscriberObject, returnedAddress, &returnedNode, &returnedContextNode);
            
            // else try to subscribe the Viewer with its name
            else
				err = jamoma_subscriber_create((t_eobj*)x, x->wrappedObject, EXTRA->name, x->subscriberObject, returnedAddress, &returnedNode, &returnedContextNode);
            
        }
        // Model patcher case :
        // try to binds on the parameter|message|return of the model without subscribing the Viewer
        else if (x->patcherContext == kTTSym_model)
			err = jamoma_subscriber_create((t_eobj*)x, empty, x->address, x->subscriberObject, returnedAddress, &returnedNode, &returnedContextNode);
        
        // Any other case : give up
        else 
            return;
        
        // if the subscription is succesfull
        if (!err) {
            
            // get the context address to make
            // a viewer on the contextAddress/model:address attribute
            x->subscriberObject.get("contextAddress", v);
            contextAddress = v[0];
            
            // observe model:address attribute (in view patcher : deferlow return_model_address)
            makeInternals_receiver(x, contextAddress, TTSymbol("model:address"), gensym("return_model_address"), anObject, x->patcherContext == kTTSym_view);
                
            return;
        }
	}
	// else, if no context, set address directly
	else {
		contextAddress = kTTAdrsRoot;
		absoluteAddress = contextAddress.appendAddress(x->address);
		x->wrappedObject.set(kTTSym_address, absoluteAddress);
		
		atom_setsym(a, gensym((char*)absoluteAddress.c_str()));
		outlet_anything((t_outlet*)x->dumpOut, gensym("address"), 1, a);
        
        // observe :description attribute
        if (x->internals->lookup(TTSymbol(":description"), v))
            
            makeInternals_receiver(x, absoluteAddress, TTSymbol(":description"), gensym("return_description"), anObject, YES);
        
        else {
            anObject = v[0];
            anObject.set(kTTSym_address, absoluteAddress.appendAttribute(kTTSym_description));
        }
        
        return;
	}
	
	// otherwise while the context node is not registered : try to binds again :(
	// (to -- this is not a good way todo. For binding we should make a subscription 
	// to a notification mechanism and each time an TTObjet subscribes to the namespace
	// using jamoma_subscriber_create we notify all the externals which have used 
	// jamoma_subscriber_create with NULL object to bind)
	
	// release the subscriber
	x->subscriberObject = TTObject();
	
	x->index++; // the index member is usefull to count how many time the external tries to bind
	if (x->index > 100) {
		pd_error((t_object*)x, "couldn't bind to j.parameter %s", x->address.c_str());
		outlet_anything((t_outlet*)x->dumpOut, gensym("error"), 0, NULL);
		return;
	}
	
	// The following must be deferred because we have to interrogate our box,
	// and our box is not yet valid until we have finished instantiating the object.
	// Trying to use a loadbang method instead is also not fully successful (as of Max 5.0.6)
//	defer_low((t_object*)x, (method)remote_subscribe, NULL, 0, 0);
	remote_subscribe(x);
}

void remote_return_value(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    
    // a gate to not output the value if it have been set by this j.remote
    if (EXTRA->setting) {
        EXTRA->setting = NO;
        return;
    }
	
	// avoid blank before data
    if (msg == _sym_nothing) {
        outlet_anything((t_outlet*)x->outlets[0], NULL, argc, argv);
    } else {
        outlet_anything((t_outlet*)x->outlets[0], msg, argc, argv);
    }
	
}

void remote_bang(TTPtr self)
{
	remote_list(self, _sym_bang, 0, NULL);
}

void remote_float(TTPtr self, t_float value)
{
	t_atom a;
	
	atom_setfloat(&a, value);
	remote_list(self, _sym_float, 1, &a);
}

TTErr remote_list(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	
	return jamoma_viewer_send(x->wrappedObject, msg, argc, argv);
}

void remote_set(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	
    EXTRA->setting = YES;
    
    if (remote_list(self, _sym_nothing, argc, argv))
        
        EXTRA->setting = NO;
}

void WrappedViewerClass_anything(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue		v;
	
	jamoma_viewer_send(x->wrappedObject, msg, argc, argv);
}

void remote_return_model_address(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTAddress           absoluteAddress;
    TTObject            anObject;
	t_atom				a[1];
	TTSymbol			service;
	TTValue				v;
	
	if (argc && argv && x->wrappedObject.valid()) {
        
		// set address attribute of the wrapped Viewer object
		absoluteAddress = TTAddress(atom_getsym(argv)->s_name).appendAddress(x->address);
		x->wrappedObject.set(kTTSym_address, absoluteAddress);
		x->index = 0; // the index member is usefull to count how many time the external tries to bind
        
        // observe :description attribute
        if (x->internals->lookup(TTSymbol(":description"), v))
            
            makeInternals_receiver(x, absoluteAddress, TTSymbol(":description"), gensym("return_description"), anObject, YES);
        
        else {
            anObject = v[0];
            anObject.set(kTTSym_address, absoluteAddress.appendAttribute(kTTSym_description));
        }
		
		atom_setsym(a, gensym((char*)absoluteAddress.c_str()));
		outlet_anything((t_outlet*)x->dumpOut, gensym("address"), 1, a);
		
		JamomaDebug logpost((t_object*)x,3, "binds on %s", absoluteAddress.c_str());
	}
}

void remote_return_description(TTPtr self, t_symbol *msg, long argc, t_atom *argv)
{
	// WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	
    // if an ui object is connected
    // if (EXTRA->connected)
        
        // set its annotation attribute
        // there is no annotation on Pd, right ?
        // object_attr_setvalueof(EXTRA->connected, _sym_annotation , argc, argv);
}

void remote_address(TTPtr self, t_symbol *address)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
    
    x->address =  TTAddress(address->s_name);
    
    // unsubscribe the remote before
    if (x->subscriberObject.valid())
        x->subscriberObject = TTObject();
    
    remote_subscribe(self);
}

void remote_loadbang(TTPtr self)
{
    remote_subscribe(self);
}

// When the mouse is moving on the j.ui (not our remote object !)
/* TODO : AV remove this for now, do we need it in Puredata ?
void remote_mousemove(TTPtr self, t_object *patcherview, t_pt pt, long modifiers)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue		v;
	TTBoolean	selected;
	t_object	*patcher;
	long        ac;
	t_atom		*av;
	t_atom		a;
	
	if (EXTRA->connected) {
		
		// if the control key is pressed
		if (modifiers & eShiftKey) {
			
			// hide gui
			atom_setlong(&a, 1);
			object_attr_setvalueof(EXTRA->connected, _sym_hidden, 1, &a);
			
			// create a comment object
			if (!EXTRA->label) {
				patcher = NULL;
				ac = 0;
				av = NULL;
				object_obex_lookup(x, gensym("#P"), &patcher);
				EXTRA->label = newobject_sprintf(patcher, "@maxclass comment @presentation 1 @textcolor 1. 1. 1. 1.");
				object_attr_getvalueof(EXTRA->connected, _sym_presentation_rect , &ac, &av);
				if (ac && av && EXTRA->label) {
					object_method_long(EXTRA->label, _sym_fontsize, 10, &a);
					object_method_sym(EXTRA->label, _sym_set, gensym((char*)x->address.c_str()), &a);
					object_method_typed(EXTRA->label, _sym_presentation_rect, ac, av, &a);
				}
			}
			
			// display selected attribute by changing background color if selected
			x->wrappedObject.get(kTTSym_highlight, v);
			selected = v[0];
			
			if (EXTRA->label) {
				if (selected)
					object_attr_setvalueof(EXTRA->label, _sym_bgcolor, 4, (t_atom*)EXTRA->color1);
				else
					object_attr_setvalueof(EXTRA->label, _sym_bgcolor, 4, (t_atom*)EXTRA->color0);
            }
		}
		// else set default color
		// TODO : do this only one time !!!
		else {
			
			// show gui
			atom_setlong(&a, 0);
			object_attr_setvalueof(EXTRA->connected, _sym_hidden, 1, &a);
			
			// delete label
			if (EXTRA->label) {
				object_free(EXTRA->label);
				EXTRA->label = NULL;
			}
		}
	}
}

// When the mouse is leaving on the j.ui (not our remote object !)
void remote_mouseleave(TTPtr self, t_object *patcherview, t_pt pt, long modifiers)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	t_atom		a;
	
	// if mouse leaves j.ui maybe it is on our object
	if (pt.x > EXTRA->x && pt.x < EXTRA->x+EXTRA->w && pt.y > EXTRA->y && pt.y < EXTRA->y+EXTRA->h)
		return;
	
	// else the mouse leaves outside the j.ui
	else {
		
		// show gui
		atom_setlong(&a, 0);
		object_attr_setvalueof(EXTRA->connected, _sym_hidden, 1, &a);
		
		// delete label
		if (EXTRA->label) {
			object_free(EXTRA->label);
			EXTRA->label = NULL;
		}
	}
}

void remote_mousedown(TTPtr self, t_object *patcherview, t_pt pt, long modifiers)
{
	WrappedModularInstancePtr	x = (WrappedModularInstancePtr)self;
	TTValue		v;
	TTBoolean	selected;
	
	// if the control key is pressed
	if (modifiers & eShiftKey) {
		
		// if mouse leave j.ui maybe it is on our object
		if (pt.x > EXTRA->x && pt.x < EXTRA->x+EXTRA->w && pt.y > EXTRA->y && pt.y < EXTRA->y+EXTRA->h) {
			
			x->wrappedObject.get(kTTSym_highlight, v);
			selected = v[0];
			
			// reverse selected attribute and change color
			if (EXTRA->label) {
				if (selected) {
					x->wrappedObject.set(kTTSym_highlight, NO);
					object_attr_setvalueof(EXTRA->label, _sym_bgcolor, 4, (t_atom*)EXTRA->color0);
				}
				else {
					x->wrappedObject.set(kTTSym_highlight, YES);
					object_attr_setvalueof(EXTRA->label, _sym_bgcolor, 4, (t_atom*)EXTRA->color1);
				}
            }
		}
	}
}
*/
