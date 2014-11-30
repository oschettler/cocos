/*
 * bind/ostream.cc --
 *
 *	Example code of some wrapper functions for C++ ostreams
 *	using the cocos API
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: ostream.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: ostream.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: ostream.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <tcl.h>
#include <iostream.h>
#include "bind.h"

Coc_EnumType* SelEnumType;

extern "C" int
ostream_Init (Coc_Info* info)
{
  printf ("ostream!\n");
  SelEnumType = 0;

  return TCL_OK;
}

enum selEnum {E_out, E_err};

extern "C" int
new_ostream (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  if (!SelEnumType) {
	// get pointer to type `sel'; we know that it is an enum, so `typeProc' is discarded
	Coc_TypeProc typeProc;
	Coc_getType (info, "sel", typeProc, SelEnumType);
  }
  Coc_Value* use;
  if (!(use = Coc_getParameter (info, "use", parmCnt, parm, (Coc_TypeProc) Coc_enumType, SelEnumType))) {
	result.oval = (void*) 0;
	return TCL_ERROR;
  }
  switch (use->ival) {
  case E_out:
	result.oval = (void*) &cout; break;
  case E_err:
	result.oval = (void*) &cerr; break;
  }
  return TCL_OK;
}

extern "C" int
env_print (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  ostream* s = (ostream*) value->oval;

  if (!(value = Coc_useParameter (1, parmCnt, parm)))
	return TCL_ERROR;
  char* me = (char*) value->sval;

  *s << me << endl;
  
  return TCL_OK;
}

int
main (int argc, char** argv)
{
  char buffer [1000];
  Tcl_Interp* interp = Tcl_CreateInterp();

  new Coc_Info (interp);
  
  while (fputs ("% ", stdout), gets (buffer)) {
	Tcl_ResetResult (interp);
	if (Tcl_Eval (interp, buffer) != TCL_OK) {
		printf ("error: %s\n", interp->result);
	} else {
		if (interp->result[0]) {
			printf ("%s\n", interp->result);
		}
	}
  }
  return 0;
}
