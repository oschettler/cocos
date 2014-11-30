Contents
========

This package contains the Cocos suite of development libraries and
applications. Contained are 

- libcocos.so, a facility to bind C/C++ objects to
  Tcl commands,
- copag, the Cocos Parser Generator, a frontend to bison/flex to parse text
  files based on incomplete syntax specifications
- a copag-based parser for the VHDL language
- Tcl code to demonstrate how to interface VHDL objects with a disign
  repository
- several other examples and test files

Availability
============

Cocos is available to registered parties free of charge. 

Platform
========

Cocos has only been tested on SPARC machines under the Solaris 2.5 operating
system.

Author
======

Please send comments or bug reports to Olav Schettler, olav@werbemittel.com

I have written a book in which this software and its application in the
encapsulation of EDA design tools is described:

	Schettler, Olav
	Encapsulating tools into an EDA framework
	Oldenburg, 1996
	ISBN 3-486-23985-6

A somewhat outdated online version is available at
http://www.metronet.de/~olav/research

License
=======

Please see file license.terms in this directory.

Required packages
=================

Mandatory
---------

- The GNU C++ compiler, GNU make utility, Bison parser generator, and Flex
  scanner generator (all at ftp://prep.ai.mit.edu/pub/gnu).
- The Tcl library, version 7.5 (ftp://ftp.sunlabs.com/pub/tcl).

Optional
--------

- The [incr Tcl] object-oriented extension to Tcl (http://www.tcltk.com/itcl/).
- The Aegis project change supervisor (ftp://ftp.agso.gov.au/pub/Aegis/).
