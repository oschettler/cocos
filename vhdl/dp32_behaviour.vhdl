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
