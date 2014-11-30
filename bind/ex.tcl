#
# bind/ex.tcl --
#
#	Cocos schema for the example classes defined in ex.h
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: ex.tcl,v 1.1 1995/10/25 20:09:22 olav Exp $
#--------------------------------------------------------------------------
# $Log: ex.tcl,v $
# Revision 1.1  1995/10/25  20:09:22  olav
# 1
#
#

cocos_enum blubber {blubber waber glibber}

cocos_class foo ./libex.so.1.0 {
  constructor new_foo
  destructor del_foo

  method int incr {{int incr}} foo_incr
  
  public int a foo_get_a foo_set_a
  public blubber soft foo_get_a foo_set_a
  public string s foo_get_s foo_set_s
  public foo obj foo_get_obj foo_set_obj
}

cocos_class bar ./libex.so.1.0 {
  inherit foo

  constructor new_bar
  destructor del_bar

  public int b bar_get_b bar_set_b
  public string t bar_get_t bar_set_t
}

cocos_class ex ./libex.so.1.0 {
  proc int add {{foo a} {foo b}} ex_add
}
