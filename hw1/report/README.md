# hw1 report

|Field|Value|
|-:|:-|
|Name|周世恩|
|ID|110704010|

## How much time did you spend on this project

I spent almost a whole day on this assignment.

## Project overview

The whole code is constructing by:
|Section|Line|
|-:|:-|
|Definitions|1 ~ 42|
|Transition Rules|43 ~ 109|
|User Subroutines|110 ~ 182|

I first write down definitions of each type of token we have in this assignment at Line 29 ~ Line 39 for later use:
```c
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
```

Then, I use the function `listToken()` and `listLiteral()` to list each of them like Line 57 ~ Line 73:
```c
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
```

As for comment and string, we have to deal with them with more steps. So I use the exclusive start condition `%x` to handle this:

- First, we add two exclusive start conditions at Line 41 `%x COMMENT_MODE STRING_MODE`
- For comments:
    - We handle the `//` type of comment at Line 49 by our priviously defined `{COMMENT}` definition at Line 39 and ignore it:
        ```c
        {COMMENT}               { /* ignore */ }
        ```
    - We handle the `/* */` type of comment at Line 51 ~ Line 53. The __first__ line enable the `COMMENT_MODE` by reading `/*`. After getting into `COMMENT_MODE`, we simply ignore every character and `\n` using the __second__ line then leaving `COMMENT_MODE` by reading `*/` using the __third__ line:
        ```c
        "/*"                    { BEGIN COMMENT_MODE; }
        <COMMENT_MODE>.|\n      { /* ignore */ }
        <COMMENT_MODE>"*/"      { BEGIN INITIAL; }
        ```
- For strings:
    - We handle it at Line 75 ~ Line 101. The __first__ transition rule enable the `STRING_MODE` and initialize the boolean `str_empty` we defined at Line 18 as `true`. The __second__ transition rule can scan the string and handle the `""` condition by counting the appearence of `"` character and put it to the new string then list it by `listLiteral()` . The __third__ transition rule leave the `STRING_MODE` by reading `"` character and list empty string if empty.
        ```c
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
        ```

For space / tabs / newline not including in the comments or strings, we handle it by the following transition rule at Line 55:
```c
[ \t\n]+                { /* ignore */ }
```

For pseudocomments, we handle it at Line 44 ~ Line 47 by plus or minus corresponding static values judged by their original value and current pseudocomment:
```c
"//&S+".*     { if(!opt_src) opt_src++; }
"//&S-".*     { if(opt_src) opt_src--; }
"//&T+".*     { if(!opt_tok) opt_tok++; }
"//&T-".*     { if(opt_tok) opt_tok--; }
```

For error handling, I didn't touch the original source code at Line 104 ~ Line 108 cause it already functional:
```c
    /* Catch the character which is not accepted by rules above */
.   {
        printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
        exit(-1);
    }
```

## What is the hardest you think in this project

I think there are three points that is hard for me:
- Comment
- String
- Pseudocomment

For Comment, the `/* */` style comment is difficult to make a regular expression for it cause the regular expression is longest match, which means if I have a code like [`16_advance_comment_1.p`](../test/test_cases/16_advance_comment_1.p) the regular expression will eat up from Line 4 to Line 17 and which is not we wanted. Not until I found out there has exclusive start condition in flex's document, this sent me to the whole new world and then quickly and simply solved it.

For String, I am not familiar with C style string since I always write C++ code for most of the time of the past few years. This also has the same problem as Comment that is it is difficult to write a regular expression (exclusive start condition is a __GOAT__ solution). Not to mention that String handling has `""` and empty string to deal with in the action part.

For Pseudocomment, I forgot how to use `static` :x:. I tested a lot of regular expressions but found the most simple one is just quote it like `"//&S+".*` so I mentally break down after that :o:. :cry:

## Feedback to T.A.s

Generally speaking, this assignment gives me so much fun (and pain) since every detail is explained in the document. The sense of accomplishment after solving a small section is no doubt and make you want to solve more :heart_eyes:.

My personnaly favorite part is the emoji :monocle_face:, I hope we can have as many emoji as we could in future assignments.

:congratulations: T.A.s can guaduate in 2 years or less and make lots of money in the future :D
