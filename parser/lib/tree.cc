/*
 * parser/lib/tree.cc
 *
 *	Parse trees used at run-time by generated language processors
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
#include <assert.h>
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

extern "C" {
  void* malloc (int);
  void free (void*);
  char* strerror (int);
  int close (int);
  int open(char*,int,...);
          /* mode_t mode */
#ifndef SVR4
  caddr_t mmap (caddr_t, size_t, int, int, int, off_t);
  int munmap (caddr_t, int);
#endif
  void exit (int);
}

Node::Node (ParseTree* _tree, int _type, Node* _first, Node* _last, int _nrKids = 0, Category _category = use):
  tree (_tree), type (_type), category (_category), nrKids (_nrKids),
  tag (0), text (0), start (0), end (0), kids (0), nxtSym (0), next (0), up (0), sym (0)
{
  if (_first) {
	start = _first->start;
  } else if (_last) {
	start = _last->start;
  }
  if (_last) {
	end = _last->end;
  } else if (_first) {
	end = _first->end;
  }
  lineno = tree->file->lineno;
  if (nrKids) {
	kids = (Node**) new Node* [nrKids];
	for (int i=0; i < nrKids; i++) {
		kids [i] = 0;
	}
  }
}

Node::Node (ParseTree* _tree, int _type, int _start, int _end, int _nrKids = 0, Category _category = use):
  tree (_tree), type (_type), category (_category), nrKids (_nrKids),
  tag (0), text (0), start (_start), end (_end), kids (0), nxtSym (0), next (0), up (0), sym (0)
{
  lineno = tree->file->lineno;
  if (nrKids) {
	kids = (Node**) new Node* [nrKids];
	for (int i=0; i < nrKids; i++) {
		kids [i] = 0;
	}
  }
}

Node::Node (int _type, char* _text, int _nrKids = 0):
  tree (0), type (_type), category (use), nrKids (_nrKids),
  tag (0), text (Coc_strdup (_text)), start (0), end (0), kids (0), nxtSym (0), next (0), up (0), sym (0),
  lineno (0)
{
  if (nrKids) {
	kids = (Node**) new Node* [nrKids];
	for (int i=0; i < nrKids; i++) {
		kids [i] = 0;
	}
  }
}

Node*
Node::special (Tcl_Interp* interp, char* tag, char* text, char* sep)
{
  if (!strcmp ("-text", tag)) {
	return new Node (-2, text, 0);
  } else if (!strcmp ("-code", tag)) {
	if (TCL_OK != Tcl_Eval (interp, text)) {
		return (Node*) 0;
	}
	return new Node (-2, interp->result, 0);
  } else if (!strcmp ("-node", tag)) {
	if (TCL_OK != Tcl_Eval (interp, text)) {
		return (Node*) 0;
	}
	Coc_Value objValue;
	objValue.string = Coc_strdup (interp->result);
	if (TCL_OK != Coc_objectType (coc_Node, objValue)) {
		delete [] objValue.string;
		return (Node*) 0;
	}
	delete [] objValue.string;
	return (Node*) objValue.oval;
  } else if (!strcmp ("-list", tag)) {
	if (TCL_OK != Tcl_Eval (interp, text)) {
		return (Node*) 0;
	}
	int elCnt;
	char** elArray;
	if (TCL_OK != Tcl_SplitList (interp, interp->result, &elCnt, &elArray)) {
		return (Node*) 0;
	}
	Node* list = new Node (-1, "", 2);
	for (int i = 0; i < elCnt; i++) {
		if (TCL_OK != Tcl_VarEval (interp, elArray [i], " unparse", (char*) 0)) {
			free ((char *) elArray);
			delete list;
			return (Node*) 0;
		}
		Coc_Value objValue;
		objValue.string = Coc_strdup (interp->result);
		if (TCL_OK != Coc_objectType (coc_Node, objValue)) {
			free ((char *) elArray);
			delete list;
			delete [] objValue.string;
			return (Node*) 0;
		}
		delete [] objValue.string;
		if (i && sep[0]) {
			list->append (new Node (-2, sep, 0));
		}
		list->append ((Node*) objValue.oval);
	}
	free ((char *) elArray);
	return list;
  } else {
	Tcl_AppendResult (interp, "illegal tag \"", tag, "\"", (char*) 0);
	return (Node*) 0;
  }
}

