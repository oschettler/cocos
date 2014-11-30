%{
/*
 * parser/lex.l --
 *
 *	The scanner for copag - flex style
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: lex.l,v 1.1 1995/10/25 20:09:24 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: lex.l,v $
 * Revision 1.1  1995/10/25  20:09:24  olav
 * 1
 *
 */

static char rcsid[] = "$Id: lex.l,v 1.1 1995/10/25 20:09:24 olav Exp $";

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <tcl.h>

#include "bind.h"
#include "tree.h"
#include "parse.tab.h"

Tcl_DString String;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
{ \
  result = ((fileBuffer + fileLength - filePtr) <= 0) \
    ? YY_NULL \
    : (buf[0] = *filePtr++, 1); \
  if (buf[0] == '\n') yylineno++; \
}
%}

%x pattern_state
%x literal
%x rem

id			[_a-zA-Z][_a-zA-Z0-9]*

%%
"/*"			{ BEGIN (rem); }
<rem>[^*\n]*		; 
<rem>"*"+[^*/\n]*	;
<rem>\n			;
<rem>"*"+"/"		{ BEGIN (INITIAL); }
<rem><<EOF>>		{ yyerror ("EOF in comment"); }

"#".*$			;

"\""			{ BEGIN (literal);
			  Tcl_DStringInit (&String);
			}
<literal>"\\\""|"\\\\"	{ Tcl_DStringAppend (&String, yytext, -1); }
<literal>"\""		{ BEGIN (INITIAL);
			  yylval = new Node (this, t_LITERAL,
					   Tcl_DStringValue (&String), 0);
			  return t_LITERAL;
			}
<literal>\n		{ Tcl_DStringAppend (&String, yytext, 1); }
<literal>.		{ Tcl_DStringAppend (&String, yytext, 1); }
<literal><<EOF>>	{ yyerror ("EOF in literal"); }

"<"			{ BEGIN (pattern_state);
			  Tcl_DStringInit (&String);
			}
<pattern_state>">"	{ BEGIN (INITIAL);
			  yylval = new Node (this, t_PATTERN,
					   Tcl_DStringValue (&String), 0);
			  return t_PATTERN;
			}
<pattern_state>\n	{ Tcl_DStringAppend (&String, yytext, 1); }
<pattern_state>.		{ Tcl_DStringAppend (&String, yytext, 1); }
<pattern_state><<EOF>>	{ yyerror ("EOF in pattern_state"); }

"{"			{ yylval = new Node (this, '{', "{", 0);
			  return '{'; }
"}"			{ yylval = new Node (this, '}', "}", 0);
			  return '}';
			}
"|"			{ yylval = new Node (this, '|', "|", 0);
			  return '|';
			}
"-syntax"		{
			  yylval = new Node (this, t__syntax, "t__syntax", 0);
			  return t__syntax;
			}
"-ignore"		{
			  yylval = new Node (this, t__ignore, "t__ignore", 0);
			  return t__ignore;
			}
"-left"			{
			  yylval = new Node (this, t__left, "t__left", 0);
			  return t__left;
			}
"-nonassoc"		{
			  yylval = new Node (this, t__nonassoc, "t__nonassoc", 0);
			  return t__nonassoc;
			}
"-pattern"		{
			  yylval = new Node (this, t__pattern, "t__pattern", 0);
			  return t__pattern;
			}
"-prec"			{
			  yylval = new Node (this, t__prec, "t__prec", 0);
			  return t__prec;
			}
"-right"		{
			  yylval = new Node (this, t__right, "t__right", 0);
			  return t__right;
			}
"-separator"		{
			  yylval = new Node (this, t__separator, "t__separator", 0);
			  return t__separator;
			}
"token"		{
			  yylval = new Node (this, t_token, "t_token", 0);
			  return t_token;
			}
"list"			{
			  yylval = new Node (this, t_list, "t_list", 0);
			  return t_list;
			}
"opt"			{
			  yylval = new Node (this, t_opt, "t_opt", 0);
			  return t_opt;
			}
"prec"			{
			  yylval = new Node (this, t_prec, "t_prec", 0);
			  return t_prec;
			}
"repeat"		{
			  yylval = new Node (this, t_repeat, "t_repeat", 0);
			  return t_repeat;
			}
"rule"			{
			  yylval = new Node (this, t_rule, "t_rule", 0);
			  return t_rule;
			}
"syntax"		{
			  yylval = new Node (this, t_syntax, "t_syntax", 0);
			  return t_syntax;
			}
{id}":"			{
			  yytext [yyleng-1] = '\0';
			  yylval = new Node (this, t_tag, yytext, 0);
			  return t_tag;
			}
{id}			{
			  yylval = new Node (this, t_ID, yytext, 0);
			  return t_ID;
			}
[ \t]*			;
.			{ yyerror ("illegal token: \"%s\"", yytext); }
\n			;

%%
