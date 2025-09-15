#ifndef AST_FUNCTION_INVOCATION_NODE_H
#define AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>
#include <string>
using namespace std;

class FunctionInvocationNode final : public ExpressionNode {
  public:
    using ExprNodes = std::vector<std::unique_ptr<ExpressionNode>>;

  private:
    std::string m_name;
    ExprNodes m_args;

  public:
    string type;

    ~FunctionInvocationNode() = default;
    FunctionInvocationNode(const uint32_t line, const uint32_t col,
                           const char *const p_name, ExprNodes &p_args)
        : ExpressionNode{line, col}, m_name(p_name), m_args(std::move(p_args)){}

    string getExprType() override { return "func"; }
    string getType() override { return type; }
    const char *getNameCString() const { return m_name.c_str(); }
    string getName() override { return m_name; }
    int getParamsSize() { return m_args.size(); }
    std::vector<std::unique_ptr<ExpressionNode>> *getExprNodes() { return &m_args; }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
