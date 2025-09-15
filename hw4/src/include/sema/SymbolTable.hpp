#ifndef SEMA_STMBOL_TABLE_H
#define SEMA_STMBOL_TABLE_H

#include "AST/PType.hpp"
#include <bits/stdc++.h>

using namespace std;

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

class SymbolTable {
public:
    ~SymbolTable() = default;
    SymbolTable() = default;
    SymbolTable(vector<SymbolEntry> entries): entries(entries){}
    void addSymbol(SymbolEntry entry/* attributes needed by a SymbolEntry */);
    bool uniqueEntry(SymbolEntry &newEntry);
    bool nameInTable(string name);
    bool varRefCheck(string name);
    SymbolEntry findEntry(string name);
    vector<SymbolEntry> getEntries() { return entries; }
    void setErr() {entries.back().err = true; }
    // other methods
private:
    // keep the order by simply using vector
    vector<SymbolEntry> entries;
};

class SymbolManager {
public:
    ~SymbolManager() = default;
    SymbolManager() = default;

    bool dumpOpt = true;
    bool hasErr = false;

    void dumpSymbol(SymbolTable* table);
    void pushScope(SymbolTable *new_scope);
    void popScope();
    int getLevel() { return level; };
    SymbolTable *top() { return (tables.size() > 0) ?tables.top() :nullptr; }
    void push(SymbolTable *table) { tables.push(table); }
    void pop() { tables.pop(); }
    // other methods
private:
    int level = -1;
    stack<SymbolTable *> tables;
};

class ContextManager {
public:
    ~ContextManager() = default;
    ContextManager() = default;
    string name;
    string type;
    string funcType = "void";
    SymbolTable* global = nullptr;
    SymbolTable* forTable = nullptr;
    string forVarType = "";
    vector<string> loop_vars;
    int funcDepth = 0;
    int binaryDepth = 0;
    bool isParam = false;
    bool inVarRef = false;
    bool inFunc = false;
    bool inFor = false;
    bool inForInit = false;
    bool inBinary = false;
    bool inBinaryErr = false;
    bool inFuncInvoUnary = false;
    bool inPrint = false;
    bool inRead = false;
    bool inAssign = false;
    bool inReturn = false;
    bool printVarRefOverScript = false;
    int head = INT_MAX;
    int back = INT_MIN;
};

#endif