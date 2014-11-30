/*
 * ibe/ibe.cs --
 *
 *	The (is,begin,end)-problem - copag style
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: ibe.cs,v 1.1 1995/10/25 20:09:23 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: ibe.cs,v $
 * Revision 1.1  1995/10/25  20:09:23  olav
 * 1
 *
 */

syntax ibe {

token junk -ignore  	-pattern <[_a-z0-9,:()=']+>
token comment -ignore  	-pattern <"--".*$>
token END		-pattern <"end"[^;]*";">

rule ibe {
    "is" ibe_list END
  | "component" ibe_list END
  | "process" ibe_list "begin" ibe_list END
  | "if" ibe_list END
  | "is" ibe_list "begin" ibe_list END
  | "is" ";"
  | "is" "record" ibe_list END
  | ";"
}

rule ibe_list { list { ibe } }

}
