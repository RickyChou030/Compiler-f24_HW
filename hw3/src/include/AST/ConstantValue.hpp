#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"

class ConstantValueNode : public ExpressionNode {
  public:
    ConstantValueNode(const uint32_t line, const uint32_t col ,std::string constant_value);
    ~ConstantValueNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    const char* getValue() const { return constant_value.c_str(); }

  private:
    // TODO: constant value
    std::string constant_value;
};

#endif
