#include "sema/SymbolTable.hpp"

bool SymbolTable::uniqueEntry(SymbolEntry &newEntry){
    return find_if(entries.begin(), entries.end(), [&newEntry](SymbolEntry &entry){ return entry.getName() == newEntry.getName(); }) == entries.end();
}

bool SymbolTable::nameInTable(string name){
    // dumpSymbol(this);
    // cout << "in nameInTable" << endl;
    // for(auto entry : entries){
    //     // cout << "entry = " << entry.getName() << ", name = " << name << endl;
    //     if(entry.getName() == name){
    //         return true;
    //     }
    // }
    // return false;
    return !(find_if(entries.begin(), entries.end(), [name](SymbolEntry &entry){ return entry.getName() == name; }) == entries.end());
}

bool SymbolTable::varRefCheck(string name){
    // cout << "in varRef" << endl;
    auto it = find_if(entries.begin(), entries.end(), [name](SymbolEntry &entry){ return entry.getName() == name; });
    string kind = it->getKind();
    return kind == "parameter" || kind == "variable" || kind == "loop_var" || kind == "constant";
}

SymbolEntry SymbolTable::findEntry(string name){
    auto it = find_if(entries.begin(), entries.end(), [name](SymbolEntry &entry){ return entry.getName() == name; });
    if(it == entries.end()) return SymbolEntry("", "", -1, "", "");
    return *it;
}

void SymbolTable::addSymbol(SymbolEntry entry){
    entries.push_back(entry);
}

void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

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

void SymbolManager::pushScope(SymbolTable *new_scope){
    tables.push(new_scope);
    level++;
    // cout << level << endl;
}

void SymbolManager::popScope(){
    SymbolTable* table = tables.top();
    if(dumpOpt) dumpSymbol(table);
    tables.pop();
    level--;
    // cout << level << endl;
}