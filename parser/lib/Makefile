#
# parser/lib/Makefile --
#
#	This Makefile describes the construction process for
#	the copag runtime-library
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: Makefile,v 1.1 1995/10/25 20:09:24 olav Exp $
#--------------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.1  1995/10/25  20:09:24  olav
# 1
#
#

include ../../config.mk

libcopag.a: tree.o treewrap.o
	ar rv libcopag.a tree.o treewrap.o

libcopag.a(%.o): %.cc

tree.o: tree.h
treewrap.o: tree.h

clean:; rm -f *.o
