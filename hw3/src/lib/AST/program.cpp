#include "AST/program.hpp"

// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                        const char *const p_name, std::vector<DeclNode*> *decl_nodes,
                        std::vector<FunctionNode*> *func_nodes, CompoundStatementNode *const p_body)
    : AstNode{line, col}, name(p_name), decl_nodes(decl_nodes), func_nodes(func_nodes), m_body(p_body) {}

// visitor pattern version: const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(decl_nodes)
    for(auto decl_node : *decl_nodes){
        decl_node->accept(p_visitor);
    }
    if(func_nodes)
    for(auto func_node : *func_nodes){
        func_node->accept(p_visitor);
    }
    if(m_body) m_body->accept(p_visitor);
}

// void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) { // visitor pattern version
//     /* TODO
//      *
//      * for (auto &decl : var_decls) {
//      *     decl->accept(p_visitor);
//      * }
//      *
//      * // functions
//      *
//      * body->accept(p_visitor);
//      */
// }
