/** @file
 *
 * @ingroup implementationPdLibrary
 *
 * @brief JamomaModular For Max Shared Library
 *
 * @details Functions and resources based on Modular framework used by Max objects
 *
 * @authors Théo de la Hogue, Tim Place, Trond Lossius, Antoine Villeret
 *
 * @copyright © 2013, Théo de la Hogue & Tim Place @n
 * Copyright © 2015, Antoine Villeret@n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#include "JamomaModularForPd.h"

/***********************************************************************************
*
*		C EXTERN METHODS
*
************************************************************************************/

// prototypes
void jamoma_subscriber_fill_list(TTList& listToFill, TTAddress address, TTPtr pointer);


// Method to deal with TTSubscriber
///////////////////////////////////////////////////////////////////////

TTErr jamoma_subscriber_create(t_eobj *x, TTObject& anObject, TTAddress relativeAddress, TTObject& returnedSubscriber, TTSymbol& returnedAddress, TTNodePtr *returnedNode, TTNodePtr *returnedContextNode)
{
	TTValue		v, args;
	TTList		aContextList;
	TTAddress	newRelativeAddress, newContextAddress;
	TTBoolean	newInstance, newContextInstance;
    TTErr       err = kTTErrNone;
		
	// prepare arguments
	args.append(anObject);
	args.append(relativeAddress);
	
	returnedSubscriber = TTObject(kTTSym_Subscriber, args);
    
    // Get all Context above the object and their name
	jamoma_subscriber_get_patcher_list(x, aContextList);
    args = TTValue((TTPtr)&aContextList);
    
    *returnedNode = NULL;
    err = returnedSubscriber.send(kTTSym_Subscribe, args, v);
    
    if (v.size() == 2) {
        *returnedNode = TTNodePtr((TTPtr)v[0]);
        *returnedContextNode = TTNodePtr((TTPtr)v[1]);
    }
    
	// Check if the subscription is ok (or the binding in case of NULL object)
	if (!err && *returnedNode) {
		
		if (anObject.valid()) {
            
			// Is a new instance have been created ?
			returnedSubscriber.get("newInstanceCreated", v);
			newInstance = v[0];
            
            // Is a new context instance have been created ?
			returnedSubscriber.get("newContextInstanceCreated", v);
			newContextInstance = v[0];
			
            // warn the user he has an object with duplicate instance
			if (newInstance) {
                
				returnedSubscriber.get("relativeAddress", v);
				newRelativeAddress = v[0];
                verbose(PD_LOG_WARN, "Jamoma cannot register multiple objects with the same OSC identifier (%s).  Using %s instead.", relativeAddress.c_str(), newRelativeAddress.c_str());
			}
            
            // check why a new context instance have been created
            if (newContextInstance) {
                
				returnedSubscriber.get("contextAddress", v);
				newContextAddress = v[0];

                t_canvas *patcher;
                TTSymbol  patcherContext;
                TTSymbol  patcherClass;
                TTSymbol  patcherName;
                TTAddress patcherArg;
                TTString  newPatcherArgument;
                long argc = 0;
                t_atom *  argv = NULL;
                
                // get patcher info
				jamoma_patcher_get_info(x->o_canvas, &patcher, patcherContext, patcherClass, patcherName);
                
                // get patcher argument (dedicated for the name)
                jamoma_patcher_get_args(patcher, &argc, &argv);
                
                if (patcherContext == kTTSym_model && argc == 1) {
                    
                    if ((argv+1)->a_type == A_SYMBOL)
                        patcherArg = TTAddress(atom_getsymbol(argv+1)->s_name);
                }
                else if (patcherContext == kTTSym_view && argc == 2) {
                    
                    if ((argv+2)->a_type == A_SYMBOL)
                        patcherArg = TTAddress(atom_getsymbol(argv+2)->s_name);
                }
                
                // warn the user that it should provide unique name
                
                // if no name has been provided
                if (patcherArg == kTTAdrsEmpty && patcherContext == kTTSym_model)
                    verbose(PD_LOG_WARN, "No name provided to %s %s. Using %s.", patcherClass.c_str(), patcherContext.c_str(), newContextAddress.getNameInstance().c_str());

                // if a duplicate name.instance was passed in argument
                else
                    verbose(PD_LOG_WARN, "Duplicate name provided to %s %s (%s). Using %s.", patcherClass.c_str(), patcherContext.c_str(), patcherArg.c_str(), newContextAddress.getNameInstance().c_str());

			}
            
            returnedSubscriber.get(kTTSym_nodeAddress, v);
            returnedAddress = v[0];
			
			JamomaDebug post("registers at %s", returnedAddress.c_str());
		}

		return kTTErrNone;
	}
	
	if (anObject.valid())
        pd_error(x, "Jamoma cannot registers %s", relativeAddress.c_str());
	else
		// don't display this message because the objects can try many times before to binds
		; //pd_error(x, "Jamoma cannot binds %s", relativeAddress->s_name);
	
    return err;
}


/** Get the hierarchy of the patcher : bpatcher, subpatcher or top level one*/
t_symbol *jamoma_patcher_get_hierarchy(t_canvas *canvas)
{
    if ( canvas->gl_owner == NULL )
        return _sym_topmost;
    else if ( canvas->gl_isgraph )
        return _sym_bpatcher;
    else return _sym_subpatcher;
}

/** Parse #N inside address and replace them by parent patcher arguments if there are */
t_symbol *jamoma_parse_dieze(t_canvas *x, t_symbol *address)
{
	TTString	diezeStr, argsStr, addressStr = address->s_name;
	t_symbol	*hierarchy;
	t_canvas	*patcher = x;
	/* TODO : use a TTRegex for this parsing
	char		dieze[5];
	char		args[64];
	size_t		found = 0;
	long		i, sd, sa;
	*/
	long		i;
	long        ac = 0;
	t_atom      *av = NULL;

	// If x is in a bpatcher, the patcher is NULL
	if (!patcher){
        patcher = (t_canvas*)object_attr_getobj(x, _sym_parentpatcher);
	}

	if (patcher) {

		hierarchy = jamoma_patcher_get_hierarchy(patcher);

		// Try to get the patcher arguments
		jamoma_patcher_get_args(patcher, &ac, &av);
		if (hierarchy == _sym_subpatcher) {

            // remove first 'pd'
			if (ac > 0 && av) {
                if (atom_getsym(av) == _sym_pd) {
					ac--;
					av++;
				}
			}

			av++;
			ac--;
		}

		// if there are arguments
		if (ac > 0 && av) {

			i = 1;

			//object_post(x, "in jamoma_parse_dieze : TODO : use a TTRegex for this parsing");
			/* TODO : use a TTRegex for this parsing
			do {

			// prepare to parse #i
			snprintf(dieze, sizeof(dieze), "#%li", i);
			found = addressStr.find(dieze);

			// if #i found
			if (found != string::npos) {

			// get av+i atom
			if (i-1 < ac) {

			if (atom_gettype(av+i-1) == A_LONG)
			snprintf(args, sizeof(args), "%li", atom_getlong(av+i-1));
			else if (atom_gettype(av+i-1) == A_SYM)
			snprintf(args, sizeof(args), "%s", atom_getsym(av+i-1)->s_name);
			else {
			i++;
			continue;
			}

			diezeStr = dieze;
			argsStr	= args;
			sd = diezeStr.size();
			sa = argsStr.size();
			addressStr.replace(found, sd, args, sa);
			}
			}
			i++;

			} while (i-1 < ac); // while there are argument
			*/

			return gensym((char*)addressStr.data());
		}
	}

	return address;
}

