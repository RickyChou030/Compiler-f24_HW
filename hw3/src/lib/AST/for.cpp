#include "AST/for.hpp"

// TODO
ForNode::ForNode(const uint32_t line, const uint32_t col,
                    DeclNode* decl_node,
                    AssignmentNode* assignment_node,
                    ConstantValueNode* constant_node,
                    CompoundStatementNode* statement_node)
    : AstNode{line, col}, decl_node(decl_node), assignment_node(assignment_node), constant_node(constant_node), statement_node(statement_node) {}

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(decl_node) decl_node->accept(p_visitor);
    if(assignment_node) assignment_node->accept(p_visitor);
    if(constant_node) constant_node->accept(p_visitor);
    if(statement_node) statement_node->accept(p_visitor);
}
