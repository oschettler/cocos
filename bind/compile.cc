/*
 * bind/compile.cc --
 *
 *	Compiler for cocos class definitions
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: compile.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: compile.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: compile.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <string.h>
#include <limits.h>
#include <dlfcn.h>

#include <tcl.h>
#include "bind.h"

extern "C" {
#ifndef SVR4

  void *dlopen(char* path, int mode);
  void *dlsym(void *handle, char *symbol);
  char* dlerror(void);
  int dlclose(void* handle);

#endif

  int free (char*);
}
Coc_ProcType* Coc_ProcTypes;

static int
doInherit (Coc_Info* info, Coc_Type*& type, char* libName, void* xlib, char* className, int argc, char** argv)
{
  //
  // inherit <ancestor> ...
  //
  int i;
  
  if (argc < 2) {
  	goto inherit_usage;
  }
  if (type) {
  	Tcl_AppendResult (info->interp, "\n\"inherit\" has to come first", (char*) 0);
  	goto inherit_usage;
  }
  if (!(type = Coc_Type::create (info, className, argc -1, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
  	goto inherit_usage;
  }
  for (i = 1; i < argc; i ++) {
  	Tcl_HashEntry* hPtr;
  	if (!(hPtr = Tcl_FindHashEntry (&info->types, argv[i]))) {
		Tcl_AppendResult (info->interp, "\ntype \"", argv[i], "\" not defined",
				  (char*) 0);
		goto inherit_usage;
	}
  	Coc_Type* ancestor = (Coc_Type*) Tcl_GetHashValue (hPtr);
  	type->addAncestor (i-1, ancestor);
  }
  return TCL_OK;

inherit_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"inherit ?ancestor ...?\"", (char*) 0);
  return TCL_ERROR;
}

static int
doConstructor (Coc_Info* info, Coc_Type*& type, char* libName, void* xlib,  char* className, int argc, char** argv)
{
  //
  // constructor <cprocName>
  //
  Coc_StaticProc constructor;

  if (argc != 2) {
  	goto constructor_usage;
  }
  if (!type) {
  	if (!(type = Coc_Type::create (info, className, 0, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
  		goto constructor_usage;
  	}
  }
  if (type->constructor) {
  	Tcl_AppendResult (info->interp, "\nconstructor already defined", (char*) 0);
  	goto constructor_usage;
  }
  constructor = (Coc_StaticProc) dlsym (xlib, argv[1]);
  if (!constructor) {
  	Tcl_AppendResult (info->interp, "\nconstructor \"", argv[1], "\" not defined in \"",
  			  libName, "\"", (char*) 0);
  	goto constructor_usage;
  }
  type->constructor = constructor;

  return TCL_OK;

constructor_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"constructor cproc\"", (char*) 0);
  return TCL_ERROR;
}

static int
doDestructor (Coc_Info* info, Coc_Type*& type, char* libName, void* xlib, char* className, int argc, char** argv)
{
  //
  // destructor <cprocName>
  //
  Coc_Proc destructor;

  if (argc != 2) {
  	goto destructor_usage;
  }
  if (!type) {
  	if (!(type = Coc_Type::create (info, className, 0, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
  		goto destructor_usage;
  	}
  }
  if (type->destructor) {
  	Tcl_AppendResult (info->interp, "\ndestructor already defined", (char*) 0);
  	goto destructor_usage;
  }
  destructor = (Coc_Proc) dlsym (xlib, argv[1]);
  if (!destructor) {
  	Tcl_AppendResult (info->interp, "\ndestructor \"", argv[1], "\" not defined in \"",
  			  libName, "\"", (char*) 0);
  	goto destructor_usage;
  }
  type->destructor = destructor;
  type->addProcedure ("delete", destructor);

  return TCL_OK;

destructor_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"destructor cproc\"", (char*) 0);
  return TCL_ERROR;
}

int
Coc_getType (Coc_Info* info, char* typeName, Coc_TypeProc& procType, Coc_Type*& context)
{
  Coc_ProcType* pt = &Coc_ProcTypes [0];
  context = (Coc_Type*) 0;
  procType = (Coc_TypeProc) 0;
  
  do {	/* printf ("getType: \"%s\" ?= %s\n", pt->name, typeName); */
	if (!strcmp (pt->name, typeName)) {
  		procType = pt->proc;
  		break;
  	}
  } while ((++pt)->name);
  if (!procType) {
  	Tcl_HashEntry* hPtr;
  	if (!(hPtr = Tcl_FindHashEntry (&info->types, typeName))) {
  		Tcl_AppendResult (info->interp, "\ntype \"", typeName, "\" not defined",
  				  (char*) 0);
  		return TCL_ERROR;
  	}
  	context = (Coc_Type*) Tcl_GetHashValue (hPtr);
  	if (context->isEnum) {
  		procType = (Coc_TypeProc) Coc_enumType;
  	} else {
  		procType = (Coc_TypeProc) Coc_objectType;
  	}
  }
  return TCL_OK;
}

