# hw2 report

|||
|-:|:-|
|Name|周世恩|
|ID|110704010|

## How much time did you spend on this project

2 hours

## Project overview

I first go to scanner.l and modify the code to make it returns tokens.\
Then I open README.md and follow each syntactic definition to implement the grammar rule in a backtracking way.

- {zero or more} - add 's' behind
```c
functions: /* empty */
         | functions1
         ;

functions1: functions1 function
          | function
          ;
```

- [optional] - using opt_ as prefix
```c
function_call: ID '(' opt_expressions ')' ;

opt_expressions: /* empty */
               | expression comma_expressions
               ;
```

## What is the hardest you think in this project

DEBUG!!!!!!!!!! I have to check each grammar rule to find out which one is the one that error :sob:

## Feedback to T.A.s

Modify scanner.l is so boring because of repeating action :persevere: \
I don't know if this report is too short since I wrote a lot last time, plz let me know :grinning:
