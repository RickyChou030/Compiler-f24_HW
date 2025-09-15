#include "AST/BinaryOperator.hpp"

// TODO
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col,
                                       std::string oper,
                                       ExpressionNode* left_operand,
                                       ExpressionNode* right_operand)
    : ExpressionNode{line, col}, oper(oper), left_operand(left_operand), right_operand(right_operand) {}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(left_operand) left_operand->accept(p_visitor);
    if(right_operand) right_operand->accept(p_visitor);
}
