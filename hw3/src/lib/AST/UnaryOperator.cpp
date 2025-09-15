#include "AST/UnaryOperator.hpp"

// TODO
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col, std::string oper, ExpressionNode* operand)
    : ExpressionNode{line, col}, oper(oper), operand(operand) {}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(operand) operand->accept(p_visitor);
}
