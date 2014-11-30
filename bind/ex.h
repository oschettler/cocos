/*
 * bind/ex.h --
 *
 *	Classes for cocos API example
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: ex.h,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: ex.h,v $
 * Revision 1.1  1995/10/25  20:09:22  olav
 * 1
 *
 */

#include <string.h>

struct foo;
struct bar;

struct foo {
  int a;
  char* s;
  foo* obj;
  foo(): a(0), s(0), obj(0) { printf ("foo (0)\n"); }
  foo (char* s_): a(0), s(strdup(s_)), obj(0) { printf ("foo (name=\"%s\")\n", s); }
  int incr (int i) { printf ("incr(%d) -> %d\n", i, a+i); return a += i; }
};

struct bar: public foo {
  int b;
  char* t;
  bar(): b(0), t(0) { printf ("bar (0)\n"); }
  bar (char* s_): b(0), t(0), foo (s_) { printf ("bar (name=\"%s\")\n", s); }
};

inline int
ex_add (foo* a, foo* b) { return a->a + a->a; }
