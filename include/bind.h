/*
 * bind/bind.cc --
 *
 *	Data structures for the cocos core module
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: bind.h,v 1.1 1995/10/25 20:09:21 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: bind.h,v $
 * Revision 1.1  1995/10/25  20:09:21  olav
 * 1
 *
 */

#ifndef BIND_H
# define BIND_H

#ifdef __GNUC__
# define BOUND_PTR &
#else
# define BOUND_PTR
#endif

struct Coc_Type;
struct Coc_EnumType;
struct Coc_Object;
struct Coc_Value;
struct Coc_Variable;
struct Coc_Info;

struct Coc_Value {
  union {
	int ival;
	double dval;
	char* sval;
	void* oval;
  };
  char* string;
  Coc_Value(): string(0), oval(0) {}
};

typedef int (*Coc_TypeProc) (Coc_Type* type, Coc_Value& value);
typedef int (*Coc_Proc) (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[]);
typedef int (*Coc_StaticProc) (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[]);

typedef char* (*Coc_GenIdProc) (Coc_Type* type, void* obj);

struct Coc_Variable {
  char* name;
  int dyn;			// name dynamically allocated
  Coc_TypeProc type;
  Coc_Type* parent;		// type which defines this attribute
  Coc_Type* context;		// context in which to evaluate for type conversions
  Coc_Value value;
  int used;
  Coc_Proc set;
  Coc_Proc get;

  Coc_Variable(): dyn(0), used(0), set(0), get(0) {}
  ~Coc_Variable();
};

void Coc_dummyUseParms (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[]);
void Coc_dummyUseParms (Coc_Info* info, Coc_Value& result, int parmCnt, Coc_Variable parm[]);

struct Coc_Procedure {
  char* name;
  int isStatic;
  Coc_TypeProc type;
  Coc_Type* parent;		// type which defines this procedure
  Coc_Type* context;		// context in which to evaluate for type conversions
  void* obj;			// temporarily filled on invocation of `cmd'
  Coc_Proc proc;
  int parmc;
  Coc_Variable *parmv;

  Coc_Procedure() {}		// only to prevent warnings about a missing constructor
  ~Coc_Procedure();
  Coc_Variable* putParameter (int ix, char* name, Coc_TypeProc type, Coc_Type* context = 0);
  int cmd (Tcl_Interp* dummy, int argc, char** argv);
};

int Coc_voidType (Coc_Type* type, Coc_Value& value);
int Coc_intType (Coc_Type* type, Coc_Value& value);
int Coc_doubleType (Coc_Type* type, Coc_Value& value);
int Coc_stringType (Coc_Type* type, Coc_Value& value);
int Coc_enumType (Coc_EnumType* type, Coc_Value& value);
int Coc_objectType (Coc_Type* type, Coc_Value& value);

struct Coc_ProcType {
  char* name;
  Coc_TypeProc proc;
};

int Coc_getType (Coc_Info* info, char* typeName, Coc_TypeProc& procType, Coc_Type*& context);
char* Coc_genId (Coc_Type* type, void* obj);

Coc_Value* Coc_getParameter (Coc_Info* info, char* name, int parmCnt, Coc_Variable parm[], Coc_TypeProc type = 0, Coc_Type* context = 0);
Coc_Value* Coc_useParameter (int ix, int parmCnt, Coc_Variable parm[]);

struct Coc_Type {
  int isEnum;				// 1 for enum types
  char* name;				// type name
  Coc_Info* info;			// the `info' where this type is defined
  int maxId;				// last id automatically allocated to an object
  Tcl_HashTable ids;			// type extent (AO->WO)

  Coc_StaticProc constructor;		// object constructor
  Coc_Proc destructor;			// object destructor
  Tcl_HashTable attributes;		// locally defined attributes
  Tcl_HashTable procedures;		// locally defined methods  Coc_Type* parent;

  Tcl_HashTable ancestors;		// ancestor types from which to inherit
  Coc_Type** ancestorArray;		// array of ancestors (last one is 0)

  int wrapObject (char* name, void* object);
					// wrap an application object with a Coc_Object
  void delObject (void* obj);		// recursively delete an object in its type extent and those of its supertypes
  void addObject (char* name, Coc_Object* tcl);
					// recursively insert an object into its type extent and those of its supertypes
  static Coc_Type* create (Coc_Info* info, char* name, int ancestorCnt = 0, Coc_StaticProc constructor = 0, Coc_Proc destructor = 0);
					// create a new type
  virtual ~Coc_Type();
  Coc_Type() { isEnum = 0; }
  int digestParameters (Coc_Info* info, void* obj, Coc_Value& result, int parmCnt, Coc_Variable parm[]);
					// called from cmd or user def'd constructor: eat up all command line
					// parameters not previously used (i.e. 'used == 0')
  int cmd (Tcl_Interp* dummy, int argc, char** argv);
					// tcl callback for constructor command
  Coc_Variable* addAttribute (char* name, Coc_Proc get, Coc_Proc set, Coc_TypeProc type, Coc_Type* context = 0);
					// add an attribute definition to the type
  Coc_Variable* findAttribute (char* name);
					// find an attribute definition given its name
  Coc_Procedure* findProcedure (char* name, int deep = 1);
					// find a method definition given its name
  Coc_Procedure* addProcedure (char* name, Coc_Proc proc, int parmc = -1, Coc_TypeProc type = 0, Coc_Type* context = 0, int isStatic = 0);
					// add a method definition to the type
  Coc_Type* addAncestor (int index, Coc_Type* ancestor);
					// add an ancestor to the type
  Coc_Type* findAncestor (char* name);
					// find an ancestor
};

struct Coc_EnumType: public Coc_Type {
  int elementCnt;
  char** elements;
  static Coc_EnumType* create (Coc_Info* info, char* name, int elementCnt, char* elements[]);
  virtual ~Coc_EnumType();
  Coc_EnumType() { isEnum = 1; }
};

const long COC_COOKIE = 0x12345678L;

struct Coc_Info {
  Tcl_Interp* interp;
  Coc_GenIdProc genId;
  Tcl_HashTable types;
  Tcl_HashTable sharedLibraries;
  Coc_Type* objectType;

  Coc_Info (Tcl_Interp* interp_, Coc_GenIdProc genId = Coc_genId);
  ~Coc_Info();
  int cmd (Tcl_Interp* dummy, int argc, char** argv);
  int schemaDef (Tcl_Interp* dummy, int argc, char** argv);
  int classDef (Tcl_Interp* dummy, int argc, char** argv);
  int enumDef (Tcl_Interp* dummy, int argc, char** argv);
};

char* Coc_strdup (char* s);

struct Coc_Object {
  long cookie;
  char* name;
  Coc_Type* type;
  void* obj;

  Coc_Object (Coc_Type* type_, char* name_, void* obj_):
	cookie (COC_COOKIE), name (name_), type (type_), obj (obj_) {}
  ~Coc_Object();
  int cmd (Tcl_Interp* dummy, int argc, char** argv);
};

int Coc_collectAttributeValues (Coc_Type* type, Coc_Object* obj, Tcl_DString* result, int asList = 1);

#endif
