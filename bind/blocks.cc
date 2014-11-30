/*
 * bind/blocks.cc -- 
 *
 *	Implemention of the Tcl command coc_blocks which allows 
 *	to read files with parametrized code blocks.
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: blocks.cc,v 1.1 1995/10/25 20:09:22 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: blocks.cc,v $
// Revision 1.1  1995/10/25  20:09:22  olav
// 1
//
 */

static char rcsid[] = "$Id: blocks.cc,v 1.1 1995/10/25 20:09:22 olav Exp $";

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tcl.h>

extern char* sys_errlist[];
extern "C" {

int read (int fd, char *buf, int nbyte);
int close (int fd);
}

typedef struct ParmOcc {
  int index;
  char* nextRange;
  struct ParmOcc* next;
} *ParmOccPtr;

typedef struct Block {
  char* name;
  int parmCnt;
  char** parmArray;
  ParmOccPtr parmOcc;
  char* block;
  int length;
} *BlockPtr;

typedef struct Info {
  Tcl_HashTable* table;
  char* file;
} *InfoPtr;

void
BlocksDeleteCmd (
  ClientData clientData)
{
  Tcl_DeleteHashTable (((InfoPtr) clientData)->table);
  free (((InfoPtr) clientData)->file);
}

int
GetBlockCmd (
  ClientData clientData,		/* InfoPtr. */
  Tcl_Interp* interp,			/* Interpreter for evaluation. */
  int argc,				/* Number of arguments. */
  char* argv[])				/* String values of arguments. */
{
  Tcl_HashEntry* hPtr;
  BlockPtr blockPtr;
  ParmOccPtr parmOcc;
  int i;

  if (argc < 2) {
	Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],
			  " blockName parameters...\"", (char *) NULL);
	return TCL_ERROR;
  }
  if ((hPtr = Tcl_FindHashEntry (((InfoPtr) clientData)->table, argv[1])) == NULL) {
	Tcl_AppendResult (interp, "block \"", argv[1],
			  "\" not found", (char *) NULL);
	return TCL_ERROR;
  }
  blockPtr = (BlockPtr) Tcl_GetHashValue (hPtr);
  if (argc != blockPtr->parmCnt +2) {
	char buffer [10];
	sprintf (buffer, "%d", blockPtr->parmCnt);
	Tcl_AppendResult (interp, "wrong # parms: \"", argv[1],
			  "\" has ", buffer, " parameters", (char *) NULL);
	return TCL_ERROR;
  }
  Tcl_AppendResult (interp, blockPtr->block, (char*) NULL);
  parmOcc = blockPtr->parmOcc;
  argv += 2;
  while (parmOcc != NULL) {
	Tcl_AppendResult (interp,
			  argv [parmOcc->index],
			  parmOcc->nextRange,
			  (char*) NULL);
	parmOcc = parmOcc->next;
  }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * BlocksCmd --
 *
 *	This is the command procedure for the "blocks" command, which
 *	reads in a file of uninterpreted block definitions and provides
 *	a new Tcl command to access the blocks defined therein.
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	The named file is read into an allocated memory area and a new Tcl
 *	command is created.
 *
 *----------------------------------------------------------------------
 */
