/*
 * bind/member-test.cc --
 *
 *	Not really part of the software. This module is used to test
 *	different uses of member address operators between the at&t and
 *	gnu compilers
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: member-test.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: member-test.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: member-test.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <stdio.h>

#ifdef __GNUC__
# define MEMB_ADDR &
#else
/* when using the CenterLine compiler, expect the following warning:
 * "t.cc", line 24: warning: cast to char *(*)(foo*) of pointer to member (anachronism)
 */
# define MEMB_ADDR
#endif

struct foo {
  char* name;
  char* whoami() { return name; }
  foo(char* name_): name(name_) {}
};

typedef char* (*whoami_proc) (foo* me);

void
print (foo* object, whoami_proc proc)
{ printf ("name=\"%s\"\n", proc (object)); }

int
main (int c, char** v)
{ foo* x = new foo ("Foo");
  print (x, (whoami_proc) MEMB_ADDR foo::whoami);
  return 0;
}
