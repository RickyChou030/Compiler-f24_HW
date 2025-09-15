#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col,
                                VariableReferenceNode* variable_ref_node,
                                ExpressionNode* expression_node)
    : AstNode{line, col}, variable_ref_node(variable_ref_node), expression_node(expression_node) {}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(variable_ref_node) variable_ref_node->accept(p_visitor);
    if(expression_node) expression_node->accept(p_visitor);
}
