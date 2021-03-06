#!/bin/sh
#
# Test the `t1' application
#
# $Id: t0003a.sh,v 1.1 1995/10/25 20:09:26 olav Exp $
#
# $Log: t0003a.sh,v $
# Revision 1.1  1995/10/25  20:09:26  olav
# 1
#
#

here=`pwd`
if test $? -ne 0; then exit 1; fi

tmp=/tmp/tmp.$$
if test $? -ne 0; then exit 1; fi

mkdir $tmp
if test $? -ne 0; then exit 1; fi

cd $tmp
if test $? -ne 0; then exit 1; fi

fail()
{
	echo FAILED 1>&2
	cd $here
	rm -rf $tmp
	exit 1
}
pass()
{
	echo PASSED 1>&2
	cd $here
	rm -rf $tmp
	exit 0
}
trap "fail" 1 2 3 15

#
# echo commands so we can tell what failed
#
dir=`pwd`
dir=`basename $dir`
if [ $dir = 'baseline' ]; then
  fail
fi

(cd $here/t1; gmake)
if test $? -ne 0; then fail; fi

diff -c - $here/t1/t1.l <<'__EOF__'

%{
/*
 * t1.l -- lex input generated by cocos-R3
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * email: olav@werbemittel.com
 */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <tcl.h>

#include <bind.h>
#include "tree.h"
#include "t1.tab.h"

extern ParseTree* t1Tree;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
{ \
  result = ((t1Tree->file->buffer + t1Tree->file->length - t1Tree->file->ptr) <= 0) \
    ? YY_NULL \
    : (buf[0] = *t1Tree->file->ptr++, 1); \
  if (buf[0] == '\n') t1Tree->file->lineno++; \
}

#define tokenNode(id) \
  new Node (t1Tree, id-258, t1Tree->file->offset, t1Tree->file->offset + yyleng); \
  t1Tree->file->offset += yyleng;
%}

%%

"-"	{ t1lval = tokenNode (t__2d); 	return t__2d; }
"e"	{ t1lval = tokenNode (t_e); 	return t_e; }
"f"	{ t1lval = tokenNode (t_f); 	return t_f; }
"g"	{ t1lval = tokenNode (t_g); 	return t_g; }
"/"	{ t1lval = tokenNode (t__2f); 	return t__2f; }
"h"	{ t1lval = tokenNode (t_h); 	return t_h; }
"i"	{ t1lval = tokenNode (t_i); 	return t_i; }
"("	{ t1lval = tokenNode (t__28); 	return t__28; }
"j"	{ t1lval = tokenNode (t_j); 	return t_j; }
")"	{ t1lval = tokenNode (t__29); 	return t__29; }
";"	{ t1lval = tokenNode (t__3b); 	return t__3b; }
"a"	{ t1lval = tokenNode (t_a); 	return t_a; }
"*"	{ t1lval = tokenNode (t__2a); 	return t__2a; }
"b"	{ t1lval = tokenNode (t_b); 	return t_b; }
"+"	{ t1lval = tokenNode (t__2b); 	return t__2b; }
"c"	{ t1lval = tokenNode (t_c); 	return t_c; }
","	{ t1lval = tokenNode (t__2c); 	return t__2c; }
"d"	{ t1lval = tokenNode (t_d); 	return t_d; }
[0-9]+	{ t1lval = tokenNode (INT); 	return INT; }
[_a-z][_a-z0-9]*	{ t1lval = tokenNode (ID); 	return ID; }
\n	{ t1Tree->file->offset += yyleng; }
[ \t]+	{ t1Tree->file->offset += yyleng; }
.	{ t1Tree->file->offset += yyleng; }

%%

int 
t1wrap() 
{ return 1; }


__EOF__
if test $? -ne 0; then fail; fi

