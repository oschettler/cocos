#
# $Id: Makefile,v 1.1 1995/10/25 20:09:27 olav Exp $
#
# $Log: Makefile,v $
# Revision 1.1  1995/10/25  20:09:27  olav
# 1
#
#

include ../config.mk

all: vhdl

vhdl: vhdl.o config.o
	$(CCC) -o vhdl vhdl.o config.o $(LDFLAGS) -lcopag

vhdl.o: vhdl.tab.o vhdl.lex.o
	ld -r -o vhdl.o vhdl.tab.o vhdl.lex.o

config.o: config.tab.o config.lex.o
	ld -r -o config.o config.tab.o config.lex.o 

vhdl.tab.o: vhdl.tab.cc
	$(CCC) $(CFLAGS) -DMAIN -o $@ -c $<

config.tab.o: config.tab.cc
	$(CCC) $(CFLAGS) -o $@ -c $<

vhdl.tab.o: vhdl.tab.h
vhdl.lex.o: vhdl.tab.h
config.tab.o: config.tab.h
config.lex.o: config.tab.h

install: all

clean:; rm -f vhdl.y vhdl.l vhdl.tab.* lex.*.c vhdl.lex.* vhdl.output *.o vhdl \
	config.y config.l config.tab.* config.lex.* config.output
