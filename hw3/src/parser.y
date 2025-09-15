%{
#include "AST/AstDumper.hpp"
#include "AST/expression.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

// This guarantees that headers do not conflict when included together.
%define api.token.prefix {TOK_}

%code requires {
    class AstNode;
    class ExpressionNode;
    class ProgramNode;
    class DeclNode;
    class VariableNode;
    class ConstantValueNode;
    class FunctionNode;
    class CompoundStatementNode;
    class PrintNode;
    class BinaryOperatorNode;
    class UnaryOperatorNode;
    class FunctionInvocationNode;
    class VariableReferenceNode;
    class AssignmentNode;
    class ReadNode;
    class IfNode;
    class WhileNode;
    class ForNode;
    class ReturnNode;
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;
    int *int_id;
    float *float_id;
    char *str_id;
    char *bool_id;
    
    AstNode *node;
    std::vector<AstNode*> *nodes;
    ExpressionNode *expression_ptr;
    std::vector<ExpressionNode*> *expression_ptr_list;
    ProgramNode *program_ptr;
    DeclNode *decl_ptr;
    std::vector<DeclNode*> *decl_ptr_list;
    VariableNode *variable_ptr;
    std::vector<VariableNode*> *variable_ptr_list;
    ConstantValueNode *constant_ptr;
    FunctionNode *func_ptr;
    std::vector<FunctionNode*> *func_ptr_list;
    CompoundStatementNode *compound_stmt_ptr;
    std::vector<CompoundStatementNode*> *compount_stmt_ptr_list;
    PrintNode *print_ptr;
    BinaryOperatorNode *binary_oper_ptr;
    UnaryOperatorNode *unary_oper_ptr;
    FunctionInvocationNode *func_invo_ptr;
    VariableReferenceNode *variable_ref_ptr;
    AssignmentNode *assignment_ptr;
    ReadNode *read_ptr;
    IfNode *if_ptr;
    WhileNode *while_ptr;
    ForNode *for_ptr;
    ReturnNode *return_ptr;
};

%type <identifier> ProgramName ID FunctionName NegOrNot Type ReturnType ScalarType ArrType ArrDecl StringAndBoolean IntegerAndReal
%type <identifier> MINUS MULTIPLY DIVIDE MOD PLUS LESS LESS_OR_EQUAL GREATER GREATER_OR_EQUAL EQUAL NOT_EQUAL NOT AND OR
%type <int_id> INT_LITERAL
%type <float_id> REAL_LITERAL
%type <str_id> STRING_LITERAL
%type <bool_id> TRUE FALSE

%type <node> Statement Simple Condition While For Return FunctionCall
%type <nodes> StatementList Statements
%type <expression_ptr> Expression
%type <expression_ptr_list> ArrRefList ArrRefs ExpressionList Expressions
%type <decl_ptr> Declaration FormalArg
%type <decl_ptr_list> DeclarationList Declarations FormalArgList FormalArgs
%type <variable_ptr_list> IdList
%type <constant_ptr> LiteralConstant
%type <func_ptr> Function FunctionDeclaration FunctionDefinition
%type <func_ptr_list> FunctionList Functions
%type <compound_stmt_ptr> CompoundStatement ElseOrNot
%type <func_invo_ptr> FunctionInvocation
%type <variable_ref_ptr> VariableReference

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
    /* TODO: specify the precedence of the following operators */

%token  OR AND NOT
%left  LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left  MINUS
%left  PLUS
%left  DIVIDE MOD
%left  MULTIPLY
%right UMINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

ProgramUnit:
    Program
    |
    Function
;

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {
        root = new ProgramNode(@1.first_line, @1.first_column,
                               $1, $3, $4, $5);

        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon  { $$ = NULL; }
    |
    Declarations { $$ = $1; }
;

Declarations:
    Declaration {
        $$ = new std::vector<DeclNode*>();
        $$->push_back($1);
    }
    |
    Declarations Declaration {
        $1->push_back($2);
        $$ = $1;
    }
;

FunctionList:
    Epsilon { $$ = NULL; }
    |
    Functions { $$ = $1; }
;

Functions:
    Function {
        $$ = new std::vector<FunctionNode*>();
        $$->push_back($1);
    }
    |
    Functions Function {
        $1->push_back($2);
        $$ = $1;
    }
;

Function:
    FunctionDeclaration { $$ = $1; }
    |
    FunctionDefinition { $$ = $1; }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3, $5, NULL);
    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3, $5, $6);
    }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon { $$ = NULL; }
    |
    FormalArgs { $$ = $1; }
