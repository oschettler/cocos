#
# parser/lib/tree.tcl --
#
#	Data structures for parse trees, used by at run-time by
#	generated language processors
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: tree.tcl,v 1.1 1995/10/25 20:09:25 olav Exp $
#--------------------------------------------------------------------------
# $Log: tree.tcl,v $
# Revision 1.1  1995/10/25  20:09:25  olav
# 1
#
#

cocos_schema tree

cocos_class TreeNode {}

cocos_class ParseTree {
  destructor					del_ParseTree
  public TreeNode tree				ParseTree_get_tree
  public string text				ParseTree_get_text
}

cocos_class TreeNode {
    constructor					new_TreeNode
    destructor					del_TreeNode

    method void all args			TreeNode_all
    method void one args			TreeNode_one
    method TreeNode parent {{string type}}	TreeNode_parent
    public string type				TreeNode_get_type
    public int start				TreeNode_get_start
    public int end				TreeNode_get_end
    public string text				TreeNode_get_text
    public string tag				TreeNode_get_tag
    method void debug {}			TreeNode_debug
    method void export {{string file}}		TreeNode_export
}
