#ifndef AST_VARIABLE_REFERENCE_NODE_H
#define AST_VARIABLE_REFERENCE_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class VariableReferenceNode final : public ExpressionNode {
  public:
    using ExprNodes = std::vector<std::unique_ptr<ExpressionNode>>;

  private:
    std::string m_name;
   

  public:
   string type;
   ExprNodes m_indices;
    ~VariableReferenceNode() = default;

    // normal reference
    VariableReferenceNode(const uint32_t line, const uint32_t col,
                          const char *const p_name)
        : ExpressionNode{line, col}, m_name(p_name){}

    // array reference
    VariableReferenceNode(const uint32_t line, const uint32_t col,
                          const char *const p_name, ExprNodes &p_indices)
        : ExpressionNode{line, col}, m_name(p_name),
          m_indices(std::move(p_indices)){}

    string getExprType() override { return "varRef"; }
    int varRef_getRefCnt() override { return m_indices.size(); }
    const char *getNameCString() const { return m_name.c_str(); }
    string getName() override { return m_name; }
    string getType() override { return type; }
    std::vector<std::unique_ptr<ExpressionNode>> *getExprNodes() { return &m_indices; }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