void jamoma_subscriber_get_patcher_list(t_eobj *x, TTList& aContextListToFill)
{
	TTValue		v;
	t_canvas	*objPtr = x->o_canvas;
    t_canvas	*patcherPtr = NULL;
	TTSymbol	patcherContext;
	TTSymbol	patcherName;
	TTSymbol	patcherClass;
	TTSymbol	lowerContext;
	
	// Edit the list of all patcher's name and pointer 
	// above the object x looking at all parent patcher
	do {
		// get all info from the current object
		jamoma_patcher_get_info(objPtr, &patcherPtr, patcherContext, patcherClass, patcherName);
		
		if (patcherName && patcherPtr) {
		/* théo - when commenting this part we allow to subscribe view into model and model into view
			// check if the patcher have the same context than lower patchers
			if (patcherContext == lowerContext || lowerContext == kTTSymEmpty) {
		
				// keep it as lowerContext
				lowerContext = patcherContext;
		*/
                // store each name.instance part of the patcherName (level.i/sub.j/name.k)
                jamoma_subscriber_fill_list(aContextListToFill, TTAddress(patcherName), patcherPtr);
				
				// replace current object by his parent patcher
				objPtr = patcherPtr->gl_owner;
		/*	théo - when commenting this part we allow to subscribe view into model and model into view
            }
			else {
				
				// skip the patcher to go directly to an upper one
				objPtr = patcherPtr;
			}
        */
		}
		else
			break;
		
    } while (jamoma_patcher_get_hierarchy(patcherPtr) != _sym_topmost);
}

void jamoma_subscriber_fill_list(TTList& listToFill, TTAddress address, TTPtr pointer)
{
    TTSymbol level = address.getNameInstance();
    TTValue v(level, pointer);
    listToFill.insert(0, v);
    
    TTAddress parent = address.getParent();
    if (parent != kTTAdrsEmpty)
        jamoma_subscriber_fill_list(listToFill, parent, pointer);
}

// Method to deal with TTContainer
///////////////////////////////////////////////////////////////////////

/**	Create a container object */
TTErr jamoma_container_create(t_object *x, TTObject& returnedContainer)
{
	TTValue     args, none;
	TTObject	returnAddressCallback, returnValueCallback;
	
	// prepare arguments
	returnAddressCallback = TTObject("callback");
	returnAddressCallback.set(kTTSym_baton, TTPtr(x));
	returnAddressCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_address));
	args.append(returnAddressCallback);
	
	returnValueCallback = TTObject("callback");
	returnValueCallback.set(kTTSym_baton, TTPtr(x));
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnValueCallback);
	
	returnedContainer = TTObject(kTTSym_Container, args);
	
	return kTTErrNone;
}

/**	Send Max data to a node (e.g., a j.parameter object) using a container object. */
TTErr jamoma_container_send(TTObject& aContainer, t_symbol *relativeAddressAndAttribute, long argc, const t_atom *argv)
{
	TTAddress anAddress;
	TTValue	v, data, none;
	
	if (aContainer.valid()) {
		
		anAddress = relativeAddressAndAttribute->s_name;
		
		if (anAddress.getType() != kAddressRelative) {
			error("%s is an absolute address", relativeAddressAndAttribute->s_name);
			return kTTErrGeneric;
		}
		
		// Are we to send a message to an attribute of the node? If not we'll send the message to the value attribute of the node, so that the node value gets updated.
		if (anAddress.getAttribute() == NO_ATTRIBUTE)
			anAddress = anAddress.appendAttribute(kTTSym_value);
		else
			anAddress = anAddress.appendAttribute(ToTTName(anAddress.getAttribute()));

		data.append(anAddress);

		jamoma_ttvalue_from_Atom(v, _sym_nothing, argc, argv);
		data.append((TTPtr)&v);
		
		// data is [address, attribute, [x, x, ,x , ...]]
		return aContainer.send(kTTSym_Send, data, none);
    }
	
	return kTTErrGeneric;
}

// Method to deal with #TTNodeInfo
///////////////////////////////////////////////////////////////////////

TTErr JAMOMA_EXPORT jamoma_node_info_create(t_object *x, TTObject& returnedNodeInfo)
{
	returnedNodeInfo = TTObject(kTTSym_NodeInfo);
	return kTTErrNone;
}

// Method to deal with TTData
///////////////////////////////////////////////////////////////////////

/**	Create a data object */
TTErr jamoma_data_create(t_object *x, TTObject& returnedData, TTSymbol service)
{
	// create a data
	returnedData = TTObject(kTTSym_Data, service);
    
    // prepare its callback
    returnedData.set(kTTSym_baton, TTPtr(x));
	returnedData.set(kTTSym_function, TTPtr(&jamoma_callback_return_value_typed));
    returnedData.set(TTSymbol("rampDriveDefault"), TTSymbol("max"));
	
	return kTTErrNone;
}

/**	Send Max data command */
TTErr jamoma_data_command(TTObject& aData, t_symbol *msg, long argc, const t_atom *argv)
{
	TTValue v, none;
	
	if (aData.valid()) {
		
        jamoma_ttvalue_from_Atom(v, msg, argc, argv);
		
		aData.send(kTTSym_Command, v, none);
		return kTTErrNone;
	} else {
		error("jamoma_data_command : invalid TTObject");
	}
	return kTTErrGeneric;
}

// Method to deal with TTSender
///////////////////////////////////////////////////////////////////////

/**	Create a sender object */
TTErr jamoma_sender_create(t_object *x, TTObject& returnedSender)
{
	returnedSender = TTObject(kTTSym_Sender);
	return kTTErrNone;
}

/**	Create a sender object for audio signal */
TTErr jamoma_sender_create_audio(t_object *x, TTObject& returnedSender)
{
	TTObject audio;
	
	// prepare arguments
	audio = TTObject(kTTSym_audiosignal, 1);
	
	returnedSender = TTObject(kTTSym_Sender, audio);
	return kTTErrNone;
}

/**	Send Max data using a sender object */
TTErr jamoma_sender_send(TTObject& aSender, t_symbol *msg, long argc, const t_atom *argv)
{
	TTValue v, none;
	
	if (aSender.valid()) {
		
		jamoma_ttvalue_from_Atom(v, msg, argc, argv);

		return aSender.send(kTTSym_Send, v, none);
	}
	
	return kTTErrGeneric;
}

// Method to deal with TTReceiver
///////////////////////////////////////////////////////////////////////

/**	Create a receiver object */
TTErr jamoma_receiver_create(t_object *x, TTObject& returnedReceiver)
{
	TTValue		args;
	TTObject	returnAddressCallback, returnValueCallback;
	
    // prepare arguments
	returnAddressCallback = TTObject("callback");
	returnAddressCallback.set(kTTSym_baton, TTPtr(x));
	returnAddressCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_address));
	args.append(returnAddressCallback);
	
	returnValueCallback = TTObject("callback");
	returnValueCallback.set(kTTSym_baton, TTPtr(x));
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value_typed));
	args.append(returnValueCallback);
	
	returnedReceiver = TTObject(kTTSym_Receiver, args);
	return kTTErrNone;
}