;

FormalArgs:
    FormalArg {
        $$ = new std::vector<DeclNode*>();
        $$->push_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg {
        $1->push_back($3);
        $$ = $1;
    }
;

FormalArg:
    IdList COLON Type {
        for(auto variable_node : *$1){
            variable_node->setType($3);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, $1, $3);
    }
;

IdList:
    ID {
        $$ = new std::vector<VariableNode*>();
        $$->push_back(new VariableNode(@1.first_line, @1.first_column, $1, "", NULL));
    }
    |
    IdList COMMA ID {
        $1->push_back(new VariableNode(@3.first_line, @3.first_column, $3, "", NULL));
        $$ = $1;
    }
;

ReturnType:
    COLON ScalarType { $$ = $2; }
    |
    Epsilon { $$ = strdup("void"); }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON {
        for(auto variable_node : *$2){
            variable_node->setType($4);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON {
        std::string val = std::string($4->getValue());
        for(auto variable_node : *$2){
            std::string type = "string";
            if(val == "true" || val == "false"){
                type = "boolean";
            }

            int hasDot = 0;
            try{
                int temp_int = stoi(val);
                type = "integer";
                if(val.find('.') != std::string::npos){
                    type = "real";
                }
            }catch(...){}
            
            
            variable_node->setType(type);
            variable_node->setValueNode($4);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, $2);
    }
;

Type:
    ScalarType { $$ = $1; }
    |
    ArrType { $$ = $1; }
;

ScalarType:
    INTEGER { $$ = strdup("integer"); }
    |
    REAL { $$ = strdup("real"); }
    |
    STRING { $$ = strdup("string"); }
    |
    BOOLEAN { $$ = strdup("boolean"); }
;

ArrType:
    ArrDecl ScalarType {
        std::string temp = std::string($2) + " " + std::string($1);
        $$ = strdup(temp.c_str());
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF {
        std::string temp = "[" + std::to_string(*$2) + "]";
        $$ = strdup(temp.c_str());
    }
    |
    ArrDecl ARRAY INT_LITERAL OF{
        std::string temp = std::string($1) + "[" + std::to_string(*$3) + "]";
        $$ = strdup(temp.c_str());
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL {
        std::string temp = std::string($1) + std::to_string(*$2);
        int neg = ($1 == "") ?0 :1;
        $$ = new ConstantValueNode(@2.first_line, @2.first_column - neg, temp);
    }
    |
    NegOrNot REAL_LITERAL {
        std::string temp = std::string($1) + std::to_string(*$2);
        int neg = ($1 == "") ?0 :1;
        $$ = new ConstantValueNode(@2.first_line, @2.first_column - neg, temp);
    }
    |
    StringAndBoolean {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
;

NegOrNot:
    Epsilon { $$ = ""; }
    |
    MINUS { $$ = "-";}
;

StringAndBoolean:
    STRING_LITERAL {
        std::string s = std::string($1);
        $$ = strdup(s.substr(1, s.length()-2).c_str());
    }
    |
    TRUE { $$ = $1; }
    |
    FALSE { $$ = $1; }
;

IntegerAndReal:
    INT_LITERAL {
        $$ = strdup(std::to_string(*$1).c_str());
    }
    |
    REAL_LITERAL {
        $$ = strdup(std::to_string(*$1).c_str());
    }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement { $$ = $1; }
    |
    Simple { $$ = $1; }
    |
    Condition { $$ = $1; }
    |
    While { $$ = $1; }
    |
    For { $$ = $1; }
    |
    Return { $$ = $1; }
    |
    FunctionCall { $$ = $1; }
;

CompoundStatement:
    BEGIN
    DeclarationList
    StatementList
    END {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON {
        $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);
    }
    |
    PRINT Expression SEMICOLON {
        $$ = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    |
    READ VariableReference SEMICOLON {
        $$ = new ReadNode(@1.first_line, @1.first_column, $2);
    }
;

VariableReference:
    ID ArrRefList {
        $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2);
    }
;

ArrRefList:
    Epsilon { $$ = NULL; }
    |
    ArrRefs { $$ = $1; }
;

ArrRefs:
    L_BRACKET Expression R_BRACKET {
        $$ = new std::vector<ExpressionNode*>();
        $$->push_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET {
        $1->push_back($3);
        $$ = $1;
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF {
        $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5);
    }
;

ElseOrNot:
    ELSE
    CompoundStatement { $$ = $2; }
    |
    Epsilon { $$ = NULL; }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO {
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO {
        VariableNode *temp_var = new VariableNode(@2.first_line, @2.first_column, $2, "integer", NULL);
        std::vector<VariableNode*> *temp_var_list = new std::vector<VariableNode*>();
        temp_var_list->push_back(temp_var);
        DeclNode *temp_decl = new DeclNode(@2.first_line, @2.first_column, temp_var_list);

        VariableReferenceNode *temp_var_ref = new VariableReferenceNode(@2.first_line, @2.first_column, $2, NULL);
        AssignmentNode *temp_ass = new AssignmentNode(@3.first_line, @3.first_column, temp_var_ref, new ConstantValueNode(@4.first_line, @4.first_column, std::to_string(*$4)));

        ConstantValueNode *temp_const = new ConstantValueNode(@6.first_line, @6.first_column, std::to_string(*$6));
        $$ = new ForNode(@1.first_line, @1.first_column, temp_decl, temp_ass, temp_const, $8);
    }
;

Return:
    RETURN Expression SEMICOLON {
        $$ = new ReturnNode(@1.first_line, @1.first_column, $2);
    }
;

FunctionCall:
    FunctionInvocation SEMICOLON { $$ = $1; }
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS {
        $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3);
    }
;

ExpressionList:
    Epsilon { $$ = NULL; }
    |
    Expressions { $$ = $1; }
;

Expressions:
    Expression {
        $$ = new std::vector<ExpressionNode*>();
        $$->push_back($1);
    }
    |
    Expressions COMMA Expression {
        $1->push_back($3);
        $$ = $1;
    }
;

StatementList:
    Epsilon { $$ = new std::vector<AstNode*>(); }
    |
    Statements { $$ = $1; }
;

Statements:
    Statement {
        $$ = new std::vector<AstNode*>();
        $$->push_back($1);
    }
    |
    Statements Statement {
        $1->push_back($2);
        $$ = $1;
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS { $$ = $2; }
    |
    MINUS Expression %prec UMINUS{
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "neg", $2);
    }
    |
    Expression MULTIPLY Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "*", $1, $3);
    }
    |
    Expression DIVIDE Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "/", $1, $3);
    }
    |
    Expression MOD Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "mod", $1, $3);
    }
    |
    Expression PLUS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "+", $1, $3);
    }
    |
    Expression MINUS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "-", $1, $3);
    }
    |
    Expression LESS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<", $1, $3);
    }
    |
    Expression LESS_OR_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<=", $1, $3);
    }
    |
    Expression GREATER Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">", $1, $3);
    }
    |
    Expression GREATER_OR_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">=", $1, $3);
    }
    |
    Expression EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "=", $1, $3);
    }
    |
    Expression NOT_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<>", $1, $3);
    }
    |
    NOT Expression {
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "not", $2);
    }
    |
    Expression AND Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "and", $1, $3);
    }
    |
    Expression OR Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "or", $1, $3);
    }
    |
    IntegerAndReal {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, std::string($1));
    }
    |
    StringAndBoolean {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    |
    VariableReference { $$ = $1; }
    |
    FunctionInvocation { $$ = $1; }
;

    /*
       misc
            */
Epsilon:
;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        AstDumper ast_dumper;
        root->accept(ast_dumper);
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
