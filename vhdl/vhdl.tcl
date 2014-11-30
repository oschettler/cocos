#
# vhdl.tcl -- data structures for parse trees
#

cocos_schema vhdl

cocos_class vhdl {
  inherit ParseTree
  constructor				new_vhdlParseTree

  proc TreeNode unparse args		vhdlTree_unparse
}