char*
Node::getText (int reset = 0)
{
  static char* lastEndPtr = 0;
  static char lastEndChar = 0;

  if (reset) {
	if (lastEndPtr) *lastEndPtr = lastEndChar;
	lastEndPtr = NULL;
	return NULL;
  }

  if (!this) {
	tree->error ("node <nil>");
  }
  
  if (text) return text;

  if (lastEndPtr) *lastEndPtr = lastEndChar;
  lastEndPtr = tree->file->buffer + end;
  lastEndChar = *lastEndPtr;
  *lastEndPtr = '\0';

  return tree->file->buffer + start;
}

Node::~Node()
{
  int i;

  if (!this)
    return;
  if (tag)
	delete [] tag;
  for (i = 0; i < nrKids; i++)
    delete kids[i];
  if (kids)
    delete [] kids;
}

void
Node::kid (int ix, Node* kid, char* tag = 0)
{
  if (ix < 0 || ix >= nrKids)
    tree->error ("kids index %d on \"%s\" out of range 0..%d", 
	     ix, text, nrKids);
  kids [ix] = kid;
  if (!kid)
    return;
  kid->up = this;
  kid->tag = Coc_strdup (tag);
}

/*
	lists have the following structure:
	- kids[0] points to the first list element
	- kids[1] points to the last list element
	- elements are linked with the `next' field

        N--------+
        |        |
	v        v
	kids[0]  kids[1]
	|        |
	v        v
	N->N->N->N
*/

void
Node::append (Node* n)
{
  if (!kids [0]) {
	kids [0] = n;
	start = n->start;
  } else {
	kids [1]->next = n;
  }
  kids [1] = n;
  n->next = 0;
  n->up = this;

  end = n->end;
}

void
Node::export (FILE* file, int lv)
{
  static int need_indent = 0;
  int i;
  if (!this) return;
  if (need_indent) {
	fputc ('\n', file);
	for (i = 0; i < lv; i++)
		fputc (' ', stdout);
	need_indent = 0;
  }
  if (text [0] != '\0') {
	fputs (text, file);
	fputc (' ', file);
	need_indent = 0;
  }
  if (type == -1) {
	need_indent = 1;
	for (Node* elem = kids [0]; elem; elem = elem->next) {
		elem->export (file, lv+1);
	}
	need_indent = 1;
  } else {
	if (nrKids) {
		need_indent = 1;
	}
	for (i = 0; i < nrKids; i++) {
		kids [i]->export (file, lv+1);
	}
	if (nrKids) {
		need_indent = 1;
	}
  }
}

void
Node::dump (int lv)
{
  int i;
  if (!this) return;
  printf ("%2d", lv);
  for (i = 0; i < lv; i++)
    fputc (' ', stdout);
  switch (type) {
  case -1:
	printf ("<list> ");
	break;
  case -2:
	printf ("<> ");
	break;
  default: {
	if (!tree) {
		printf ("<%d> ", type);
	} else {
		Coc_Value typeValue;
		typeValue.ival = type;
		if (TCL_OK != Coc_enumType (tree->coc_NodeType, typeValue)) {
			printf ("invalid type %d\n", type);
		} else {
			printf ("<%s> ", typeValue.string);
		}
	}
  }}
  if (tag && tag [0] != '\0') {
	printf ("tag=\"%s\" ", tag);
  }
  printf ("line=%d sym=%d <%d-%d> ", lineno, sym, start, end);
  if (type == -1) {
	int n = 0;
	Node* elem;
	for (elem = kids [0]; elem; elem = elem->next) {
		n++;
	}
	printf ("#%d\n", n);
	for (elem = kids [0]; elem; elem = elem->next) {
		elem->dump (lv+1);
	}
  } else {
	printf ("#%d \"%s\"\n", nrKids, getText());
	for (i = 0; i < nrKids; i++) {
		kids [i]->dump (lv+1);
	}
  }
}

