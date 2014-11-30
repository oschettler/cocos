#!/bin/sh
#
# Test the `ibe' application
#
# $Id: t0002a.sh,v 1.1 1995/10/25 20:09:26 olav Exp $
#
# $Log: t0002a.sh,v $
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

(cd $here/ibe; gmake)
if test $? -ne 0; then fail; fi

cat >in <<__EOF__
architecture behaviour of dp32 is

  subtype reg_addr is 

begin



end tralla
__EOF__

$here/ibe/ibe >ibe.out <<__EOF__
source $here/lib/copag/tree.tcl

cocos_schema ibe

cocos_class ibe {
  inherit ParseTree
  constructor			new_ibeParseTree

  proc TreeNode unparse args	ibeTree_unparse
}

ibe T -file $here/vhdl/dp32_behaviour.vhdl
[T tree] debug
__EOF__

if test $? -ne 0; then fail; fi

diff ibe.out - <<__EOF__
tree!
ibe!
 1 <ibe> line=40 sym=0 <31-719> #5 "is

	subtype reg_addr is natural range 0 to 255;
	type reg_array is array (reg_addr) of bit_32;

	type instruction is 
		record
			op_code: processor_op;
			address_mode: mode;
			operand1, operand2: integer range 0 to 15;
		end record;

begin -- behaviour of dp32

	process
		-- lots of variables

		alias cm_i: bit is current_instr(19);

		procedure memory_read (addr: in bit_32; 
				       fetch_cycle: in boolean;
				       result: out bit_32) is
		begin
			-- body of procedure memory_read
			if reset = '1' then
				return;
			end if;
			-- ...
		end memory_read;

	begin
		if reset = '1' then
			-- ...
			wait until reset = '0';
		end if;
		-- ...
	end process;


