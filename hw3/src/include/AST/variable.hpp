#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col, 
                std::string variable_name, std::string type, ConstantValueNode *constant_value_node);
    ~VariableNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    const char* getName() const { return variable_name.c_str(); }
    const char* getType() const { return type.c_str(); }
    ConstantValueNode* getValueNode() { return constant_value_node; }
    void setType(std::string new_type) { type = new_type; }
    void setValueNode(ConstantValueNode* new_constant) { constant_value_node = new_constant; }

  private:
    // TODO: variable name, type, constant value
    std::string variable_name;
    std::string type;
    ConstantValueNode *constant_value_node;

};

#endif