/**	Create a receiver object for audio signal */
TTErr jamoma_receiver_create_audio(t_object *x, TTObject& returnedReceiver)
{
	TTValue      args;
	TTObject     returnAddressCallback, audio, empty;
	
	// prepare arguments
	returnAddressCallback = TTObject("callback");
	returnAddressCallback.set(kTTSym_baton, TTPtr(x));
	returnAddressCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_address));
	args.append(returnAddressCallback);
	
	args.append(empty);	// no return value callback
	
	audio = TTObject(kTTSym_audiosignal, 1);
	args.append(audio);
	
	returnedReceiver = TTObject(kTTSym_Receiver, args);
	return kTTErrNone;
}

// Method to deal with TTPresetManager
///////////////////////////////////////////////////////////////////////

/**	Create a preset manager object */
TTErr jamoma_presetManager_create(t_object *x, TTObject& returnedPresetManager)
{
    TTObject returnLineCallback;
	
	// prepare arguments
	returnLineCallback = TTObject("callback");
	returnLineCallback.set(kTTSym_baton, TTPtr(x));
	returnLineCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	
	returnedPresetManager = TTObject(kTTSym_PresetManager, returnLineCallback);
	return kTTErrNone;
}

/**	Create a cue manager object */
TTErr jamoma_cueManager_create(t_object *x, TTObject& returnedCueManager)
{
	TTObject returnLineCallback;
	
	// prepare arguments
	returnLineCallback = TTObject("callback");
	returnLineCallback.set(kTTSym_baton, TTPtr(x));
	returnLineCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	
	returnedCueManager = TTObject(kTTSym_CueManager, returnLineCallback);
	return kTTErrNone;
}

// Method to deal with TTInput
///////////////////////////////////////////////////////////////////////

/**	Create an input object for any signal */
TTErr jamoma_input_create(t_object *x, TTObject& returnedInput)
{	
	TTValue		baton;
	TTObject	signalOutCallback;
	
	// prepare arguments
	signalOutCallback = TTObject("callback");
	baton = TTValue(TTPtr(x), TTPtr(jps_return_signal));
	signalOutCallback.set(kTTSym_baton, baton);
	signalOutCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value_typed));
	
	returnedInput = TTObject(kTTSym_Input, signalOutCallback);
	returnedInput.set("type", TTSymbol("control"));
	return kTTErrNone;
}

/**	Create an input object for audio signal */
TTErr jamoma_input_create_audio(t_object *x, TTObject& returnedInput)
{
	returnedInput = TTObject("Input.audio");
    return kTTErrNone;
}

/**	Send any signal to an input object */
TTErr jamoma_input_send(TTObject& anInput, t_symbol *msg, long argc, const t_atom *argv)
{	
	TTValue v, none;
	
	if (anInput.valid()) {
		
		jamoma_ttvalue_from_Atom(v, msg, argc, argv);
		
		return anInput.send(kTTSym_Send, v, none);
	}
	
	return kTTErrGeneric;
}


// Method to deal with TTOutput
///////////////////////////////////////////////////////////////////////

/**	Create an output object for signal */
TTErr jamoma_output_create(t_object *x, TTObject& returnedOutput)
{	
	TTValue		baton;
	TTObject	signalOutCallback;
	
	// prepare arguments
	signalOutCallback = TTObject("callback");
	baton = TTValue(TTPtr(x), TTPtr(jps_return_signal));
	signalOutCallback.set(kTTSym_baton, baton);
	signalOutCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value_typed));
	
	returnedOutput = TTObject(kTTSym_Output, signalOutCallback);
    returnedOutput.set("type", TTSymbol("control"));
	return kTTErrNone;
}

/**	Create an output object for audio signal */
TTErr jamoma_output_create_audio(t_object *x, TTObject& returnedOutput)
{
    // prepare arguments
	returnedOutput = TTObject("Output.audio");
    return kTTErrNone;
}

/**	Send any signal to an output object */
TTErr jamoma_output_send(TTObject& anOutput, t_symbol *msg, long argc, const t_atom *argv)
{	
	TTValue v, none;
	
	if (anOutput.valid()) {
		
		jamoma_ttvalue_from_Atom(v, msg, argc, argv);
		
		return anOutput.send(kTTSym_Send, v, none);
	}
	
	return kTTErrGeneric;
}


// Method to deal with TTMapper
///////////////////////////////////////////////////////////////////////

/**	Create a mapper object */
TTErr jamoma_mapper_create(t_object *x, TTObject& returnedMapper)
{
	TTValue		args, baton, none;
	TTObject	returnValueCallback, returnInputGoingDownCallback, returnInputGoingUpCallback, returnOutputGoingDownCallback, returnOutputGoingUpCallback;
	
	// prepare arguments
	returnValueCallback = TTObject("callback");
	returnValueCallback.set(kTTSym_baton, TTPtr(x));
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnValueCallback);
    
    returnInputGoingDownCallback = TTObject("callback");
    baton = TTValue(TTPtr(x), TTPtr(gensym("return_input_going_down")));
	returnInputGoingDownCallback.set(kTTSym_baton, baton);
	returnInputGoingDownCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnInputGoingDownCallback);
    
    returnInputGoingUpCallback = TTObject("callback");
    baton = TTValue(TTPtr(x), TTPtr(gensym("return_input_going_up")));
	returnInputGoingUpCallback.set(kTTSym_baton, baton);
	returnInputGoingUpCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnInputGoingUpCallback);
    
    returnOutputGoingDownCallback = TTObject("callback");
    baton = TTValue(TTPtr(x), TTPtr(gensym("return_output_going_down")));
	returnOutputGoingDownCallback.set(kTTSym_baton, baton);
	returnOutputGoingDownCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnOutputGoingDownCallback);
    
    returnOutputGoingUpCallback = TTObject("callback");
    baton = TTValue(TTPtr(x), TTPtr(gensym("return_output_going_up")));
	returnOutputGoingUpCallback.set(kTTSym_baton, baton);
	returnOutputGoingUpCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnOutputGoingUpCallback);
	
	returnedMapper = TTObject(kTTSym_Mapper, args);
	
	return kTTErrNone;
}


// Method to deal with TTViewer
///////////////////////////////////////////////////////////////////////

/**	Create a viewer object */
TTErr jamoma_viewer_create(t_object *x, TTObject& returnedViewer)
{
	returnedViewer = TTObject(kTTSym_Viewer);
    
    returnedViewer.set(kTTSym_baton, TTPtr(x));
	returnedViewer.set(kTTSym_function, TTPtr(&jamoma_callback_return_value_typed));
	
	return kTTErrNone;
}

/**	Send Max data using a viewer object */
TTErr jamoma_viewer_send(TTObject& aViewer, t_symbol *msg, long argc, const t_atom *argv)
{
	TTValue v, none;
	
	if (aViewer.valid()) {
		
		jamoma_ttvalue_from_Atom(v, msg, argc, argv);
		
		return aViewer.send(kTTSym_Send, v, none);
	}
	
	return kTTErrGeneric;
}


// Method to deal with TTExplorer
///////////////////////////////////////////////////////////////////////

/**	Create an explorer object */
TTErr jamoma_explorer_create(t_object *x, TTObject& returnedExplorer)
{
	TTValue		args, baton;
	TTObject	returnValueCallback, returnSelectionCallback;
	
	// prepare arguments
	returnValueCallback = TTObject("callback");
	returnValueCallback.set(kTTSym_baton, TTPtr(x));
	returnValueCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnValueCallback);
	
	args.append((TTPtr)jamoma_explorer_default_filter_bank());

	returnSelectionCallback = TTObject("callback");
    baton = TTValue(TTPtr(x), (TTPtr)gensym("return_selection"));
	returnSelectionCallback.set(kTTSym_baton, baton);
	returnSelectionCallback.set(kTTSym_function, TTPtr(&jamoma_callback_return_value));
	args.append(returnSelectionCallback);
	
	returnedExplorer = TTObject(kTTSym_Explorer, args);
	
	return kTTErrNone;
}

