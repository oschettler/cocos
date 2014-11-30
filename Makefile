#
# Makefile --
#
#	Top-level Makefiles. This Makfiles is invoked by the
#	'aegis -build' command and recursively invokes all the
#	Makefiles in the various sub-directories. 
#
#	Note that gnu-make is required! 
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: Makefile,v 1.1 1995/10/25 20:09:21 olav Exp $
#--------------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.1  1995/10/25  20:09:21  olav
# 1
#
#

SOURCES		= bind parser 
TESTS 		= calc ibe vhdl

install:;
		mkdir -p lib/copag
		mkdir -p include
		for d in $(SOURCES) $(TESTS); do (cd $$d; $(MAKE) install); done

all:; 		for d in $(SOURCES) $(TESTS); do (cd $$d; $(MAKE) all); done

clean:;		for d in $(SOURCES) $(TESTS); do (cd $$d; $(MAKE) clean); done

tests:;		for d in $(TESTS); do (cd $$d; $(MAKE) all); done		
