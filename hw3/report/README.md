# hw3 report

|      |                 |
| ---: | :-------------- |
| Name | yourname        |
|周世恩 | 110704010       |

## How much time did you spend on this project

2 whole days before the deadline since I barely have time to do it because of other homeworks QQ

## Project overview

> I first read the whole document for 20 mins and get lost for 2hrs coding random stuff since I didn't see the [ast_guileline.md](/ast_guideline.md) at the first time. Then I discarded everything I've changed and get started from the beginning with the guide of [ast_guileline.md](/ast_guideline.md) .

First, I add functions and content of node into the .hpp of every node to let us use the AST dumper and then modify their constructor for later use.
```cpp
/* e.g. function.hpp */
  public:
    // modified consturctor
    FunctionNode(const uint32_t line, const uint32_t col,
                 std::string name, 
                 std::vector<DeclNode*> *decl_nodes,
                 std::string type,
                 CompoundStatementNode* m_body);
    ~FunctionNode() = default;

    // for AST dumper
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    // for accessing private variable
    const char* getName() const { return name.c_str(); }
    const char* getType() const { return type.c_str(); }
    std::vector<DeclNode*> *getDeclNodes() { return decl_nodes; }

  private:
    // content of node
    std::string name;
    std::vector<DeclNode*> *decl_nodes;
    std::string type;
    CompoundStatementNode* m_body;
```

Then I opened up .cpp files to complete `visitChildNodes()` and initilization.
```cpp
/* e.g. function.hpp */
// initialization
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, std::string name, 
                std::vector<DeclNode*> *decl_nodes, std::string type,
                CompoundStatementNode* f_body)
    : AstNode{line, col}, name(name), decl_nodes(decl_nodes), type(type), m_body(f_body) {}

// for AST dumper
void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(decl_nodes)
    for(auto decl_node : *decl_nodes){
        decl_node->accept(p_visitor);
    }
    if(m_body) m_body->accept(p_visitor);
}
```

> Now we are ready to the most difficult part, which is modifying `parser.y`

At First, I add class of each node in `%code requires` section.
```cpp
%code requires {
    class AstNode;
    class ExpressionNode;
    ...
}
```

Then, add lots of pointers in the union part.
```cpp
%union {
    /* basic semantic value */
    char *identifier;
    int *int_id;
    ...

    AstNode *node;
    std::vector<AstNode*> *nodes;
    ...
};
```

Then, edit the code and add type in the type section whenever we need it.
```cpp
// type section
%type <identifier> ProgramName ID FunctionName ...
%type <int_id> INT_LITERAL
...

%type <node> Statement Simple Condition While For Return FunctionCall
%type <nodes> StatementList Statements
...
```

For editing the code for production rules, I just edit one production rule's code then make the file to see at which line has warning and go there to fix it, then repeat the whole process back and forth.

    A: B  ->  A: B {...}  ->  make  ->  warning:... at Line x  ->  go to Line x and fix it  ->  (repeat)

# Some problems while implementing the homework.

1. `char*` and `std::string` conversion is a whole mess, using string for flexibility at first but get fxcked afterward :sob:
    - Solution: NONE, just remember everything's type :disappointed_relieved:
2. Need to go to `scanner.l` to set `yylval` value. Otherwise, the scanner will parse weird value in
    - Solution: Define pointers in union for `INT_LITERAL`, `REAL_INTERAL`, ... ,and take care of the value you want to put inside `yylval`
3. Nesting issue of various nodes. Sometimes we have to construct a node all the way down to its leaf
    - Solution: Keep a backtracking manner while constructing nodes
4. Had a hard time dealing with function dumping
    - Solution: After debugging and seeing ADDRESS SANITIZER warning for 1hr, I finally saw that I forgot to initialize the `IfNode` :rage::angry::expressionless::tired_face::sob:
5. `neg` and `-` precedence problem
    - Solution: Add a pseudo token `UMINUS` with high order and add `%prec UMINUS` in the production rule of `neg`
6. Add type for constant declaration
    - Solution: Use `getValue()` to get `ConstantNode`'s value and judge its type, then put it back to the `VariableNode` in the `DeclNode`

## What is the hardest you think in this project

The __DEBUG HELL__ of course. Can you imagine I spent almost 1hr just to find out that I didn't initialize something :sob: Not to mention that the above questions are just a small part of all questions I've met :sob:

## Feedback to T.A.s

Time flies so fast whlie doing this homework. Sometimes you feel that only 30 minutes have passed, but it turns out that four hours have passed. The only things that can surprise me about the change of time are my sore eyes, dry tongue, and sore spine. I should have started my homework earlier :sob:
