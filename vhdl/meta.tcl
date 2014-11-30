#
# meta.tcl -- simple class definitions for Configuration meta data
# os-20.07.94
#

itcl_class Configuration {
  constructor {config} {
  }
  method config {config} {}

  method dump {x} {
	puts "${x}Configuration=$Name ($this)"
  }
  method unparse {} {
	return [config::unparse -rule block_configuration \
		-implName { -code [[[lindex $Configurations 0] Component] parent_DesignObject] Name } \
		-items { -list set Configurations } ]
  }
  method addCC {cc} {
	lappend Configurations $cc
  }
  method Name {} { return $Name }
  public Name
  method Configurations {} { return $Configurations }
  public Configurations {}
}

itcl_class ComponentConfiguration {
  constructor {config} {
	$Configuration addCC $this
  }
  method config {config} {}
  method dump {x} {
	puts "${x}ComponentConfiguration=$Name"
	$Configuration dump "$x  "
	$Component dump "$x  "
	$Child dump "$x  child-"
  }
  method unparse {} {
	switch [$Child info class] {
	DesignObject {
		return [config::unparse -rule component_configuration \
			-component_specification { -code concat "all:" [$Component Name] } \
			-binding { -code concat "use entity" [$Child Name] } ]
	}
	Configuration {
		return [config::unparse -rule component_configuration \
			-component_specification { -code concat "all:" [$Component Name] } \
			-binding { -code concat "use entity" [[[[lindex [$Child Configurations] 0] Component] parent_DesignObject] Name] } \
			-block { -node $Child unparse } ]
	}}
  }
  public Name
  public Configuration
  method Component {} { return $Component }
  public Component
  public Child
}

itcl_class Component {
  constructor {config} {
  }
  method config {config} {}
  method dump {x} {
	puts "${x}Component=$Name"
	$parent_DesignObject dump "$x  parent-"
  }
  method Name {} { return $Name }
  public Name
  method parent_DesignObject {} { return $parent_DesignObject } 
  public parent_DesignObject
}

itcl_class DesignObject {
  constructor {config} {
  }
  method config {config} {}
  method dump {x} {
	puts "${x}DesignObject=$Name"
  }
  method Name {} { return $Name }
  public Name
}
