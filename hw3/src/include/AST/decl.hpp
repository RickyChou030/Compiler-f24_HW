#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"
#include "AST/variable.hpp"
#include "AST/ConstantValue.hpp"

class DeclNode : public AstNode {
  public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> *variable_nodes, std::string type
             /* TODO: identifiers, type */);

    // constant variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> *variable_nodes
            /* TODO: identifiers, constant */);

    ~DeclNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    std::string getType() { return type; }
    std::vector<VariableNode*>* getVarNodes() { return variable_nodes; }


  private:
    std::string type;
    std::vector<VariableNode*> *variable_nodes;
};

#endif
