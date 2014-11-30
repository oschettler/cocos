/*
 * calc/calc.cs --
 *
 *	The calculator example - copag style
 *
 * Copyright (c) 1993-1997 Olav Schettler
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: calc.cs,v 1.1 1995/10/25 20:09:23 olav Exp $
 *--------------------------------------------------------------------------
 * $Log: calc.cs,v $
 * Revision 1.1  1995/10/25  20:09:23  olav
 * 1
 *
 */

syntax coc {
  token INT	-pattern <[0-9]+>
  token ID	-pattern <[_a-z][_a-z0-9]*>
  token UMINUS

  prec -left "+" "-"
  prec -left "*" "/"
  prec -right UMINUS
  prec -nonassoc "(" ")"

  rule calc {
	  calc expr
	| /**/
  }
  rule expr {
	  expr "+" expr
	| expr "-" expr
	| expr "*" expr
	| expr "/" expr
	| "(" expr ")"
	| "-" expr -prec UMINUS 
	| INT
	| ID
  }
}
