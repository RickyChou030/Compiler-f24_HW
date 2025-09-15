#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, ExpressionNode* expression_node)
    : AstNode{line, col}, expression_node(expression_node) {}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(expression_node) expression_node->accept(p_visitor);
}
