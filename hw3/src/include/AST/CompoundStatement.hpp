#ifndef __AST_COMPOUND_STATEMENT_NODE_H
#define __AST_COMPOUND_STATEMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"

class CompoundStatementNode : public AstNode {
  public:
    CompoundStatementNode(const uint32_t line, const uint32_t col,
                          std::vector<DeclNode*> *decl_nodes,
                          std::vector<AstNode*> *statement_nodes
                          /* TODO: declarations, statements */);
    ~CompoundStatementNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: declarations, statements
    std::vector<DeclNode*> *decl_nodes;
    std::vector<AstNode*> *statement_nodes;
};

#endif
