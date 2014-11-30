-----------------------------------------------------------------
--
-- dp32.vhdl -- nearly complete example with configurations
-- os-03.08.94
--
-----------------------------------------------------------------

use work.dp32_types.all;

entity dp32 is
	generic (Tpd: Time := unit_delay);
	port (	d_bus: inout bus_bit_32 bus;
		a_bus: out bit_32;
		read, write: out bit;
		fetch: out bit;
		ready: in bit;
		phi1, phi2: in bit;
		rest: in bit);
end dp32;

architecture behaviour of dp32 is

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


end behaviour;

--
-- clock_gen 
--

entity clock_gen is
	generic (...);
	port (...);
end clock_gen;

architecture behaviour of clock_gen is ... end behaviour;

--
-- memory
--

entity memory is
	generic (...);
	port (...);
end memory;

architecture behaviour of memory is ... end behaviour;

--
-- test bench
--

entity dp32_test is end dp32_test;

architecture structure of dp32_test is

	component clock_gen
		port (...);
	end component;

	component dp32
		port (...);
	end component;

	component memory
		port (...);
	end component;

	signal d_bus: bus_bit_32 bus;
	signal a_bus: bit_32;
	signal read, write: bit;
	signal fetch: bit;
	signal ready: bit;
	signal phi1, phi2: bit;
	signal reset: bit;

begin
	cg: clock_gen
		port map (...);
	proc: dp32
		port map (...);
	mem: memory
		port map (...);
end structure;

configuration dp32_rtl_test of dp32_test is

	for structure
		for cg: clock_gen
			use entity work.clock_gen (behaviour)
				generic map (...);
		end for;
		for mem: memory
			use entity work.memory (behaviour);
		end for;
		for proc: dp32
			use entity work.dp32 (rtl);
			for rtl
				for all: reg_file_32_rrw
					use entity work.reg_file_32_rrw (behaviour);
				end for;
				for all: mux2
					use entity work.mux2 (behaviour);
				end for;
			end for;
		end for;
	end for;
end dp32_rtl_test;

-- end of file dp32.vhdl
