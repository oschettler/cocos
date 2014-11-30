/*
 * bind/x.cc --
 *
 *	A minimal wish interpreter with cocos capability
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: x.cc,v 1.1 1995/10/25 20:09:23 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: x.cc,v $
// Revision 1.1  1995/10/25  20:09:23  olav
// 1
//
 */

#include <tcl.h>
#include <stdio.h>
#include "bind.h"

extern "C" void exit (int);

int
main (int argc, char** argv)
{
  char buffer [1000];
  Tcl_Interp* interp = Tcl_CreateInterp();

//  Init_Schema (new TclInfo (interp));
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