TTHashPtr jamoma_explorer_default_filter_bank(void)
{
	TTHashPtr defaultFilterBank = new TTHash();
	TTDictionaryBasePtr aFilter;
	
	// Create some ready-made filters
	
	// to look for any data (parameter | message | return)
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Data);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("data"), (TTPtr)aFilter);
	
	// to look for j.parameter
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Data);
	aFilter->append(kTTSym_attribute, kTTSym_service);
	aFilter->append(kTTSym_value, kTTSym_parameter);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("parameter"), (TTPtr)aFilter);
	
	// to look for j.message
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Data);
	aFilter->append(kTTSym_attribute, kTTSym_service);
	aFilter->append(kTTSym_value, kTTSym_message);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("message"), (TTPtr)aFilter);
	
	// to look for j.return
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Data);
	aFilter->append(kTTSym_attribute, kTTSym_service);
	aFilter->append(kTTSym_value, kTTSym_return);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("return"), (TTPtr)aFilter);
	
	// to look for j.model
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Container);
	aFilter->append(kTTSym_attribute, kTTSym_service);
	aFilter->append(kTTSym_value, kTTSym_model);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("model"), (TTPtr)aFilter);
	
	// to look for j.view
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Container);
	aFilter->append(kTTSym_attribute, kTTSym_service);
	aFilter->append(kTTSym_value, kTTSym_view);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("view"), (TTPtr)aFilter);
	
	// to look for empty nodes
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, TTSymbol("none"));
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("none"), (TTPtr)aFilter);
	
	// to look for j.remote
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_object, kTTSym_Viewer);
	aFilter->append(kTTSym_mode, kTTSym_include);
	defaultFilterBank->append(TTSymbol("remote"), (TTPtr)aFilter);
	
	// to look for user-defined object
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_attribute, kTTSym_tags);
	aFilter->append(kTTSym_value, kTTSym_generic);
	aFilter->append(kTTSym_mode, kTTSym_exclude);
	defaultFilterBank->append(TTSymbol("noGenericTag"), (TTPtr)aFilter);
	
	// to look for generic tagged object
	aFilter = new TTDictionaryBase;
	aFilter->setSchema(kTTSym_filter);
	aFilter->append(kTTSym_attribute, kTTSym_tags);
	aFilter->append(kTTSym_value, kTTSym_generic);
	aFilter->append(kTTSym_mode, kTTSym_restrict);
	defaultFilterBank->append(TTSymbol("genericTag"), (TTPtr)aFilter);
	
	return defaultFilterBank;
}


// Method to deal with TTRamp
///////////////////////////////////////////////////////////////////////
TTErr jamoma_ramp_create(t_object *x, TTObject& returnedRamp)
{
    TTValue args;
    
    // prepare arguments
    args.append((TTPtr)&jamoma_callback_return_ramped_value);
    args.append((TTPtr)x);
    
    returnedRamp = TTObject(kTTSym_Ramp, args);
    return kTTErrNone;
}

void JAMOMA_EXPORT jamoma_callback_return_ramped_value(void *o, TTUInt32 n, TTFloat64 *v)
{
    t_object	*x = (t_object*)o;
    long		i, argc = n;
	t_atom      *argv = NULL;
    
    if (!argc)
        return;
    
    // copy the array of ramped float value into an atom
    argv = (t_atom*)malloc(sizeof(t_atom) * argc);
	
	for (i = 0; i < argc; i++)
            atom_setfloat(argv+i, v[i]);

    // send the atom to the object using return_value method
    if (argc == 1)
		object_method_typed(x, jps_return_value, _sym_float, argc, argv);
    else
		object_method_typed(x, jps_return_value, _sym_list, argc, argv);

    free(argv);
}

// Method to return data
///////////////////////////////////////////////////////////////////////

void jamoma_callback_return_address(const TTValue& baton, const TTValue& v)
{
	t_object	*x;
	TTSymbol	address;
	
	// unpack baton (a t_object *and the name of the method to call)
	x = (t_object*)((TTPtr)baton[0]);
    
    if (v.size() == 1) {
        
        if (v[0].type() == kTypeSymbol) {
            
            // unpack data (address)
            address = v[0];
            
            // send data to a data using the return_value method
			object_method_typed(x, jps_return_address, SymbolGen(address.c_str()), 0, 0);
        }
    }
}

/** Return the value to a j. external as _sym_nothing, argc, argv */
void jamoma_callback_return_value(const TTValue& baton, const TTValue& v)
{
	t_object	*x;
	t_symbol	*s_method;
    TTBoolean   deferlow = NO;
	long		argc = 0;
	t_atom      *argv = NULL;
    method      p_method = NULL;
	
	// unpack baton (a t_object *and the name of the method to call (default : jps_return_value))
    
    // get object
	x = (t_object*)((TTPtr)baton[0]);
	
    // get method
	if (baton.size() >= 2) {
        
		s_method = (t_symbol*)((TTPtr)baton[1]);
        
		if (s_method == NULL || s_method == _sym_nothing)
			return;
        
        // do we need to deferlow it ?
        if (baton.size() == 3)
            deferlow = baton[2];
    }
	else
		s_method = jps_return_value;
    
	jamoma_ttvalue_to_Atom(v, &argc, &argv);
	
	// send data to an external
    if (!deferlow)
		object_method_typed(x, s_method, _sym_nothing, argc, argv);
    
    else {
        
        p_method = object_getmethod(x, s_method);
        
        if (p_method)
            p_method(x, _sym_nothing, argc, argv);
    }
	
    sysmem_freeptr(argv);
}

/** Return the value to a j. external as msg, argc, argv */
// TODO #556 : merge this function into jamoma_callback_return_value
// TODO : why don't we pass a #TTDictionary into the baton
void jamoma_callback_return_value_typed(const TTValue& baton, const TTValue& v)
{
	t_object	*x;
	t_symbol	*msg, *s_method;
    TTBoolean   deferlow = NO;
	long		argc = 0;
	t_atom      *argv = NULL;
    method      p_method = NULL;
	TTBoolean	shifted = false;
	
	// unpack baton (a t_object *and the name of the method to call (default : jps_return_value))
    
     // get Max wrapper object that was passed in
	x = (t_object*)((TTPtr)baton[0]);
	
     // get name of the method to call
	if (baton.size() >= 2) {
        
		s_method = (t_symbol*)((TTPtr)baton[1]);
        
		if (s_method == NULL || s_method == _sym_nothing)
			return;
        
        // do we need to deferlow it ?
        if (baton.size() == 3)
            deferlow = baton[2];
    }
    // by default we call "return_value" method
	else
		s_method = jps_return_value;
	
	// convert TTValue into a typed atom array
	jamoma_ttvalue_to_typed_Atom(v, &msg, &argc, &argv, shifted);
	
	// send data to an external
    if (!deferlow)
		object_method_typed(x, s_method, msg, argc, argv);
    
    else {
        
        p_method = object_getmethod(x, s_method);
        
        if (p_method)
            p_method(x, msg, argc, argv);
    }
	
	if (shifted)
        argv--;
    sysmem_freeptr(argv);
}

