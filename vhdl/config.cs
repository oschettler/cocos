/*
	config.cs -- specification of VHDL configuration declarations
	os-20.07.94
*/
syntax config {

token IDENTIFIER -pattern <[_a-zA-Z][_a-zA-Z0-9]*>
token COMMENT -ignore -pattern <"--".*$>

rule configuration_declaration {
	  "configuration" configName:IDENTIFIER "of" interfaceName:IDENTIFIER "is"
		list { use_clause }
		block_configuration
	  "end" opt { IDENTIFIER } ";"
}
rule block_configuration {
	  "for" implName:IDENTIFIER 
		  list { use_clause }
		  items:list { configuration_item }
	  "end" "for" ";"
}
rule configuration_item {
	  block_configuration
	| component_configuration
}
rule component_configuration {
	  "for" component_specification
		binding:opt { "use" binding_indication ";" }
		block:opt { block_configuration }
	  "end" "for" ";"
}
rule component_specification {
	  instantiation_list ":" componentName:IDENTIFIER
}
rule instantiation_list {
	  repeat -separator "," { instanceName:IDENTIFIER }
	| "others"
	| "all"
}
rule binding_indication {
	  entity_aspect
		opt { "generic" "map" association_list }
		opt { "port" "map" association_list }
}
rule entity_aspect {
	  "entity" "work." childInterfaceName:IDENTIFIER opt { "(" childImplName:IDENTIFIER ")" }
	| "configuration" "work." childConfigName:IDENTIFIER
	| "open"
}
rule use_clause {
	"use" "..."
}
rule association_list {
	"(" "..." ")"
}
} # end of syntax