Node*
Node::enter ()
{
  int newSym;
  Tcl_HashEntry* hPtr;

  assert (tree);
  hPtr = Tcl_CreateHashEntry (&tree->symTb, getText(), &newSym);
  if (newSym) {
    Tcl_SetHashValue (hPtr, (char*) this);
//    nsyms++;
    return (Node*) 0;
  } else {
    Node* head = (Node*) Tcl_GetHashValue (hPtr);

    /* declarations are alway kept front */
    if (head->category == dcl) {
      nxtSym = head->nxtSym;
      head->nxtSym = this;
      return head;
    } else {
      nxtSym = head;
      Tcl_SetHashValue (hPtr, (char*) this);
      return this;
    }
  }
}

int
Node::traverse (
  char* _tag,
   int _type,
   char* var,
   char* action,
   int one = 0,
   int outermost = 1)
{
  int code;

  assert (tree);
  if ((_type == 0 || _type == type) && (_tag[0] == '\0' || (tag && !strcmp (_tag, tag)))) {
    Tcl_SetVar (tree->interp, "text", getText(), 0);
    Tcl_SetVar (tree->interp, "tag", (tag) ? tag : "", 0);

    Coc_Value typeValue;
    typeValue.ival = type;
    if (TCL_OK != Coc_enumType (tree->coc_NodeType, typeValue)) {
	return TCL_ERROR;
    } else {
	Tcl_SetVar (tree->interp, "type", typeValue.string, 0);
    }
    Coc_Value obj;
    obj.oval = this;
    if (TCL_OK != Coc_objectType (coc_Node, obj)) {
      return TCL_ERROR;
    } else {
      Tcl_SetVar (tree->interp, var, obj.string, 0);
    }
    code = Tcl_Eval (tree->interp, action);
    switch (code) {
    case TCL_OK:
	if (one) return TCL_BREAK;
	break;
    default:
	return code;
    }
  }

  /*
   * type values are different in different parse trees so don't
   * traverse down there. This is only relevant if we are not
   * in the root and are not called directly from Tcl but rather
   * recursively.
   */
  if (up && outermost && up->tree != tree) {
	return TCL_OK;
  }
  
  if (type == -1) {
	for (Node* elem = kids [0]; elem; elem = elem->next) {
		int code = elem->traverse (_tag, _type, var, action, one, 0);
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
		    if (TCL_OK != Coc_objectType (coc_Node, obj)) {
		      return TCL_ERROR;
		    } else {
			Node* newElem = (Node*) obj.oval;
			newElem->up = this;
			newElem->next = elem->next;
			if (kids [1] == elem) {
				kids [1] = newElem;
			}
			delete elem;
		    }
		  }
		  break;
		default:
		  return code;
		}
	}
  } else {
	for (int i = 0; i < nrKids; i++) {
		if (!kids [i]) {
			/* stems from an optional rule */
			continue;
		}
		int code = kids [i]->traverse (_tag, _type, var, action, one, 0);
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
		    if (TCL_OK != Coc_objectType (coc_Node, obj)) {
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
  }
  return TCL_OK;
}



Node*
Node::parent (int _type)
{
  if (up) {
	if (_type == 0 || up->type == _type) {
		return up;
	}
	return up->parent (_type);
  }
  return (Node*) 0;
}

File::File (Tcl_Interp* _interp, char* _name)
{
  extern char* Coc_strdup (char*);

  name = Coc_strdup (_name);
  interp = _interp;
  lineno = 1;
  valid = 1;

  struct stat buf;
  
  if (-1 == (desc = open (name, O_RDONLY))) {
	Tcl_AppendResult (interp, "can't open file \"", name, "\": ",
		      strerror (errno), (char *) NULL);
	goto error;
  }
  if (-1 == (fstat (desc, &buf))) {
	Tcl_AppendResult (interp, "can't stat file \"", name, "\": ",
		      strerror (errno), (char *) NULL);
	goto error;
  }
  length = buf.st_size +1;
  
  if (-1 == (int) (buffer = mmap (0, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, desc, 0))) {
	Tcl_AppendResult (interp, "can't map file \"", name, "\": ",
			  strerror (errno), (char *) NULL);
	goto error;
  }  
  buffer [length] = '\0';
  ptr = buffer;
  offset = 0;
  return;

error:
  munmap (buffer, length);
  close (desc);
  valid = 0;
  return;
}

File::~File()
{
  delete [] name;
  munmap (buffer, length);
  close (desc);
}

char*
ParseTree::getText ()
{
  return file->buffer;  
}

ParseTree::ParseTree (Tcl_Interp* _interp, char* _name, ParseTree*& g_parseTree, Coc_EnumType* _nodeType, fct _parse)
{
  int i, j;
  
  parent = 0;
  interp = _interp;
  file = new File (interp, _name);
  if (!file->valid) {
	goto error;
  }
//  prec = 1;
  valid = 1;

  Tcl_InitHashTable (&symTb, TCL_STRING_KEYS);

  mark++;

  coc_NodeType = _nodeType;
  parse = _parse;
  g_parseTree = this;

  if (setjmp (errorEnv)) {
	goto error;
  } else if (parse()) {
	goto error;
  } else {
	return;
  }
  return;
  
error:
  delete file;
  valid = 0;
  return;
}

ParseTree::ParseTree (ParseTree& tree, ParseTree*& g_parseTree, Coc_EnumType* _nodeType, fct _parse)
{
  parent = &tree;
  root = 0;
//  start = 0
//  prec = tree.prec;
  valid = tree.valid;
  mark = tree.mark;
  file = tree.file;
  interp = tree.interp;
  
  Tcl_InitHashTable (&symTb, TCL_STRING_KEYS);

  coc_NodeType = _nodeType;
  parse = _parse;
  g_parseTree = this;

  if (setjmp (errorEnv)) {
	goto error;
  } else if (parse()) {
	goto error;
  } else {
	return;
  }
  return;
  
error:
  valid = 0;
  return;
}

ParseTree::~ParseTree ()
{
  if (!parent) {
	delete [] file;
  }
  Tcl_DeleteHashTable (&symTb);
}

void
ParseTree::error (char* s, ...)
{
  char buffer [1024];
  Tcl_AppendResult (interp, file->name, (char*) 0);
  sprintf (buffer, ":%d: ", file->lineno);
  Tcl_AppendResult (interp, buffer, (char*) 0);
  va_list ap;
  va_start (ap, s);
  vsprintf (buffer, s, ap);
  va_end (ap);
  Tcl_AppendResult (interp, buffer, "\n", (char*) 0);

  longjmp (errorEnv, 0);
}

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
      while (n = n->nxtSym) {
	printf ("  %d \"%s\" line=%d\n", n->type, n->text, n->lineno);
      }
    }
    while (hPtr = Tcl_NextHashEntry (&search)) {
      n = (Node*) Tcl_GetHashValue (hPtr);
      if (!n) {
	printf ("<nil>\n");
      } else {
	printf ("%d \"%s\" line=%d\n", n->type, n->text, n->lineno);
	while (n = n->nxtSym) {
	  printf ("  %d \"%s\" line=%d\n", n->type, n->text, n->lineno);
	}
      }
    }
  }
}