/** Return any signal */
void jamoma_callback_return_signal(const TTValue& baton, const TTValue& v)
{
	t_object	*x;
	long		argc = 0;
	t_atom      *argv = NULL;
	
	// unpack baton (a t_object*)
	x = (t_object*)((TTPtr)baton[0]);
	
	jamoma_ttvalue_to_Atom(v, &argc, &argv);
	
	// send signal using the return_signal method
	object_method_typed(x, jps_return_signal, _sym_nothing, argc, argv);
	
	sysmem_freeptr(argv);
}

/** Return audio signal */
/** disable audio support for now
void jamoma_callback_return_signal_audio(const TTValue& baton, const TTValue& v)
{
	t_object	*x;
	TTPtr		signal;
	long		i, argc = 0;
	t_atom      *argv = NULL;
	
	// unpack baton (a t_object*)
	x = (t_object*)((TTPtr)baton[0]);
	
	// unpack data (signal)
	argc = v.size();
	argv = (t_atom*)sysmem_newptr(sizeof(t_atom) * argc);
	for (i = 0; i < argc; i++) {
		signal = v[i];
		atom_setobj(argv+i, signal);
	}
	
	// send signal using the return_signal method
	object_method(x, jps_return_signal, _sym_nothing, argc, argv);
	
	sysmem_freeptr(argv);
}
**/

// Method to deal with TTValue
/////////////////////////////////////////

/** Make a typed Atom array from a TTValue (!!! this method allocate memory for the Atom array ! free it after ! */
void jamoma_ttvalue_to_typed_Atom(const TTValue& v, t_symbol **msg, long *argc, t_atom **argv, TTBoolean& shifted)
{
	long        i;
	TTFloat64	f;
	TTSymbol	s;
	
	*msg = _sym_nothing;
	*argc = v.size();
	
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom*)sysmem_newptr(sizeof(t_atom) * (*argc));
	
	if (*argc && v.size() > 0) {
		
		for (i = 0; i < *argc; i++) {
			
			if(v[i].type() == kTypeSymbol){
				s = v[i];
				if (s == kTTSymEmpty || s == kTTAdrsEmpty)
					atom_setsym((*argv)+i, _sym_bang);
				else
					atom_setsym((*argv)+i, gensym((char*)s.c_str()));
				//*msg = _sym_symbol;
			}
			else{	// assume int
				f = v[i];
				atom_setfloat((*argv)+i, f);
				*msg = _sym_float;
			}
		}
		
		if (i>0) {
			
			if (atom_gettype(*argv) == A_SYM) {
				
				*msg = atom_getsym(*argv);
				*argc = (*argc)-1;
				*argv = (*argv)+1;
				shifted = true;
			}
			else if (i>1)
				*msg = _sym_list;
		}
	}
	else {
		
		*msg = _sym_bang;
		*argc = 0;
		*argv = NULL;
	}
}

/** Make an Atom array from a TTValue (!!! this method allocate memory for the Atom array ! free it after ! */
void jamoma_ttvalue_to_Atom(const TTValue& v, long *argc, t_atom **argv)
{
	long        i;
	TTFloat64	f;
	TTSymbol	s;
	
	*argc = v.size();
	
	if (*argc == 0 || v.size() == 0)
		return;
	
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom*)sysmem_newptr(sizeof(t_atom) * (*argc));
	
	for (i = 0; i < v.size(); i++)
    {
		if (v[i].type() == kTypeSymbol)
        {
			s = v[i];
            if (s == kTTSymEmpty)
                atom_setsym((*argv)+i, _sym_bang); // because empty symbol can't be filtered in Max
			else
                atom_setsym((*argv)+i, gensym((char*)s.c_str()));
		}
		else
		{
			f = v[i];
			atom_setfloat((*argv)+i, f);
		}
	}
}

/** Make a TTValue from Atom array */
void jamoma_ttvalue_from_Atom(TTValue& v, t_symbol *msg, long argc, const t_atom *argv)
{
	long i, start;
	
	if ((msg == _sym_bang || msg == _sym_nothing) && argc == 0)
		v.clear();
	else
    {
		// add msg to the value
        if (msg && msg != _sym_nothing && msg != _sym_int && msg != _sym_float && msg != _sym_symbol && msg != _sym_list)
        {
			v.resize(argc+1);
			v[0] = TTSymbol(msg->s_name);
			start = 1;
		}
		else
        {
			v.resize(argc);
			start = 0;
		}
			
		// convert Atom to TTValue
		for (i = 0; i < argc; i++)
		{
            if (atom_gettype(argv+i) == A_SYMBOL){
                if ( atom_getsym((t_atom*)argv+i)->s_name[0] == '"'){
                    std::string concatSym = std::string(atom_getsym((t_atom*)argv+i)->s_name+1);
                    int j = i;
                    i++;
                    for (;i < argc; i++){
                        if (argv[i].a_type == A_SYMBOL) {
                            char* sym = atom_getsym((t_atom*)argv+i)->s_name;
                            concatSym += std::string(" ");
                            char* c = strchr(sym,'"');
                            if (c) {
                                concatSym+=sym;
                                break;
                            }
                            concatSym+=sym;
                        } else if ( argv[i].a_type == A_FLOAT ) {
                            concatSym += std::string(" ");
                            concatSym+= std::to_string(atom_getfloat((t_atom*)argv+i));
                        }
                    }
                    v[j+start] = TTSymbol(concatSym.substr(0,concatSym.size()-1));
                }
                else
                    v[i+start] = TTSymbol(atom_getsym((t_atom*)argv+i)->s_name);
            }
			else
                v[i+start] = (TTFloat64)atom_getfloat((t_atom*)argv+i); 
		}
	}
}

/** Convert a TTSymbol "MyObjectMessage" into a t_symbol *"my/object/message" 
 or return NULL if the TTSymbol doesn't begin by an uppercase letter */
t_symbol *jamoma_TTName_To_PdName(TTSymbol TTName)
{
    TTSymbol    AppName;
    TTAddress   anAddress;
    TTErr       err;
    
    AppName = ToAppName(TTName);
    
    if (AppName == kTTSymEmpty)
        err = convertUpperCasedNameInAddress(TTName, anAddress);
    else
        err = convertUpperCasedNameInAddress(AppName, anAddress);
    
	if (!err)
		return gensym((char*)anAddress.c_str());
	else
		return NULL;
}

/** Load an external for internal use. Returns true if successful */
/* AV - seems to be unused
TTBoolean jamoma_extern_load(t_symbol *objectname, long argc, t_atom *argv, t_object **object)
{
	t_class *c = NULL;
	t_object *p = NULL;
	
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
	
	if (*object != NULL) { // if there was an object set previously, free it first...
		object_free(*object);
		*object = NULL;
	}
	
	*object = (t_object*)object_new_typed(CLASS_BOX, objectname, argc, argv);
	return true;
}
*/

// Don't pass memory in for this function!  It will allocate what it needs
// -- then the caller is responsible for freeing
void jamoma_patcher_get_args(t_canvas *canvas, long *argc, t_atom **argv)
{
    t_binbuf *b = NULL;

    b = canvas->gl_obj.te_binbuf;

    if(b)
    {
        long ac = binbuf_getnatom(b);
        t_atom* av = binbuf_getvec(b);
        if(atom_gettype(av) == A_SYM && atom_getsym(av) == gensym("pd"))
        {
            ac--;
            av++;
        }
        *argc = ac;
        *argv = av;
    } else {
        *argc = 0;
        *argv = NULL;
    }
}

