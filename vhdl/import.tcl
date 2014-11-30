
# design objects
DesignObject dp32_test.structure	-Name "/work/dp32_test/structure"
DesignObject clock_gen.behaviour 	-Name "/work/clock_gen/behaviour"
DesignObject memory.behaviour		-Name "/work/memory/behaviour"
DesignObject dp32.rtl			-Name "/dp32/rtl"
DesignObject reg_file_32_rrw.behaviour	-Name "/reg_file_32_rrw/behaviour"
DesignObject mux2.behaviour		-Name "/mux2/behaviour"

# components
Component dp32_test.structure.clock_gen	-Name "clock_gen"	-parent_DesignObject dp32_test.structure
Component dp32_test.structure.memory	-Name "memory"		-parent_DesignObject dp32_test.structure
Component dp32_test.structure.dp32	-Name "dp32"		-parent_DesignObject dp32_test.structure
Component dp32.rtl.reg_file_32_rrw	-Name "reg_file_32_rrw"	-parent_DesignObject dp32.rtl
Component dp32.rtl.mux2			-Name "mux2"		-parent_DesignObject dp32.rtl

set tree [[vhdl T -file dp32.vhdl] tree]
#$tree debug

proc doBlockConfig {n1 path interfaceName configName} {
  set C [Configuration #auto -Name $configName]

  $n1 one -tag implName -type IDENTIFIER -var x -command { set implName $text }
  puts "====== <$path> $interfaceName.$implName ====="

  $n1 all -type component_configuration -var n2 -command {
	$n2 one -tag componentName -type IDENTIFIER -var x -command { set componentName $text }
	puts "  ------ $componentName -----"
  	$n2 one -type entity_aspect -var n3 -command {
  		$n3 one -tag childInterfaceName -type IDENTIFIER -var x -command { set childInterfaceName $text }
  		set childImplName ""
  		catch { $n3 one -tag childImplName -type IDENTIFIER -var x -command { set childImplName .$text } }
  	}
	catch { unset CC }
	$n2 all -type block_configuration -var n3 -command {
		set nestedC [doBlockConfig $n3 $path.$implName $childInterfaceName $configName]
		if {$childImplName != ""} {
			# check whether the block configuration just created is for childImpl
			$n3 one -tag implName -type IDENTIFIER -var x -command { set nestedImplName $text }
			if {"$childImplName" == ".$nestedImplName"} {
				# create a component configuration that points to the block configuration
				set CC [ComponentConfiguration #auto \
					-Name "cc:$path.$implName.$componentName" \
					-Configuration $C \
					-Component $interfaceName.$implName.$componentName \
					-Child $nestedC]
			}
		}
	}

	if [catch { set CC }] {
		# no nested block configuration has configured the childImpl
		# create a component configuration that points to the childimpl directly
		set CC [ComponentConfiguration #auto \
			-Name "cc:$path.$implName.$componentName" \
			-Configuration $C \
			-Component $interfaceName.$implName.$componentName \
			-Child $childInterfaceName$childImplName]
	}
  	continue; # don't search for component_configuration's in nested block_configuration's
  }
  return $C
}

# configurations and component configurations
$tree all -type config -var n1 -command {
	puts "=== [$n1] ==="
	$n1 one -tag configName -type IDENTIFIER -var x -command { set configName $text }
	$n1 one -tag interfaceName -type IDENTIFIER -var x -command { set interfaceName $text }

	$n1 one	-type block_configuration -var n2 -command {
		set C [doBlockConfig $n2 $interfaceName $interfaceName $configName]
		puts "Configuration $C: [$C]"
	}
}

puts "\n====== D U M P ====="

# dump all configurations
foreach cc [itcl_info objects -class Configuration] {
	if [catch { $cc dump "" } X] {
		puts "failed to dump $cc: $X"
	}
}

# dump all component configurations
foreach cc [itcl_info objects -class ComponentConfiguration] {
	if [catch { $cc dump "" } X] {
		puts "failed to dump $cc: $X"
	}
}

puts "\n====== U N P A R S E ====="

[TreeNode #auto -text "library work;\nuse work.all;\n\nconfiguration" -kids {
		-code { $C Name }
		-text of
		-text /work/dp32_test
		-text is
		-node { $C unparse }
		-text end;
}] export stdout
