#
# simple.mk --
#
#	Makefile include file for simple language processors 
#	generated by copag. See the test case in sub-directory './t1'
#	for an example of its use.
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: simple.mk,v 1.1 1995/10/25 20:09:26 olav Exp $
#--------------------------------------------------------------------------
# $Log: simple.mk,v $
# Revision 1.1  1995/10/25  20:09:26  olav
# 1
#
#

include ../config.mk

.PRECIOUS: $(GRAMMAR).l $(GRAMMAR).y

all: $(GRAMMAR)

install: all

$(GRAMMAR): $(GRAMMAR).o $(LIBDIR)/libcopag.a
	$(CCC) -o $(GRAMMAR) $(GRAMMAR).o $(LDFLAGS) -lcopag

$(GRAMMAR).o: $(GRAMMAR).tab.o $(GRAMMAR).lex.o 
	ld -r -o $(GRAMMAR).o $(GRAMMAR).tab.o $(GRAMMAR).lex.o 

$(GRAMMAR).tab.o: $(GRAMMAR).tab.cc
	$(CCC) $(CFLAGS) -DMAIN -o $@ -c $<

$(GRAMMAR).tab.o: $(GRAMMAR).tab.h
$(GRAMMAR).lex.o: $(GRAMMAR).tab.h

clean:; rm -f $(GRAMMAR).y $(GRAMMAR).l $(GRAMMAR).tab.* lex.*.c $(GRAMMAR).lex.* $(GRAMMAR).output *.o $(GRAMMAR)
