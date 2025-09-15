#ifndef AST_UNARY_OPERATOR_NODE_H
#define AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "AST/operator.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class UnaryOperatorNode final : public ExpressionNode {
  private:
    Operator m_op;
    std::unique_ptr<ExpressionNode> m_operand;

  public:
    string type;

    ~UnaryOperatorNode() = default;
    UnaryOperatorNode(const uint32_t line, const uint32_t col, Operator op,
                      ExpressionNode *p_operand)
        : ExpressionNode{line, col}, m_op(op), m_operand(p_operand) {}

    string getExprType() override { return "unary"; }
    string unary_getType() override { return type; }
    string getType() override { return type; }
    const char *getOpCString() const {
        return kOpString[static_cast<size_t>(m_op)];
    }
    string getOperandExprType() { return m_operand->getExprType(); }
    string getOpString() {
        return kOpString[static_cast<size_t>(m_op)];
    }
    Operator getOp() {
        return m_op;
    }
    std::unique_ptr<ExpressionNode> &getOperand() { return m_operand; }
    const PTypeSharedPtr & getPType() {
        return m_operand->getPType();
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