diff -c - $here/t1/t1.y <<'__EOF__'
%{
/*
 * t1.y -- yacc input generated by cocos-R3
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * email: olav@werbemittel.com
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#include <tcl.h>

#include "bind.h"

extern "C" {
void* malloc (int);
void free (void*);
}

#include "tree.h"
ParseTree* t1Tree;

extern int t1lex();

%}

%left t__2b t__2d 
%left t__2a t__2f 
%right UMINUS 
%nonassoc t__28 t__29 
%token t_e
%token t_f
%token t_g
%token t_h
%token t_i
%token ID
%token t_j
%token t__3b
%token INT
%token t_a
%token t_b
%token t_c
%token t__2c
%token t_d

%start test

%%

test
	: 
	  { $$ = t1Tree->root = (Node*) 0; }
	| test1 
	  { $$ = t1Tree->root = new Node (t1Tree, 21, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| test2 
	  { $$ = t1Tree->root = new Node (t1Tree, 21, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| test3 
	  { $$ = t1Tree->root = new Node (t1Tree, 21, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| test4 
	  { $$ = t1Tree->root = new Node (t1Tree, 21, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| test5 
	  { $$ = t1Tree->root = new Node (t1Tree, 21, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	;
test5
	: t_a t_b t_c 
	  { $$ = t1Tree->root = new Node (t1Tree, 22, $1, $3, 3);
	    $$->kid (0, $1, "");
	    $$->kid (1, $2, "");
	    $$->kid (2, $3, "");
	  }
	;
__rule__1
	: t_b 
	| t_c 
	;
test1
	: t_a __rule__1 t_d 
	  { $$ = t1Tree->root = new Node (t1Tree, 23, $1, $3, 3);
	    $$->kid (0, $1, "");
	    $$->kid (1, $2, "");
	    $$->kid (2, $3, "");
	  }
	;
__rule__2
	: t_e 
	;
__rule__4
	: __rule__2 
	  { $$ = t1Tree->root = new Node (t1Tree, -1, $1, $1, 2); 
	    $$->append ($1);
	  }
	| __rule__4 t__2c __rule__2
	  { $$ = $1;
	    $$->append ($3);
	  }
	;
__rule__3	: { $$ = t1Tree->root = (Node*) 0; } 
	| __rule__4
	;

__rule__5
	: t_f 
	;
__rule__6
	: { $$ = t1Tree->root = new Node (t1Tree, -1, 0, 0, 2); }
	| __rule__6 __rule__5
	  { $$ = $1;
	    $$->append ($2);
	  }
	;

test2
	: __rule__3 
	  { $$ = t1Tree->root = new Node (t1Tree, 25, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| __rule__6 
	  { $$ = t1Tree->root = new Node (t1Tree, 25, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	;
__rule__8
	: t_g 
	;
__rule__9
	: { $$ = t1Tree->root = (Node*) 0; }
	| __rule__8
	;

test3
	: __rule__9 
	  { $$ = t1Tree->root = new Node (t1Tree, 28, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	;
__rule__10
	: t_h 
	| t_i 
	;
__rule__11
	: __rule__10
	  { $$ = t1Tree->root = new Node (t1Tree, -1, $1, $1, 2); 
	    $$->append ($1);
	  }
	| __rule__11 t__3b __rule__10
	  { $$ = $1;
	    $$->append ($3);
	  }
	;

__rule__12
	: t_j 
	;
__rule__13
	: __rule__12
	  { $$ = t1Tree->root = new Node (t1Tree, -1, $1, $1, 2); 
	    $$->append ($1);
	  }
	| __rule__13 __rule__12
	  { $$ = t1Tree->root = $1;
	    $$->append ($2);
	  }
	;

test4
	: __rule__11 
	  { $$ = t1Tree->root = new Node (t1Tree, 30, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	| __rule__13 
	  { $$ = t1Tree->root = new Node (t1Tree, 30, $1, $1, 1);
	    $$->kid (0, $1, "");
	  }
	;
%%

static char* NodeTypeName[] = {

 "+", 
 "-", "*", "/", "UMINUS", "(", ")", "t_e", "t_f", "t_g", "t_h", 
 "t_i", "ID", "t_j", "t__3b", "INT", "t_a", "t_b", "t_c", "t__2c", "t_d", 
 "test", "test5", "test1", "__rule__1", "test2", "__rule__2", "__rule__5", "test3", "__rule__8", "test4", 
 "__rule__10", "__rule__12", 
 "any"
};

struct Item {
  enum Type { nil_, id_, literal_, list_, repeat_, opt_ };
  Type type;
  char* tag;
  char* sep;
  char* text;
};

struct Rule {
  char* name;
  int type;
  int first;
  int last;
};

static Coc_EnumType* NodeType;

static struct Item ItemTable[] = {
  
  { Item::id_, "", "", "test1" }, 
  { Item::id_, "", "", "test2" }, 
  { Item::id_, "", "", "test3" }, 
  { Item::id_, "", "", "test4" }, 
  { Item::id_, "", "", "test5" }, 
  { Item::literal_, "", "", "a" }, { Item::literal_, "", "", "b" }, { Item::literal_, "", "", "c" }, 
  { Item::literal_, "", "", "b" }, 
  { Item::literal_, "", "", "c" }, 
  { Item::literal_, "", "", "a" }, { Item::literal_, "", "", "d" }, 
  { Item::literal_, "", "", "e" }, 
  { Item::list_, "", ",", "__rule__2" }, 
  { Item::literal_, "", "", "f" }, 
  { Item::list_, "", "", "__rule__5" }, 
  { Item::literal_, "", "", "g" }, 
  { Item::opt_, "", "", "__rule__8" }, 
  { Item::literal_, "", "", "h" }, 
  { Item::literal_, "", "", "i" }, 
  { Item::repeat_, "", ";", "__rule__10" }, 
  { Item::literal_, "", "", "j" }, 
  { Item::repeat_, "", "", "__rule__12" }, 
  { Item::nil_, 0, 0, 0 }
};

static struct Rule RuleTable[] = {
  { "test", 21, 0, 0 },
  { "test_1", 21, 0, 0 },
  { "test_2", 21, 0, 1 },
  { "test_3", 21, 1, 2 },
  { "test_4", 21, 2, 3 },
  { "test_5", 21, 3, 4 },
  { "test_6", 21, 4, 5 },
  { "test5", 22, 5, 8 },
  { "test5_1", 22, 5, 8 },
  { "__rule__1", 24, 8, 9 },
  { "__rule__1_1", 24, 8, 9 },
  { "__rule__1_2", 24, 9, 10 },
  { "test1", 23, 10, 12 },
  { "test1_1", 23, 10, 12 },
  { "__rule__2", 26, 12, 13 },
  { "__rule__2_1", 26, 12, 13 },
  { "test2", 25, 13, 14 },
  { "test2_1", 25, 13, 14 },
  { "__rule__5", 27, 14, 15 },
  { "__rule__5_1", 27, 14, 15 },
  { "test2_2", 25, 15, 16 },
  { "__rule__8", 29, 16, 17 },
  { "__rule__8_1", 29, 16, 17 },
  { "test3", 28, 17, 18 },
  { "test3_1", 28, 17, 18 },
  { "__rule__10", 31, 18, 19 },
  { "__rule__10_1", 31, 18, 19 },
  { "__rule__10_2", 31, 19, 20 },
  { "test4", 30, 20, 21 },
  { "test4_1", 30, 20, 21 },
  { "__rule__12", 32, 21, 22 },
  { "__rule__12_1", 32, 21, 22 },
  { "test4_2", 30, 22, 23 },
  { 0, 0, 0 }
};

Tcl_HashTable t1Rules;

static Node*
unparse (Coc_Info* info, Rule* rule, int parmCnt, Coc_Variable parm[])
{
  printf ("unparse: <%d: %s>\n", rule->type, rule->name);
  Node* result = new Node (rule->type, "", rule->last - rule->first);
  int ix, i;
  for (i = rule->first, ix = 0; i < rule->last; i++, ix++) {
	Item item = ItemTable [i];
	switch (item.type) {
	case Item::literal_:
		result->kid (ix, new Node (-2, item.text));
		break;
	default: {
		Node* kid;

		char* name;
		if (item.tag[0] != '\0') {
			name = item.tag;
		} else {
			name = item.text;
		} 
		// check if we got a matching command line argument
		Coc_Value* parmValue;
		if (parmValue = Coc_getParameter (info, name, parmCnt, parm, (Coc_TypeProc) Coc_stringType)) {
			// take command line parameter instead of rule text 
			char* tag = strtok (parmValue->sval, " \t");
			
			if (!tag) {
				Tcl_AppendResult (info->interp, "usage: <tag> <text>", (char*) 0);
				return (Node*) 0;
			}
			kid = Node::special (info->interp, tag, tag + strlen (tag) + 1, item.sep);
			if (!kid) {
				return (Node*) 0;
			}
		} else {
			if (item.type == Item::list_ || item.type == Item::opt_) {
				result->kid (ix, (Node*) 0);
				continue;
			} else if (item.type == Item::repeat_) {
				Tcl_ResetResult (info->interp);
				Tcl_AppendResult (info->interp, "\nrepeat of \"", name, "\" needs command line parameter", (char*) 0);
				return (Node*) 0;
			}
		}										
		if (item.type == Item::list_ || item.type == Item::repeat_) {
			if (kid->type != -1) {
				delete kid;
				Tcl_AppendResult (info->interp, "\n\"", name, "\" needs a list", (char*) 0);
				return (Node*) 0;
			}
			result->kid (ix, kid);
		} else {
			if (parmValue) {
				result->kid (ix, kid);
			} else {
				Tcl_HashEntry* hPtr;
				if (!(hPtr = Tcl_FindHashEntry (&t1Rules, item.text))) {
					Tcl_AppendResult (info->interp, "no such rule \"", item.text, "\"", (char*) 0);
					return 0;
				}
				kid = unparse (info, (Rule*) Tcl_GetHashValue (hPtr), parmCnt, parm);
				if (!kid) {
					return (Node*) 0;
				}
				result->kid (ix, kid);
			}
		}
		break;
	}}
  }
  return result; 
}

extern "C" int
t1Tree_unparse (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ 
  Coc_Value* ruleName;
  if (!(ruleName = Coc_getParameter (info, "rule", parmCnt, parm, (Coc_TypeProc) Coc_stringType))) {
	Tcl_AppendResult (info->interp, "\nusage: <tree> unparse -rule <rule> ...", (char*) 0);
	return TCL_ERROR;
  }
  Tcl_HashEntry* hPtr;
  if (!(hPtr = Tcl_FindHashEntry (&t1Rules, ruleName->sval))) {
	Tcl_AppendResult (info->interp, "\nno such rule \"", ruleName->sval, "\"", (char*) 0);
	return TCL_ERROR;
  }
  Rule* rule = (Rule*) Tcl_GetHashValue (hPtr);

  result.oval = unparse (info, rule, parmCnt, parm);
  if (!result.oval) {
	return TCL_ERROR;
  }
  return TCL_OK;
}

extern "C" int
t1_Init (Coc_Info* info) 
{
  printf ("t1!\n");
  NodeType = Coc_EnumType::create (info, "t1NodeType", 34, NodeTypeName);

  Coc_TypeProc cocosProc;
  assert (TCL_OK == Coc_getType (info, "TreeNode", cocosProc, coc_Node));

  Tcl_InitHashTable (&t1Rules, TCL_STRING_KEYS);

  int newObj;
  for (int i = 0; RuleTable [i].name; i++) {
	Tcl_HashEntry* hPtr;
	hPtr = Tcl_CreateHashEntry (&t1Rules, RuleTable [i].name, &newObj);
	assert (newObj);
	Tcl_SetHashValue (hPtr, (char*) &RuleTable [i]);
  }
//  Coc_TypeProc cocosProc;
//  Coc_Type* treeType;
//  assert (TCL_OK == Coc_getType (info, "t1ParseTree", cocosProc, treeType));
//
//  Coc_Procedure* unparseProc = treeType->addProcedure ("unparse", t1Tree_unparse, -1, Coc_stringType, 0, 0);

  return TCL_OK;
}

ParseTree*
t1ParseTree_create (Tcl_Interp* interp, char* name, int debug)
{
  t1debug = debug;
  return new ParseTree (interp, name, t1Tree, NodeType, t1parse);  
}

ParseTree*
t1ParseTree_create (ParseTree* tree, int debug)
{
  t1debug = debug;
  return new ParseTree (*tree, t1Tree, NodeType, t1parse);
}

extern "C" int
new_t1ParseTree (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ 
  Coc_Value* fileName;
  Coc_Value* option;
  int debug = 0;
  int dump = 0;

  if (!(fileName = Coc_getParameter (info, "file", parmCnt, parm, Coc_stringType))) {
    result.oval = (void*) 0;
    return TCL_ERROR;
  }
  if ((option = Coc_getParameter (info, "debug", parmCnt, parm, Coc_intType))) {
	debug = option->ival;
  }
  if ((option = Coc_getParameter (info, "dump", parmCnt, parm, Coc_intType))) {
    dump = option->ival;
  }
  Tcl_ResetResult (info->interp);

  ParseTree* pt = t1ParseTree_create (info->interp, fileName->sval, debug);
  if (!pt->valid) {
    result.oval = (void*) 0;
    return TCL_ERROR;
  } else { 
	result.oval = (void*) pt;
  }

  return TCL_OK;
}

#ifdef MAIN
int
main (int argc, char** argv)
{
  Tcl_Main (argc, argv, Tcl_AppInit);
  return 0;
}
#endif


__EOF__
if test $? -ne 0; then fail; fi

#
# the things tested in this test, worked
#
pass
