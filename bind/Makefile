#
# bind/Makefile --
#
#	This Makefile describes the construction process for
#	the cocos library and some test programs
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

include ../config.mk

ATT-CC = /vol/ObjectStore/bin/OSCC
GNU-CC = "g++"

all: gnu/libcocos.so gnu/x # att/libcocos.a att/x

install: all
	-mkdir -p $(LIBDIR)
	-mkdir -p $(BINDIR)
	-mkdir -p $(INCDIR)
	cp bind.h $(INCDIR)
	cp gnu/libcocos.so $(LIBDIR); ranlib $(LIBDIR)/libcocos.so
#	cp att/libcocos.so $(LIBDIR)/libcocos-att.so

gnu/libcocos.so: bind.h bind.cc compile.cc blocks.cc
	(mkdir -p gnu; cd gnu; $(MAKE) -f ../Makefile.lib CC=$(GNU-CC) CFLAGS="$(CFLAGS)")

att/libcocos.so: bind.h bind.cc compile.cc blocks.cc
	(mkdir -p att; cd att; $(MAKE) -f ../Makefile.lib CC=$(ATT-CC) CFLAGS="$(CFLAGS)")

gnu/x:
	(cd gnu; $(MAKE) -f ../Makefile.lib CC=$(GNU-CC) CFLAGS="$(CFLAGS)")

att/x:
	(cd att; $(MAKE) -f ../Makefile.lib CC=$(ATT-CC))

y: y.o gnu/libcocos.so
	$(CC) -g -o y y.o -L./gnu -lcocos -ltcl74 -lm

s: ostream.o gnu/libcocos.so
	$(CC) -g -o s ostream.o -L. -lcocos -ltcl74 -lm

libex.so.1.0: ex.tcl ex.h
	./cbind ex

clean:; rm -f gnu/* att/*
