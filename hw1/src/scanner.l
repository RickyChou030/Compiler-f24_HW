%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LEN 512
/* Code runs each time a token is matched. */
#define YY_USER_ACTION updateCurrentLine(yytext);

uint32_t line_num = 1;
uint32_t col_num = 1;
bool str_empty = true;
static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char current_line[MAX_LINE_LEN];

static void updateCurrentLine(const char *source);
static void listToken(const char *name);
static void listLiteral(const char *name, const char *literal);

%}

DELIMITER   ","|";"|":"|"("|")"|"["|"]"
OPERATOR    "+"|"-"|"*"|"/"|"mod"|":="|"<"|"<="|"<>"|">="|">"|"="|"and"|"or"|"not"
KW          "var"|"def"|"array"|"of"|"boolean"|"integer"|"real"|"string"|"true"|"false"|"while"|"do"|"if"|"then"|"else"|"for"|"to"|"begin"|"end"|"print"|"read"|"return"
IDSUPPLY    (_[a-zA-Z0-9]+)*
ID          [a-zA-Z][a-zA-Z0-9]*{IDSUPPLY}
DEC         0|[1-9][0-9]*
OCT         0[0-7]+
FLOAT       {DEC}"."[0-9]+[1-9]|{DEC}"."[0-9]
SCIENTIFIC  [1-9][0-9]*("."[0-9]+[1-9]|"."[0-9])?([eE][-+]?{DEC})
STRING      ([\t\40\41\43-\176]|\"\")*
COMMENT     \/\/.*

%x COMMENT_MODE STRING_MODE

%%
"//&S+".*     { if(!opt_src) opt_src++; }
"//&S-".*     { if(opt_src) opt_src--; }
"//&T+".*     { if(!opt_tok) opt_tok++; }
"//&T-".*     { if(opt_tok) opt_tok--; }

{COMMENT}               { /* ignore */ }

"/*"                    { BEGIN COMMENT_MODE; }
<COMMENT_MODE>.|\n      { /* ignore */ }
<COMMENT_MODE>"*/"      { BEGIN INITIAL; }

[ \t\n]+                { /* ignore */ }

{DELIMITER}|{OPERATOR}  { listToken(yytext); }

{KW}                    {
                            char keyword[10] = "KW";
                            strcat(keyword, yytext);
                            listToken(keyword);
                        }

{ID}                    { listLiteral("id", yytext); }

{DEC}                   { listLiteral("integer", yytext); }

{OCT}                   { listLiteral("oct_integer", yytext); }

{FLOAT}                 { listLiteral("float", yytext); }

{SCIENTIFIC}            { listLiteral("scientific", yytext); }

\"                      { 
                            str_empty = true;
                            BEGIN STRING_MODE;
                        }
<STRING_MODE>{STRING}   {
                            str_empty = false;
                            char* srcstr = strdup(yytext);
                            char str[2048];
                            int count = 0;
                            int cnt = 0;
                            for(int i = 0 ; i < yyleng ; i++){
                                if(srcstr[i] == '\"'){
                                    count++;
                                    if(count == 2){
                                        count = 0;
                                        continue;
                                    }
                                }
                                str[cnt++] = srcstr[i];
                            }
                            str[cnt] = '\0';
                            listLiteral("string", str);
                        }
<STRING_MODE>\"         { 
                            if(str_empty) listLiteral("string", "");
                            BEGIN INITIAL;
                        }


    /* Catch the character which is not accepted by rules above */
.   {
        printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
        exit(-1);
    }

%%

/** @note The line is printed out and flushed when a newline character is encountered. */
static void updateCurrentLine(const char *source) {
    /* col_num is one-based */
    for (const char *c = source; *c; ++c) {
        if (*c == '\n') {
            if (opt_src) {
                printf("%d: %s\n", line_num, current_line);
            }
            ++line_num;
            col_num = 1;
            current_line[0] = '\0';
        } else {
            if (col_num < MAX_LINE_LEN) {
                current_line[col_num - 1] = *c;
                current_line[col_num] = '\0';
            } else {
                /* Truncate silently; doesn't affect the program's correctness. */
            }
            ++col_num;
        }
    }
}

static void listToken(const char *name) {
    if (opt_tok) {
        printf("<%s>\n", name);
    }
}

static void listLiteral(const char *name, const char *literal) {
    if (opt_tok) {
        printf("<%s: %s>\n", name, literal);
    }
}

/** @note This function is not required if the input file is guaranteed to end
 * with a newline. However, students may find it useful to handle the case where
 * the input file does not end with a newline, as it has been reported several
 * times in the past.
 */
int yywrap(void) {
    /* If the file is not ended with a newline, fake it to print out the last line. */
    if (col_num > 1) {
        updateCurrentLine("\n");
    }
    /* no more input file */
    return 1;
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
