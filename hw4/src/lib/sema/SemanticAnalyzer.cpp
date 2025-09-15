#include "sema/Error.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"
using namespace std;

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
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

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    if(c_manager.inForInit) return;
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    bool redeclErr = false;
    bool isConstant = false;
    // 1
    // in a scope
    // 2
    SymbolTable *currtable = s_manager.top();

    if(p_variable.getConstant() == nullptr){
        bool loop = false;
        for(auto loop_var : c_manager.loop_vars) if(loop_var == p_variable.getName()) loop = true;
        SymbolEntry entry = SymbolEntry(p_variable.getName(), (c_manager.isParam) ?"parameter" :"variable", s_manager.getLevel(), p_variable.getPType(), "");
        if(currtable->uniqueEntry(entry) && !loop){
            currtable->addSymbol(entry);
        }else{
            s_manager.hasErr = true;
            redeclErr = true;
            m_error_printer.print(SymbolRedeclarationError(p_variable.getLocation(), p_variable.getNameCString()));
        }
    }else{
        isConstant = true;
        c_manager.name = p_variable.getName();
        c_manager.type = p_variable.getPType();
    }
    
    // 3
    p_variable.visitChildNodes(*this);
    // 4
    if(!redeclErr && !isConstant){
        vector<uint64_t> dimensions = p_variable.getDimensions();
        for(auto dimension : dimensions){
            if(dimension == 0){
                s_manager.hasErr = true;
                currtable->setErr();
                m_error_printer.print(NonPositiveArrayDimensionError(p_variable.getLocation(), p_variable.getNameCString()));
                p_variable.type = "error";
            }
        }
    }
    p_variable.type = p_variable.getPType();
    // 5
    // in a scope
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    if(c_manager.inForInit){
        if(c_manager.head == INT_MAX){
            c_manager.head = stoi(p_constant_value.getConstantValueCString());
        }else{
            c_manager.back = stoi(p_constant_value.getConstantValueCString());
            c_manager.inForInit = false;
        }
        return;
    }
    if(c_manager.inBinary){
        return;
    }
    if(c_manager.inFuncInvoUnary){
        return;
    }
    if(c_manager.inAssign){
        return;
    }
    if(c_manager.inReturn){
        return;
    }
    if(c_manager.inVarRef){
        return;
    }
    // 1
    // in a scope
    // 2
    SymbolTable *currtable = s_manager.top();
    SymbolEntry entry = SymbolEntry(c_manager.name, "constant", s_manager.getLevel(), c_manager.type, p_constant_value.getConstantValueCString());
    if(currtable->uniqueEntry(entry)){
        currtable->addSymbol(entry);
    }else{
        s_manager.hasErr = true;
        m_error_printer.print(SymbolRedeclarationError(p_constant_value.getLocation(), c_manager.name));
        return;
    }
    p_constant_value.type = p_constant_value.getType();
    // 3
    // no child
    // 4
    // no semantic
    // 5
    // in a scope
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    SymbolTable *prevtable = s_manager.top();
    // 1
    SymbolTable *currtable = new SymbolTable;
    s_manager.pushScope(currtable);
    // 2
    SymbolEntry entry = SymbolEntry(p_function.getName(), "function", s_manager.getLevel()-1, p_function.getType(), p_function.getParams());
    if(prevtable->uniqueEntry(entry)){
        prevtable->addSymbol(entry);
        c_manager.funcType = p_function.getType();
    }else{
        s_manager.hasErr = true;
        m_error_printer.print(SymbolRedeclarationError(p_function.getLocation(), p_function.getNameCString()));
    }
    // 3
    c_manager.isParam = true;
    c_manager.inFunc = true;
    c_manager.funcDepth++;
    p_function.visitChildNodes(*this);
    // 4
    // none
    // 5
    c_manager.funcDepth--;
    c_manager.inFunc = false;
    c_manager.funcType = "void";
    s_manager.popScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    c_manager.isParam = false;
    // 1
    SymbolTable *currtable;

    if(c_manager.inFunc){
        if(c_manager.funcDepth == 1){
            // cout << "inFUNCTION" << endl;
            // cout << "compound = " << c_manager.funcDepth << endl;
            currtable = s_manager.top();
        }else{
            // cout << "compound = " << c_manager.funcDepth << endl;
            currtable = new SymbolTable;
            s_manager.pushScope(currtable);
        }
        // 2
        // no symbol
        // 3
        c_manager.funcDepth++;
        p_compound_statement.visitChildNodes(*this);
        c_manager.funcDepth--;
        // 4
        // none
        // 5
        if(c_manager.funcDepth > 1){
            // cout << "compound_leave = " << c_manager.funcDepth << endl;
            s_manager.popScope();
        }
        // cout << "compound_leave = " << c_manager.funcDepth << endl;
    }else{
        currtable = new SymbolTable;
        s_manager.pushScope(currtable);
        p_compound_statement.visitChildNodes(*this);
        s_manager.popScope();
    }

    
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    c_manager.printVarRefOverScript = false;
    c_manager.inPrint = true;
    p_print.visitChildNodes(*this);
    c_manager.inPrint = false;
    std::unique_ptr<ExpressionNode> &exprNode = p_print.getExprNode();
    string type = exprNode->getType();
    // cout << "==========================" << endl;
    // cout << "name = " << exprNode->getName() << endl;
    // cout << "type = " << type << endl;
    if(c_manager.printVarRefOverScript || type == "error"){
        c_manager.printVarRefOverScript = false;
        return;
    }
    
    // cout << "printType = " << type << endl;
    if(!(type == "integer" || type == "string" || type == "boolean" || type == "real")){
        s_manager.hasErr = true;
        m_error_printer.print(PrintOutNonScalarTypeError(exprNode->getLocation()));
    }

}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // if(c_manager.inBinary && c_manager.inBinaryErr){
    //     return;
    // }
    if(c_manager.binaryDepth == 0){
        c_manager.inBinary = false;
        c_manager.inBinaryErr = false;
    }
    if(c_manager.inFuncInvoUnary){
        return;
    }
    // 1
    // in a scope
    // 2
    SymbolTable *currtable = s_manager.top();
    
    // 3
    // p_bin_op.visitChildNodes(*this);
    // 4
    string op = p_bin_op.getOpString();
    std::unique_ptr<ExpressionNode> &lhs = p_bin_op.getLhs(); 
    std::unique_ptr<ExpressionNode> &rhs = p_bin_op.getRhs();
    string lhsExprType = p_bin_op.getLhsExprType();
    string rhsExprType = p_bin_op.getRhsExprType();
    string lhsCstr, rhsCstr, lhsType, rhsType;
    int lhsVarRefCnt = 0, rhsVarRefCnt = 0;

    // find lhs and rhs type
    // expr node check
    if(lhsExprType == "constant"){
        lhsCstr = p_bin_op.getLhsType()->getPTypeCString();
    }else if(lhsExprType == "func"){
        string name = lhs->getName();
        if((lhsCstr = currtable->findEntry(name).getType()) == ""){
            lhsCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(lhsExprType == "varRef"){
        string name = lhs->getName();
        lhsVarRefCnt = lhs->varRef_getRefCnt();
        if((lhsCstr = currtable->findEntry(name).getType()) == ""){
            lhsCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(lhsExprType == "binary"){
        c_manager.binaryDepth++;
        c_manager.inBinary = true;
        p_bin_op.visitChildNodes(*this);
        c_manager.binaryDepth--;
        lhsType = lhs->binary_getType();
        if(c_manager.inBinary && c_manager.inBinaryErr){
            p_bin_op.type = "error";
            return;
        }
    }else if(lhsExprType == "unary"){

    }

    if(rhsExprType == "constant"){
        rhsCstr = p_bin_op.getRhsType()->getPTypeCString();
    }else if(rhsExprType == "func"){
        string name = rhs->getName();
        if((rhsCstr = currtable->findEntry(name).getType()) == ""){
            rhsCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(rhsExprType == "varRef"){
        string name = rhs->getName();
        rhsVarRefCnt = rhs->varRef_getRefCnt();
        if((rhsCstr = currtable->findEntry(name).getType()) == ""){
            rhsCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(rhsExprType == "binary"){
        rhsType = rhs->binary_getType();
        if(rhsType == ""){
            c_manager.binaryDepth++;
            c_manager.inBinary = true;
            p_bin_op.visitChildNodes(*this);
            c_manager.binaryDepth--;
            rhsType = rhs->binary_getType();
        }
    }else if(rhsExprType == "unary"){

    }

    if(lhsType == "error" || rhsType == "error"){
        p_bin_op.type = "error";
        return;
    }

    // pure type
    for(int i = 0 ; i < lhsCstr.length() ; i++){
        if(lhsCstr[i] == ' '){
            lhsType = lhsCstr.substr(0, i);
            break;
        }
        if(i == lhsCstr.length()-1){
            lhsType = lhsCstr.substr(0, i+1);
        }
    }
    for(auto c : lhsCstr) if(c == ']') lhsVarRefCnt--;
    lhsVarRefCnt *= -1;
    if(lhsVarRefCnt > 0){
        lhsType += " ";
        for(int i = 0 ; i < lhsVarRefCnt ; i++){
            lhsType += "[1]";
        }
    }

    for(int i = 0 ; i < rhsCstr.length() ; i++){
        if(rhsCstr[i] == ' '){
            rhsType = rhsCstr.substr(0, i);
            break;
        }
        if(i == rhsCstr.length()-1){
            rhsType = rhsCstr.substr(0, i+1);
        }
    }
    for(auto c : rhsCstr) if(c == ']') rhsVarRefCnt--;
    rhsVarRefCnt *= -1;
    if(rhsVarRefCnt > 0){
        rhsType += " ";
        for(int i = 0 ; i < rhsVarRefCnt ; i++){
            rhsType += "[1]";
        }
    }

    if(lhsType == "") lhsType = "integer";
    if(rhsType == "") rhsType = "integer";

    // Semantic Check
    if(op == "+"){
        if((lhsType == "integer" || lhsType == "real") && (rhsType == "integer" || rhsType == "real")){
            if(lhsType == "real" || rhsType == "real"){
                p_bin_op.type = "real";
            }else{
                p_bin_op.type = "integer";
            }
        }else if(lhsType == "string" && rhsType == "string"){
            p_bin_op.type = "string";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOp(), lhsType, rhsType));
            c_manager.inBinaryErr = true;
            p_bin_op.type = "error";
            // cout << "p_bin_op.type = " << p_bin_op.type << endl;
        }
    }else if(op == "-" || op == "*" || op == "/"){
        if((lhsType == "integer" || lhsType == "real") && (rhsType == "integer" || rhsType == "real")){
            if(lhsType == "real" || rhsType == "real"){
                p_bin_op.type = "real";
            }else{
                p_bin_op.type = "integer";
            }
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOp(), lhsType, rhsType));
            c_manager.inBinaryErr = true;
            p_bin_op.type = "error";
        }
    }else if(op == "mod"){
        if(lhsType == "integer" && rhsType == "integer"){
            p_bin_op.type = "integer";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOp(), lhsType, rhsType));
            c_manager.inBinaryErr = true;
            p_bin_op.type = "error";
        }
    }else if(op == "and" || op == "or" || op == "not"){
        if(lhsType == "boolean" && rhsType == "boolean"){
            p_bin_op.type = "boolean";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOp(), lhsType, rhsType));
            c_manager.inBinaryErr = true;
            p_bin_op.type = "error";
        }
    }else if(op == "<" || op == "<=" || op == "=" || op == ">=" || op == ">" || op == "<>"){
        if((lhsType == "integer" || lhsType == "real") && (rhsType == "integer" || rhsType == "real")){
            p_bin_op.type = "boolean";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOp(), lhsType, rhsType));
            c_manager.inBinaryErr = true;
            p_bin_op.type = "error";
        }
    }

    // cout << "------------------------------------------------------" << endl;
    // cout << lhs->getName() << " " << rhs->getName() << endl;
    // cout << lhsExprType << " " << op << " " << rhsExprType << endl;
    // cout << lhsCstr << " " << op << " " << rhsCstr << endl;
    // cout << lhsType << " " << op << " " << rhsType << endl;
    if(c_manager.binaryDepth == 0){
        // cout << "------------------------------------------------------" << endl;
        // cout << lhsExprType << " " << op << " " << rhsExprType << endl;
        // cout << lhsCstr << " " << op << " " << rhsCstr << endl;
        // cout << lhsType << " " << op << " " << rhsType << endl;
        // if(c_manager.inBinaryErr) p_bin_op.type = "error";
        c_manager.inBinary = false;
        c_manager.inBinaryErr = false;
    }
    // 5
    // in a scope
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    if(c_manager.binaryDepth == 0){
        c_manager.inBinary = false;
        c_manager.inBinaryErr = false;
    }
    // 1
    // in a scope
    // 2
    SymbolTable *currtable = s_manager.top();
    
    // 3
    // p_bin_op.visitChildNodes(*this);
    // 4
    string op = p_un_op.getOpString();
    std::unique_ptr<ExpressionNode> &lhs = p_un_op.getOperand();
    string ExprType = p_un_op.getOperandExprType();
    string operandCstr, operandType;
    int operandVarRefCnt = 0;

    // find lhs and rhs type
    // expr node check
    if(ExprType == "constant"){
        operandCstr = p_un_op.getPType()->getPTypeCString();
        // cout << "pick constant" << endl;
        // cout << operandCstr << endl;
    }else if(ExprType == "func"){
        string name = lhs->getName();
        if((operandCstr = currtable->findEntry(name).getType()) == ""){
            operandCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(ExprType == "varRef"){
        string name = lhs->getName();
        operandVarRefCnt = lhs->varRef_getRefCnt();
        if((operandCstr = currtable->findEntry(name).getType()) == ""){
            operandCstr = c_manager.global->findEntry(name).getType();
        }
    }else if(ExprType == "binary"){
        c_manager.binaryDepth++;
        c_manager.inBinary = true;
        p_un_op.visitChildNodes(*this);
        c_manager.binaryDepth--;
        operandType = lhs->binary_getType();
        operandCstr = operandType;
        if(c_manager.inBinary && c_manager.inBinaryErr){
            return;
        }
    }else if(ExprType == "unary"){

    }

    if(operandType == "error"){
        p_un_op.type = "error";
        return;
    }

    // pure type
    for(int i = 0 ; i < operandCstr.length() ; i++){
        if(operandCstr[i] == ' '){
            operandType = operandCstr.substr(0, i);
            break;
        }
        if(i == operandCstr.length()-1){
            operandType = operandCstr.substr(0, i+1);
        }
    }
    for(auto c : operandCstr) if(c == ']') operandVarRefCnt--;
    operandVarRefCnt *= -1;
    if(operandVarRefCnt > 0){
        operandType += " ";
        for(int i = 0 ; i < operandVarRefCnt ; i++){
            operandType += "[1]";
        }
    }
    if(operandVarRefCnt < 0){
        m_error_printer.print(OverArraySubscriptError(lhs->getLocation(), lhs->getName()));
    }

    // cout << "operator = " << op << endl;
    // cout << "operandType = " << operandType << endl;
    // cout << "operand = " << lhs->getName() << endl;

    // Semantic Check
    if(op == "neg"){
        if(operandType == "integer"){
            p_un_op.type = "integer";
        }else if(operandType == "real"){
            p_un_op.type = "real";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.getOp(), operandCstr));
            c_manager.inBinaryErr = true;
            p_un_op.type = "error";
        }
    }else if(op == "not"){
        if(operandType == "boolean"){
            p_un_op.type = "boolean";
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.getOp(), operandCstr));
            c_manager.inBinaryErr = true;
            p_un_op.type = "error";
        }
    }

    if(c_manager.binaryDepth == 0){
        c_manager.inBinary = false;
        c_manager.inBinaryErr = false;
    }
    // cout << "final type = " << p_un_op.type << endl;
    // cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$3" << endl;
    // 5
    // in a scope
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    if(c_manager.inBinary){
        return;
    }
    if(c_manager.inFuncInvoUnary){
        return;
    }
    // 1
    // in a scope
    // 2
    SymbolTable *currtable = s_manager.top();
    // 3
    // p_func_invocation.visitChildNodes(*this);
    // 4
    // step 1
    string name = p_func_invocation.getName();
    bool local = currtable->nameInTable(name);
    if(!currtable->nameInTable(name) && !c_manager.global->nameInTable(name)){
        s_manager.hasErr = true;
        m_error_printer.print(UndeclaredSymbolError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
        p_func_invocation.type = "error";
        return;
    }
    // step 2
    SymbolEntry entry = ((local)?currtable :c_manager.global)->findEntry(name);
    if(entry.getKind() != "function"){
        s_manager.hasErr = true;
        m_error_printer.print(NonFunctionSymbolError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
        p_func_invocation.type = "error";
        return;
    }
    // step 3
    string args_str = entry.getAttribute();
    vector<string> args;
    string substr = "";
    for(int i = 0 ; i < args_str.length() ; i++){
        if(args_str[i] == ' '){
            continue;
        }
        if(args_str[i] == ','){
            args.push_back(substr);
            substr.clear();
        }else{
            if(args_str[i] == '[') substr.push_back(' ');
            substr.push_back(args_str[i]);
            if(i == args_str.length()-1) args.push_back(substr);
        }
    }
    if(args.size() != p_func_invocation.getParamsSize()){
        // cout << args << " " << p_func_invocation.getParamsSize() << endl;
        s_manager.hasErr = true;
        m_error_printer.print(ArgumentNumberMismatchError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
        p_func_invocation.type = "error";
        return;
    }
    // step 4
    std::vector<std::unique_ptr<ExpressionNode>> *params = p_func_invocation.getExprNodes();
    for(int i = 0 ; i < args.size() ; i++){
        // cout << i << endl;
        string name = params->at(i)->getName();
        string type = params->at(i)->getType();
        if(params->at(i)->getExprType() == "unary"){
            c_manager.inFuncInvoUnary = true;
            p_func_invocation.visitChildNodes(*this);
            c_manager.inFuncInvoUnary = false;
            type = params->at(i)->unary_getType();
        }
        if(type == ""){
            if((type = currtable->findEntry(name).getType()) == ""){
                type = c_manager.global->findEntry(name).getType();
            }
        }
        
        if(args[i] != type){
            if(type == "error") return;
            if(args[i] == "real" && type == "integer"){
                continue;
            }
            s_manager.hasErr = true;
            m_error_printer.print(IncompatibleArgumentTypeError(params->at(i)->getLocation(), args[i], type));
            p_func_invocation.type = "error";
            return;
        }
    }

    // 5
    // in a scope
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    bool needOuter = false;
    SymbolTable *currtable = s_manager.top();
    SymbolTable *prevtable = nullptr;
    if(c_manager.inForInit){
        for(auto loop_var : c_manager.loop_vars){
            if(p_variable_ref.getName() == loop_var){
                s_manager.hasErr = true;
                c_manager.printVarRefOverScript = true;
                m_error_printer.print(SymbolRedeclarationError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
                p_variable_ref.type = "error";
                return;
            }
        }
        // s_manager.pop();
        // if(s_manager.top() != nullptr){
        //     prevtable = s_manager.top();
        //     string type;
        //     if((type = prevtable->findEntry(p_variable_ref.getName()).getType()) != ""){
        //         s_manager.push(currtable);
        //         p_variable_ref.type = type;
        //         return;
        //     }
        // }
        // s_manager.push(currtable);

        c_manager.loop_vars.push_back(p_variable_ref.getName());
    }
    // if(c_manager.inBinary){
    //     return;
    // }
    if(c_manager.inFuncInvoUnary){
        return;
    }
    // 1
    // in a scope
    // 2
   
    // 3
    c_manager.inVarRef = true;
    p_variable_ref.visitChildNodes(*this);
    c_manager.inVarRef = false;
    // 4
    // read
    if(c_manager.inFor && c_manager.inRead){
        s_manager.pop();
        prevtable = s_manager.top();
        p_variable_ref.type = prevtable->findEntry(p_variable_ref.getName()).getType();
            if(prevtable->findEntry(p_variable_ref.getName()).getKind() == "loop_var"){
                s_manager.hasErr = true;
                c_manager.printVarRefOverScript = true;
                m_error_printer.print(ReadToConstantOrLoopVarError(p_variable_ref.getLocation()));
                s_manager.push(currtable);
                p_variable_ref.type = "error";
                return;
            }
        s_manager.push(currtable);
    }

    // assign
    if(c_manager.inFor && c_manager.inAssign){
        s_manager.pop();
        prevtable = s_manager.top();
        p_variable_ref.type = prevtable->findEntry(p_variable_ref.getName()).getType();
            if(prevtable->findEntry(p_variable_ref.getName()).getKind() == "loop_var"){
                s_manager.hasErr = true;
                c_manager.printVarRefOverScript = true;
                m_error_printer.print(AssignToLoopVarError(p_variable_ref.getLocation()));
                s_manager.push(currtable);
                return;
            }
        s_manager.push(currtable);
    }

    if(c_manager.inFor && !c_manager.inForInit){
        for(auto loop_var : c_manager.loop_vars){
            if(p_variable_ref.getName() == loop_var){
                p_variable_ref.type = "integer";
                return;
            }
        }
    }

    string name = p_variable_ref.getName();
    // cout << "---------------------------" << endl;
    // cout << "step 1" << endl;
    // cout << name << endl;
    if(!currtable->nameInTable(name)){
        if(c_manager.inFunc){
            // cout << c_manager.funcname << " " << p_variable_ref.getName() << endl;
            s_manager.pop();
            prevtable = s_manager.top();
            
            if(!prevtable->nameInTable(p_variable_ref.getName())){
                s_manager.hasErr = true;
                c_manager.printVarRefOverScript = true;
                m_error_printer.print(UndeclaredSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
                s_manager.push(currtable);
                p_variable_ref.type = "error";
                return;
            }
            needOuter = true;
        }else{
            s_manager.hasErr = true;
            c_manager.printVarRefOverScript = true;
            m_error_printer.print(UndeclaredSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
            p_variable_ref.type = "error";
            return;
        }
    }

    // cout << "step 2" << endl;
    if(!((needOuter)?prevtable:currtable)->varRefCheck(name)){
        s_manager.hasErr = true;
        c_manager.printVarRefOverScript = true;
        m_error_printer.print(NonVariableSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
        p_variable_ref.type = "error";
        if(needOuter) s_manager.push(currtable);
        return;
    }

    // cout << "step 3" << endl;
    if(((needOuter)?prevtable:currtable)->findEntry(name).err){
        if(needOuter) s_manager.push(currtable);
        p_variable_ref.type = "error";
        return;
    }
    for(int i = 0 ; i < p_variable_ref.m_indices.size() ; i++){
        // cout << "type = " << p_variable_ref.m_indices[i]->getType() << endl;
        if(p_variable_ref.m_indices[i]->getType() != "integer" && p_variable_ref.m_indices[i]->getType() != "error"){
            s_manager.hasErr = true;
            c_manager.printVarRefOverScript = true;
            m_error_printer.print(NonIntegerArrayIndexError(p_variable_ref.m_indices[i]->getLocation()));
            p_variable_ref.type = "error";
            if(needOuter){
                s_manager.push(currtable);
                needOuter = false;
            }
            return;
        }
    }

    // cout << "step 4" << endl;
    string typestr = ((needOuter)?prevtable:currtable)->findEntry(name).getType();
    int cnt = 0;
    for(auto c : typestr) if(c == ']') cnt++;
    if(p_variable_ref.m_indices.size() > cnt){
        s_manager.hasErr = true;
        c_manager.printVarRefOverScript = true;
        m_error_printer.print(OverArraySubscriptError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
        p_variable_ref.type = "error";
        if(needOuter){
            s_manager.push(currtable);
            needOuter = false;
            return;
        }
        return;
    }

    int refCnt = p_variable_ref.varRef_getRefCnt();
    string refType;
    // if(refCnt == 0){
    //     refType = typestr;
    // }else{
        int brackets = 0;
        for(auto c : typestr) if(c == '[') brackets++;
        for(int i = 0 ; i < typestr.length() ; i++){
            if(typestr[i] == ' '){
                refType = typestr.substr(0, i);
                break;
            }
        }
        if(refType == ""){
            refType = typestr;
        }else{
            refCnt = brackets - refCnt;
            if(refCnt > 0) refType += " ";
            int back = typestr.length()-1;
            for(; back >= 0 ; back--){
                if(refCnt > 0){
                    if(typestr[back] == '['){
                        refCnt--;
                        if(refCnt == 0) break;
                    }
                }else{
                    break;
                }
            }
            if(back != typestr.length()-1) refType += typestr.substr(back);
        }
        

        // for(; refCnt > 0 ; refCnt--){
        //     refType += "[1]";
        // }
    // }
    
    p_variable_ref.type = refType;
    if(needOuter) s_manager.push(currtable);
    // cout << "========================" << endl;
    // cout << "name = " << p_variable_ref.getName() << endl;
    // cout << "typestr = " << typestr << endl;
    // cout << "refType = " << refType << endl;
    
    // 5
    // in a scope
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    
    SymbolTable *currtable = s_manager.top();

    c_manager.printVarRefOverScript = false;
    c_manager.inAssign = true;
    p_assignment.visitChildNodes(*this);
    c_manager.inAssign = false;

    std::unique_ptr<VariableReferenceNode> &varRefNode = p_assignment.getVarRefNode();
    std::unique_ptr<ExpressionNode> &exprNode = p_assignment.getExprNode();
    string lhsType = varRefNode->getType(), rhsType = exprNode->getType();
    
    if(c_manager.printVarRefOverScript){
        c_manager.printVarRefOverScript = false;
        return;
    }

    string kind = currtable->findEntry(varRefNode->getName()).getKind();
    // cout << "---------------------------------------" << endl;
    // cout << varRefNode->getName() << endl;
    // cout << "kind = " << kind << endl;
    if(!c_manager.inForInit && (kind == "loop_var" || kind == "constant" || kind == "")){
        m_error_printer.print(AssignToConstantError(varRefNode->getLocation(), varRefNode->getName()));
        return;
    }
    
    // cout << "lhsType = " << lhsType << endl;
    // cout << "rhsType = " << rhsType << endl;
    bool lhsIsArr = false;
    for(auto c : lhsType){
        if(c == '['){
            lhsIsArr = true;
            break;
        }
    }

    bool rhsIsArr = false;
    for(auto c : rhsType){
        if(c == '['){
            rhsIsArr = true;
            break;
        }
    }
    
    if(lhsIsArr){
        m_error_printer.print(AssignWithArrayTypeError(varRefNode->getLocation()));
        return;
    }
    if(rhsIsArr){
        m_error_printer.print(AssignWithArrayTypeError(exprNode->getLocation()));
        return;
    }
    if(lhsType != rhsType){
        if((lhsType == "real" && rhsType == "integer") || rhsType == "error"){

        }else{
            m_error_printer.print(IncompatibleAssignmentError(p_assignment.getLocation(), lhsType, rhsType));
            return;
        }
    }
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    c_manager.inRead = true;
    SymbolTable *currtable = s_manager.top();

    c_manager.printVarRefOverScript = false;
    p_read.visitChildNodes(*this);
    std::unique_ptr<VariableReferenceNode> &varRefNode = p_read.getVarRefNode();
    string type = varRefNode->getType();
    if(c_manager.printVarRefOverScript){
        c_manager.printVarRefOverScript = false;
        c_manager.inRead = false;
        return;
    }

    string kind = currtable->findEntry(varRefNode->getName()).getKind();
    // cout << "kind = " << kind << endl;
    if(kind == "loop_var" || kind == "constant" || kind == ""){
        s_manager.hasErr = true;
        m_error_printer.print(ReadToConstantOrLoopVarError(varRefNode->getLocation()));
        c_manager.inRead = false;
        return;
    }
    
    // cout << "printType = " << type << endl;
    if(!(type == "integer" || type == "string" || type == "boolean" || type == "real")){
        s_manager.hasErr = true;
        m_error_printer.print(ReadToNonScalarTypeError(varRefNode->getLocation()));
        c_manager.inRead = false;
        return;
    }
    c_manager.inRead = false;
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_if.visitChildNodes(*this);
    std::unique_ptr<ExpressionNode> &exprNode = p_if.getExprNode();
    string type = exprNode->getType();
    if(type != "boolean"){
        // cout << "wrong type input = " << type << endl;
        if(type == "error"){
            return;
        }
        s_manager.hasErr = true;
        m_error_printer.print(NonBooleanConditionError(exprNode->getLocation()));
        return;
    }
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_while.visitChildNodes(*this);
    std::unique_ptr<ExpressionNode> &exprNode = p_while.getExprNode();
    string type = exprNode->getType();
    if(type != "boolean"){
        // cout << "wrong type input = " << type << endl;
        if(type == "error"){
            return;
        }
        s_manager.hasErr = true;
        m_error_printer.print(NonBooleanConditionError(exprNode->getLocation()));
        return;
    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    c_manager.inFor = true;
    c_manager.inForInit = true;
    c_manager.head = INT_MAX;
    c_manager.back = INT_MIN;
    // 1
    SymbolTable *prevtable = s_manager.top();
    SymbolTable *currtable = new SymbolTable;
    s_manager.pushScope(currtable);
    // 2
    bool hasLoopVar = false;
    for(auto loop_var : c_manager.loop_vars){
        if(loop_var == p_for.getLoopVar()){
            hasLoopVar = true;
        }
    }
    // if(prevtable->findEntry(p_for.getLoopVar()).getType() != "") hasLoopVar = true;
    if(!hasLoopVar){
        // string type;
        // if((type = prevtable->findEntry(p_for.getLoopVar()).getType()) == "") type = "integer";
        SymbolEntry entry = SymbolEntry(p_for.getLoopVar(), "loop_var", s_manager.getLevel(), "integer", "");
        if(currtable->uniqueEntry(entry)){
            currtable->addSymbol(entry);
        }else{
            s_manager.hasErr = true;
            m_error_printer.print(SymbolRedeclarationError(p_for.getLocation(), p_for.getLoopVar().c_str()));
        }
    }
    
    // 3
    p_for.visitChildNodes(*this);
    // cout << "==============================" << endl;
    // cout << "loop_vars: ";
    // for(auto loop_var : c_manager.loop_vars){
    //     cout << loop_var << " ";
    // }
    // cout << endl;
    // cout << "------------------------------" << endl;
    // cout << "currtable: ";
    // vector<SymbolEntry> entries = currtable->getEntries();
    // for(auto entry : entries){
    //     cout << entry.getName() << " ";
    // }
    // cout << endl;
    if(!c_manager.loop_vars.empty() && !hasLoopVar) c_manager.loop_vars.pop_back();
    // 4
    if(c_manager.head > c_manager.back){
        // cout << c_manager.head << " " << c_manager.back << endl;
        s_manager.hasErr = true;
        m_error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
    }
    // 5
    s_manager.popScope();
    c_manager.inFor = false;
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    c_manager.inReturn = true;
    SymbolTable *currtable = s_manager.top();

    p_return.visitChildNodes(*this);
    std::unique_ptr<ExpressionNode> &exprNode = p_return.getExprNode();
    string type = exprNode->getType();

    if(c_manager.inFunc){
        if(type == ""){
            c_manager.inReturn = false;
            return;
        }
        if(type != c_manager.funcType){
            if((c_manager.funcType == "real" && type == "integer") || type == "error"){

            }else{
                s_manager.hasErr = true;
                m_error_printer.print(IncompatibleReturnTypeError(exprNode->getLocation(), c_manager.funcType, type));
                c_manager.inReturn = false;
            }
        }
    }else if(type != "error"){
        s_manager.hasErr = true;
        m_error_printer.print(ReturnFromVoidError(p_return.getLocation()));
        c_manager.inReturn = false;
    }
    
    c_manager.inReturn = false;
}
