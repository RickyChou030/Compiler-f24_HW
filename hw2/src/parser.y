%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token ID MOD ASSIGN LL LE LR RE RR AND OR NOT VAR ARRAY OF BOOL INT REAL STR TRUE FALSE DEF RETURN
%token BEGAN END WHILE DO IF THEN ELSE FOR TO PRINT READ DEC_INT OCT_INT FLOAT_NUM SCI_NUM STR_CON
%start program

%%

program: ID ';' declarations functions compound_statement END ;

declarations: /* empty */
            | declarations1
            ;

declarations1: declaration
             | declarations1 declaration
             ;

declaration: variable_declaration
           | constant_declaration
           ;

variable_declaration: VAR identifier_list ':' type ';' ;

constant_declaration: VAR identifier_list ':' negative_sign integer_literal ';'
                    | VAR identifier_list ':' negative_sign real_literal ';'
                    | VAR identifier_list ':' string_literal ';'
                    | VAR identifier_list ':' boolean_literal ';'
                    ;

negative_sign: /* empty */
             | '-'
             ;

identifier_list: ID ids;

ids: /* empty */
   | ',' ID
   | ids ',' ID
   ;

type: scalar_type
    | array_type
    ;

scalar_type: INT | REAL | STR | BOOL ;

array_type: ARRAY integer_literal OF type ;

integer_literal: OCT_INT | DEC_INT ;

real_literal: FLOAT_NUM | SCI_NUM ;

string_literal: STR_CON ;

boolean_literal: TRUE | FALSE ;

functions: /* empty */
         | functions1
         ;

functions1: functions1 function
          | function
          ;

function: function_declaration
        | function_definition
        ;

function_declaration: function_header ';' ;

function_header: ID '(' opt_formal_argument ')' ':' scalar_type
               | ID '(' opt_formal_argument ')'
               ;

function_definition: function_header compound_statement END ;


opt_formal_argument: /* empty */
                   | formal_argument formal_arguments
                   ;

formal_arguments: /* empty */
                | formal_arguments ';' formal_argument
                | ';' formal_argument
                ;

formal_argument: identifier_list ':' type ;

compound_statement: BEGAN declarations statements END ;

statements: /* empty */
          | statements1
          ;

statements1: statements1 statement
           | statement
           ;

statement: simple_statement
         | conditional_statement
         | function_call_statement
         | loop_statement
         | return_statement
         | compound_statement
         ;

simple_statement: variable_reference ASSIGN expression ';'
                | PRINT expression ';'
                | READ variable_reference ';'
                ;

variable_reference: ID expressions

expressions: /* empty */
           | expressions '[' expression ']'
           | '[' expression ']'
           ;

expression: literal_constant
          | variable_reference
          | function_call
          | binary_operation
          | unary_operation
          | '(' expression ')'
          ;

literal_constant: integer_literal
                | real_literal
                | string_literal
                | boolean_literal
                ;

function_call: ID '(' opt_expressions ')' ;

opt_expressions: /* empty */
               | expression comma_expressions
               ;

comma_expressions: /* empty */
                 | comma_expressions1
                 ;

comma_expressions1: comma_expressions1 ',' expression
                  | ',' expression
                  ;

binary_operation: expression binary_operator expression ;

binary_operator: '+' | '-' | '*' | '/' | MOD | LL | LE | LR | RE | RR | '=' | AND | OR ;

unary_operation: unary_operator expression ;

unary_operator: '-' | NOT ;

conditional_statement: IF expression THEN compound_statement ELSE compound_statement END IF
                     | IF expression THEN compound_statement END IF
                     ;

function_call_statement: function_call ';' ;

loop_statement: WHILE expression DO compound_statement END DO
              | FOR ID ASSIGN integer_literal TO integer_literal DO compound_statement END DO
              ;

return_statement: RETURN expression ';' ;

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