/** Get the context from the upper j.model|view in the patcher or from patcher's name */
void jamoma_patcher_get_context(t_canvas *patcher, TTSymbol& returnedContext)
{
	t_symbol	*hierarchy, *_sym_j_context;
    t_canvas    *upperPatcher;
	TTBoolean	found = NO;
	
	t_gobj *obj=patcher->gl_list;

	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;
		if (_sym_j_context == _sym_j_model ){

			returnedContext = kTTSym_model;
			found = YES;
			break;

		} else if ( _sym_j_context == _sym_j_view ) {

			returnedContext = kTTSym_view;
			found = YES;
			break;
		}
	}
	
	// if no context
	if (!found) {
		
		// in subpatcher look upper
		hierarchy = jamoma_patcher_get_hierarchy(patcher);
		if (hierarchy == _sym_subpatcher || hierarchy == _sym_bpatcher || hierarchy == SymbolGen("poly")) {
			
			// get the patcher where is the patcher to look for the context one step upper
			upperPatcher = patcher->gl_owner;
			
			jamoma_patcher_get_context(upperPatcher, returnedContext);
			
			// keep the upperPatcher if no j.model|view around
            // because it is where the context is defined
            patcher = upperPatcher;
		}
		// default case : a patcher has no type
		else if (hierarchy == _sym_topmost)
			returnedContext = kTTSymEmpty;
	}
}

/** Get the class of the patcher from the file name (removing .model and .view convention name if they exist) */
void jamoma_patcher_get_class(t_canvas *patcher, TTSymbol context, TTSymbol& returnedClass)
{
	t_symbol	*patcherName, *hierarchy;
    t_canvas	*upperPatcher;
	TTString	s_toParse;
	TTStringIter begin, end;
    TTBoolean   intoHelp = NO;
	
	// extract class from the file name
	if(patcher->gl_name)
		patcherName = patcher->gl_name;
	else
		patcherName =  _sym_nothing;
	
	if (patcherName != _sym_nothing) {
		
		s_toParse = patcherName->s_name;
	
		begin = s_toParse.begin();
		end = s_toParse.end();
        
        // parse .maxpat
        if (!ttRegexForPdpat->parse(begin, end)) {
            s_toParse = TTString(begin, ttRegexForPdpat->begin());
			begin = s_toParse.begin();
			end = s_toParse.end();
		}
		// parse .maxhelp
        else if (!ttRegexForPdhelp->parse(begin, end)) {
            s_toParse = TTString(begin, ttRegexForPdhelp->begin());
			begin = s_toParse.begin();
			end = s_toParse.end();
            
            intoHelp = YES;
		}

		// parse jmod.
		if (!ttRegexForJmod->parse(begin, end)) {
			
			pd_error(patcher, "jmod. prefix in %s is a 0.5 convention. Please use .module suffix instead", patcherName->s_name);
			
			s_toParse = TTString(ttRegexForJmod->end(), end);
			begin = s_toParse.begin();
			end = s_toParse.end();
		}
		// parse j.
		else if (!ttRegexForJcom->parse(begin, end)) {
			
			s_toParse = TTString(ttRegexForJcom->end(), end);
			begin = s_toParse.begin();
			end = s_toParse.end();
		}
		
		// parse .module or if not parse context (model or view)
		if (!ttRegexForModule->parse(begin, end)) {
			s_toParse = TTString(begin, ttRegexForModule->begin());
			begin = s_toParse.begin();
			end = s_toParse.end();
		}
		else if (context == kTTSym_model) {
			
			if (!ttRegexForModel->parse(begin, end)) {
				s_toParse = TTString(begin, ttRegexForModel->begin());
				begin = s_toParse.begin();
				end = s_toParse.end();
			}
		}
		else if (context == kTTSym_view) {
			
			if (!ttRegexForView->parse(begin, end)) {
				s_toParse = TTString(begin, ttRegexForView->begin());
				begin = s_toParse.begin();
				end = s_toParse.end();
			}
		}
		
        // in help patcher : append _help to the class to clarify the namespace
        if (intoHelp)
            s_toParse += "_help";
        
		returnedClass = TTSymbol(s_toParse);
	}
	else {
		
		// in subpatcher : look to the name else look upper
		hierarchy = jamoma_patcher_get_hierarchy(patcher);
		if (hierarchy == _sym_subpatcher) {
            
            long    ac = 0;
            t_atom  *av = NULL;
            
            jamoma_patcher_get_args(patcher, &ac, &av);
            
            // remove first 'pd'
            if (ac > 0 && av) {
                if (atom_getsym(av) == _sym_pd) {
                    ac--;
                    av++;
                }
            }
            
            // is there still arguments ?
            if (ac > 0 && av) {
                
                // the first argument is the class
                returnedClass = TTSymbol(atom_getsym(av)->s_name);
            }
            else {
                
                // get the patcher where is the patcher to look for the class one step upper
				upperPatcher = patcher->gl_owner;
			
                jamoma_patcher_get_class(upperPatcher, context, returnedClass);
            }
		}
        // in bpatcher : look upper
        else if (hierarchy == _sym_bpatcher) {
            
            // get the patcher where is the patcher to look for the class one step upper
			upperPatcher = patcher->gl_owner;
			
            jamoma_patcher_get_class(upperPatcher, context, returnedClass);
        }
		// default case : a patcher has no class
		else if (hierarchy == _sym_topmost)
			returnedClass = kTTSymEmpty;
	}
}

/** Get the name of the patcher from his arguments */
void jamoma_patcher_get_name(t_canvas *patcher, TTSymbol context, TTSymbol& returnedName)
{
	long		ac = 0;
	t_atom      *av = NULL;
	t_symbol	*hierarchy, *argName = _sym_nothing;
	
	// returnedName = TTSymbol(canvas->gl_name->s_name);
	returnedName = kTTSymEmpty;
	
	// try to get context name from the patcher arguments
	hierarchy = jamoma_patcher_get_hierarchy(patcher);
	jamoma_patcher_get_args(patcher, &ac, &av);
	
	// for subpatcher :
	if (hierarchy == _sym_subpatcher) {
        
        // remove first 'pd'
        if (ac > 0 && av) {
            if (atom_getsym(av) == _sym_pd) {
                ac--;
                av++;
            }
        }
        
        // remove the next argument because it is the class
		ac--;
        av++;
	}
	
    // is there still arguments ?
	if (ac > 0 && av) {
        
        // notice we have to test view case before model case 
        // because a j.view can be in subpatcher too
        
        // for view : the second argument is the name
		// (the first is reserved for the model:address)
        if (context == kTTSym_view) {
            if (ac > 1)
                argName = atom_getsym(av+1);
		
		// for model : the first argument is the name
		} else if (context == kTTSym_model || hierarchy == _sym_subpatcher)
			argName = atom_getsym(av);
		
		// if the argname begin by a @ ignore it
		if (argName->s_name[0] == '@')
			argName = _sym_nothing;
		
		if (argName != _sym_nothing)
			returnedName = TTSymbol(jamoma_parse_dieze(patcher, argName)->s_name);
	}
}

