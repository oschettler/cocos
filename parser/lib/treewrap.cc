/*
 * parser/lib/treewrap.cc
 *
 *	Wrappers of parse trees methods used at run-time
 *	by the generated language processors
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: treewrap.cc,v 1.1 1995/10/25 20:09:25 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: treewrap.cc,v $
// Revision 1.1  1995/10/25  20:09:25  olav
// 1
//
 */

static char rcsid[] = "$Id: treewrap.cc,v 1.1 1995/10/25 20:09:25 olav Exp $";

#include <stdio.h>
#include <malloc.h>
#include <setjmp.h>
#include <assert.h>
#include <tcl.h>

#ifdef HAVE_ITCL
#include "itcl.h"
#endif

#include "bind.h"
#include "tree.h"

extern "C" int
Tcl_AppInit (Tcl_Interp* interp)
{
  if (Tcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }

#ifdef HAVE_ITCL
  if (Itcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }
#endif

  new Coc_Info (interp);

  Tcl_SetVar (interp, "cocos_library", COC_LIBRARY, TCL_GLOBAL_ONLY);
  Tcl_SetVar (interp, "cocos_version", COC_VERSION, TCL_GLOBAL_ONLY);

  return TCL_OK;
}

extern "C" int
tree_Init (Coc_Info* info)
{
  printf ("tree!\n");
  return TCL_OK;
}

extern "C" int
del_ParseTree (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ delete (ParseTree*) obj;
  return TCL_OK;
}

extern "C" int
ParseTree_get_tree (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.oval = ((ParseTree*) obj)->root;
  return TCL_OK;
}

extern "C" int
ParseTree_get_text (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((ParseTree*) obj)->getText();
  return TCL_OK;
}

