#include "AST/FunctionInvocation.hpp"

// TODO
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line,
                                               const uint32_t col,
                                               std::string name,
                                               std::vector<ExpressionNode*> *expression_nodes)
    : ExpressionNode{line, col}, name(name), expression_nodes(expression_nodes) {}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(expression_nodes)
    for(auto expression_node : *expression_nodes){
        expression_node->accept(p_visitor);
    }
}
