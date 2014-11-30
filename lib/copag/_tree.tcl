#
# parser/_tree.tcl --
#
#	Data structures for parse trees, used by copag
#
# Copyright (c) 1993-1997 Olav Schettler
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# $Id: _tree.tcl,v 1.1 1995/10/25 20:09:24 olav Exp $
#--------------------------------------------------------------------------
# $Log: _tree.tcl,v $
# Revision 1.1  1995/10/25  20:09:24  olav
# 1
#
#

cocos_schema tree

cocos_class TreeNode {}

cocos_class ParseTree {
  constructor					new_ParseTree
  destructor					del_ParseTree
  public TreeNode tree				ParseTree_get_tree
}

cocos_enum NodeType {
  any syntax token rule prec ID LITERAL PATTERN opt list
  repeat single left right nonassoc words -prec -separator word
  word_list alternative_list alternatives tag -ignore -syntax
}

cocos_class TreeNode {
    proc string tokenize {{string s}}			TreeNode_tokenize
    method string all {{NodeType type} {string var} {string action}}	TreeNode_all
    method void one {{NodeType type} {string var} {string action}}	TreeNode_one
    method TreeNode parent {{NodeType type}}		TreeNode_parent
    public string type					TreeNode_get_type
    public string tag					TreeNode_get_tag
    public string text					TreeNode_get_text

    method void debug {}				TreeNode_debug
}