extern "C" int
new_TreeNode (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  Node* n;

  Coc_Value* kids = (Coc_Value*) 0;
  kids = Coc_getParameter (info, "kids", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  int kidsCnt = 0;
  int descrCnt = 0;
  char** descrArray = (char**) 0;
  if (kids) {
	if (TCL_OK != Tcl_SplitList (info->interp, kids->sval, &descrCnt, &descrArray)) {
		result.oval = (void*) 0;
		return TCL_ERROR;
	}
	if (descrCnt % 2) {
		Tcl_AppendResult (info->interp, "\nillegal descriptor count", (char*) 0);
		return TCL_ERROR;
	}
	kidsCnt = descrCnt / 2;
  }

  Coc_Value* text = (Coc_Value*) 0;
  text = Coc_getParameter (info, "text", parmCnt, parm, (Coc_TypeProc) Coc_stringType);

  n = new Node (-2, (text) ? text->sval : "", kidsCnt);

  int i, j;
  for (i = 0, j = 0; i < descrCnt; i += 2, j++) {
	Node* kid = Node::special (info->interp, descrArray [i], descrArray [i+1]);
	if (!kid) {
		result.oval = (void*) 0;
		free ((char*) descrArray);
		return TCL_ERROR;
	}
	n->kid (j, kid);
  }

  if (descrArray) {
	free ((char *) descrArray);
  }
  result.oval = (void*) n;
  return TCL_OK;
}

extern "C" int
del_TreeNode (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ delete (Node*) obj;
  return TCL_OK;
}

extern "C" int
TreeNode_get_start (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.ival = ((Node*) obj)->start;
  return TCL_OK;
}

extern "C" int
TreeNode_get_end (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.ival = ((Node*) obj)->end;
  return TCL_OK;
}

extern "C" int
TreeNode_get_text (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((Node*) obj)->getText();
  return TCL_OK;
}

extern "C" int
TreeNode_get_tag (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((Node*) obj)->tag;
  return TCL_OK;
}

extern "C" int
TreeNode_get_type (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  Node* n = (Node*) obj;
  if (n->tree) {
	Coc_Value type;
	type.ival = n->type;
	assert (TCL_OK == Coc_enumType (n->tree->coc_NodeType, type));
	result.sval = type.string;
  } else {
	static char buffer [32];
	sprintf (buffer, "%d", n->type);
	result.sval = buffer;
  }
  return TCL_OK;
}

extern "C" int
TreeNode_debug (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ ((Node*) obj)->dump (1);
  return TCL_OK;
}

extern "C" int
TreeNode_export (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  Coc_Value* file;
  FILE* f;
  if (!(file = Coc_useParameter (0, parmCnt, parm))) {
	return TCL_ERROR;
  }
  if (TCL_OK != Tcl_GetOpenFile (info->interp, file->sval, 1, 1, (ClientData*) &f)) {
	return TCL_ERROR;
  }
	
  ((Node*) obj)->export (f, 0);
  return TCL_OK;
}

extern "C" int
TreeNode_all (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  Node* n = (Node*) obj;

  Coc_Value* typeName = Coc_getParameter (info, "type", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!typeName) {
	return TCL_ERROR;
  }
  if (!n->tree) {
	Tcl_AppendResult (info->interp, "\nno tree!", (char*) 0);
	return TCL_ERROR;
  }
  Coc_Value type;
  type.string = typeName->sval;
  if (TCL_OK != Coc_enumType (n->tree->coc_NodeType, type)) {
	return  TCL_ERROR;
  }

  Coc_Value* tagName = Coc_getParameter (info, "tag", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  char* tag;
  if (tagName) {
	tag = Coc_strdup (tagName->sval);
  } else {
	tag = Coc_strdup ("");
  }
  Coc_Value* varName = Coc_getParameter (info, "var", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!varName) {
	return TCL_ERROR;
  }
  char* var = Coc_strdup (varName->sval);
  
  Coc_Value* cmdValue = Coc_getParameter (info, "command", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!cmdValue) {
	return TCL_ERROR;
  }
  char* cmd = Coc_strdup (cmdValue->sval);

  Tcl_ResetResult (info->interp);
  
  if (TCL_ERROR == n->traverse (tag, type.ival, var, cmd)) {
	delete [] tag;
	delete [] var;
	delete [] cmd;
	return TCL_ERROR;
  } else {
	delete [] tag;
	delete [] var;
	delete [] cmd;
  }
  return TCL_OK;
}

extern "C" int
TreeNode_one (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  Node* n = (Node*) obj;

  Coc_Value* typeName = Coc_getParameter (info, "type", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!typeName) {
	return TCL_ERROR;
  }
  if (!n->tree) {
	Tcl_AppendResult (info->interp, "\nno tree!", (char*) 0);
	return TCL_ERROR;
  }
  Coc_Value type;
  type.string = typeName->sval;
  if (TCL_OK != Coc_enumType (n->tree->coc_NodeType, type)) {
	return  TCL_ERROR;
  }

  Coc_Value* tagName = Coc_getParameter (info, "tag", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  char* tag;
  if (tagName) {
	tag = Coc_strdup (tagName->sval);
  } else {
	tag = Coc_strdup ("");
  }

  Coc_Value* varName = Coc_getParameter (info, "var", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!varName) {
	return TCL_ERROR;
  }
  char* var = Coc_strdup (varName->sval);
  
  Coc_Value* cmdValue = Coc_getParameter (info, "command", parmCnt, parm, (Coc_TypeProc) Coc_stringType);
  if (!cmdValue) {
	return TCL_ERROR;
  }
  char* cmd = Coc_strdup (cmdValue->sval);
  
  Tcl_ResetResult (info->interp);
  
  int code = n->traverse (tag, type.ival, var, cmd, 1);
  switch (code) {
  case TCL_BREAK:
	delete [] tag;
	delete [] var;
	delete [] cmd;
	return TCL_OK;
  case TCL_OK:
	type.string = 0;
	Coc_enumType (n->tree->coc_NodeType, type);

	Tcl_AppendResult (info->interp, "\nno node with tag \"", tag, "\" of type \"",
			  type.string, "\" found", (char*) 0);
	/* fall through */
  default:
	/* ONE returns TCL_BREAK to indicate that a node was found, otherwise it fails */
	delete [] tag;
	delete [] var;
	delete [] cmd;
	return TCL_ERROR;
  }
  return TCL_OK;
}

extern "C" int
TreeNode_parent (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  Node* n = (Node*) obj;
  if (!n->tree) {
	Tcl_AppendResult (info->interp, "\nno tree!", (char*) 0);
	return TCL_ERROR;
  }
  Coc_Value type;
  type.string = value->sval;
  assert (TCL_OK == Coc_enumType (n->tree->coc_NodeType, type));

  if (!(result.oval = ((Node*) obj)->parent (type.ival))) {
	Tcl_AppendResult (info->interp, "\nno parent of type \"",
			  type.string, "\" found", (char*) 0);
	return TCL_ERROR;
  }
  return TCL_OK;
}
