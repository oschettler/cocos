%{
/*
 * parser/parse.l --
 *
 *	The parser for copag - bison style
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: parse.y,v 1.1 1995/10/25 20:09:25 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: parse.y,v $
 * Revision 1.1  1995/10/25  20:09:25  olav
 * 1
 *
 */

static char rcsid[] = "$Id: parse.y,v 1.1 1995/10/25 20:09:25 olav Exp $";

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <tcl.h>

extern "C" {
void* malloc (int);
void free (void*);
}

#include "bind.h"
#include "tree.h"

%}

%token t_ID
%token t_LITERAL
%token t_PATTERN
%token t__export
%token t__ignore
%token t__left
%token t__nonassoc
%token t__pattern
%token t__prec
%token t__right
%token t__separator
%token t__syntax
%token t__type
%token t_tag
%token t_token
%token t_list
%token t_opt
%token t_prec
%token t_range
%token t_repeat
%token t_rule
%token t_syntax

%%
cocos	: t_syntax t_ID '{' declaration_list rule_list '}'
	  { tree = $$ = new Node (this, t_syntax, $2->text, 2);
	    $$->kid (0, $4);
	    $$->kid (1, $5);
	  }
	;

declaration_list
	: declaration_list declaration
	  { $$ = new Node (this, Node::t_declaration_list, "declaration_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $2);
	  }
	| /**/
	  { $$ = (Node*) 0; }
	;

declaration
	: token
	| prec

token	: t_token t_ID token_property_list
	  { $$ = new Node (this, t_token, $2->text, 1);
	    $$->kid (0, $3);
            if ($$->enter())
		yyerror ("symbol \"%s\" already defined", $$->text);
	    }
	;

token_property_list
	: token_property_list token_property
	  { $$ = new Node (this, Node::t_token_property_list, "token_property_list", 2);
	    $$->kids [0] = $1;
	    $$->kids [1] = $2;
	  }
	| { $$ = (Node*) 0; }
	;

token_property
        : t__pattern t_PATTERN
          { $$ = $2; }
        | t__ignore
	;

prec	: t_prec prec_property id_or_literal_list
	  { $$ = new Node (this, t_prec, "prec", 2);
	    $$->kid (0, $2);
	    $$->kid (1, $3);
	  }
	;

id_or_literal_list
	: id_or_literal_list id_or_literal
	  { $$ = new Node (this, Node::t_id_or_literal_list, "id_or_literal_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $2);
	  }
	| id_or_literal
	;

id_or_literal
	: t_ID | t_LITERAL
	;

prec_property
	: t__left
	| t__right
	| t__nonassoc
	| /**/
	;

rule_list
	: rule_list rule
	  { $$ = new Node (this, Node::t_rule_list, "rule_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $2);
	  }
	| rule
	;

rule	: t_rule t_ID rule_property_list alternatives
	  { $$ = new Node (this, t_rule, $2->text, 2);
	    $$->kid (0, $3);
	    $$->kid (1, $4);
	  }
	;

rule_property_list
	: rule_property_list rule_property
	  { $$ = new Node (this, Node::t_rule_property_list, "rule_property_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $2);
	  }
	| /**/
	  { $$ = (Node*) 0; }
	;

rule_property
	: t__syntax
	;

alternatives
	: '{' alternative_list '}'
	  { $$ = new Node (this, Node::t_alternatives, "alternatives", 1);
	    $$->kid (0, $2); 
	  }
	;

alternative_list
	: alternative_list '|' words
	  { $$ = new Node (this, Node::t_alternative_list, "alternative_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $3);
	  }
	| words
	;

words	: word_list rule_prec
	  { $$ = new Node (this, Node::t_words, "words", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $2);
	    if (nrules == 0) {
		start = $$;
	    }
	    nrules++;
	    /* nitems++;		/* one extra per rule for trailing 0 */
	  }
	;

word_list
	: word_list tag word
	  { $$ = new Node (this, Node::t_word_list, "word_list", 2);
	    $$->kid (0, $1);
	    $$->kid (1, $3, ($2) ? $2->text : 0);
	  }
	| /**/
	  { $$ = (Node*) 0; }

word
	: t_PATTERN
	| t_ID
	| t_LITERAL
	| t_opt			alternatives
	  { $$ = new Node (this, t_opt, "word", 1);
	    $$->kid (0, $2);
	  }
	| t_list separator	alternatives
	  { $$ = new Node (this, t_list, "word", 2);
	    $$->kid (0, $2);
	    $$->kid (1, $3);
	  }
	| t_repeat separator	alternatives
	  { $$ = new Node (this, t_repeat, "word", 2);
	    $$->kid (0, $2);
	    $$->kid (1, $3);
	  }
	|			alternatives
	;

tag	: /**/
	  { $$ = (Node*) 0; }
	| t_tag
	;

separator
	: { $$ = (Node*) 0; }
	| t__separator t_LITERAL
	  { $$ = $1; 
	    $$->setText ($2->text); 
	  }
	;

rule_prec	
	: t__prec t_ID
	  { $$ = $1;
	    $$->setText ($2->text);
	  }
	| /**/
	  { $$ = (Node*) 0; }
	;

%%

