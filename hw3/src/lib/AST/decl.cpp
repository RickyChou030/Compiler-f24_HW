#include "AST/decl.hpp"

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> *variable_nodes, std::string type)
    : AstNode{line, col}, variable_nodes(variable_nodes), type(type) {}

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> *variable_nodes)
    : AstNode{line, col}, variable_nodes(variable_nodes) {}


void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(variable_nodes)
    for(auto variable_node : *variable_nodes){
        variable_node->accept(p_visitor);
    }
}
