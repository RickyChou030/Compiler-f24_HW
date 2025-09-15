#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col,
                ExpressionNode* expression_node,
                CompoundStatementNode* body,
                CompoundStatementNode* body_else)
    : AstNode{line, col}, expression_node(expression_node), body(body), body_else(body_else) {}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(expression_node) expression_node->accept(p_visitor);
    if(body) body->accept(p_visitor);
    if(body_else) body_else->accept(p_visitor);
}
