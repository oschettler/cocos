#
# config.mk --
#
#	The central include file for all Makefiles. This file provides
#	macros and rules common to all Makefiles in the sub-directories
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: config.mk,v 1.1 1995/10/25 20:09:23 olav Exp $
#--------------------------------------------------------------------------
# $Log: config.mk,v $
# Revision 1.1  1995/10/25  20:09:23  olav
# 1
#
#

VERSION		= $(version)
#VERSION		= 2.0

ROOTDIR		= $(shell aegis -cd)
#ROOTDIR		= /opt/home/olav/baseline

ITCLDIR		= /usr/local/itcl
GNUDIR		= /usr/local

BINDIR		= $(ROOTDIR)/bin
LIBDIR		= $(ROOTDIR)/lib/copag
INCDIR		= $(ROOTDIR)/include

COCOS		= $(ROOTDIR)

CCC 		= g++
CC 		= gcc

# Use [incr Tcl]
# LDLIBS 		= -L$(LIBDIR) -L$(ITCLDIR)/lib -lcocos -litcl2.0 -ltcl7.4 -lm
# DEFINES		= -DHAVE_ITCL -DCOC_LIBRARY=\"$(LIBDIR)\" -DCOC_VERSION=\"$(VERSION)\" -DSVR4

LDLIBS 	= -L$(COCOS)/lib/copag -R$(COCOS)/lib/copag -lcocos -ltcl7.5 -ldl -lm -lsocket -lnsl
DEFINES	= -DCOC_LIBRARY=\"$(LIBDIR)\" -DCOC_VERSION=\"$(VERSION)\" -DSVR4

CFLAGS 		= -g -DYYDEBUG -DYYERROR_VERBOSE $(DEFINES) $(INCLUDES)
INCLUDES 	= -I$(GNUDIR)/include -I$(COCOS)/include
YFLAGS          = -dvl
LFLAGS  	= -sIL
LDFLAGS 	= -L$(GNUDIR)/lib $(LDLIBS)

.SUFFIXES: $(SUFFIXES) .cs .tab.cc .tab.o .tab.h .lex.cc .lex.o

.cc.o:
	$(CCC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

%.lex.o: %.lex.cc
	$(CCC) $(CFLAGS) -o $@ -c $<

%.tab.cc %.tab.h: %.y
	bison $(YFLAGS) -p $* $<
	$(LIBDIR)/plus $* $*.tab.c $*.tab.cc

%.lex.cc: %.l
	flex $(LFLAGS) -P$* $*.l
	$(LIBDIR)/plus $* lex.$*.c $*.lex.cc

%.y %.l: %.cs $(LIBDIR)/plus \
	$(LIBDIR)/hdr.code $(LIBDIR)/yacc.code $(LIBDIR)/lex.code \
	$(LIBDIR)/_genparse.tcl
	$(BINDIR)/copag $*.cs