int
BlocksCmd (
  ClientData clientData,		/* Not used. */
  Tcl_Interp* interp,			/* Interpreter for evaluation. */
  int argc,				/* Number of arguments. */
  char* argv[])				/* String values of arguments. */
{
  struct stat fileStat;			/* buffer to hold result of stat */
  int fd;				/* file descriptor for blocks file */
  char* file;				/* buffer to hold file contents */
  char* fileEnd;			/* ptr behind last byte in file buffer */
  Tcl_HashTable* blocksTablePtr;	/* associate block names with block structs */
  char* filePtr;			/* ptr into the file's memory image */
  char* parms;				/* ptr to unsplit parms list */
  Tcl_RegExp parmExp;			/* compiled regexp for parms */
  ParmOccPtr parmOcc;			/* ptr to next parm occurrence */
  int match;				/* found a parm in block */
  char* matchPtr;			/* start-position for next parm search */
  int i;
  char* sep;
  BlockPtr blockPtr;			/* infos about one block */
  InfoPtr infoPtr;			/* infos about file; gets passed to GetBlockCmd */

  if (argc != 3) {
	Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],
			  " cmdName fileName\"", (char *) NULL);
	return TCL_ERROR;
  }
  if ((fd = open (argv[2], O_RDONLY)) == -1) {
	Tcl_AppendResult (interp, "can't open file \"", argv[2], "\": ",
			  sys_errlist [errno], (char *) NULL);
	return TCL_ERROR;
  }
  if (fstat (fd, &fileStat) == -1) {
	Tcl_AppendResult (interp, "can't stat file \"", argv[2], "\": ",
			  sys_errlist [errno], (char *) NULL);
	return TCL_ERROR;
  }
  if ((file = (char*) malloc (fileStat.st_size +1)) == (char*) 0) {
	Tcl_AppendResult (interp, "can't allocate buffer for file \"", argv[2], "\"",
			  (char *) NULL);
	return TCL_ERROR;
  }
  if (read (fd, file, fileStat.st_size) == -1) {
	Tcl_AppendResult (interp, "can't read file \"", argv[2], "\": ",
			  sys_errlist [errno], (char *) NULL);
	free (file);
	return TCL_ERROR;
  }
  close (fd);
  fileEnd = file + fileStat.st_size;
  *fileEnd = '\0';
  infoPtr = (struct Info*) malloc (sizeof (struct Info));
  infoPtr->table = (Tcl_HashTable*) malloc (sizeof (Tcl_HashTable));
  infoPtr->file = file;
  Tcl_InitHashTable (infoPtr->table, TCL_STRING_KEYS);

  for (filePtr = file; filePtr < fileEnd; /* no incr */) {
	int newBlock;
	Tcl_HashEntry* hPtr;

	while (*filePtr == '#') {
		while (*filePtr++ != '\n')
			;
	}
	if ((filePtr = strstr (filePtr, "block")) == (char*) 0)
		break;
	filePtr += 5;				/* length of `block' keyword */
	while (*filePtr == ' ' || *filePtr == '\t')
		filePtr++;
	if (*filePtr == '\0') {
		Tcl_AppendResult (interp, "incomplete \"block\" line", (char *) NULL);
		free (file);
		free ((char*) infoPtr);
		return TCL_ERROR;
	}
	blockPtr = (struct Block*) calloc (1, sizeof (struct Block));
	blockPtr->name = filePtr;
	filePtr = strpbrk (filePtr, " \t\n");
	if (filePtr == blockPtr->name) {
		Tcl_AppendResult (interp, "block needs a name", (char *) NULL);
		free ((char*) file);
		free ((char*) infoPtr);
		free ((char*) blockPtr);
		return TCL_ERROR;
	}
	if (*filePtr == '\n') {
		*filePtr = '\0';
	} else {
		*filePtr++ = '\0';
		while (*filePtr == ' ' || *filePtr == '\t')
			filePtr++;
		if (*filePtr == '\n') {
			*filePtr = '\0';
		}
	}
	parms = filePtr;
	if (*filePtr != '\0') {
		if ((filePtr = strchr (filePtr, '\n')) == (char*) 0) {
			Tcl_AppendResult (interp, "incomplete \"block\" line", (char *) NULL);
			free ((char*) file);
			free ((char*) infoPtr);
			free ((char*) blockPtr);
			return TCL_ERROR;
		}
		*filePtr = '\0';	/* mark end of parameters */
	}
	filePtr++;
	blockPtr->block = filePtr;	/* actual block begins with beginning of next line */
	if ((filePtr = strstr (filePtr, "end_block")) == (char*) 0) {
		Tcl_AppendResult (interp, "missing \"end_block\"", (char *) NULL);
		free ((char*) file);
		free ((char*) infoPtr);
		free ((char*) blockPtr);
		return TCL_ERROR;
	}
	*filePtr = '\0'; 
	blockPtr->length = filePtr - blockPtr->block;
	hPtr = Tcl_CreateHashEntry (infoPtr->table, blockPtr->name, &newBlock);
	if (newBlock) {
		Tcl_SetHashValue (hPtr, (char*) blockPtr);
	} else {
		fprintf (stderr, "warning: duplicate block \"%s\"\n", blockPtr->name);
	}
	filePtr += 9;	/* length of `block_end' keyword */
	
	if (*parms == '\0') {
		blockPtr->parmCnt = 0;
		blockPtr->parmArray = (char**) NULL;
		blockPtr->parmOcc = NULL;
		continue;
	}
	if (Tcl_SplitList (interp, parms, &blockPtr->parmCnt, &blockPtr->parmArray)
	    != TCL_OK) {
		Tcl_AppendResult (interp, "split-list failed", (char *) NULL);
		free ((char*) file);
		free ((char*) infoPtr);
		free ((char*) blockPtr);
		return TCL_ERROR;
	}
#if 0
	if (blockPtr->parmCnt > NSUBEXP) {
		Tcl_AppendResult (interp, "block \"", blockPtr->name,
				  "\" has too many parameters", (char*) NULL);
		free ((char*) file);
		free ((char*) infoPtr);
		free ((char*) blockPtr);
		return TCL_ERROR;
	}
#endif
	/* allocate space for the parameter regexp to be constructed:
	 *	  length of unsplit parms-list
	 *	+ # of parameters * 2 for the additional parentheses
	 */
	parms = (char*) malloc (filePtr - parms + 2 * blockPtr->parmCnt);
	*parms = '\0';
	sep = "";
	for (i = 0; i < blockPtr->parmCnt; i++) {
		strcat (parms, sep);
		strcat (parms, "(");
		strcat (parms, blockPtr->parmArray [i]);
		strcat (parms, ")");
		sep = "|";
	}
	parmExp = Tcl_RegExpCompile (interp, parms);
	if (parmExp == NULL) {
		free ((char*) file);
		free ((char*) infoPtr);
		free ((char*) blockPtr);
		free ((char*) parms);
		return (TCL_ERROR);
	}
	free ((char*) parms);
	blockPtr->parmOcc = NULL;
	matchPtr = blockPtr->block;
	while (matchPtr < blockPtr->block + blockPtr->length) {
		match = Tcl_RegExpExec (interp, parmExp, matchPtr, matchPtr);
		if (match == -1) {
			free ((char*) file);
			free ((char*) infoPtr);
			free ((char*) blockPtr);
			return (TCL_ERROR);
		}
		if (match == 0) {
			break;
		}
		for (i = 1; i <= blockPtr->parmCnt; i++) {
			char* start;
			char* end;

			Tcl_RegExpRange (parmExp, i, &start, &end);
			if (start != NULL) {
				int j;
				char save = *end;
				*end = '\0';
				
				if (blockPtr->parmOcc == NULL) {
					blockPtr->parmOcc = (struct ParmOcc*) malloc (sizeof (struct ParmOcc));
					parmOcc = blockPtr->parmOcc;
				} else {
					parmOcc->next = (struct ParmOcc*) malloc (sizeof (struct ParmOcc));
					parmOcc = parmOcc->next;
				}
				for (j = 0; j < blockPtr->parmCnt; j++) {
					if (!strcmp (start,
						     blockPtr->parmArray [j]))
						break;
				}
				parmOcc->index = j;
				*end = save;
				*start = '\0';
				parmOcc->nextRange = end;
				parmOcc->next = NULL;
				matchPtr = end;
				break;
			}
		}
	}
  }
  Tcl_CreateCommand (interp, argv[1],
		     (Tcl_CmdProc*) GetBlockCmd,
		     (ClientData) infoPtr, (Tcl_CmdDeleteProc *) BlocksDeleteCmd);
  return TCL_OK;
}

#ifdef BLOCKS_ALONE
/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */
    Tcl_CreateCommand (interp, "blocks",
		       (Tcl_CmdProc*) BlocksCmd,
		       (ClientData) 0, (Tcl_CmdDeleteProc *) 0);
    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */
    return TCL_OK;
}
#endif
