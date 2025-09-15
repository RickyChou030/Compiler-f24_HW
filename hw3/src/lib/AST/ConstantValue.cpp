#include "AST/ConstantValue.hpp"

// TODO
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, const std::string constant_value)
    : ExpressionNode{line, col}, constant_value(constant_value){}