/** Get all context info from the root j.model|view in the patcher */
void jamoma_patcher_share_info(t_canvas *patcher, t_canvas **returnedPatcher, TTSymbol& returnedContext, TTSymbol& returnedClass,  TTSymbol& returnedName)
{
	TTValue		patcherInfo;
	t_symbol	*_sym_j_context;
	
	t_gobj *obj=patcher->gl_list;

	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;
		if (_sym_j_context == _sym_j_model || _sym_j_context == _sym_j_view) {

            t_ret_method _method = (t_ret_method)zgetfn(&obj->g_pd,_sym_share_patcher_info);
			if ( _method )
				_method(&obj->g_pd, &patcherInfo);

			if (patcherInfo.size() == 4) {
				*returnedPatcher = (t_canvas*)((TTPtr)patcherInfo[0]);
				returnedContext = patcherInfo[1];
				returnedClass = patcherInfo[2];
				returnedName = patcherInfo[3];
				break;
			}
		}
	}

}

/** Get j.model or j.view of a patcher */
void jamoma_patcher_get_model_or_view(t_canvas *patcher, t_object **returnedModelOrView)
{
	TTValue		patcherInfo;
	t_symbol	*_sym_j_context;
    
    *returnedModelOrView = NULL;

	t_gobj *obj=patcher->gl_list; // objects list

	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;
		if (_sym_j_context == _sym_j_model || _sym_j_context == _sym_j_view) {

			*returnedModelOrView = (t_object*) obj;
			break;
		}
	}

	return (void)returnedModelOrView;
}

/** Look for input and output (data and audio) */
void jamoma_patcher_get_input_output(t_canvas *patcher, TTBoolean& dataInput, TTBoolean& dataOutput, TTBoolean& audioInput, TTBoolean& audioOutput)
{
	TTValue		patcherInfo;
	t_symbol	*_sym_jcomcontext;
    
    dataInput = NO;
    dataOutput = NO;
    audioInput = NO;
    audioOutput = NO;
	
	t_gobj *obj=patcher->gl_list; // pointer to the first object
	
	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_jcomcontext = obj->g_pd->c_name;
		
		dataInput = dataInput || _sym_jcomcontext == _sym_j_in;
		dataOutput = dataOutput || _sym_jcomcontext == _sym_j_out;
		audioInput = audioInput || _sym_jcomcontext == _sym_j_intilda;
		audioOutput = audioOutput || _sym_jcomcontext == _sym_j_outtilda;
	}
}

/** Is there a j.ui object */
TTBoolean jamoma_patcher_get_ui(t_canvas *patcher)
{
	t_symbol	*_sym_j_context;
    TTBoolean   uiObject = NO;

	t_gobj *obj=patcher->gl_list;

	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;
		uiObject = _sym_j_context == _sym_j_ui;
		if ( uiObject ) {
			break;
		}
	}
    
    return uiObject;
}

/** Get the "aClass.model" external in the patcher */
void jamoma_patcher_get_model_patcher(t_canvas *patcher, TTSymbol modelClass, t_object **returnedModelPatcher)
{
	t_symbol	*_sym_modelfilename, *_sym_j_context, *_sym_objfilename;
	
	jamoma_edit_filename(*ModelPatcherFormat, modelClass, &_sym_modelfilename);
		
	*returnedModelPatcher = NULL;

	t_gobj *obj=patcher->gl_list; // pointer to the first object

	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;
		if (_sym_j_context == _sym_jpatcher) {

			// look for _sym_modelfilename
			_sym_objfilename = patcher->gl_name;
			if (_sym_objfilename == _sym_modelfilename) {

				*returnedModelPatcher = (t_object*)obj->g_pd;
				break;
			}
		}
	}
}

/** Get patcher's node from the root j.model|view in the patcher */
void jamoma_patcher_share_node(t_canvas *patcher, TTNodePtr *patcherNode)
{
	t_symbol	*_sym_j_context;
	
	*patcherNode = NULL;
	
	t_gobj *obj=patcher->gl_list; // pointer to the first object

//    obj = (t_object*)object_attr_getobj(patcher, _sym_firstobject);
	for ( ; obj ;  obj = obj->g_next )
	{
		_sym_j_context = obj->g_pd->c_name;

		if (_sym_j_context == _sym_j_model || _sym_j_context == _sym_j_view) {
            t_ret_method _method = (t_ret_method)zgetfn(&obj->g_pd,_sym_share_patcher_info);
			if ( _method )
				_method(&obj->g_pd, patcherNode);
			if (*patcherNode)
				break;
		}
	}
}

/** Get all context info from an object (his patcher and the context, the class and the name of his patcher) */
TTErr jamoma_patcher_get_info(t_canvas *obj, t_canvas **returnedPatcher, TTSymbol& returnedContext, TTSymbol& returnedClass, TTSymbol& returnedName)
{
	TTBoolean	canShare;
	t_symbol	*_sym_j_context;
	TTString	viewName;
    t_canvas	*patcher;
    t_canvas	*sharedPatcher = NULL;
	TTSymbol	sharedContext;
	TTSymbol	sharedClass;
	TTSymbol	sharedName;
	
	// t_eobj* x = (t_eobj*) obj;
	*returnedPatcher = obj;

	_sym_j_context = object_classname(obj);
	canShare = _sym_j_context == gensym("j.model") || _sym_j_context == gensym("j.view");
	
	patcher = *returnedPatcher;

	// Get the context, the class and the name of the patcher
	if (*returnedPatcher) {
		
		// try to get them from a j.model|view around to go faster (except for j.model|view of course)
		if (!canShare) {
			
            jamoma_patcher_share_info(*returnedPatcher, &sharedPatcher, sharedContext, sharedClass, sharedName);
			
			if (sharedPatcher && sharedContext && sharedClass && sharedName) {

				*returnedPatcher = sharedPatcher;
				returnedContext = sharedContext;
				returnedClass = sharedClass;
				returnedName = sharedName;
				return kTTErrNone;
			}
		}
		
		// get the context looking for a j.model|view in the patcher
		// it will also return a patcher above where a j.model|view has been found
		jamoma_patcher_get_context(*returnedPatcher, returnedContext);
		
		// if still no context : stop the subscription process
		if (returnedContext == kTTSymEmpty) {
            
			returnedName = S_SEPARATOR;
            returnedClass = kTTSymEmpty;
            
			// can't find any j.model|view with a correct context attribute in the patcher
			// so this means the object have to be registered under the root
			return kTTErrGeneric;
		}
		
		// get the class from the patcher filename
		jamoma_patcher_get_class(*returnedPatcher, returnedContext, returnedClass);
		
		// if no class : set it as "Untitled" to continue the process
		if (returnedClass == kTTSymEmpty)
			returnedClass = TTSymbol("Untitled");
		
		// for j.model|view object, use the patcher where it is to get the name
		if (canShare)
			jamoma_patcher_get_name(patcher, returnedContext, returnedName);
		// else get the name from the argument of the patcher
		else
			jamoma_patcher_get_name(*returnedPatcher, returnedContext, returnedName);
		
		// if no name
		if (returnedName == kTTSymEmpty) {
			
			// for model : used "class"
			if (returnedContext == kTTSym_model)
				returnedName = returnedClass;
			
			// for view : used "class(view)"
			else if (returnedContext == kTTSym_view) {
				viewName = returnedClass.c_str();
				viewName += "(view)";
				returnedName = TTSymbol(viewName.data());
			}
            
            // format name coming from class name replacing '.' and ' ' by '_'
            TTString s_toParse = returnedName.c_str();
            std::replace(s_toParse.begin(), s_toParse.end(), '.', '_');
            std::replace(s_toParse.begin(), s_toParse.end(), ' ', '_');
            
            /*
             * AV - there is no poly in pd
            // in poly case, the index is used to edit an instance
			// according to the voice index of the poly~
            if (jamoma_patcher_get_hierarchy(patcher) == gensym("poly")) {
                
                t_object	*assoc = NULL;
                method		m = NULL;
                
                object_method(patcher, gensym("getassoc"), &assoc);
                if (assoc) {

                    m = zgetfn(assoc, gensym("getindex"));
                    if(m) {
                        
                        char    *s_num;
                        long    index = (long)(*m)(assoc, patcher);
                        TTInt32 len;
                        
                        s_toParse += ".%d";
                        len = s_toParse.size() + (TTInt32)log10((TTFloat32)index); // note : %d (lenght = 2) is replaced by 1 character (0::9), 2 charecters (10 :: 99), 3 char...
                        s_num = (char *)malloc(sizeof(char)*len);
                        snprintf(s_num, len, s_toParse.c_str(), index);
                        s_toParse = s_num;
                        free(s_num);
                    }
                }
            }
            */
            
            returnedName = TTSymbol(s_toParse);
		}
	}
		// if no patcher : stop the subscription process
	else {
		pd_error(obj, "Can't get the patcher. Subscription failed");
		return kTTErrGeneric;
	}

	return kTTErrNone;
}

