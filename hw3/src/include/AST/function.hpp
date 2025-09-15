#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/CompoundStatement.hpp"

class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col,
                 std::string name, 
                 std::vector<DeclNode*> *decl_nodes,
                 std::string type,
                 CompoundStatementNode* m_body);
    ~FunctionNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char* getName() const { return name.c_str(); }
    const char* getType() const { return type.c_str(); }
    std::vector<DeclNode*> *getDeclNodes() { return decl_nodes; }

  private:
    // TODO: name, declarations, return type, compound statement
    std::string name;
    std::vector<DeclNode*> *decl_nodes;
    std::string type;
    CompoundStatementNode* m_body;
};

#endif