static int
doMethod (Coc_Info* info, Coc_Type*& type, char* libName, void* xlib, char* className, int argc, char** argv)
{
  //
  // (method|proc) <typeName> <methodName> <paramList> <cprocName>
  //

  int isStatic = (argv[0][0] == 'p') ? 1 : 0;
  char* procName;
  Coc_Proc proc;
  Coc_TypeProc procType;
  Coc_Type* context;

  if (argc != 5) {
	if (argc > 2) {
		procName = argv[2];
	} else {
		procName = "<unknown>";
	}
  	goto method_usage;
  }
  procName = argv[2];
  if (!type) {
  	if (!(type = Coc_Type::create (info, className, 0, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
  		goto method_usage;
  	}
  }

  if (TCL_OK != Coc_getType (info, argv[1], procType, context)) {
	goto method_usage;
  }

  if (type->findProcedure (argv[2], 0)) {
  	Tcl_AppendResult (info->interp, "\nmethod \"", argv[2], "\" already defined",
  			  (char*) 0);
  	goto method_usage;
  }
  Tcl_ResetResult (info->interp);

  proc = (Coc_Proc) dlsym (xlib, argv[4]);
  if (!proc) {
  	Tcl_AppendResult (info->interp, "\nfunction \"", argv[4], "\" not defined in \"",
  			  libName, "\"", (char*) 0);
  	goto method_usage;
  }

  if (!strcmp ("args", argv[3])) {
	// parmc = -1: variable parameter count
	type->addProcedure (argv[2], proc, -1, procType, context, isStatic);
	return TCL_OK;
  }

  int parmc;
  char** parmv;
  if (TCL_OK != Tcl_SplitList (info->interp, argv[3], &parmc, &parmv)) {
	Tcl_AppendResult (info->interp, "\nsyntax error in parameter list \"",
			  argv[3], "\"", (char*) 0);
	return TCL_ERROR;
  }
  Coc_Procedure* tclProc;
  tclProc = type->addProcedure (argv[2], proc, parmc, procType, context, isStatic);
  int i;
  for (i = 0; i < parmc; i++) {
	int cnt;
	char** parm;
	if (TCL_OK != Tcl_SplitList (info->interp, parmv[i], &cnt, &parm)
	    || cnt != 2) {
		Tcl_AppendResult (info->interp, "\nsyntax error in parameter \"",
				  parmv[i], "\"", (char*) 0);
		free ((char*) parmv);
		return TCL_ERROR;
	}
	Coc_TypeProc procType;
	Coc_Type* context;
	
	if (TCL_OK != Coc_getType (info, parm[0], procType, context)) {
		Tcl_AppendResult (info->interp, "\nunknown type \"",
				  parm[0], "\" in parameter", (char*) 0);
		free ((char*) parm);
		free ((char*) parmv);
		return TCL_ERROR;
	}
	tclProc->putParameter (i, parm[1], procType, context);
	free ((char*) parm);
  }
  free ((char*) parmv);
  return TCL_OK;

method_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"(method|proc) type ", procName, " parameters cproc\"", (char*) 0);
  return TCL_ERROR;
}

static int
doPublic (Coc_Info* info, Coc_Type*& type, char* libName, void* xlib, char* className, int argc, char** argv)
{
  //
  // private <typeName> <attrName> <getCprocName> <setCprocName>
  //
  Coc_TypeProc procType = 0;
  Coc_Type* context = 0;
  Coc_ProcType* pt = &Coc_ProcTypes [0];
  Coc_Proc getProc = 0;
  Coc_Proc setProc = 0;

  if (argc < 4 || argc > 5) {
  	goto public_usage;
  }
  if (!type) {
  	if (!(type = Coc_Type::create (info, className, 0, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
  		goto public_usage;
  	}
  }
  do {	if (!strcmp (pt->name, argv[1])) {
  		procType = pt->proc;
  		break;
  	}
  } while ((++pt)->name);
  if (!procType) {
  	Tcl_HashEntry* hPtr;
  	if (!(hPtr = Tcl_FindHashEntry (&info->types, argv[1]))) {
  		Tcl_AppendResult (info->interp, "\ntype \"", argv[1], "\" not defined",
  				  (char*) 0);
  		goto public_usage;
  	}
  	context = (Coc_Type*) Tcl_GetHashValue (hPtr);
  	if (context->isEnum) {
  		procType = (Coc_TypeProc) Coc_enumType;
  	} else {
  		procType = (Coc_TypeProc) Coc_objectType;
  	}
  }
  if (type->findAttribute (argv[2])) {
  	Tcl_AppendResult (info->interp, "\nattribute \"", argv[2], "\" already defined",
  			  (char*) 0);
  	goto public_usage;
  }
  Tcl_ResetResult (info->interp);
  
  if (*argv[3]) {
  	getProc = (Coc_Proc) dlsym (xlib, argv[3]);
  	if (!getProc) {
  		Tcl_AppendResult (info->interp, "\nfunction \"", argv[3], "\" not defined in \"",
  				  libName, "\"", (char*) 0);
  		goto public_usage;
  	}
  }
  if (argc == 5 && *argv[4]) {
  	setProc = (Coc_Proc) dlsym (xlib, argv[4]);
  	if (!setProc) {
  		Tcl_AppendResult (info->interp, "\nfunction \"", argv[4], "\" not defined in \"",
  				  libName, "\"", (char*) 0);
  		goto public_usage;
  	}
  }
  
  type->addAttribute (argv[2], getProc, setProc, procType, context);
  
  return TCL_OK;

public_usage:
  Tcl_AppendResult (info->interp, "\nusage: \"public type name get_cproc ?set_cproc?\"", (char*) 0);
  return TCL_ERROR;
}

int
Coc_Info::classDef (Tcl_Interp* dummy, int argc, char** argv)
{
  void *xlib;
  char* className = argv [1];
  char* libName;
  char* feature;
  
  if (argc == 3) {
	//
	// do not load a shared object
	// all functions must have been resolved by the linker
	//
	
	libName = "program";
	if ((xlib = dlopen (0, 1)) == NULL) {
		Tcl_AppendResult (interp, "can't access symbols in \"",
				  libName, "\": ",
				  dlerror(), (char*) 0);
		return TCL_ERROR;
	}
	feature = strtok (argv[2], "\n");
  } else if (argc == 4) {
	//
	// load shared object lib
	//

	libName = argv[2];
	char* s;
	char* fileName;
	char initName [_POSIX_PATH_MAX];
	int (*init) (Coc_Info*);

	/*
	 *    Extract the name of the initialization function from a library 
	 *    path name in the form:
	 *        [<path>/]lib<exName>.so.<major>[.<minor> ...]
	 */
	fileName = ((s = strrchr (libName, '/')) != NULL )? s+1 : libName;

	if (sscanf (fileName, "lib%[^.].so.%*[0123456789.]", initName) != 1 ) {
		Tcl_AppendResult (interp, "bad library name", (char*) 0);
		return TCL_ERROR;
	}

	strcat (initName, "_Init");

	/*
	 *    If the named library does not exist, if it's not loadable or if
	 *    it does not define the init proc, report an error
	 */
	if ((xlib = dlopen (libName, 1)) == NULL) {
		Tcl_AppendResult (interp, "loading \"", libName, "\": ",
				  dlerror(), (char*) 0);
		return TCL_ERROR;
	}
	if ((init = (int (*) (Coc_Info*)) dlsym (xlib, initName)) == NULL) {
		Tcl_AppendResult (interp, "searching \"", initName, "\": ",
				  dlerror(), (char*) 0);
		if (xlib != NULL) dlclose(xlib);
		return TCL_ERROR;
	}

	/*    
	 *    Try to init the extension and report success or failure
	 */
	int newObj;
	Tcl_HashEntry* hPtr;

	hPtr = Tcl_CreateHashEntry (&sharedLibraries, initName, &newObj);
	if (newObj) {
		Tcl_SetHashValue (hPtr, (char*) xlib);
//		printf ("new shared object %x\n", xlib);
		if (TCL_OK !=  (*init) (this)) return TCL_ERROR;
	} else {
//		printf ("old shared object %x\n", xlib);
	}
	
	feature = strtok (argv[3], "\n");
	
  } else {
	Tcl_AppendResult (interp, "\nusage: \"", argv[0],
			  " className ?libName? body\"", (char*) 0);
	return TCL_ERROR;
  }

  Coc_Type* type = (Coc_Type*) 0;

  //
  // define type features
  //
  if (feature) do {
	int cnt;
	char c;
	char** list = (char**) 0;
	if (TCL_OK != Tcl_SplitList (interp, feature, &cnt, &list)) {
		Tcl_AppendResult (interp, "\nsyntax error in body of cocos_class, feature \"",
				  feature, "\"", (char*) 0);
		return TCL_ERROR;
	}
	if (cnt == 0) continue;
	c = *list[0];
	if (c == 'i' && !strcmp (list[0], "inherit")) {
		if (TCL_OK != doInherit (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else if (c == 'c' && !strcmp (list[0], "constructor")) {
		if (TCL_OK != doConstructor (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else if (c == 'd' && !strcmp (list[0], "destructor")) {
		if (TCL_OK != doDestructor (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else if (c == 'm' && !strcmp (list[0], "method")) {
		if (TCL_OK != doMethod (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else if (c == 'p' && !strcmp (list[0], "proc")) {
		if (TCL_OK != doMethod (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else if (c == 'p' && !strcmp (list[0], "public")) {
		if (TCL_OK != doPublic (this, type, libName, xlib, className, cnt, list)) {
			free((char *) list);
			return TCL_ERROR;
		}
	} else {
		Tcl_AppendResult (interp, "\nusage: \"cocos_class ", className, " ",
				  libName, " { ... ", list[0], " ... }\"", (char*) 0);
		free((char *) list);
		return TCL_ERROR;
	}
	free((char *) list);
	continue;
  } while (feature = strtok (NULL, "\n"));

  if (!type) {
  	if (!(type = Coc_Type::create (this, className, 0, (Coc_StaticProc) 0, (Coc_Proc) 0))) {
		Tcl_AppendResult (interp, "\ncan't define type \"",
				  className, "\"", (char*) 0);
		return TCL_ERROR;
  	}
  }
  return TCL_OK;
}

int
Coc_Info::enumDef (Tcl_Interp* dummy, int argc, char** argv)
{
  if (argc != 3) {
	Tcl_AppendResult (interp, "\nusage: \"cocos_enum enumName elements\"",
			  (char*) 0);
	return TCL_ERROR;
  }
  char* enumName = argv[1];
  char* elements = argv[2];

  int cnt;
  char** list = (char**) 0;
  if (TCL_OK != Tcl_SplitList (interp, elements, &cnt, &list)) {
	Tcl_AppendResult (interp, "\nsyntax error in element list of cocos_enum",
			  (char*) 0);
	return TCL_ERROR;
  }
  if (!Coc_EnumType::create (this, enumName, cnt, list)) {
	return TCL_ERROR;
  } else {
	return TCL_OK;
  }
}

int
Coc_Info::schemaDef (Tcl_Interp* dummy, int argc, char** argv)
{
  if (argc != 2) {
	Tcl_AppendResult (interp, "\nusage: \"cocos_schema schemaName\"",
			  (char*) 0);
	return TCL_ERROR;
  }

  //
  // load shared object lib
  //

  void* xlib;
  char initName [_POSIX_PATH_MAX];
  char* libName = argv[1];
  char* s;
  char* fileName;
  int (*init) (Coc_Info*);

  /*
   *    Extract the name of the initialization function from a library 
   *    path name in the form:
   *        [<path>/]lib<exName>.so.<major>[.<minor> ...]
   */
  fileName = ((s = strrchr (libName, '/')) != NULL )? s+1 : libName;

  if (sscanf (fileName, "lib%[^.].so.%*[0123456789.]", initName) != 1 ) {
	strcpy (initName, fileName);
	libName = (char*) 0;
  }

  strcat (initName, "_Init");

  /*
   *    If the named library does not exist, if it's not loadable or if
   *    it does not define the init proc, report an error
   */
  if ((xlib = dlopen (libName, 1)) == NULL) {
  	Tcl_AppendResult (interp, "loading \"", libName, "\": ",
  			  dlerror(), (char*) 0);
  	return TCL_ERROR;
  }
  if ((init = (int (*) (Coc_Info*)) dlsym (xlib, initName)) == NULL) {
  	Tcl_AppendResult (interp, "searching \"", initName, "\": ",
  			  dlerror(), (char*) 0);
  	if (xlib != NULL) dlclose(xlib);
  	return TCL_ERROR;
  }

  /*    
   *    Try to init the extension and report success or failure
   */
  int newObj;
  Tcl_HashEntry* hPtr;

  hPtr = Tcl_CreateHashEntry (&sharedLibraries, initName, &newObj);
  if (newObj) {
  	Tcl_SetHashValue (hPtr, (char*) xlib);
//		printf ("new shared object %x\n", xlib);
  	if (TCL_OK !=  (*init) (this)) return TCL_ERROR;
  } else {
//		printf ("old shared object %x\n", xlib);
  }
  return TCL_OK;  
}

