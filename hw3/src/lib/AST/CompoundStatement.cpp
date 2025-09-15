#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line,
                                             const uint32_t col,
                                             std::vector<DeclNode*> *decl_nodes,
                                             std::vector<AstNode*> *statement_nodes)
    : AstNode{line, col}, decl_nodes(decl_nodes), statement_nodes(statement_nodes) {}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(decl_nodes)
    for(auto decl_node : *decl_nodes){
        decl_node->accept(p_visitor);
    }
    if(statement_nodes)
    for(auto statement_node : *statement_nodes){
        statement_node->accept(p_visitor);
    }
}
