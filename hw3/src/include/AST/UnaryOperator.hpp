#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col, std::string oper, ExpressionNode* operand
                      /* TODO: operator, expression */);
    ~UnaryOperatorNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char* getOper() const { return oper.c_str(); }

  private:
    // TODO: operator, expression
    std::string oper;
    ExpressionNode* operand;
};

#endif
