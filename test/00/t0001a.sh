#!/bin/sh
#
# Test the `calc' application
#
# $Id: t0001a.sh,v 1.1 1995/10/25 20:09:26 olav Exp $
#
# $Log: t0001a.sh,v $
# Revision 1.1  1995/10/25  20:09:26  olav
# 1
#
#

here=`pwd`
if test $? -ne 0; then exit 1; fi

tmp=/tmp/tmp.$$
if test $? -ne 0; then exit 1; fi

mkdir $tmp
if test $? -ne 0; then exit 1; fi

cd $tmp
if test $? -ne 0; then exit 1; fi

fail()
{
	echo "== < stdout >=="
	cat calc.out
	echo "== < stderr >=="
	cat calc.err
	echo FAILED 1>&2
	cd $here
	rm -rf $tmp
	exit 1
}
pass()
{
	echo PASSED 1>&2
	cd $here
	rm -rf $tmp
	exit 0
}
trap "fail" 1 2 3 15

#
# echo commands so we can tell what failed
#
dir=`pwd`
dir=`basename $dir`
if [ $dir = 'baseline' ]; then
  fail
fi

(cd $here/calc; gmake)
if test $? -ne 0; then fail; fi

cat >in <<__EOF__
tralla + 14 * hudli;
6*8; 2*(3-hudli)
__EOF__

$here/calc/calc >calc.out 2>calc.err <<__EOF__
source $here/lib/copag/tree.tcl

cocos_schema calc

cocos_class calc {
  inherit ParseTree
  constructor			new_calcParseTree

  proc TreeNode unparse args	calcTree_unparse
}

calc T -file in

puts "================================="
[T tree] all -type expr -var x -command {
  puts [\$x]
}
puts "================================="

[T tree] debug

__EOF__

if test $? -ne 0; then fail; fi

# diff calc.err - <<__EOF__
# __EOF__
# if test $? -ne 0; then fail; fi

diff calc.out - <<__EOF__
=================================
{-end 19} {-tag {}} {-text {tralla + 14 * hudli}} {-start 0} {-type expr}
{-end 6} {-tag {}} {-text tralla} {-start 0} {-type expr}
{-end 19} {-tag {}} {-text {14 * hudli}} {-start 9} {-type expr}
{-end 11} {-tag {}} {-text 14} {-start 9} {-type expr}
{-end 19} {-tag {}} {-text hudli} {-start 14} {-type expr}
{-end 24} {-tag {}} {-text 6*8} {-start 21} {-type expr}
{-end 22} {-tag {}} {-text 6} {-start 21} {-type expr}
{-end 24} {-tag {}} {-text 8} {-start 23} {-type expr}
{-end 37} {-tag {}} {-text 2*(3-hudli)} {-start 26} {-type expr}
{-end 27} {-tag {}} {-text 2} {-start 26} {-type expr}
{-end 37} {-tag {}} {-text (3-hudli)} {-start 28} {-type expr}
{-end 36} {-tag {}} {-text 3-hudli} {-start 29} {-type expr}
{-end 30} {-tag {}} {-text 3} {-start 29} {-type expr}
{-end 36} {-tag {}} {-text hudli} {-start 31} {-type expr}
=================================
tree!
calc!
 1 <calc> line=3 sym=0 <0-37> #2 "tralla + 14 * hudli;
6*8; 2*(3-hudli)"
 2  <calc> line=2 sym=0 <0-24> #2 "tralla + 14 * hudli;
6*8"
 3   <calc> line=2 sym=0 <0-19> #2 "tralla + 14 * hudli"
 4    <expr> line=2 sym=0 <0-19> #3 "tralla + 14 * hudli"
 5     <expr> line=1 sym=0 <0-6> #1 "tralla"
 6      <ID> line=1 sym=0 <0-6> #0 "tralla"
 5     <+> line=1 sym=0 <7-8> #0 "+"
 5     <expr> line=1 sym=0 <9-19> #3 "14 * hudli"
 6      <expr> line=1 sym=0 <9-11> #1 "14"
 7       <INT> line=1 sym=0 <9-11> #0 "14"
 6      <*> line=1 sym=0 <12-13> #0 "*"
 6      <expr> line=1 sym=0 <14-19> #1 "hudli"
 7       <ID> line=1 sym=0 <14-19> #0 "hudli"
 3   <expr> line=2 sym=0 <21-24> #3 "6*8"
 4    <expr> line=2 sym=0 <21-22> #1 "6"
 5     <INT> line=2 sym=0 <21-22> #0 "6"
 4    <*> line=2 sym=0 <22-23> #0 "*"
 4    <expr> line=2 sym=0 <23-24> #1 "8"
 5     <INT> line=2 sym=0 <23-24> #0 "8"
 2  <expr> line=2 sym=0 <26-37> #3 "2*(3-hudli)"
 3   <expr> line=2 sym=0 <26-27> #1 "2"
 4    <INT> line=2 sym=0 <26-27> #0 "2"
 3   <*> line=2 sym=0 <27-28> #0 "*"
 3   <expr> line=2 sym=0 <28-37> #3 "(3-hudli)"
 4    <(> line=2 sym=0 <28-29> #0 "("
 4    <expr> line=2 sym=0 <29-36> #3 "3-hudli"
 5     <expr> line=2 sym=0 <29-30> #1 "3"
 6      <INT> line=2 sym=0 <29-30> #0 "3"
 5     <-> line=2 sym=0 <30-31> #0 "-"
 5     <expr> line=2 sym=0 <31-36> #1 "hudli"
 6      <ID> line=2 sym=0 <31-36> #0 "hudli"
 4    <)> line=2 sym=0 <36-37> #0 ")"
__EOF__

if test $? -ne 0; then fail; fi

#
# the things tested in this test, worked
#
pass
