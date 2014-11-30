/*
 * parser/tree.cc
 *
 *	Parse trees needed to boot-strap copag
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: tree.cc,v 1.1 1995/10/25 20:09:25 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: tree.cc,v $
// Revision 1.1  1995/10/25  20:09:25  olav
// 1
//
 */

static char rcsid[] = "$Id: tree.cc,v 1.1 1995/10/25 20:09:25 olav Exp $";

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef SVR4
# include <sys/fcntl.h>
#else
# include <sys/fcntlcom.h>
#endif
#include <sys/mman.h>
#include <sys/stat.h>

#include <tcl.h>
#include "bind.h"
#define COC_EXTERN
#include "tree.h"
#include "parse.tab.h"

extern "C" {
    void* malloc (int);
    void free (void*);
    char* strerror (int);
    int close (int);
#ifndef SVR4
    caddr_t mmap (caddr_t, size_t, int, int, int, off_t);
    int munmap (caddr_t, int);
#endif
    void exit (int);
}

Coc_Type* Node::cocosType;

Node::Node (ParseTree* _tree, int _type, char* _text, int _nrKids):
	tag (0), kids (0), next (0), up (0), sym (0), mark (0)
{
    tree = _tree;
    lineno = tree->yylineno;
    type = _type;
    text = strdup (_text);
    nrKids = _nrKids;
    if (nrKids) {
	kids = (Node**) new Node* [nrKids];
    }
}

Node::~Node()
{
    int i;

    if (!this)
	return;
    if (text)
	delete [] text;
    if (tag)
	delete [] tag;
    for (i = 0; i < nrKids; i++)
	delete kids[i];
    if (kids)
	delete [] kids;
}

void
Node::kid (int ix, Node* kid, char* _tag = 0)
{
    if (ix < 0 || ix >= nrKids)
	tree->yyerror ("kids index %d on \"%s\" out of range 0..%d", 
		       ix, text, nrKids-1);
    kids [ix] = kid;
    if (!kid)
	return;
    kid->up = this;
    if (_tag) {
	kid->tag = strdup (_tag);
    }
}

void
Node::setText (char* _text)
{
    if (text)
	delete [] text;
    text = new char [strlen (_text)+1];
    strcpy (text, _text);
}

void
Node::dump (int lv, int shallow = 0)
{
    int i;
    if (!this) return;
    for (i = 0; i < lv; i++)
	fputc (' ', stdout);
    printf ("%d tag=\"%s\" line=%d #%d sym=%d mark=%d \"%s\"\n", 
	    type, tag, lineno, nrKids, sym, mark, text);
    if (shallow)
	return;
    for (i = 0; i < nrKids; i++) {
	kids [i]->dump (lv+2);
    }
}

Node*
Node::enter ()
{
    int newSym;
    Tcl_HashEntry* hPtr;

    hPtr = Tcl_CreateHashEntry (&tree->symTb, text, &newSym);
    if (newSym) {
	Tcl_SetHashValue (hPtr, (char*) this);
	/* nsyms++; */
	return (Node*) 0;
    } else {
	Node* head = (Node*) Tcl_GetHashValue (hPtr);
	prec = head->prec;
	assoc = head->assoc;

	/* nterms, tokens, and ranges are alway kept front */
	switch (head->type) {
	case t_words:
	case t_token:
	case t_range:
	    next = head->next;
	    head->next = this;
	    return head;
	default:
	    next = head;
	    Tcl_SetHashValue (hPtr, (char*) this);
	    return this;
	}
    }
}

int
mapType (int type, char*& name)
{
    static int typeMap [] = {
	0, t_syntax, t_token, t_rule, t_prec,
	t_ID, t_LITERAL, t_PATTERN, t_opt, t_list, t_repeat, Node::t_single,
	t__left, t__right, t__nonassoc, Node::t_words, t__prec, t__separator, 
	Node::t_word, Node::t_word_list, Node::t_alternative_list, Node::t_alternatives,
	t_tag, t__ignore, t__syntax
    };
    char num [32];
    switch (type) {
    case t_ID:
	name = "ID";
	break;
    case t_LITERAL:
	name = "LITERAL";
	break;
    case t_PATTERN:
	name = "PATTERN";
	break;
    case t_syntax:
	name = "syntax";
	break;
    case t_token:
	name = "token";
	break;
    case t_rule:
	name = "rule";
	break;
    case t_prec:
	name = "prec";
	break;
    case t_opt:	
	name = "opt";
	break;
    case t_list:
	name = "list";	
	break;
    case t_repeat:
	name = "repeat";
	break;
    case Node::t_single:
	name = "single";
	break;
    case t__left:
	name = "left";
	break;
    case t__right:
	name = "right";
	break;
    case t__nonassoc:
	name = "nonassoc";
	break;
    case Node::t_words:
	name = "words";
	break;
    case t__prec:
	name = "-prec";
	break;
    case t__separator:
	name = "-separator";
	break;
    case Node::t_word:
	name = "word";
	break;
    case Node::t_word_list:
	name = "word_list";
	break;
    case Node::t_alternative_list:
	name = "alternative_list";
	break;
    case Node::t_alternatives:
	name = "alternatives";
	break;
    case t_tag:
	name = "tag";
	break;
    case t__ignore:
	name = "-ignore";
	break;
    case t__syntax:
	name = "-syntax";
	break;
    default:
	sprintf (num, "%d", type);
	name = num;
	break;
    }
    return typeMap [type];
}

