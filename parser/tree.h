/*
 * parser/tree.h
 *
 *	Declarations for parse trees needed to boot-strap copag
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
  enum Type {
    t_declaration_list = -1,
    t_rule_property_list = -2,
    t_range_property_list = -3,
    t_rule_list = -4,
    t_id_list = -5,
    t_alternative_list = -6,
    t_words = -7,
    t_word_list = -8,
    t_single = -9,
    t_token_property_list = -10,
    t_id_or_literal_list = -11,
    t_word = -12,
    t_alternatives = -13
};

  int type;
  char* tag;
  int lineno;
  int sym;
  int prec;
  int assoc;
  char* text;
  Node* up;
  int nrKids;
  Node** kids;
  Node* next;
  ParseTree* tree;
  int mark;

  static Coc_Type* cocosType;

  Node (ParseTree* tree, int type, char* text, int nrKids);
  ~Node();
  void kid (int ix, Node* kid, char* tag = 0);
  void setText (char* _text);
  void dump (int lv, int shallow = 0);
  Node* enter();
  int traverse (int type, char* var, char* action, int one, int traversal);
  Node* parent (int type);
};

struct ParseTree {
  Node* tree;
  Node* start;
  int prec;
  int valid;
  int mark;

  Tcl_HashTable symTb;
  char* fileName;
  int fileLength;
  char* filePtr;
  char* fileBuffer;
  int fileDesc;
  jmp_buf errorEnv;

  int yyparse ();
  void yyerror (char* s, ...);
  int yy_get_next_buffer();
  int yyinput();
  int yywrap();
  int yylex();

  int yydebug;
  int yylineno;
  Tcl_Interp* interp;

  Node* findSymbol (char* text);
  void dumpSymbols ();
  ParseTree (Tcl_Interp* _interp, char* fileName, int dump, int debug);
  ~ParseTree ();
};

int mapType (int type, char*& name);
char* tokenize (char* string);

#define YYSTYPE Node*

COC_EXTERN int nrules;

#if 0
COC_EXTERN char** tags;
COC_EXTERN int nitems;
COC_EXTERN int nsyms;
COC_EXTERN int ntokens;
COC_EXTERN int nvars;

COC_EXTERN short *ritem;
COC_EXTERN short *rlhs;
COC_EXTERN short *rrhs;
COC_EXTERN short *rprec;
COC_EXTERN short *rprecsym;
COC_EXTERN short *sprec;
COC_EXTERN short *rassoc;
COC_EXTERN short *sassoc;
COC_EXTERN short *rline;		/* Source line number of each rule */

COC_EXTERN int start_symbol;
#endif
#endif
