/*
 * bind/bind.cc --
 *
 *	The cocos core module. Provide wrappers for objects
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: bind.cc,v 1.1 1995/10/25 20:09:21 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: bind.cc,v $
// Revision 1.1  1995/10/25  20:09:21  olav
// 1
//
 */

static char rcsid[] = "$Id: bind.cc,v 1.1 1995/10/25 20:09:21 olav Exp $";

#include <tcl.h>
#include <string.h>
#include "bind.h"

static void collectHashedStrings (Tcl_HashTable* table, Tcl_DString* result, int key);

void
Coc_dummyUseParms (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{}

void
Coc_dummyUseParms (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{}

char*
Coc_strdup (char* s)
{
  if (!s) return 0;
  char* t = new char [strlen (s) +1];
  return strcpy (t, s);
}

char*
prepend_dash (char* s)
{
  char* t = new char [strlen (s) +2];
  t [0] = '-';
  strcpy (t+1, s);
  return t;
}

char*
Coc_genId (Coc_Type* type, void* obj)
{
  static char id [512];
  Tcl_CmdInfo cmdInfo;
  do {
  	sprintf (id, "%s_%d", type->name, ++type->maxId);
  } while (Tcl_GetCommandInfo (type->info->interp, id, &cmdInfo));
  return id;
}

int
Coc_intType (Coc_Type* type, Coc_Value& value)
{
  if (value.string) {
	return Tcl_GetInt (type->info->interp, value.string, &value.ival);
  } else {
	static char string [32];
	sprintf (string, "%d", value.ival);
	value.string = string;
	return TCL_OK;
  }
}

int
Coc_voidType (Coc_Type* type, Coc_Value& value)
{ return TCL_OK; }

int
Coc_doubleType (Coc_Type* type, Coc_Value& value)
{
  if (value.string) {
	return Tcl_GetDouble (type->info->interp, value.string, &value.dval);
  } else {
	static char string [32];
	sprintf (string, "%f", value.dval);
	value.string = string;
	return TCL_OK;
  }
}

int
Coc_stringType (Coc_Type* type, Coc_Value& value)
{
  if (value.string) {
#if 0
	static char* string = (char*) 0;
	static long length = 0;
	int new_len = strlen (value.string) +1;
	if (new_len > length) {
		if (string) {
			delete [] string;
		}
		length = new_len;
		string = new char [length];
	}
	strcpy (string, value.string);
#endif
	value.sval = value.string;
  } else {
	value.string = value.sval;
  }
  return TCL_OK;
}

int
Coc_enumType (Coc_EnumType* type, Coc_Value& value)
{
  if (value.string) {
	for (int i = 0; i < type->elementCnt; i++) {
		if (!strcmp (value.string, type->elements [i])) {
			value.ival = i;
			return TCL_OK;
		}
	}
	Tcl_AppendResult (type->info->interp, "\nno such element \"",
			  value.string, "\" in enumeration \"",
			  type->name, "\"", (char*) 0);
	return TCL_ERROR;
  } else {
	if (value.ival < 0 || value.ival >= type->elementCnt) {
		Tcl_AppendResult (type->info->interp, "\nenumeration index out of range",
				  (char*) 0);
		return TCL_ERROR;
	}
	value.string = type->elements [value.ival];
	return TCL_OK;
  }
}

static int
typeIsa (Coc_Type* baseType, char* typeName)
{
  if (!strcmp (baseType->name, typeName)) {
	return 1;
  }
  Coc_Type** ancestor = baseType->ancestorArray;
  while (*ancestor) {
	if (typeIsa (*ancestor, typeName))
		return 1;
	ancestor++;
  }
  return 0;
}

#if 0
static char*
findId (Coc_Type* type, void* obj)
{
  Tcl_HashEntry* hPtr;
  
  if (hPtr = Tcl_FindHashEntry (&type->ids, (char*) obj)) {
	return ((Coc_Object*) Tcl_GetHashValue (hPtr))->name;
  }
  Coc_Type** ancestor = type->ancestorArray;
  char* id;
  while (*ancestor) {
	if (id = findId (*ancestor, obj)) {
		return id;
	}
	ancestor++;
  }
  return (char*) 0;
}
#endif

int
Coc_objectType (Coc_Type* type, Coc_Value& value)
{
  if (value.string) {
	if (!strcmp (value.string, "<nil>")) {
		value.oval = (void*) 0;
		return TCL_OK;
	}
	Tcl_CmdInfo cmdInfo;
	int found = Tcl_GetCommandInfo (type->info->interp, value.string, &cmdInfo);
	if (!found || ((Coc_Object*) cmdInfo.clientData)->cookie != COC_COOKIE) {
		Tcl_AppendResult (type->info->interp,
				  "\nno such object \"",
				  value.string, "\"", (char*) 0);
		return TCL_ERROR;
	} else {
		Coc_Object* tcl = (Coc_Object*) cmdInfo.clientData;
		if (!typeIsa (tcl->type, type->name)) {
			Tcl_AppendResult (type->info->interp, "\nobject \"",
					  value.string, "\" of type \"",
					  tcl->type->name, "\", expected \"",
					  type->name, "\"", (char*) 0);
			return TCL_ERROR;
		}		  
		value.oval = tcl->obj;
	}
  } else {
	if (value.oval == (void*) 0) {
		value.string = "<nil>";
		return TCL_OK;
	}
	Tcl_HashEntry* hPtr;
  
	if (!(hPtr = Tcl_FindHashEntry (&type->ids, (char*) value.oval))) {
		value.string = type->info->genId (type, value.oval);
		return type->wrapObject (value.string, value.oval);
	} else {
		value.string = ((Coc_Object*) Tcl_GetHashValue (hPtr))->name;
	}
  }
  return TCL_OK;
}

Coc_Value*
Coc_getParameter (Coc_Info* info, char* name, int parmCnt, Coc_Variable parm[], Coc_TypeProc type, Coc_Type* context)
{
  char* configName = (char*) 0;
  int dyn;
  if (parmCnt < 0) {
	// stems from NAMED parameters. need to prepend '-' to parameter name
	configName = prepend_dash (name);
	parmCnt = -1 * parmCnt;
	dyn = 1;
  } else {
	configName = name;
	dyn = 0;
  }
  for (int i = 0; i < parmCnt; i++) {
	if (!strcmp (configName, parm [i].name)) {
		parm [i].used = 1;
		if (type) {
			if (TCL_OK != type ((context) ? context : info->objectType, parm [i].value)) {
				return (Coc_Value*) 0;
			}
		}
		if (dyn) {
			delete [] configName;
		}
		return &parm [i].value;
	}
  }
  if (dyn) {
	delete [] configName;
  }
  Tcl_AppendResult (info->interp, "\nparameter \"",
	name, "\" not given", (char*) 0);
				
  return (Coc_Value*) 0;
}

Coc_Value*
Coc_useParameter (int ix, int parmCnt, Coc_Variable parm[])
{
  if (ix < 0 || ix >= parmCnt) {
	return (Coc_Value*) 0;
  }
  parm [ix].used = 1;
  return &parm [ix].value;
}

Coc_Procedure::~Coc_Procedure()
{
  printf ("delete Coc_Procedure \"%s\"\n", name);

  if (parmc >= 0) {
	delete [] parmv;
  }
  if (isStatic) {
	Tcl_DString cmdName;
	Tcl_DStringInit (&cmdName);
	Tcl_DStringAppend (&cmdName, parent->name, -1);
	Tcl_DStringAppend (&cmdName, "::", 2);
	Tcl_DStringAppend (&cmdName, name, -1);
	Tcl_DeleteCommand (parent->info->interp, Tcl_DStringValue (&cmdName));
	Tcl_DStringFree (&cmdName);
  }
  delete [] name;
}

int
Coc_Procedure::cmd (Tcl_Interp* dummy, int argc, char** argv)
{
  int offs = (isStatic) ? 1 : 2;
  int code;
  int callParmc;
  Coc_Variable* callParmv;
  Coc_Value result; result.string = 0;
  if (parmc >= 0) {
  	// fixed number of POSITIONAL parameters
	callParmc = argc - offs;
  	if (callParmc != parmc) {
  		Tcl_AppendResult (parent->info->interp, "\nusage: ",
  				  parent->name, "::", name, " ", (char*) 0);
  		for (int i = 0; i < parmc; i++) {
  			Tcl_AppendResult (parent->info->interp, 
  					  parmv [i].name, "Value ", (char*) 0);
  		}
  		return TCL_ERROR;
  	}
  	callParmv = new Coc_Variable [callParmc];
  	for (int i = 0; i < callParmc; i++) {
  		callParmv [i].name = parmv [i].name;
		callParmv [i].value.string = argv [i + offs];
  		if (TCL_OK != parmv [i].type (
  				      (parmv [i].context)
  				      ? parmv [i].context
  					      : parent,
  				      callParmv [i].value)) {
  			delete [] callParmv;
  			return TCL_ERROR;
  		}
  	}	
  } else {
  	// variable number of NAMED parameters
  	if ((argc - offs) % 2) {
  		Tcl_AppendResult (parent->info->interp, "\nusage: \"",
  				  parent->name, "::", name,
  			  " ? -name value ...?\"", (char*) 0);
  		return TCL_ERROR;
  	}
  	callParmc = -1 * ((argc - offs) / 2);
  	callParmv = new Coc_Variable [-callParmc];
  	for (int i = 0; i < -callParmc; i++) {
  		callParmv [i].name = argv [offs + 2*i];
  		callParmv [i].value.string = argv [offs + 1 + 2*i];
  	}	
  }

  if (isStatic) {
	code = ((Coc_StaticProc) proc) (parent->info, result, callParmc, callParmv);
  } else {
	code = proc (parent->info, obj, result, callParmc, callParmv);
  }
  delete [] callParmv;
  if (code != TCL_OK) {
	Tcl_AppendResult (parent->info->interp, "\nprocedure \"",
			  parent->name, "::", name, "\" failed", (char*) 0);
  	return code;
  }
  if (type) {
  	if (TCL_OK != (code = type ((context) ? context : parent->info->objectType, result))) {
  		return code;
  	}
  	Tcl_SetResult (parent->info->interp, result.string, TCL_STATIC);
  }
  return TCL_OK;
}

Coc_Variable*
Coc_Procedure::putParameter (int ix, char* name, Coc_TypeProc type, Coc_Type* context)
{
  if (ix < 0 || ix >= parmc) {
	return (Coc_Variable*) 0;
  }

  parmv [ix].name = Coc_strdup (name);
  parmv [ix].dyn = 1;
  parmv [ix].context = context;
  parmv [ix].type = type;

  return &parmv [ix];
}

void
Coc_Type::addObject (char* name, Coc_Object* tcl)
{
  int newObj;
  Tcl_HashEntry* hPtr;

  hPtr = Tcl_CreateHashEntry (&ids, (char*) tcl->obj, &newObj);
  if (newObj) {
	Tcl_SetHashValue (hPtr, (char*) tcl);
  }
  Coc_Type** ancestor = ancestorArray;
  while (*ancestor) {
	(*ancestor)->addObject (name, tcl);
	ancestor++;
  }
}

int
Coc_Type::wrapObject (char* name, void* object)
{
  Tcl_CmdInfo cmdInfo;
  Coc_Object* tcl;
  if (Tcl_GetCommandInfo (info->interp, name, &cmdInfo)) {
	// command of this name already exists
	Tcl_AppendResult (info->interp,
			  "\ncommand \"", name,
			  "\" already exists", (char*) 0);
	return TCL_ERROR;
  } else {
	tcl = new Coc_Object (this, Coc_strdup (name), object);
	Tcl_CreateCommand (info->interp, name,
			   (Tcl_CmdProc*) BOUND_PTR Coc_Object::cmd,
			   (ClientData) tcl,
			   (Tcl_CmdDeleteProc*) 0);
	addObject (name, tcl);
  }
  return TCL_OK;
}

Coc_Type*
Coc_Type::create (Coc_Info* info, char* name, int ancestorCnt, Coc_StaticProc constructor, Coc_Proc destructor)
{
  int newObj;
  Tcl_HashEntry* hPtr;
  Coc_Type* newType;

  hPtr = Tcl_CreateHashEntry (&info->types, name, &newObj);
  if (newObj) {
	newType = new Coc_Type;
	Tcl_SetHashValue (hPtr, (char*) newType);
  } else {
	Coc_Type* oldType = (Coc_Type*) Tcl_GetHashValue (hPtr);
	Coc_Type** ancestor = oldType->ancestorArray;
	int cnt;
	for (cnt = 0; *ancestor; ancestor++) {
		cnt++;
	}
	if (cnt < ancestorCnt) {
		delete [] oldType->ancestorArray;
		oldType->ancestorArray = new Coc_Type* [ancestorCnt + 1];
		for (int i = 0; i <= ancestorCnt; i++) {
			oldType->ancestorArray [i] = (Coc_Type*) 0;
		}
	}
	return oldType;
  }
  Tcl_InitHashTable (&newType->ids, TCL_ONE_WORD_KEYS);
  Tcl_InitHashTable (&newType->attributes, TCL_STRING_KEYS);
  Tcl_InitHashTable (&newType->procedures, TCL_STRING_KEYS);

  Tcl_InitHashTable (&newType->ancestors, TCL_STRING_KEYS);
  newType->ancestorArray = new Coc_Type* [ancestorCnt + 1];
  for (int i = 0; i <= ancestorCnt; i++) {
	newType->ancestorArray [i] = (Coc_Type*) 0;
  }
  
  Tcl_CreateCommand (info->interp, name,
		     (Tcl_CmdProc*) BOUND_PTR Coc_Type::cmd,
		     (ClientData) newType,
		     (Tcl_CmdDeleteProc*) 0);
  newType->info = info;
  newType->name = Coc_strdup (name);
  newType->maxId = 0;

  newType->constructor = constructor;
  newType->destructor = destructor;
  if (destructor) {
	  newType->addProcedure ("delete", destructor);
  }
  return newType;
}

Coc_Type::~Coc_Type()
{
  Tcl_HashSearch search;
  Tcl_HashEntry* hPtr;

  Tcl_DeleteCommand (info->interp, name);

  hPtr = Tcl_FindHashEntry (&info->types, name);
  Tcl_DeleteHashEntry (hPtr);
  
  if (isEnum) {
	// name deleted in destructor of Coc_EnumType
	return;
  }
  
  printf ("delete Coc_Type \"%s\"\n", name);

  // delete objects
  if (hPtr = Tcl_FirstHashEntry (&ids, &search)) {
	delete (Coc_Object*) Tcl_GetHashValue (hPtr);
	while (hPtr = Tcl_NextHashEntry (&search)) {
		delete (Coc_Object*) Tcl_GetHashValue (hPtr);
	}
  }
  Tcl_DeleteHashTable (&ids);

  // delete attributes
  if (hPtr = Tcl_FirstHashEntry (&attributes, &search)) {
	delete (Coc_Variable*) Tcl_GetHashValue (hPtr);
	while (hPtr = Tcl_NextHashEntry (&search)) {
		delete (Coc_Variable*) Tcl_GetHashValue (hPtr);
	}
  }
  Tcl_DeleteHashTable (&attributes);

  // delete procedures
  if (hPtr = Tcl_FirstHashEntry (&procedures, &search)) {
	delete (Coc_Procedure*) Tcl_GetHashValue (hPtr);
	while (hPtr = Tcl_NextHashEntry (&search)) {
		delete (Coc_Procedure*) Tcl_GetHashValue (hPtr);
	}
  }
  Tcl_DeleteHashTable (&procedures);

  Tcl_DeleteHashTable (&ancestors);
  delete [] ancestorArray;
  delete [] name;
}

Coc_EnumType::~Coc_EnumType()
{
  printf ("delete Coc_EnumType \"%s\"\n", name);
  delete [] name;
}

Coc_EnumType*
Coc_EnumType::create (Coc_Info* info, char* name, int elementCnt, char* elements[])
{
  int newObj;
  Tcl_HashEntry* hPtr;
  Coc_EnumType* newType;

  hPtr = Tcl_CreateHashEntry (&info->types, name, &newObj);
  if (newObj) {
	newType = new Coc_EnumType;
	Tcl_SetHashValue (hPtr, (char*) newType);
  } else {
	return 0;
  }
  newType->name = Coc_strdup (name);
  newType->info = info;
  newType->elementCnt = elementCnt;
  newType->elements = elements;
  return newType;
}

static void
collectProcedures (Coc_Type* type, Tcl_DString* result)
{
  collectHashedStrings (&type->procedures, result, 1);
  Coc_Type** ancestor = type->ancestorArray;
  while (*ancestor) {
	collectProcedures (*ancestor, result);
	ancestor++;
  }
}

static void
collectAttributes (Coc_Type* type, Tcl_DString* result)
{
  collectHashedStrings (&type->attributes, result, 1);
  Coc_Type** ancestor = type->ancestorArray;
  while (*ancestor) {
	collectAttributes (*ancestor, result);
	ancestor++;
  }
}

static void
collectAncestors (Coc_Type* type, Tcl_DString* result)
{
  Tcl_DStringAppendElement (result, type->name);
  Coc_Type** ancestor = type->ancestorArray;
  while (*ancestor) {
	collectAncestors (*ancestor, result);
	ancestor++;
  }
}

int
Coc_Type::digestParameters (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  // consume un-used command line parameters
  for (int i = 0; i < parmCnt; i++) {
	if (parm [i].used) continue;
	Coc_Variable* attr = findAttribute (parm [i].name);
	if (!attr) {
		char ancestorName [512];
		char attrName [512];
		char* configAttrName;
		Coc_Type* ancestor;
		if (2 == sscanf (parm [i].name, "-%[^:]::%s", ancestorName, attrName)) {
			// try named ancestor
			if (!(ancestor = findAncestor (ancestorName))) {
				return TCL_ERROR;
			}
			configAttrName = prepend_dash (attrName);
			if (!(attr = ancestor->findAttribute (configAttrName))) {
				return TCL_ERROR;
			}
			delete [] configAttrName;
		} else {
			return TCL_ERROR;
		}
	}
	if (TCL_OK != attr->type ((attr->context)
				  ? attr->context : info->objectType, parm [i].value)) {
		return TCL_ERROR;
	}
	if (!attr->set) {
		Tcl_AppendResult (info->interp, "\ncan't set \"", attr->name, "\"", (char*) 0);
		return TCL_ERROR;
	}
	int code;
	if (TCL_OK != (code = attr->set (info, obj, parm [i].value, 1, &parm [i]))) {
		return code;
	}
  }
  return TCL_OK;
}

int
Coc_Type::cmd (Tcl_Interp* dummy, int argc, char** argv)
{
  char c;
  int code;
  void* obj;
  char* objName;
  int i;
  int parmc;
  Coc_Variable* parm;
  Coc_Value result;

  if (argc < 2) {
	goto type_usage;
  }
  c = *argv[1];

  if (c == 'i' && !strcmp (argv[1], "info")) {
	if (argc != 3 || strcmp (argv[2], "inherit")) {
		Tcl_AppendResult (info->interp, "\nusage: \"", argv[0],
				  " info inherit\"", (char*) 0);
		return TCL_ERROR;
	}
	Tcl_DString result;
	Tcl_DStringInit (&result);
	Coc_Type** ancestor = ancestorArray;
	while (*ancestor) {
		Tcl_DStringAppendElement (&result, (*ancestor)->name);
		ancestor++;
	}
	Tcl_DStringResult (info->interp, &result);
	return TCL_OK;
  }

  if (!constructor) {
	Tcl_AppendResult (info->interp, "\nnot allowed to create objects of type \"",
			  name, "\"", (char*) 0);
	return TCL_ERROR;
  }
  if ((argc -2) % 2) {
	goto type_usage;
  }
  parmc = (argc - 2) / 2;
  parm = new Coc_Variable [parmc];
  for (i = 0; i < parmc; i++) {
	parm [i].name = argv [2 + 2*i];
	if (*parm [i].name != '-') {
		delete [] parm;
		goto type_usage;
	}
	parm [i].value.string = argv [3 + 2*i];
  }	

// parameters of constructors are always expected to have NAMED parameters
// signal this to getParameter with negative parmc
  if (TCL_OK != constructor (info, result, -parmc, parm)
      || !(obj = result.oval)) {
	delete [] parm;
	Tcl_AppendResult (info->interp, "\nconstructor \"",
			  name, "\" failed", (char*) 0);
	return TCL_ERROR;
  }

  if (c == '#' && !strcmp (argv[1], "#auto")) {
	objName = info->genId (this, obj);
  } else {
	objName = argv[1];
  }

  code = wrapObject (objName, obj);
  if (code != TCL_OK) {
	if (destructor)
		destructor (info, obj, result, 0, (Coc_Variable*) 0);
	delete [] parm;
	return TCL_ERROR;
  }

  // consume un-used command line parameters
  if (TCL_OK != (code = digestParameters (info, obj, result, parmc, parm))) {
	if (destructor)
		destructor (info, obj, result, 0, (Coc_Variable*) 0);
	delete [] parm;
	return TCL_ERROR;
  }

  delete [] parm;
  Tcl_SetResult (info->interp, objName, TCL_STATIC);
  return TCL_OK;

type_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"", argv[0],
		    " (id | #auto) ?-name value ...?\"", (char*) 0);
  return TCL_ERROR;
}

Coc_Variable::~Coc_Variable()
{
  if (!dyn) return;
  printf ("delete Coc_Variable \"%s\"\n", name);
  delete [] name;
}

Coc_Variable*
Coc_Type::addAttribute (char* name, Coc_Proc get, Coc_Proc set, Coc_TypeProc type, Coc_Type* context)
{
  int newObj;
  Tcl_HashEntry* hPtr;
  char* configName = new char [strlen (name) +2];
  configName [0] = '-';
  strcpy (configName +1, name);
  hPtr = Tcl_CreateHashEntry (&attributes, configName, &newObj);
  if (!newObj) {
	Tcl_AppendResult (info->interp, "\nattribute \"", name,
			  "\" already exists in type", (char*) 0);
	return (Coc_Variable*) 0;
  }
  Coc_Variable* attr = new Coc_Variable;
  attr->name = configName;
  attr->dyn = 1;
  attr->parent = this;
  attr->context = context;
  attr->type = type;
  attr->set = set;
  attr->get = get;
  Tcl_SetHashValue (hPtr, attr);

  addProcedure (name, get, 0, type, context);
  
  return attr;
}

Coc_Procedure*
Coc_Type::addProcedure (char* name, Coc_Proc proc, int parmc, Coc_TypeProc type, Coc_Type* context, int isStatic)
{
  int newObj;
  Tcl_HashEntry* hPtr;
  hPtr = Tcl_CreateHashEntry (&procedures, name, &newObj);
  if (!newObj) {
	Tcl_AppendResult (info->interp, "\nprocedure \"", name,
			  "\" already exists in type \"", name, "\"", (char*) 0);
	return (Coc_Procedure*) 0;
  }
  Coc_Procedure* newProc = new Coc_Procedure;
  newProc->name = Coc_strdup (name);
  newProc->parent = this;
  newProc->context = context;
  newProc->type = type;
  newProc->proc = proc;
  newProc->isStatic = isStatic;
  newProc->parmc = parmc;
  if (parmc >= 0) 
	newProc->parmv = new Coc_Variable [parmc];
  if (newProc->isStatic) {
	Tcl_DString cmdName;
	Tcl_DStringInit (&cmdName);
	Tcl_DStringAppend (&cmdName, this->name, -1);
	Tcl_DStringAppend (&cmdName, "::", 2);
	Tcl_DStringAppend (&cmdName, name, -1);
	Tcl_CreateCommand (info->interp, Tcl_DStringValue (&cmdName),
			   (Tcl_CmdProc*) BOUND_PTR Coc_Procedure::cmd,
			   (ClientData) newProc, (Tcl_CmdDeleteProc*) 0);
	Tcl_DStringFree (&cmdName);
  }
  Tcl_SetHashValue (hPtr, newProc);
  return newProc;
}

Coc_Type*
Coc_Type::addAncestor (int index, Coc_Type* ancestor)
{
  int newObj;
  Tcl_HashEntry* hPtr;
  if (!ancestor) {
	Tcl_AppendResult (info->interp, "\nancestor for type \"", name, "\" is <nil>",
			  (char*) 0);
	return (Coc_Type*) 0;
  }
  hPtr = Tcl_CreateHashEntry (&ancestors, ancestor->name, &newObj);
  if (!newObj) {
	Tcl_AppendResult (info->interp, "\nancestor \"", ancestor->name,
			  "\" already exists in type \"", name, "\"", (char*) 0);
	return (Coc_Type*) 0;
  }
  Tcl_SetHashValue (hPtr, ancestor);
  
  ancestorArray [index] = ancestor;
  
  return ancestor;
}

Coc_Variable*
Coc_Type::findAttribute (char* nm)
{
  Tcl_HashEntry* hPtr;
  if ((hPtr = Tcl_FindHashEntry (&attributes, nm)) != NULL) {
	return (Coc_Variable*) Tcl_GetHashValue (hPtr);
  } else {
	Coc_Type** ancestor = ancestorArray;
	Coc_Variable* result;
	while (*ancestor) {
		if (result = (*ancestor)->findAttribute (nm)) {
			return result;
		}
		ancestor++;
	}
	Tcl_AppendResult (info->interp, "\nno such attribute \"",
			  nm, "\" in type \"", name, "\"", (char*) 0);
	return (Coc_Variable*) 0;
  }
}

Coc_Procedure*
Coc_Type::findProcedure (char* nm, int deep)
{
  Tcl_HashEntry* hPtr;
  if ((hPtr = Tcl_FindHashEntry (&procedures, nm)) != NULL) {
	return (Coc_Procedure*) Tcl_GetHashValue (hPtr);
  } else {
	if (!deep) return (Coc_Procedure*) 0;
	Coc_Type** ancestor = ancestorArray;
	Coc_Procedure* result;
	while (*ancestor) {
		if (result = (*ancestor)->findProcedure (nm)) {
			return result;
		}
		ancestor++;
	}
	Tcl_AppendResult (info->interp, "\nno such procedure \"",
			  nm, "\" in type \"", name, "\"", (char*) 0);
	return (Coc_Procedure*) 0;
  }
}

Coc_Type*
Coc_Type::findAncestor (char* nm)
{
  Tcl_HashEntry* hPtr;
  if ((hPtr = Tcl_FindHashEntry (&ancestors, nm)) != NULL) {
	return (Coc_Type*) Tcl_GetHashValue (hPtr);
  } else {
	Tcl_AppendResult (info->interp, "\nno such ancestor \"",
			  nm, "\" in type \"", name, "\"", (char*) 0);
	return (Coc_Type*) 0;
  }
}

int
Coc_collectAttributeValues (Coc_Type* type, Coc_Object* obj, Tcl_DString* result, int asList)
{
  Tcl_HashSearch search;
  Tcl_HashEntry* hPtr;
  
  int code;
  Coc_Variable* attr;
  Coc_Variable parm;

  for (hPtr = Tcl_FirstHashEntry (&type->attributes, &search); hPtr;
       hPtr = Tcl_NextHashEntry (&search)) {
	
	attr = (Coc_Variable*) Tcl_GetHashValue (hPtr);
	if (attr->get) {
		if (TCL_OK != (code = attr->get (type->info,
						 obj->obj, parm.value, 1, &parm))) {
			return code;
		}
		parm.value.string = 0;
		attr->type ((attr->context)
			    ? attr->context : type->info->objectType, parm.value);
		if (asList) {
			Tcl_DStringStartSublist (result);
		}
		Tcl_DStringAppendElement (result, attr->name);
		Tcl_DStringAppendElement (result, parm.value.string);
		if (asList) {
			Tcl_DStringEndSublist (result);
		}
	}
  }
  Coc_Type** ancestor = type->ancestorArray;
  while (*ancestor) {
	Coc_collectAttributeValues (*ancestor, obj, result, asList);
	ancestor++;
  }
  return TCL_OK;
}

void
Coc_Type::delObject (void* obj)
{
  Tcl_HashEntry* hPtr;
  hPtr = Tcl_FindHashEntry (&ids, (char*) obj);
  Tcl_DeleteHashEntry (hPtr);
  
  Coc_Type** ancestor = ancestorArray;
  while (*ancestor) {
	(*ancestor)->delObject (obj);
	ancestor++;
  }
}

Coc_Object::~Coc_Object()
{
  printf ("delete Coc_Object \"%s\", obj=%#x\n", name, obj);

  // delete object from all its supertypes
  type->delObject (obj);

  if (type->destructor) {
	Coc_Value result;
	type->destructor (type->info, obj, result, 0, (Coc_Variable*) 0);
  }
  delete [] name;
}

int
Coc_Object::cmd (Tcl_Interp* dummy, int argc, char** argv)
{
  int code;

  if (argc == 1) {
	Tcl_DString result;
	Tcl_DStringInit (&result);
	if (TCL_OK != (code = Coc_collectAttributeValues (type, this, &result, 1))) {
		return code;
	} else {
		Tcl_DStringResult (type->info->interp, &result);
		return TCL_OK;
	}
  }
	
  Coc_Procedure* proc = type->findProcedure (argv[1]);
  if (!proc) {
	char ancestorName [512];
	char procName [512];
	Coc_Type* ancestor;
	if (2 == sscanf (argv[1], "%[^:]::%s", ancestorName, procName)) {
		// try named ancestor
		if (!(ancestor = type->findAncestor (ancestorName))) {
			return TCL_ERROR;
		}
		proc = ancestor->findProcedure (procName);
	}
  }
  if (proc) {
	proc->obj = obj;
	if (TCL_OK != proc->cmd (type->info->interp, argc, argv)) {
		return TCL_ERROR;
	}
	if (proc->proc == type->destructor) {
		Tcl_HashEntry* hPtr;

		Tcl_DeleteCommand (type->info->interp, argv[0]);
		
		hPtr = Tcl_FindHashEntry (&type->ids, (char*) obj);
		Tcl_DeleteHashEntry (hPtr);
		
		return TCL_OK;
	}
	return TCL_OK;
  }

  char c = *argv[1];
  if (c == 'c' && !strcmp (argv[1], "config")) {
	Tcl_ResetResult (type->info->interp);

	Coc_Variable parm;
	argv += 2;
	while (argc -= 2) {
		parm.name = *argv++;
		Coc_Variable* attr = type->findAttribute (parm.name);
		if (!attr) {
			return TCL_ERROR;
		}
		parm.value.string = *argv++;
		if (!parm.value.string) {
			Tcl_AppendResult (type->info->interp, "\nvalue for \"",
				  parm.name, "\" is nil", (char*) 0);
			return TCL_ERROR;
		}
		if (TCL_OK != attr->type ((attr->context)
					  ? attr->context : type->info->objectType, parm.value)) {
			return TCL_ERROR;
		}
		if (!attr->set) {
			Tcl_AppendResult (type->info->interp, "\ncan't set \"", attr->name, "\"", (char*) 0);
			return TCL_ERROR;
		}
		if (TCL_OK != (code = attr->set (type->info, obj, parm.value, 1, &parm))) {
			return code;
		}
	}
	return TCL_OK;
  } else if (c == 'i' && !strcmp (argv[1], "isa")) {
	if (argc == 3) {
		type->info->interp->result =
			(typeIsa (type, argv[2]))
				? "1" : "0";
		return TCL_OK;
	} else {
		Tcl_AppendResult (type->info->interp, "\nusage: \"",
				  argv[0], " isa classs\"",
				  (char*) 0);
		return TCL_ERROR;
	}	
  } else if (c == 'i' && !strcmp (argv[1], "info")) {
	if (argc == 3 && !strcmp (argv[2], "class")) {
		Tcl_SetResult (type->info->interp, type->name, TCL_STATIC);
		return TCL_OK;
	} else if (argc == 3 && !strcmp (argv[2], "inherit")) {
		Tcl_DString result;
		Tcl_DStringInit (&result);
		Coc_Type** ancestor = type->ancestorArray;
		while (*ancestor) {
			Tcl_DStringAppendElement (&result, (*ancestor)->name);
			ancestor++;
		}
		Tcl_DStringResult (type->info->interp, &result);
		return TCL_OK;	
	} else if (argc == 3 && !strcmp (argv[2], "heritage")) {
		Tcl_DString result;
		Tcl_DStringInit (&result);
		collectAncestors (type, &result);
		Tcl_DStringResult (type->info->interp, &result);
		return TCL_OK;			
	} else if (argc == 3 && !strcmp (argv[2], "method")) {
		Tcl_DString result;
		Tcl_DStringInit (&result);
		collectProcedures (type, &result);
		Tcl_DStringResult (type->info->interp, &result);
		return TCL_OK;			
	} else if (argc == 3 && !strcmp (argv[2], "public")) {
		Tcl_DString result;
		Tcl_DStringInit (&result);
		collectAttributes (type, &result);
		Tcl_DStringResult (type->info->interp, &result);
		return TCL_OK;			
	} else {
		Tcl_ResetResult (type->info->interp);
		Tcl_AppendResult (type->info->interp, "\nusage: \"",
				  argv[0], " info (class | inherit | method | public)\"",
				  (char*) 0);
		return TCL_ERROR;
	}
  } else {
	return TCL_ERROR;
  }
}

Tcl_CmdProc BlocksCmd;

Coc_Info::Coc_Info (Tcl_Interp* interp_, Coc_GenIdProc genId_):
	interp (interp_), genId (genId_)
{
  /*
   * Initialize global tables. Why doesn't the compiler do this ???
   */
  extern Coc_ProcType* Coc_ProcTypes;
  Coc_ProcTypes = new Coc_ProcType [5];
  Coc_ProcTypes [0].name = "int";	Coc_ProcTypes [0].proc = Coc_intType;
  Coc_ProcTypes [1].name = "double";	Coc_ProcTypes [1].proc = Coc_doubleType;
  Coc_ProcTypes [2].name = "string";	Coc_ProcTypes [2].proc = Coc_stringType;
  Coc_ProcTypes [3].name = "void";	Coc_ProcTypes [3].proc = Coc_voidType;
  Coc_ProcTypes [4].name = (char*) 0;	Coc_ProcTypes [4].proc =(Coc_TypeProc) 0;
  
  Tcl_InitHashTable (&types, TCL_STRING_KEYS);
  Tcl_InitHashTable (&sharedLibraries, TCL_STRING_KEYS);
  Tcl_CreateCommand (interp, "cocos_info",
		     (Tcl_CmdProc*) BOUND_PTR Coc_Info::cmd,
		     (ClientData) this,
		     (Tcl_CmdDeleteProc*) 0);
  Tcl_CreateCommand (interp, "cocos_schema",
		     (Tcl_CmdProc*) BOUND_PTR Coc_Info::schemaDef,
		     (ClientData) this,
		     (Tcl_CmdDeleteProc*) 0);
  Tcl_CreateCommand (interp, "cocos_class",
		     (Tcl_CmdProc*) BOUND_PTR Coc_Info::classDef,
		     (ClientData) this,
		     (Tcl_CmdDeleteProc*) 0);
  Tcl_CreateCommand (interp, "cocos_enum",
		     (Tcl_CmdProc*) BOUND_PTR Coc_Info::enumDef,
		     (ClientData) this,
		     (Tcl_CmdDeleteProc*) 0);
  Tcl_CreateCommand (interp, "cocos_blocks",
		     (Tcl_CmdProc*) BOUND_PTR BlocksCmd,
		     (ClientData) this,
		     (Tcl_CmdDeleteProc*) 0);

  objectType = Coc_Type::create (this, "object");
  if (!objectType) {
	fprintf (stderr, "can't create type \"object\": %s\n", interp->result);
  }
}

Coc_Info::~Coc_Info()
{
  extern Coc_ProcType* Coc_ProcTypes;

  delete [] Coc_ProcTypes;

  Tcl_HashSearch search;
  Tcl_HashEntry* hPtr;

  /*
   * delete types
   */
  if (hPtr = Tcl_FirstHashEntry (&types, &search)) {
	delete (Coc_Type*) Tcl_GetHashValue (hPtr);
	while (hPtr = Tcl_NextHashEntry (&search)) {
		delete (Coc_Type*) Tcl_GetHashValue (hPtr);
	}
  }
  Tcl_DeleteHashTable (&types);
  Tcl_DeleteHashTable (&sharedLibraries);
}

static void
collectHashedStrings (Tcl_HashTable* table, Tcl_DString* result, int key)
{
  Tcl_HashSearch search;
  Tcl_HashEntry* hPtr;
    
  if (!(hPtr = Tcl_FirstHashEntry (table, &search))) return;
  
  Tcl_DStringAppendElement (result,
	(key)	? (char*) Tcl_GetHashKey (table, hPtr)
		: ((Coc_Object*) Tcl_GetHashValue (hPtr))->name);
  while (hPtr = Tcl_NextHashEntry (&search)) {
	Tcl_DStringAppendElement (result,
	(key)	? (char*) Tcl_GetHashKey (table, hPtr)
		: ((Coc_Object*) Tcl_GetHashValue (hPtr))->name);
  }
}

int
Coc_Info::cmd (Tcl_Interp* dummy, int argc, char** argv)
{
  char c;
  Tcl_HashSearch search;
  Tcl_HashEntry* hPtr;
  Tcl_DString result;
  Coc_Type* type;
  
  if (argc < 2) {
	goto info_usage;
  }
  c = *argv[1];
  if (c == 'c' && !strcmp (argv[1], "classes")) {
	Tcl_DStringInit (&result);

	collectHashedStrings (&types, &result, 1);
	
	Tcl_DStringResult (interp, &result);
	Tcl_DStringFree (&result);

	return TCL_OK;
  } else if (c == 'o' && !strcmp (argv[1], "objects")) {
	if (argc > 2) {
		if (!strcmp (argv[2], "-class")) {
			if (argc != 4) {
				goto info_usage;
			}
			if (hPtr = Tcl_FindHashEntry (&types, argv[3])) {
				Tcl_DStringInit (&result);
				type = (Coc_Type*) Tcl_GetHashValue (hPtr);

				collectHashedStrings (&type->ids, &result, 0);

				Tcl_DStringResult (interp, &result);
				Tcl_DStringFree (&result);
			}
			return TCL_OK;
		}
		goto info_usage;
	}
	Tcl_DStringInit (&result);

	if (hPtr = Tcl_FirstHashEntry (&types, &search)) {
		type = (Coc_Type*) Tcl_GetHashValue (hPtr);

		collectHashedStrings (&type->ids, &result, 0);

		while (hPtr = Tcl_NextHashEntry (&search)) {
			type = (Coc_Type*) Tcl_GetHashValue (hPtr);

			collectHashedStrings (&type->ids, &result, 0);
		}
	}
	Tcl_DStringResult (interp, &result);
	Tcl_DStringFree (&result);

	return TCL_OK;
  } else {
	goto info_usage;
  }
  return TCL_OK;
  
info_usage:
  Tcl_AppendResult (interp, "\nusage: \"", argv[0],
		    " (classes | objects ?-class className?)\"", (char*) 0);
  return TCL_ERROR;
}
