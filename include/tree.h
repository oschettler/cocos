/*
 * parser/lib/tree.h
 *
 *	Declarations for parse trees used at run-time by
 *	generated language processors
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: tree.h,v 1.1 1995/10/25 20:09:25 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: tree.h,v $
 * Revision 1.1  1995/10/25  20:09:25  olav
 * 1
 *
 */

#ifndef TREE_H
# define TREE_H

#ifndef COC_EXTERN
# define COC_EXTERN extern
#endif

class ParseTree;

struct Node {
  enum Category { use, dcl };
  int type;
  char* text;
  char* tag;
  Category category;
  int lineno;
  int sym;
  int start;
  int end;
Node* up;
  int nrKids;
  Node** kids;
  Node* nxtSym;
  Node* next;
  ParseTree* tree;

  Node (ParseTree* tree, int type, Node* first, Node* last, int nrKids = 0, Category _category = use);
  Node (ParseTree* tree, int type, int _start, int _end, int nrKids = 0, Category _category = use);
  Node (int type, char* text, int nrKids = 0);
  static Node* special (Tcl_Interp* interp, char* tag, char* text, char* sep = "");
  ~Node();
  void kid (int ix, Node* kid, char* tag = 0);
  void append (Node* kid);
  char* getText (int reset = 0);
  void dump (int lv);
  void export (FILE* f, int lv);
  Node* enter();
  int traverse (char* tag, int type, char* var, char* action, int one = 0, int outermost = 1);
  Node* parent (int type);
};

struct File {
  Tcl_Interp* interp;
  char* name;
  int length;
  char* ptr;
  int offset;
  char* buffer;
  int desc;
  int lineno;
  int valid;

  File (Tcl_Interp* interp, char* name);
  ~File();
};

COC_EXTERN Coc_Type* coc_Node;

typedef int (*fct) ();

struct ParseTree {
  ParseTree* parent;

  Node* root;
//  Node* start;
//  int prec;
  int valid;
  int mark;
  Tcl_HashTable symTb;

  jmp_buf errorEnv;
  
  Coc_EnumType* coc_NodeType;
  File* file;
  
  fct parse;

  Tcl_Interp* interp;

  void error (char*,...);
  Node* findSymbol (char* text);
  void dumpSymbols ();
  char* getText();
  ParseTree (Tcl_Interp* _interp, char* fileName, ParseTree*& g_parseTree, Coc_EnumType* nodeType, fct parse);
  ParseTree (ParseTree& tree, ParseTree*& g_parseTree, Coc_EnumType* nodeType, fct parse);
  virtual ~ParseTree ();

#if 0
  static int yyparse ();
  static void yyerror (char* s, ...);
  static int yy_get_next_buffer();
  static int yyinput();
  static int yywrap();
  static int yylex();
#endif
};

char* tokenize (char* string);

#define YYSTYPE Node*

// COC_EXTERN char** tags;
// COC_EXTERN int nitems;
// COC_EXTERN int nrules;
// COC_EXTERN int nsyms;
// COC_EXTERN int ntokens;
// COC_EXTERN int nvars;

// COC_EXTERN short *ritem;
// COC_EXTERN short *rlhs;
// COC_EXTERN short *rrhs;
// COC_EXTERN short *rprec;
// COC_EXTERN short *rprecsym;
// COC_EXTERN short *sprec;
// COC_EXTERN short *rassoc;
// COC_EXTERN short *sassoc;
// COC_EXTERN short *rline;		/* Source line number of each rule */

// COC_EXTERN int start_symbol;
#endif
