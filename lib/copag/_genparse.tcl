#
# parser/_genparse.tcl --
#
#	Algorithms to generate YACC and LEX files
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: _genparse.tcl,v 1.1 1995/10/25 20:09:24 olav Exp $
#--------------------------------------------------------------------------
# $Log: _genparse.tcl,v $
# Revision 1.1  1995/10/25  20:09:24  olav
# 1
#
#
# os-10.12.93: generate yacc output
# os-11.03.94: use memory mapped files instead of explicit strings
# os-10.06.94: adapted to copag (cocos R3)
# os-02.08.94: recursive syntaxes
# os-07.08.94: unparse tables
# os-25.07.95: use $cocos_library
# os-28.03.97: clean up for public distribution
#

if {$argc != 1} {
  error "usage: \"copag grammar\""
}

puts "Copag, V$cocos_version"
puts "Copyright (c) 1993-1997 Olav Schettler"

set id 0
proc newsym {} {
  global id
  return "__rule__[incr id]"
}

#
# proc genrule
#
# in	string		nm		syntax name
# in	file		y		output YACC file
# in	boolean		syntax		generate code for nested syntax
# in	string		lhs		name of left-hand-side symbol
# in	Node		rule		tree node for syntax rule
# inout int		n		last symbol number assigned
# inout	array		symbol		mapping symbol -> symbol number
# inout	text		symNames	generated array of symbol names
# inout	text		items		generated array of rule items
# inout int		itemNo
# inout	text		rules		generated array of rules (name, type, first item, last item)

proc genrule {nm y syntax lhs rule} {
  global n symbol symNames items itemNo rules

#T  puts "genrule $lhs:"

  if [catch { set symbol($lhs) }] {
		set symbol($lhs) [incr n]
		append symNames "\"$lhs\", "
		if {$n % 10 == 0} { append symNames "\n " }
  }
  set result $lhs
  set sep ":"

  set altNo 1
  
  $rule all words alt {
	set localItems "\n  "
	set item_cnt 0
	
	set word_cnt 0
  	set add_kids ""
  	append result "\n\t$sep "
  	$alt all any word {
		switch -- $type {
  		ID	{ append localItems "{ Item::id_, \"$tag\", \"\", \"$text\" }, "
			  incr item_cnt
			  append result "$text "
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
  			}
  		LITERAL	{ append localItems "{ Item::literal_, \"$tag\", \"\", \"$text\" }, "
			  incr item_cnt
			  set id [TreeNode::tokenize $text]
  			  append result "$id "
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
  			}
  		PATTERN	{ append localItems "{ Item::literal_, \"$tag\", \"\", \"$text\" }, "
			  incr item_cnt
  			}
		list	{ set id1 [newsym]
			  set id2 [newsym]
			  set id3 [newsym]
			  puts $y [genrule $nm $y 0 $id1 $word]
			  set myTag $tag
			  if ![catch { $word one -separator x {} }] {
#				puts $y [blk sep_list $nm $id2 $id3 $id1 [
#					 TreeNode::tokenize $text]]
				puts $y [blk sep_list $nm $id2 $id3 $id1 [
					 TreeNode::tokenize $text]]
				append localItems "{ Item::list_, \"$myTag\", \"$text\", \"$id1\" }, "
			  } else {
				puts $y [blk list $nm $id2 $id1]
				append localItems "{ Item::list_, \"$myTag\", \"\", \"$id1\" }, "
			  }
			  incr item_cnt
			  append result "$id2 "
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
			  break
			}
		repeat	{ set id1 [newsym]
			  set id2 [newsym]
			  puts $y [genrule $nm $y 0 $id1 $word]
			  set myTag $tag
			  if ![catch { $word one -separator x {} }] {
				puts $y [blk sep_repeat $nm $id2 $id1 [
					 TreeNode::tokenize $text]]
				append localItems "{ Item::repeat_, \"$myTag\", \"$text\", \"$id1\" }, "
			  } else {
				puts $y [blk repeat $nm $id2 $id1]
				append localItems "{ Item::repeat_, \"$myTag\", \"\", \"$id1\" }, "
			  }
			  incr item_cnt
			  append result "$id2 "
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
			  break
			}
		opt	{ set id1 [newsym]
			  set id2 [newsym]
			  puts $y [genrule $nm $y 0 $id1 $word]
			  puts $y [blk opt $nm $id2 $id1]
			  append localItems "{ Item::opt_, \"$tag\", \"\", \"$id1\" }, "
			  incr item_cnt
			  append result "$id2 "
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
			  break
			}
		alternatives	{
			  set id [newsym]
  			  append result "$id "
			  puts $y [genrule $nm $y 0 $id $word]
			  append add_kids "  \$\$->kid ($word_cnt, \$[incr word_cnt], \"$tag\");\n\t  "
			  break
  			}
  		word_list {}
  		-prec	{ append result "%prec $text"
  			  continue
  			}
  		default	{ puts "$type \"$text\" not emmitted"
  			  continue
  			}
		}

	}

	if $syntax {
		puts "  rule \"$lhs\" has syntax specification"
		append result "\n\t  \{\n"
		if {$word_cnt > 0} {
			append result [blk rst_offset $nm]
		}
		for {set i 1} {$i <= $word_cnt} {incr i} {
			append result [blk del_node $i]
		}
		append result [blk invoke $nm $lhs $symbol($lhs)]
	} else {
#		1	{ if {$tag != ""} {
#				append result [blk add_tag $nm $symbol($lhs) \"$tag\"]
#			  }
#			}
		switch $word_cnt {
		0	{ append result [blk crt_nil_node $nm ] }
		1	{ if ![string match {__rule__*} $lhs] {
				append result [blk crt_node $nm $symbol($lhs) $word_cnt $add_kids]
			  }
			}
		default	{ append result [blk crt_node $nm $symbol($lhs) $word_cnt $add_kids] }
		}
	}
#	puts "\"$result\" has $word_cnt words"
  	set sep "|"

	set lastItem [expr $itemNo+$item_cnt]
	append items $localItems

#	puts "${lhs}_$altNo <$itemNo:$lastItem>:\t$localItems"

	if {$altNo > 1} {
		append rules "\n  { \"${lhs}_$altNo\", $symbol($lhs), $itemNo, $lastItem },"
	} else {
		append rules "\n  { \"$lhs\", $symbol($lhs), $itemNo, $lastItem },"
		append rules "\n  { \"${lhs}_$altNo\", $symbol($lhs), $itemNo, $lastItem },"
	}
	set itemNo $lastItem
	incr altNo
  }
  append result "\n\t;"

  return $result
}

