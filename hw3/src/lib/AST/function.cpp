#include "AST/function.hpp"

// TODO
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, std::string name, 
                std::vector<DeclNode*> *decl_nodes, std::string type,
                CompoundStatementNode* f_body)
    : AstNode{line, col}, name(name), decl_nodes(decl_nodes), type(type), m_body(f_body) {}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(decl_nodes)
    for(auto decl_node : *decl_nodes){
        decl_node->accept(p_visitor);
    }
    if(m_body) m_body->accept(p_visitor);
}
