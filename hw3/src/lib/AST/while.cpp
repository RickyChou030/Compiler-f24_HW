#include "AST/while.hpp"

// TODO
WhileNode::WhileNode(const uint32_t line, const uint32_t col,
                    ExpressionNode* expression_node,
                    CompoundStatementNode* statement_node)
    : AstNode{line, col}, expression_node(expression_node), statement_node(statement_node) {}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(expression_node) expression_node->accept(p_visitor);
    if(statement_node) statement_node->accept(p_visitor);
}