int
Node::traverse (int _type, char* var, char* action, int one, int traversal)
{
    if (traversal)
	mark = traversal;

    for (int i = 0; i < nrKids; i++) {
	if (!kids [i] || (traversal && kids [i]->mark >= traversal)) {
	    continue;
	}
	if (traversal) 
	    kids [i]->mark = mark;
	char* typeName;
	int code;
	if (_type == 0 || mapType (_type, typeName) == kids [i]->type) {
	    Tcl_SetVar (tree->interp, "text", kids [i]->text, 0);
	    Tcl_SetVar (tree->interp, "tag", (kids [i]->tag) ? kids [i]->tag : "", 0);
	    mapType (kids [i]->type, typeName);
	    Tcl_SetVar (tree->interp, "type", typeName, 0);

	    Coc_Value obj;
	    obj.oval = kids [i];
	    if (TCL_OK != Coc_objectType (cocosType, obj)) {
		return TCL_ERROR;
	    } else {
		Tcl_SetVar (tree->interp, var, obj.string, 0);
	    }
	    code = Tcl_Eval (tree->interp, action);
	    switch (code) {
	    case TCL_BREAK:
		if (one) {
		    return TCL_BREAK;
		} else {
		    return TCL_OK;
		}
	    case TCL_OK:
		if (one) {
		    return TCL_BREAK;
		}
		break;
	    case TCL_CONTINUE:
		break;
	    default: 
		return code;
	    }
	}
	if (code == TCL_CONTINUE) {
	    continue;
	}
	code = kids [i]->traverse (_type, var, action, one, traversal);
	switch (code) {
	case TCL_OK:
	case TCL_CONTINUE:
	    break;
	case TCL_BREAK:
	    return TCL_BREAK;
	case TCL_RETURN:
	    if (tree->interp->result [0]) {
		Coc_Value obj;
		obj.string = tree->interp->result;
		if (TCL_OK != Coc_objectType (cocosType, obj)) {
		    return TCL_ERROR;
		} else {
		    delete kids [i];
		    kid (i, (Node*) obj.oval);
		}
	    }
	    break;
	default:
	    return code;
	}
    }
    return TCL_OK;
}

Node*
Node::parent (int _type)
{
    if (up) {
	char* typeName;
	if (_type == 0 || up->type == mapType (_type, typeName)) {
	    return up;
	}
	return up->parent (_type);
    }
    return (Node*) 0;
}

ParseTree::ParseTree (Tcl_Interp* _interp, char* _fileName, int debug, int dump)
{
    int i, j;
    extern char* Coc_strdup (char*);
    
    fileName = Coc_strdup (_fileName);
    interp = _interp;

    yydebug = debug;
    yylineno = 1;
    prec = 1;
    valid = 1;

    Tcl_InitHashTable (&symTb, TCL_STRING_KEYS);

    struct stat buf;
    
    if (-1 == (fileDesc = open (fileName, O_RDONLY))) {
	Tcl_AppendResult (interp, "can't open file \"", fileName, "\": ",
			  strerror (errno), (char *) NULL);
	goto error;
    }
    if (-1 == (fstat (fileDesc, &buf))) {
	Tcl_AppendResult (interp, "can't stat file \"", fileName, "\": ",
			  strerror (errno), (char *) NULL);
	goto error;
    }
    fileLength = buf.st_size;
    
    if (-1 == (int) (fileBuffer = mmap (0, fileLength, PROT_READ|PROT_WRITE, MAP_PRIVATE, fileDesc, 0))) {
	Tcl_AppendResult (interp, "can't map file \"", fileName, "\": ",
			  strerror (errno), (char *) NULL);
	goto error;
    }  
    filePtr = fileBuffer;
    mark++;

    if (setjmp (errorEnv)) {
	goto error;
    } else if (yyparse()) {
	goto error;
    } else {
	if (dump) {
	    tree->dump(1);
	    dumpSymbols();
	}
	return;
    }
    return;
    
 error:
    munmap (fileBuffer, fileLength);
    close (fileDesc);
    valid = 0;
    return;
}

ParseTree::~ParseTree ()
{
    delete [] fileName;
    munmap (fileBuffer, fileLength);
    close (fileDesc);
}

void
ParseTree::yyerror (char* s, ...)
{
    char buffer [1024];
    Tcl_AppendResult (interp, fileName, (char*) 0);
    sprintf (buffer, ":%d: ", yylineno);
    Tcl_AppendResult (interp, buffer, (char*) 0);
    va_list ap;
    va_start (ap, s);
    vsprintf (buffer, s, ap);
    va_end (ap);
    Tcl_AppendResult (interp, buffer, (char*) 0);

    longjmp (errorEnv, 0);
}


int 
ParseTree::yywrap() 
{ return 1; }

Node*
ParseTree::findSymbol (char* text)
{
    Tcl_HashEntry* hPtr;
    if (hPtr = Tcl_FindHashEntry (&symTb, text)) {
	return (Node*) Tcl_GetHashValue (hPtr);
    } else {
	return (Node*) 0;
    }
}

void
ParseTree::dumpSymbols ()
{
    Tcl_HashSearch search;
    Tcl_HashEntry* hPtr;

    if (hPtr = Tcl_FirstHashEntry (&symTb, &search)) {
	Node* n = (Node*) Tcl_GetHashValue (hPtr);
	int i;
	if (!n) {
	    printf ("<nil>\n");
	} else {
	    printf ("%d \"%s\" line=%d\n", n->type, n->text, n->lineno);
	    while (n = n->next) {
		printf ("  %d \"%s\" line=%d\n", n->type, n->text, n->lineno);
	    }
	}
	while (hPtr = Tcl_NextHashEntry (&search)) {
	    n = (Node*) Tcl_GetHashValue (hPtr);
	    if (!n) {
		printf ("<nil>\n");
	    } else {
		printf ("%d \"%s\" line=%d\n", n->type, n->text, n->lineno);
		while (n = n->next) {
		    printf ("  %d \"%s\" line=%d\n", n->type, n->text, n->lineno);
		}
	    }
	}
    }
}

