/*
 * bind/schema.cc --
 *
 *	A self contained example for using the cocos API
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: schema.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: schema.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: schema.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <stdio.h>
#include <string.h>
#include <tcl.h>
#include "bind.h"

extern "C" void exit (int);

struct Foo {
  int a;
  char* s;
  Foo* obj;
  Foo(): a(0), s(0), obj(0) { printf ("Foo (0)\n"); }
  Foo (char* s_): a(0), s(strdup(s_)), obj(0) { printf ("Foo (name=\"%s\")\n", s); }
};

struct Bar: public Foo {
  int b;
  char* t;
  Bar(): b(0), t(0) { printf ("Bar (0)\n"); }
  Bar (char* s_): b(0), t(0), Foo (s_) { printf ("Bar (name=\"%s\")\n", s); }
};

extern "C" void*
new_Foo (TclInfo* info, int parmCnt, TclVariable parm[])
{ TclValue* name = getParameter (info, "name", parmCnt, parm, stringType);
  if (name) {
	return (void*) new Foo (name->sval);
  }
  return (void*) new Foo;
}

extern "C" int
del_Foo (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ TclValue* tralla = getParameter (info, "tralla", parmCnt, parm, intType);
  if (tralla) {
	printf ("deleted with -tralla!\n");
  } else {
	printf ("deleted!\n");
  }
  delete (Foo*) obj;
  return TCL_OK;
}

extern "C" void*
new_Bar (TclInfo* info, int parmCnt, TclVariable parm[])
{ TclValue* name = getParameter (info, "name", parmCnt, parm, stringType);
  if (name) {
	return (void*) new Bar (name->sval);
  }
  return (void*) new Bar;
}

extern "C" int
del_Bar (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ TclValue* tralla = getParameter (info, "tralla", parmCnt, parm, intType);
  if (tralla) {
	printf ("deleted with -tralla!\n");
  } else {
	printf ("deleted!\n");
  }
  delete (Bar*) obj;
  return TCL_OK;
}

extern "C" int
set_a (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ ((Foo*) obj)->a = useParameter (0, parmCnt, parm)->ival;
  return TCL_OK;
}

extern "C" int
get_a (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.ival = ((Foo*) obj)->a;
  return TCL_OK;
}

extern "C" int
set_b (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ ((Bar*) obj)->b = useParameter (0, parmCnt, parm)->ival;
  return TCL_OK;
}

extern "C" int
get_b (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.ival = ((Bar*) obj)->b;
  return TCL_OK;
}

extern "C" int
set_s (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ ((Foo*) obj)->s = my_strdup (useParameter (0, parmCnt, parm)->sval);
  return TCL_OK;
}

extern "C" int
get_s (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.sval = ((Foo*) obj)->s;
  return TCL_OK;
}

extern "C" int
set_t (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ ((Bar*) obj)->t = my_strdup (useParameter (0, parmCnt, parm)->sval);
  return TCL_OK;
}

extern "C" int
get_t (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.sval = ((Bar*) obj)->t;
  return TCL_OK;
}

extern "C" int
set_obj (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ ((Foo*) obj)->obj = (Foo*) useParameter (0, parmCnt, parm)->oval;
  return TCL_OK;
}

extern "C" int
get_obj (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.oval = ((Foo*) obj)->obj;
  return TCL_OK;
}

extern "C" int
incr_a (TclInfo* info, void* obj, TclValue& result, int parmCnt, TclVariable parm[])
{ result.ival = ((Foo*) obj)->a += useParameter (0, parmCnt, parm)->ival;
  return TCL_OK;
}

char* blubberEnum [] = { "blubber", "waber", "glibber" };

extern "C" int
Schema_Init (TclInfo* info)
{
  static int initialized = 0;

  if (initialized) {
	printf ("Schema_Init already done\n");
	initialized = 1;
	return TCL_OK;
  }
  printf ("Schema_Init\n");
/*
  TclType* type1;
  if (0 == (type1 = TclType::create (info, "foo", 0, new_Foo, del_Foo))) {
	fprintf (stderr, "can't create type1 \"foo\"\n");
	exit (-1);
  }
  type1->addAttribute ("a", get_a, set_a, intType);
  type1->addAttribute ("s", get_s, set_s, stringType);
  type1->addAttribute ("obj", get_obj, set_obj, objectType, type1);

  TclEnumType* type0 = TclEnumType::create (info, "blubberEnum", 3, blubberEnum);

  type1->addAttribute ("soft", get_a, set_a, (TypeProc) enumType, type0);
  
  TclProcedure* proc = type1->addProcedure ("incr", incr_a, 1, intType);
  proc->putParameter (0, "value", intType);

  TclType* type2;
  if (0 == (type2 = TclType::create (info, "bar", 0, new_Foo, del_Foo))) {
	fprintf (stderr, "can't create type2 \"bar\"\n");
	exit (-1);
  }
  type2->addAttribute ("a", get_a, set_a, intType);
  type2->addAttribute ("s", get_s, set_s, stringType);
  type2->addAttribute ("obj", get_obj, set_obj, objectType, type1);
  
  proc = type2->addProcedure ("incr", incr_a, 1, intType);
  proc->putParameter (0, "value", intType);
*/
  return TCL_OK;
}

