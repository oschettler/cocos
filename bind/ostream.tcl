#
# bind/ostream.tcl --
#
#	Cocos schema for the example classes for C++ streams
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: ostream.tcl,v 1.1 1995/10/25 20:09:22 olav Exp $
#--------------------------------------------------------------------------
# $Log: ostream.tcl,v $
# Revision 1.1  1995/10/25  20:09:22  olav
# 1
#
#

cocos_enum sel { out err }

cocos_class ostream {
  constructor  
}

cocos_class env {
  proc void print {{string me}}
}
