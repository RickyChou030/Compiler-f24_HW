#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include <memory>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col,
                       std::string oper,
                       ExpressionNode* left_operand,
                       ExpressionNode* right_operand);
    ~BinaryOperatorNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char* getOper() const { return oper.c_str(); }

  private:
    // TODO: operator, expressions
    std::string oper;
    ExpressionNode* left_operand;
    ExpressionNode* right_operand;

};

#endif
