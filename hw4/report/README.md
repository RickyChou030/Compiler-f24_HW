# hw4 report

|      |                 |
| ---: | :-------------- |
| Name | yourname        |
|周世恩 | 110704010       |

## How much time did you spend on this project

2 regular days and 3 days without sleep and skip the class in the morning to finish this homework :sob::sob::sob:

## Project overview

> The beginning of the __NIGHTMARE__

First of all, I added two file, [SymbolTable.hpp](/src/include/sema/SymbolTable.hpp) and [SymbolTable.cpp](/src/lib/sema/SymbolTable.cpp), for easier handling of the project.

In [SymbolTable.hpp](/src/include/sema/SymbolTable.hpp), I defined `SymbolEntry` to store data, `SymbolTable` to store entries, `SymbolManager` to handle the tables, `ContextManager` to handle the context problem. Below is an example, please go to [SymbolTable.hpp](/src/include/sema/SymbolTable.hpp) for more info.
```cpp
class SymbolEntry {
public:
    ~SymbolEntry() = default;
    SymbolEntry(string name, string kind, int level, string type, string attribute): name(name), kind(kind), level(level), type(type), attribute(attribute){}

    string getName() { return name; }
    string getKind() { return kind; }
    int getLevel() { return level; }
    string getType() { return type; }
    string getAttribute() { return attribute; }
    bool err = false;

private:
    string name;
    string kind;
    int level;
    string type;
    string attribute;
};
```

In [SymbolTable.cpp](/src/lib/sema/SymbolTable.cpp), implement the functions. Below is an example, please go to [SymbolTable.cpp](/src/lib/sema/SymbolTable.cpp) for more info.
```cpp
void SymbolManager::dumpSymbol(SymbolTable* table) {
    dumpDemarcation('=');
    printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                        "Attribute");
    dumpDemarcation('-');
    {
        for(auto entry : table->getEntries()){
            printf("%-33s", entry.getName().c_str());
            printf("%-11s", entry.getKind().c_str());
            printf("%d%-10s", entry.getLevel(), (entry.getLevel() == 0) ?"(global)" :"(local)");
            printf("%-17s", entry.getType().c_str());
            printf("%-11s", entry.getAttribute().c_str());
            puts("");
        }
    }
    dumpDemarcation('-');
}
```


Then, add `SymbolManager` and `ContextManager` into the [SemanticAnalyzer.hpp](/src/include/sema/SemanticAnalyzer.hpp) like below to make the use of them.
```cpp
class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    ErrorPrinter m_error_printer{stderr};
    SymbolManager s_manager;     <---- HERE
    ContextManager c_manager;    <---- HERE
    ...

  public:
    ...
};
```

> Now we can start to handle the [SemanticAnalyzer.cpp](/src/lib/sema/SemanticAnalyzer.cpp), which is our mainly modified file.

In this part, just follow the testcase to modify their `SemanticAnalyzer::visit()` function and `.hpp` file respectively.

```cpp
void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     * ...
     */
    // 1
    SymbolTable *currtable = new SymbolTable;
    c_manager.global = currtable;
    s_manager.pushScope(currtable);
    // 2
    SymbolEntry entry = SymbolEntry(p_program.getName(), "program", s_manager.getLevel(), p_program.getType(), "");
    if(currtable->uniqueEntry(entry)){
        currtable->addSymbol(entry);
    }else{
        s_manager.hasErr = true;
        m_error_printer.print(SymbolRedeclarationError(p_program.getLocation(), p_program.getNameCString()));
    }
    // 3
    p_program.visitChildNodes(*this);
    // 4
    // none
    // 5
    s_manager.popScope();
}
```
```cpp
...
class ProgramNode final : public AstNode {
  public:
    ...

  private:
    ...

  public:
    ...
    string getName() { return m_name; }
    string getType() { return m_ret_type->getPTypeCString(); }
    ...
};
...
```

After modifying the code, `make` the file to test if it worked fine. If not, review the [diff.txt](/test/result/diff.txt) to see where is wrong and ~~make guesses of the problem then debug for at least 3hrs for a single line problem (I'm dead)~~.

Repeat the above process again and again until the color of score turned green.

## Some problems while implementing the homework.

1. The context problem: It always happened to me that I need something but it is deep in the childnode or somewhere else.
    - Solution: Add `ContextManager` to handle it (for more info: [SymbolTable.hpp](/src/include/sema/SymbolTable.hpp) and [SymbolTable.cpp](/src/lib/sema/SymbolTable.cpp)).
2. Need of previous table's contents: 
    - Solution: I tried using multiple ways but it all failed, so I eventually choose to `pop()` the current table then `push()` it back (which invoke a debug disaster in next problem :sob:)
3. `pop()` the table and forget to `push()` it back:
    - Solution: This literally took me 6hrs to debug and it turned out that I forgot to `push()` the table back in one line. ___(You can skip this part because I'm complaining)___ At first, it trigger `ADDRESS SANITIZER` and I don't know what is going on. I haven't realize the importance that I need to print out a lot of info to make sure where is wrong. So I rewrite and modify other parts of the code instead of the one that has problem for 20 times and it took me 6hrs to realize I'm doing nothing ~~(after that I want to end myself)~~.
4. Scoping problem: Like `FunctionNode` or `ForNode` while creating a new table.
    - Solution: Since `FunctionNode` has to share its table with the closest `CompoundStatementNode`, I turn on `bool inFunc` and add one to `int funcDepth` in the `ContextManager` to make sure only `CompoundStatementNode` with `funcDepth > 1` can create a new table.
5. `//&D-` problem:
    - Solution: Go to [scanner.l](/src/scanner.l) (code block 1) to make it scan `D` and send token `OPT_DUMP` to [parser.y](/src/parser.y), after that in [parser.y](/src/parser.y) (code block 2) read that token and turn on/off the switch.
    ```cpp
        /* Pseudocomment */
    "//&"[STD][+-].* {    <---- MODIFY
        char option = yytext[3];
        switch (option) {
        ...
        case 'D':    <---- ADD
            yylval.boolean = (yytext[4] == '+') ?true :false;
            return TOK_OPT_DUMP;
            break;
        }
    }
    ```
    ```cpp
    Program:
        DumpSwitch ProgramName SEMICOLON    <---- MODIFY
        /* ProgramBody */
        DeclarationList FunctionList CompoundStatement
        /* End of ProgramBody */
        END {...}
    ;

    DumpSwitch:    <---- ADD GRAMMAR
    OPT_DUMP {
        opt_dump = $1;
    }
    |
    Epsilon

    int main(int argc, const char *argv[]) {
        ...
        SemanticAnalyzer sema_analyzer;
        sema_analyzer.setDumpOpt(opt_dump);    <---- TURN ON SWITCH
        ...
    }
    ```
6. `BinaryOperatorNode` problem: It need to get types of both side.
    - Solution: Add `virtual string getType()` to `ExpressionNode` and `override` in their own version.

The above is just a few problems I got, there are still a lot more that I met with (like x2 the last project).

## What is the hardest you think in this project

The __DEBUG HELL__ again, the total debug time maybe took 2/3 of the project time.

`ForNode`, `BinaryOperatorNode` and `VariableReferenceNode` are nodes I took more time to handle with.

Basically, the whole time modifying [SemanticAnalyzer.cpp](/src/lib/sema/SemanticAnalyzer.cpp) made me ill both physically and mentally.

## Feedback to T.A.s

I entered a state of high tension and began to experience heart palpitations, dandruff, and even sometimes forgot to eat and drink. I hope next time the homework won't be so scary and difficult :skull::skull::skull:
