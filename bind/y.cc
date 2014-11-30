/*
 * bind/y.cc --
 *
 *	A test for a user-defined id generator
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: y.cc,v 1.1 1995/10/25 20:09:23 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: y.cc,v $
// Revision 1.1  1995/10/25  20:09:23  olav
// 1
//
 */

static char rcsid[] = "$Id: y.cc,v 1.1 1995/10/25 20:09:23 olav Exp $";

#include <stdio.h>
#include <tcl.h>
#include "bind.h"

extern "C" void exit (int);

char*
my_genId (Coc_Type* type, void* obj)
{
  static char id [512];
  Tcl_CmdInfo cmdInfo;
  do {
	sprintf (id, "tralla_%s_%d", type->name, ++type->maxId);
  } while (Tcl_GetCommandInfo (type->info->interp, id, &cmdInfo));
  return id;
}

int
main (int argc, char** argv)
{
  char buffer [1000];
  Tcl_Interp* interp = Tcl_CreateInterp();

//  Init_Schema (new TclInfo (interp));
  new Coc_Info (interp, my_genId);
  
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
