#
# config.tcl -- data structures for parse trees
#

cocos_schema config

cocos_class config {
  inherit ParseTree
  constructor				new_configParseTree

  proc TreeNode unparse args		configTree_unparse
}



