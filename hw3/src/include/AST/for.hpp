#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/assignment.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/CompoundStatement.hpp"

class ForNode : public AstNode {
  public:
    ForNode(const uint32_t line, const uint32_t col, 
            DeclNode* decl_node,
            AssignmentNode* assignment_node,
            ConstantValueNode* constant_node,
            CompoundStatementNode* statement_node
            /* TODO: declaration, assignment, expression,
             *       compound statement */);
    ~ForNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: declaration, assignment, expression, compound statement
    DeclNode* decl_node;
    AssignmentNode* assignment_node;
    ConstantValueNode* constant_node;
    CompoundStatementNode* statement_node;
};

#endif