/** returned the N inside "pp/xx.[N]/yyy" and a format string as "pp/xx.%d/yy" and a format string as "pp/xx.%s/yy" */
TTUInt32 jamoma_parse_bracket(t_symbol *s, TTString& si_format, TTString& ss_format)
{
	long		number = 0;
	TTString	s_toParse = s->s_name;
	TTString	s_number;
	TTString	s_before;
	TTString	s_after;
	TTStringIter begin, end;
	TTStringIter beginNumber, endNumber;
	
	begin = s_toParse.begin();
	end = s_toParse.end();
	
	// parse braket
	if (!ttRegexForBracket->parse(begin, end))
	{
		beginNumber = ttRegexForBracket->begin();
		endNumber = ttRegexForBracket->end();
		
		s_before = TTString(begin, ttRegexForBracket->begin()-1);
		s_number = TTString(ttRegexForBracket->begin(), ttRegexForBracket->end());
		s_after = TTString(ttRegexForBracket->end()+1, end);
		
		sscanf(s_number.c_str(), "%ld", &number);
		
		si_format = s_before;
		si_format += "%d";
		si_format += s_after;
		
		ss_format = s_before;
		ss_format += "%s";
		ss_format += s_after;
	}
	else {
		si_format = "";
		ss_format = "";
	}
	
	return number;
}

/** edit a new instance of the given format address using interger */
void jamoma_edit_numeric_instance(TTString format, t_symbol** returnedName, long i)
{
	char *s_num;
	TTInt32 len;
	
	if (i > 0) {
		len = format.size() + (TTInt32)log10((TTFloat32)i); // note : %d (lenght = 2) is replaced by 1 character (0::9), 2 charecters (10 :: 99), 3 char...
		s_num = (char *)malloc(sizeof(char)*len);
		snprintf(s_num, len, format.c_str(), i);
		*returnedName = gensym(s_num);
		free(s_num);
	}
}

/** edit a new instance of the given format address using string */
void jamoma_edit_string_instance(TTString format, t_symbol** returnedName, TTString s)
{
    char *s_str;
	long len;
    
	len = format.size() + s.size();
	s_str = (char *)malloc(sizeof(char)*len);
	snprintf(s_str, len, format.c_str(), s.c_str());
	*returnedName = gensym(s_str);
	free(s_str);
}

/** edit a file name from a given file format and a class name */
void jamoma_edit_filename(TTString format, TTSymbol className, t_symbol **returnedFileName)
{
	char *s_str;
	long len;
	
	len = format.size() + className.string().size();
	s_str = (char *)malloc(sizeof(char)*len);
	snprintf(s_str, len, format.c_str(), className.c_str());
	*returnedFileName = gensym(s_str);
	free(s_str);
}


// Files
///////////////////////////////////////////////


/** Get BOOT style filepath from args or, if no args open a dialog to write a file */
TTSymbol jamoma_file_write(t_object *x, long argc, t_atom *argv, char* default_filename)
{
	char 			fullpath[MAXPDSTRING];		// for storing the absolute path of the file
    char            filename[MAXPDSTRING];      // buffer to avoid scrambling argv
	t_symbol        *userpath;
	TTSymbol		result = kTTSymEmpty;
	
	// Give a path ...
	if (argc && argv) {
		if (atom_gettype(argv) == A_SYM) {
			userpath = atom_getsym(argv);
			
			if (userpath != _sym_nothing && userpath != _sym_bang) {
				// Use BOOT style path

                snprintf(filename, MAX_FILENAME_CHARS, userpath->s_name);
                canvas_makefilename(((t_eobj*)x)->o_canvas,filename,fullpath,MAXPDSTRING);
				result = TTSymbol(fullpath);
			}
		}
	} 
	/* TODO rewrite an open panel for Pd ?
	// ... or open a dialog
	if (result == kTTSymEmpty) {
		
		saveas_promptset("Save Preset...");												// Instructional Text in the dialog
		err = saveasdialog_extended(default_filename, &path, &outtype, &filetype, 1);	// Returns 0 if successful
		if (!err) { // User Cancelled
			char posixpath[MAX_PATH_CHARS];
			
			// Create a file using Max API
			path_createsysfile(default_filename, path, filetype, &file_handle);
			
			// Use BOOT style path
			path_topathname(path, default_filename, fullpath);
			path_nameconform(fullpath, posixpath, PATH_STYLE_NATIVE, PATH_TYPE_BOOT);
			
			result = TTSymbol(posixpath);
		}
	}
	*/
	
	return result;
}

/** Get BOOT style filepath from args or, if no args open a dialog to read a file */
TTSymbol jamoma_file_read(t_object *x, long argc, t_atom *argv, t_fourcc filetype)
{
	char 			fullpath[MAXPDSTRING];		// path and name passed on to the xml parser
	t_symbol        *userpath;
	TTSymbol		result = kTTSymEmpty;
	
	// Give a path ...
	if (argc && argv) {
		if (atom_gettype(argv) == A_SYM) {
			userpath = atom_getsym(argv);
			
			if (userpath != _sym_nothing && userpath != _sym_bang) {

				canvas_makefilename(((t_eobj*)x)->o_canvas,userpath->s_name,fullpath,MAXPDSTRING);

				result = TTSymbol(fullpath);

//                strcpy(filepath, userpath->s_name);    // must copy symbol before calling locatefile_extended
//                if (locatefile_extended(filepath, &path, &outtype, &filetype, 1)) {     // Returns 0 if successful
                    
//                    pd_error(x, "%s : not found", filepath);
//                    return result;
//                }
			}
		}
	}
	
	// ... or open a dialog
	/* TODO rewrite an open panel for Pd ?
	if (!path)
		open_dialog(filepath, &path, &outtype, &filetype, 1);
	*/

	/*
    if (path) {
        
		open_via_path(path,filepath,"",fullpath,posixpath,MAXPDSTRING,0);
//        path_topathname(path, filepath, fullpath);
//        path_nameconform(fullpath, posixpath, PATH_STYLE_NATIVE, PATH_TYPE_BOOT);
        result = TTSymbol(posixpath);
    }
	*/
	
	return result;
}
