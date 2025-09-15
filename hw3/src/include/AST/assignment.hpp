#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/VariableReference.hpp"
#include "AST/expression.hpp"

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col, 
                    VariableReferenceNode* variable_ref_node,
                    ExpressionNode* expression_node
                   /* TODO: variable reference, expression */);
    ~AssignmentNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: variable reference, expression
    VariableReferenceNode* variable_ref_node;
    ExpressionNode* expression_node;
};

#endif
