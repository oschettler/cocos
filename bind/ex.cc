/*
 * bind/ex.cc --
 *
 *	Generated example code for using the cocos API
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: ex.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: ex.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: ex.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <tcl.h>
#include "bind.h"
#include <stdio.h>

#include "ex.h"

extern "C" int
ex_Init (Coc_Info* info)
{
  printf ("ex!\n");
  return TCL_OK;
}

extern "C" int
new_foo (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.oval = (void*) new foo;
  return TCL_OK;
}

extern "C" int
del_foo (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ delete (foo*) obj;
  return TCL_OK;
}

extern "C" int
foo_incr (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  int incr = (int) value->ival;

  result.ival =  ((foo*) obj)->incr (incr);
  return TCL_OK;
}

extern "C" int
foo_set_a (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value = Coc_useParameter (0, parmCnt, parm);
  if (!value)
	return TCL_ERROR;
  ((foo*) obj)->a = (int) value->ival;
  return TCL_OK;
}

extern "C" int
foo_get_a (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.ival = ((foo*) obj)->a;
  return TCL_OK;
}

extern "C" int
foo_set_s (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value = Coc_useParameter (0, parmCnt, parm);
  if (!value)
	return TCL_ERROR;
  ((foo*) obj)->s = strdup (value->sval);
  return TCL_OK;
}

extern "C" int
foo_get_s (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((foo*) obj)->s;
  return TCL_OK;
}

extern "C" int
foo_set_obj (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value = Coc_useParameter (0, parmCnt, parm);
  if (!value)
	return TCL_ERROR;
  ((foo*) obj)->obj = (foo*) value->oval;
  return TCL_OK;
}

extern "C" int
foo_get_obj (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.oval = ((foo*) obj)->obj;
  return TCL_OK;
}

extern "C" int
new_bar (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.oval = (void*) new bar;
  return TCL_OK;
}

extern "C" int
del_bar (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ delete (bar*) obj;
  return TCL_OK;
}

extern "C" int
bar_set_b (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value = Coc_useParameter (0, parmCnt, parm);
  if (!value)
	return TCL_ERROR;
  ((bar*) obj)->b = (int) value->ival;
  return TCL_OK;
}

extern "C" int
bar_get_b (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.ival = ((bar*) obj)->b;
  return TCL_OK;
}

extern "C" int
bar_set_t (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value = Coc_useParameter (0, parmCnt, parm);
  if (!value)
	return TCL_ERROR;
  ((bar*) obj)->t = strdup (value->sval);
  return TCL_OK;
}

extern "C" int
bar_get_t (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((bar*) obj)->t;
  return TCL_OK;
}

extern "C" int
ex_add (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  foo* a = (foo*) value->oval;

  if (!(value = Coc_useParameter (1, parmCnt, parm)))
	return TCL_ERROR;
  foo* b = (foo*) value->oval;

  result.ival =  ex_add (a, b);
  return TCL_OK;
}

