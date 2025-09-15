#ifndef AST_FOR_NODE_H
#define AST_FOR_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/assignment.hpp"
#include "AST/expression.hpp"
#include "AST/CompoundStatement.hpp"

class ForNode final : public AstNode {
  private:
    std::unique_ptr<DeclNode> m_loop_var_decl;
    std::unique_ptr<AssignmentNode> m_init_stmt;
    std::unique_ptr<ExpressionNode> m_end_condition;
    std::unique_ptr<CompoundStatementNode> m_body;

  public:
    ~ForNode() = default;
    ForNode(const uint32_t line, const uint32_t col,
            DeclNode *p_loop_var_decl, AssignmentNode *p_init_stmt,
            ExpressionNode *p_end_condition, CompoundStatementNode *p_body)
        : AstNode{line, col}, m_loop_var_decl(p_loop_var_decl),
          m_init_stmt(p_init_stmt), m_end_condition(p_end_condition),
          m_body(p_body) {}

    const ConstantValueNode &getLowerBound() const;
    const ConstantValueNode &getUpperBound() const;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void visitDecl(AstNodeVisitor &p_visitor){ m_loop_var_decl->accept(p_visitor); }
    void visitAssignment(AstNodeVisitor &p_visitor){ m_init_stmt->accept(p_visitor); }
    void visitCondition(AstNodeVisitor &p_visitor){ m_end_condition->accept(p_visitor); }
    void visitCompound(AstNodeVisitor &p_visitor){ m_body->accept(p_visitor); }
};

#endif
