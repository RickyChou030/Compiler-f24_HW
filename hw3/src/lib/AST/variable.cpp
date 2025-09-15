#include "AST/variable.hpp"

// TODO
VariableNode::VariableNode(const uint32_t line, const uint32_t col, std::string variable_name, std::string type, ConstantValueNode *constant_value_node)
    : AstNode{line, col}, variable_name(variable_name), type(type), constant_value_node(constant_value_node) {}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(constant_value_node) constant_value_node->accept(p_visitor);
}
