/*
 * parser/treewrap.cc
 *
 *	Wrappers of parse trees methods needed to boot-strap copag
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: treewrap.cc,v 1.1 1995/10/25 20:09:26 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: treewrap.cc,v $
// Revision 1.1  1995/10/25  20:09:26  olav
// 1
//
 */

static char rcsid[] = "$Id: treewrap.cc,v 1.1 1995/10/25 20:09:26 olav Exp $";

#include <stdio.h>
#include <setjmp.h>
#include <tcl.h>

#ifdef HAVE_ITCL
#include <itcl.h>
#endif

#include "bind.h"
#include "tree.h"

extern "C" int
tree_Init (Coc_Info* info)
{
  printf ("tree!\n");
  return TCL_OK;
}

extern "C" int
new_ParseTree (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
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
  ParseTree* pt = new ParseTree (info->interp, fileName->sval, debug, dump);
  if (!pt->valid) {
    result.oval = (void*) 0;
    return TCL_ERROR;
  } else { 
	Coc_TypeProc cocosProc;
	Coc_getType (info, "TreeNode", cocosProc, Node::cocosType);

	result.oval = (void*) pt;
  }
  return TCL_OK;
}

extern "C" int
del_ParseTree (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ delete (ParseTree*) obj;
  return TCL_OK;
}

extern "C" int
ParseTree_get_tree (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.oval = ((ParseTree*) obj)->tree;
  return TCL_OK;
}

extern "C" int
TreeNode_get_text (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((Node*) obj)->text;
  return TCL_OK;
}

extern "C" int
TreeNode_get_tag (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ result.sval = ((Node*) obj)->tag;
  return TCL_OK;
}

extern "C" int
TreeNode_debug (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ ((Node*) obj)->dump(1,0);
  return TCL_OK;
}

extern "C" int
TreeNode_get_type (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ char* typeName;
  mapType (((Node*) obj)->type, typeName);
  result.sval = typeName;
  return TCL_OK;
}

extern "C" int
TreeNode_tokenize (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* string;
  Coc_Value* comment;

  if (!(string = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;

  result.sval = tokenize (string->sval);
  return TCL_OK;
}

extern "C" int
TreeNode_all (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{
  static int traversal = 0;
  Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  int type = value->ival;

  if (!(value = Coc_useParameter (1, parmCnt, parm)))
	return TCL_ERROR;
  char* var = Coc_strdup (value->sval);

  if (!(value = Coc_useParameter (2, parmCnt, parm)))
	return TCL_ERROR;
  char* action = value->sval;

  if (TCL_ERROR == ((Node*) obj)->traverse (type, var, action, 0, ++traversal)) {
    delete [] var;
    return TCL_ERROR;
  } else {
    delete [] var;
    result.sval = info->interp->result;
  }
  return TCL_OK;
}

extern "C" int
TreeNode_one (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  int type = value->ival;

  if (!(value = Coc_useParameter (1, parmCnt, parm)))
	return TCL_ERROR;
  char* var = Coc_strdup (value->sval);

  if (!(value = Coc_useParameter (2, parmCnt, parm)))
	return TCL_ERROR;
  char* action = (char*) value->sval;

  int code = ((Node*) obj)->traverse (type, var, action, 1, 0);

  Coc_TypeProc typeProc;
  Coc_Type* _typeEnum;
  Coc_EnumType* typeEnum;
  Coc_Value typeValue;

  switch (code) {
  case TCL_BREAK:
    delete [] var;
    return TCL_OK;

  case TCL_OK:
    Coc_getType (info, "NodeType", typeProc, _typeEnum);
    typeEnum = (Coc_EnumType*) _typeEnum;

    typeValue.ival = type;
    Coc_enumType (typeEnum, typeValue);

    Tcl_AppendResult (info->interp, "\nno node of type \"",
		      typeValue.string, "\" found", (char*) 0);
    /* fall through */
  default:
    /* ONE returns TCL_BREAK to indicate that a node was found, otherwise it fails */
    delete [] var;
    return TCL_ERROR;
  }
  return TCL_OK;
}

extern "C" int
TreeNode_parent (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[])
{ Coc_Value* value;

  if (!(value = Coc_useParameter (0, parmCnt, parm)))
	return TCL_ERROR;
  int type = value->ival;

  if (!(result.oval = ((Node*) obj)->parent (type))) {
    Coc_TypeProc typeProc;
    Coc_Type* _typeEnum;
    Coc_EnumType* typeEnum;
    Coc_getType (info, "NodeType", typeProc, _typeEnum);
    typeEnum = (Coc_EnumType*) _typeEnum;

    Coc_Value typeValue;
    typeValue.ival = type;
    Coc_enumType (typeEnum, typeValue);

    Tcl_AppendResult (info->interp, "\nno parent of type \"",
		      typeValue.string, "\" found", (char*) 0);
    return TCL_ERROR;
  }
  return TCL_OK;
}

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

  Tcl_DString genparse;
  Tcl_DStringInit (&genparse);
  Tcl_DStringAppend (&genparse, COC_LIBRARY, -1);
  Tcl_DStringAppend (&genparse, "/_genparse.tcl", 14);
  if (Tcl_EvalFile (interp, Tcl_DStringValue (&genparse))) {
	fprintf (stderr, "%s\n", interp->result);
	exit (-1);
  }
  Tcl_DStringFree (&genparse);
  return TCL_OK;
}

extern "C" int
main (int argc, char** argv)
{
  int free (char*);

  char buffer[1000];
  char* args;
  Tcl_Interp* interp = Tcl_CreateInterp();

  args = Tcl_Merge(argc-1, argv+1);
  Tcl_SetVar (interp, "argv", args, TCL_GLOBAL_ONLY);
  free (args);
  sprintf (buffer, "%d", argc-1);
  Tcl_SetVar (interp, "argc", buffer, TCL_GLOBAL_ONLY);
 
  return Tcl_AppInit (interp);
}

