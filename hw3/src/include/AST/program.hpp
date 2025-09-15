#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "AST/CompoundStatement.hpp"
#include <memory>

class ProgramNode final : public AstNode {
  private:
    std::string name;
    std::vector<DeclNode*> *decl_nodes;
    std::vector<FunctionNode*> *func_nodes;
    CompoundStatementNode *m_body;

  public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col,
            const char *const p_name, std::vector<DeclNode*> *decl_nodes,
            std::vector<FunctionNode*> *func_nodes, CompoundStatementNode *const p_body
            /* TODO: return type, declarations, functions,
             *       compound statement */);

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *getNameCString() const { return name.c_str(); }
    
};

#endif
