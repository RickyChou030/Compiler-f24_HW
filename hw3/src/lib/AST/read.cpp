#include "AST/read.hpp"

// TODO
ReadNode::ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_ref_node)
    : AstNode{line, col}, variable_ref_node(variable_ref_node) {}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(variable_ref_node) variable_ref_node->accept(p_visitor);
}