proc genyacc {tree nm} {
  global cocos_library n symbol symNames items itemNo rules

  cocos_blocks blk $cocos_library/yacc.code

  set y [open ${nm}.y w]
  puts "generating: ${nm}.y"

  if [regsub {.*/([^/]*)$} $nm {\1} stripped] {
	set nm $stripped
  }
  puts $y [blk banner $nm]

  #
  # collect all literals and tokens
  #
  $tree all LITERAL lit {
	set token([TreeNode::tokenize $text]) 1
  }
  $tree all -separator lit {
	set token([TreeNode::tokenize $text]) 1
  }
  $tree all token tok {
	set lexem $text
	if [catch { $tok one -ignore x {} }] {
		set token($lexem) 1
	}
  }
  set token(0) "make-an-array"
  unset token(0)
  set symbol(0) "make-an-array"
  unset symbol(0)
  set symNames " "
  set n -1

  #
  # emmit tokens associated with a precedence
  #
  $tree all prec prec {
	if ![catch { $prec one left x {} }] {
		set assoc left
	} else {
		if ![catch { $prec one right x {} }] {
			set assoc right
		} else {
			set assoc nonassoc
		}
	}
	set list ""
	$prec all LITERAL tok {
		set id [TreeNode::tokenize $text]

		append list "$id "
		set symbol($text) [incr n]
		append symNames "\"$text\", "
		if {$n % 10 == 0} { append symNames "\n " }

		unset token($id)
	}
	$prec all ID tok {
		append list "$text "

		set symbol($text) [incr n]
		append symNames "\"$text\", "
		if {$n % 10 == 0} { append symNames "\n " }

		unset token($text)
	}
	puts $y [blk token $assoc $list]
  }

  #
  # emmit remaining ones
  #
  foreach id [array names token] {
	puts $y [blk token token $id]

	set symbol($id) [incr n]
	append symNames "\"$id\", "
	if {$n % 10 == 0} { append symNames "\n " }
  }

  $tree one rule x {}
  puts $y "\n%start $text\n\n%%\n"

  set items "static struct Item ItemTable\[\] = \{"
  set itemNo 0
  set rules "  \{ Item::nil_, 0, 0, 0 \}\n\};\n\nstatic struct Rule RuleTable\[\] = \{"

  #
  # emmit rules
  #
  $tree all rule rule {
	if [catch {$rule one -syntax x {}}] {
		set syntax 0
	} else {
		set syntax 1
	}
	puts $y [genrule $nm $y $syntax [$rule text] $rule]
  }

  puts $y [blk end_rules]

  puts $y $symNames
  
  puts $y [blk trailer_1]

  #
  # emmit rule describers
  #
  puts $y $items
  puts $y $rules
  
  puts $y [blk trailer_2 $nm [incr n 2]]

  puts $y [blk trailer_3 $nm]
  close $y
}

proc genlex {tree nm} {
  global cocos_library

  cocos_blocks blk $cocos_library/lex.code

  set l [open ${nm}.l w]
  puts "generating: ${nm}.l"

  if [regsub {.*/([^/]*)$} $nm {\1} stripped] {
	set nm $stripped
  }
  puts $l [blk banner $nm]

  set literal(0) "make-an-array"
  unset literal(0)
  $tree all LITERAL lit {
	set literal([TreeNode::tokenize $text]) \"$text\"
  }
  $tree all -separator sep {
	set literal([TreeNode::tokenize $text]) \"$text\"
  }
  foreach lit [array names literal] {
	puts $l [blk token $nm $lit $literal($lit)]
  }
  $tree all token t {
	set token $text
	if ![catch { $t one PATTERN pat {} }] {
		# has a pattern
		set lexem $text
		if ![catch { $t one -ignore x {} }] {
			puts $l [blk ignore $nm $lexem]
		} else {
			puts $l [blk token $nm $token $lexem]
		}
	} else {
#T		puts "$token has no pattern"
	} 
  }

  puts $l [blk trailer $nm]
  close $l
}

if {$argc != 1} {
    error "usage: copag <grammar-file>"
}

source $cocos_library/_tree.tcl
ParseTree T -file $argv
scan $argv {%[^.]%*s} name

genlex [T tree] $name 
genyacc [T tree] $name
