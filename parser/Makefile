#
# parser/Makefile --
#
#	This Makefile describes the construction process for
#	the copag parser/scanner generator
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

include ../config.mk

TARGETS		= copag lib/libcopag.a
OBJECTS		= parse.tab.o lex.yy.o tree.o treewrap.o tokenize.o

all: $(TARGETS)

copag: $(OBJECTS)
	$(CCC) -o copag $(OBJECTS) $(LDFLAGS)

lib/libcopag.a:
	cd lib; $(MAKE)

parse.tab.cc: parse.y
	bison $(YFLAGS) parse.y
	sed -f plus.sed parse.tab.c > parse.tab.cc && rm parse.tab.c

lex.yy.cc: lex.l
	flex $(LFLAGS) lex.l
	sed -f plus.sed lex.yy.c > lex.yy.cc && rm lex.yy.c

install: $(TARGETS)
	-mkdir -p $(BINDIR)
	-mkdir -p $(LIBDIR)
	-mkdir -p $(INCDIR)
	cp copag $(BINDIR)
	cp lib/plus $(LIBDIR); chmod a+x $(LIBDIR)/plus
	cp _genparse.tcl _tree.tcl $(LIBDIR)
	cp lib/bind.code lib/def.code lib/hdr.code lib/lex.code $(LIBDIR)
	cp lib/range.code lib/yacc.code $(LIBDIR)
	cp lib/libcopag.a lib/tree.tcl $(LIBDIR); ranlib $(LIBDIR)/libcopag.a
	cp lib/tree.h $(INCDIR)

clean:; rm -f *.o *.tab.* lex.yy.* *.output

parser.tab.cc: tree.h parse.tab.h
lex.yy.cc: tree.h parse.tab.h
parse.cc: tree.h
driver.cc: tree.h
