/* 
	a *minimal* VHDL grammar

	os-16.08.94:	token WAIT because of things like "wait for 10ns;"
			deleted "for" guts END; "loop" suffices
 */

syntax vhdl {

token comment -ignore  	-pattern <"--".*$>
token ID	 	-pattern <[_a-zA-Z0-9]+>
token USE		-pattern <"use"[^;]*";">
token WAIT		-pattern <"wait"[^;]*";">
token LIBRARY		-pattern <"library"[^;]*";">
token END		-pattern <"end"[^;]*";">

rule design_file {
	  repeat { toplevel }
}

rule toplevel {
	  LIBRARY
	| USE
	| entity
	| architecture
	| package
	| config
}

rule entity { 
	  "entity" Name:ID <\n> guts <\n> END 
}
rule architecture { 
	  "architecture" Name:ID ID EntityName:ID guts END
}
rule package {
	  "package" guts END
}
rule config -syntax {
	  "configuration"
}

rule guts {
	  list { item | ID | WAIT }
}

rule item {
	  "procedure" guts END
	| "function" guts END
	| "units" guts END
	| "record" guts END
	| "block" guts END
	| "generate" guts END
	| "process" guts END
	| "case" guts END
	| "if" guts END
	| "loop" guts END
	| component
/*	| "for" guts END */
	| USE
}

rule component {
	"component" Name:ID guts END
}

}