end behaviour;"
 2  <t_is> line=2 sym=0 <31-33> #0 "is"
 2  <ibe_list> line=13 sym=0 <53-267> #1 "is natural range 0 to 255;
	type reg_array is array (reg_addr) of bit_32;

	type instruction is 
		record
			op_code: processor_op;
			address_mode: mode;
			operand1, operand2: integer range 0 to 15;
		end record;"
 3   <list> line=3 sym=0 <53-267> #3
 4    <ibe> line=3 sym=0 <53-79> #2 "is natural range 0 to 255;"
 5     <t_is> line=3 sym=0 <53-55> #0 "is"
 5     <t__3b> line=3 sym=0 <78-79> #0 ";"
 4    <ibe> line=4 sym=0 <96-126> #2 "is array (reg_addr) of bit_32;"
 5     <t_is> line=4 sym=0 <96-98> #0 "is"
 5     <t__3b> line=4 sym=0 <125-126> #0 ";"
 4    <ibe> line=11 sym=0 <146-267> #4 "is 
		record
			op_code: processor_op;
			address_mode: mode;
			operand1, operand2: integer range 0 to 15;
		end record;"
 5     <t_is> line=6 sym=0 <146-148> #0 "is"
 5     <t_record> line=8 sym=0 <152-158> #0 "record"
 5     <ibe_list> line=11 sym=0 <183-253> #1 ";
			address_mode: mode;
			operand1, operand2: integer range 0 to 15;"
 6      <list> line=8 sym=0 <183-253> #3
 7       <ibe> line=8 sym=0 <183-184> #1 ";"
 8        <t__3b> line=8 sym=0 <183-184> #0 ";"
 7       <ibe> line=9 sym=0 <206-207> #1 ";"
 8        <t__3b> line=9 sym=0 <206-207> #0 ";"
 7       <ibe> line=10 sym=0 <252-253> #1 ";"
 8        <t__3b> line=10 sym=0 <252-253> #0 ";"
 5     <END> line=11 sym=0 <256-267> #0 "end record;"
 2  <t_begin> line=13 sym=0 <269-274> #0 "begin"
 2  <ibe_list> line=40 sym=0 <298-702> #1 "process
		-- lots of variables

		alias cm_i: bit is current_instr(19);

		procedure memory_read (addr: in bit_32; 
				       fetch_cycle: in boolean;
				       result: out bit_32) is
		begin
			-- body of procedure memory_read
			if reset = '1' then
				return;
			end if;
			-- ...
		end memory_read;

	begin
		if reset = '1' then
			-- ...
			wait until reset = '0';
		end if;
		-- ...
	end process;"
 3   <list> line=13 sym=0 <298-702> #1
 4    <ibe> line=37 sym=0 <298-702> #5 "process
		-- lots of variables

		alias cm_i: bit is current_instr(19);

		procedure memory_read (addr: in bit_32; 
				       fetch_cycle: in boolean;
				       result: out bit_32) is
		begin
			-- body of procedure memory_read
			if reset = '1' then
				return;
			end if;
			-- ...
		end memory_read;

	begin
		if reset = '1' then
			-- ...
			wait until reset = '0';
		end if;
		-- ...
	end process;"
 5     <t_process> line=16 sym=0 <298-305> #0 "process"
 5     <ibe_list> line=32 sym=0 <348-602> #1 "is current_instr(19);

		procedure memory_read (addr: in bit_32; 
				       fetch_cycle: in boolean;
				       result: out bit_32) is
		begin
			-- body of procedure memory_read
			if reset = '1' then
				return;
			end if;
			-- ...
		end memory_read;"
 6      <list> line=16 sym=0 <348-602> #4
 7       <ibe> line=18 sym=0 <348-369> #2 "is current_instr(19);"
 8        <t_is> line=18 sym=0 <348-350> #0 "is"
 8        <t__3b> line=18 sym=0 <368-369> #0 ";"
 7       <ibe> line=20 sym=0 <411-412> #1 ";"
 8        <t__3b> line=20 sym=0 <411-412> #0 ";"
 7       <ibe> line=21 sym=0 <448-449> #1 ";"
 8        <t__3b> line=21 sym=0 <448-449> #0 ";"
 7       <ibe> line=29 sym=0 <481-602> #5 "is
		begin
			-- body of procedure memory_read
			if reset = '1' then
				return;
			end if;
			-- ...
		end memory_read;"
 8        <t_is> line=23 sym=0 <481-483> #0 "is"
 8        <ibe_list> line=24 sym=0 <0-0> #1 ""
 9         <list> line=24 sym=0 <0-0> #0
 8        <t_begin> line=24 sym=0 <486-491> #0 "begin"
 8        <ibe_list> line=29 sym=0 <531-573> #1 "if reset = '1' then
				return;
			end if;"
 9         <list> line=24 sym=0 <531-573> #1
10          <ibe> line=27 sym=0 <531-573> #3 "if reset = '1' then
				return;
			end if;"
11           <t_if> line=25 sym=0 <531-533> #0 "if"
11           <ibe_list> line=27 sym=0 <561-562> #1 ";"
12            <list> line=25 sym=0 <561-562> #1
13             <ibe> line=26 sym=0 <561-562> #1 ";"
14              <t__3b> line=26 sym=0 <561-562> #0 ";"
11           <END> line=27 sym=0 <566-573> #0 "end if;"
 8        <END> line=29 sym=0 <586-602> #0 "end memory_read;"
 5     <t_begin> line=32 sym=0 <605-610> #0 "begin"
 5     <ibe_list> line=37 sym=0 <613-679> #1 "if reset = '1' then
			-- ...
			wait until reset = '0';
		end if;"
 6      <list> line=32 sym=0 <613-679> #1
 7       <ibe> line=35 sym=0 <613-679> #3 "if reset = '1' then
			-- ...
			wait until reset = '0';
		end if;"
 8        <t_if> line=32 sym=0 <613-615> #0 "if"
 8        <ibe_list> line=35 sym=0 <668-669> #1 ";"
 9         <list> line=32 sym=0 <668-669> #1
10          <ibe> line=34 sym=0 <668-669> #1 ";"
11           <t__3b> line=34 sym=0 <668-669> #0 ";"
 8        <END> line=35 sym=0 <672-679> #0 "end if;"
 5     <END> line=37 sym=0 <690-702> #0 "end process;"
 2  <END> line=40 sym=0 <705-719> #0 "end behaviour;"
__EOF__

if test $? -ne 0; then fail; fi

#
# the things tested in this test, worked
#
pass
