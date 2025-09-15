#ifndef AST_EXPRESSION_NODE_H
#define AST_EXPRESSION_NODE_H

#include "AST/PType.hpp"
#include "AST/ast.hpp"
#include <string>
using namespace std;

class ExpressionNode : public AstNode {
  public:
    ~ExpressionNode() = default;
    ExpressionNode(const uint32_t line, const uint32_t col)
        : AstNode{line, col} {}

    virtual string getExprType() { return ""; }
    virtual string getType(){ return ""; }
    virtual string getName(){ return ""; }
    virtual const PTypeSharedPtr &getPType(){ return nullptr; }
    virtual int varRef_getRefCnt(){ return 0; }
    virtual string binary_getType() { return ""; }
    virtual string unary_getType() { return ""; }
  protected:
    // for carrying type of result of an expression
    // TODO: for next assignment
    
};

#endif
