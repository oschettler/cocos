#
# bind/bind.tcl --
#
#	Tcl body of cbind, the wrapper generator used to
#	generate shared libraries for cocos class definitions
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: bind.tcl,v 1.1 1995/10/25 20:09:22 olav Exp $
#--------------------------------------------------------------------------
# $Log: bind.tcl,v $
# Revision 1.1  1995/10/25  20:09:22  olav
# 1
#
#

set cflags "-I/vol/gnu/include -I/users/olav/cocos/src/bind"

proc cocos_enum {name elements} {
  global t e
  puts "  enum  $name"

  set e($name) 1
  
  puts $t "cocos_enum $name { $elements }"
}

proc _ {cls args} {}

proc _inherit {cls args} {
  global f t

  puts $t "  $args"
}

proc _constructor {cls args} {
  global f t

  puts $t "  constructor new_$cls"
  puts $f [blk constructor $cls]
}

proc _destructor {cls args} {
  global f t

  puts $t "  destructor del_$cls"
  puts $f [blk destructor $cls]  
}

proc type2comp {type} {
  global e
  
  switch $type {
  int		{ return ival }
  double	{ return dval }
  string	{ return sval }
  void		{ return "" }
  default	{
	if [catch { set e($type) }] {
		return oval
	} else {
		return ival
	}
  }}
}

proc _method {cls key type name parms args} {
  global f t procs

#  puts "    method $name"
  if {[llength $args] > 0} {
	set cproc $args
	puts $t "  method $type $name { $parms } $cproc"
	return
  } else {
	set cproc ${cls}_$name
	puts $t "  method $type $name { $parms } $cproc"
  }

  if ![catch { set procs($cproc) }] return
  set procs($cproc) 1

  puts $f [blk method_prefix $cproc]
  set idx -1
  foreach p $parms {
	set tp [lindex $p 0]
	set comp [type2comp $tp]
	switch $comp {
	  oval	{ set tp $tp* }
	  sval	{ set tp char* }
	  ""	{ set tp void }
	}
	puts $f [blk method_parm $tp [lindex $p 1] [incr idx] $comp]
  }
  set comp [type2comp $type]
  if {$comp == ""} {
	set asgn ""
  } else {
	set asgn "result.$comp = "
  }
  puts $f [blk method_call $asgn $cls $name]
  set sep "\t"
  foreach p $parms {
	puts $f $sep[lindex $p 1]
	set sep ", "
  }
  puts $f [blk method_suffix]
}

proc _proc {cls key type name parms args} {
  global f t procs

#  puts "    proc  $name"
  if {[llength $args] > 0} {
	set cproc $args
#	puts "      : explicit c-func"
	puts $t "  proc $type $name { $parms } $cproc"
#	return
  } else {
	set cproc ${cls}_$name
	puts $t "  proc $type $name { $parms } $cproc"
  }

  if ![catch { set procs($cproc) }] {
#	puts "      : procs($cproc) not defined"
	return
  }
  set procs($cproc) 1

  puts $f [blk proc_prefix $cproc]
  set idx -1
  foreach p $parms {
	set tp [lindex $p 0]
	set comp [type2comp $tp]
	switch $comp {
	  oval	{ set tp $tp* }
	  sval	{ set tp char* }
	  ""	{ set tp void }
	}
	puts $f [blk proc_parm $tp [lindex $p 1] [incr idx] $comp]
  }
  set comp [type2comp $type]
  if {$comp == ""} {
	set asgn ""
  } else {
	set asgn "result.$comp = "
  }
  puts $f [blk proc_call $asgn $cls $name]
  set sep "\t"
  foreach p $parms {
	puts $f $sep[lindex $p 1]
	set sep ", "
  }
  puts $f [blk proc_suffix]
}

proc _public {cls key type name args} {
  global f t procs

#  puts "    public $name"
  set comp [type2comp $type]
  switch $comp {
    oval	{ set tp $type* }
    default	{ set tp $type }
  }

  if {[llength $args] > 0} {
	set get [lindex $args 0]
	set set [lindex $args 1]
  } else {
	set set ${cls}_set_$name
	set get ${cls}_get_$name
	if [catch { set procs($set) }] {
		if {$comp == "sval"} {
			puts $f [blk set_string $set $cls $name]
		} else {
			puts $f [blk set $set $cls $name $tp $comp]
		}
		set procs($set) 1
	}
	if [catch { set procs($get) }] {
		puts $f [blk get $get $cls $name $comp]
		set procs($get) 1
	}
  }
  puts $t "  public $type $name $get $set"
}

proc cocos_class {name body} {
  global f t s
  puts "  class $name"

  puts $t "cocos_class $name lib$s.so.1.0 {"

  foreach x [split $body "\n"] {
	set feature [lindex $x 0]
	eval _$feature $name $x
  }
  puts $t "}"
}

proc cocos_schema {name} {
  global s
  puts "  schema $name"

  set s $name
}

proc cocos_bind {schema} {
  global f t s procs cflags env
  cocos_blocks blk $env(dir)/bind.code

  set procs(0) make-me-an-array
  unset procs(0)

  set f [open $schema.cc w]
  puts $f [blk banner $schema]

  set t [open $schema.gen.tcl w]

  set s $schema

  puts "parsing ..."
  source $schema.tcl

  close $f
  close $t

  set cmd "g++ -o $schema.o -fpic $cflags -c $schema.cc"
  puts "compiling ($cmd) ..."
  eval exec $cmd
  set cmd "ld -assert pure-text -o lib$schema.so.1.0 $schema.o"
  puts "linking ($cmd) ..."
  eval exec $cmd

  puts "done."
}
