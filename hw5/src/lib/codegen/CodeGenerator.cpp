#include "AST/CompoundStatement.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/program.hpp"
#include "codegen/CodeGenerator.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
using namespace std;

CodeGenerator::CodeGenerator(const std::string &source_file_name,
                             const std::string &save_path,
                             std::unordered_map<SemanticAnalyzer::AstNodeAddr,
                                                      SymbolManager::Table>
                                 &&p_symbol_table_of_scoping_nodes)
    : m_symbol_manager(false /* no dump */),
      m_source_file_path(source_file_name),
      m_symbol_table_of_scoping_nodes(std::move(p_symbol_table_of_scoping_nodes)) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path =
        save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind('/');
    auto dot_pos = source_file_name.rfind('.');

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";

    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the scope for looking up the symbol entry.
    // Hint: Use m_symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_program)));

    // Find string literals
    findString = true;
    p_program.visitChildNodes(*this);
    findString = false;

    // DeclNode
    dumpInstructions(m_output_file.get(), ".section    .rodata       # emit rodata section\n");
    for(int i = 0 ; i < str_literals.size() ; i++){
        string insturctions =
        "    .align	2\n"
        "S" + to_string(i) + ":\n"
        "    .string	\"" + str_literals[i] + "\"\n";
        dumpInstructions(m_output_file.get(), insturctions.c_str());
    }
    inProgramDecl = true;
    p_program.visitDeclNodes(*this);
    inProgramDecl = false;

    // FuncNode
    dumpInstructions(m_output_file.get(), ".section    .text\n");
    p_program.visitFuncNodes(*this);

    // CompoundStmtNode
    constexpr const char *const compound_in_program_prologue =
        ".section    .text\n"
        "    .align 2\n"
        "    .globl main          # emit symbol 'main' to the global symbol table\n"
        "    .type main, @function\n"
        "main:\n"
        "    # in the function prologue\n"
        "    addi sp, sp, -128    # move stack pointer to lower address to allocate a new stack\n"
        "    sw ra, 124(sp)       # save return address of the caller function in the current stack\n"
        "    sw s0, 120(sp)       # save frame pointer of the last stack in the current stack\n"
        "    addi s0, sp, 128     # move frame pointer to the bottom of the current stack\n";
    dumpInstructions(m_output_file.get(), compound_in_program_prologue);
    fp = -8;
    p_program.visitBody(*this);
    constexpr const char *const compound_in_program_epilogue =
        "    # pop all in stack\n"
        "popbegin:\n"
        "    mv t0, sp\n"
        "    addi t0, t0, 128\n"
        "    beq s0, t0, popend\n"
        "    addi sp, sp, 4\n"
        "    j popbegin\n"
        "popend:\n"
        "    # in the function epilogue\n"
        "    lw ra, 124(sp)       # load return address saved in the current stack\n"
        "    lw s0, 120(sp)       # move frame pointer back to the bottom of the last stack\n"
        "    addi sp, sp, 128     # move stack pointer back to the top of the last stack\n"
        "    jr ra                # jump back to the caller function\n"
        "    .size main, .-main\n";
    dumpInstructions(m_output_file.get(), compound_in_program_epilogue);

    m_symbol_manager.popScope();

    // test string literals
    // for(auto str : str_literals){
    //     printf("%s\n", str.c_str());
    // }
}

void CodeGenerator::visit(DeclNode &p_decl) {
    if(findString){
        for(auto &variable : p_decl.getVariables()){
            if(variable->getTypePtr()->isString()){
                if(variable->getConstantPtr() == nullptr) continue;
                string str = string(variable->getConstantPtr()->getConstantValueCString());
                if(find(str_literals.begin(), str_literals.end(), str) == str_literals.end()){
                    str_literals.push_back(str);
                }
            }
        }
        return;
    }

    string decl_instructions = "";
    if(inProgramDecl){
        for(auto &variable : p_decl.getVariables()){
            const SymbolEntry * entry;
            if((entry = m_symbol_manager.lookup(variable->getName())) != nullptr){
                if(entry->getKind() == SymbolEntry::KindEnum::kVariableKind){
                    decl_instructions += ".comm " + entry->getName() + ", 4, 4\n";
                }else{
                    decl_instructions +=
                    "    .align 2\n"
                    "    .globl " + entry->getName() + "\n"
                    "    .type " + entry->getName() + ", @object\n"
                    "" + entry->getName() + ":\n";

                    string str = string(entry->getAttribute().constant()->getConstantValueCString());
                    if(entry->getTypePtr()->isString()){
                        int str_idx = distance(str_literals.begin(), find(str_literals.begin(), str_literals.end(), str));
                        decl_instructions +=
                            "    .word S" + to_string(str_idx) + "\n";
                    }else{
                        decl_instructions +=
                            "    .word " + str + "\n";
                    }
                }
            }
        }
    }else{
        for(auto &variable : p_decl.getVariables()){
            const SymbolEntry * entry;
            if((entry = m_symbol_manager.lookup(variable->getName())) != nullptr){
                if(entry->getKind() == SymbolEntry::KindEnum::kVariableKind || entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind || entry->getKind() == SymbolEntry::KindEnum::kParameterKind){
                    if(entry->getTypePtr()->isScalar()){
                        fp += -4;
                    }else{
                        auto dims = entry->getTypePtr()->getDimensions();
                        int dim_size = 1;
                        for(auto dim : dims) dim_size *= (int)dim;
                        fp += -(dim_size*4);
                    }
                }else{
                    fp += -4;
                    decl_instructions +=
                        "    addi t0, s0, " + to_string(fp) + "\n";
                    string str = string(entry->getAttribute().constant()->getConstantValueCString());
                    if(entry->getTypePtr()->isString()){
                        int str_idx = distance(str_literals.begin(), find(str_literals.begin(), str_literals.end(), str));
                        decl_instructions +=
                            "    la t1, S" + to_string(str_idx) + "\n";
                    }else{
                        decl_instructions +=
                            "    li t1, " + str + "\n";
                    }
                    decl_instructions +=
                        "    sw t1, 0(t0) \n";
                }
            }
        }
    }
    dumpInstructions(m_output_file.get(), decl_instructions.c_str());
}

void CodeGenerator::visit(VariableNode &p_variable) {}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    if(findString){
        if(p_constant_value.getTypePtr()->isString()){
            string str = string(p_constant_value.getConstantValueCString());
            if(find(str_literals.begin(), str_literals.end(), str) == str_literals.end()){
                str_literals.push_back(str);
            }
        }
        return;
    }
    string instructions = "";
    if(p_constant_value.getTypePtr()->isString()){
        string str = string(p_constant_value.getConstantValueCString());
        int str_idx = distance(str_literals.begin(), find(str_literals.begin(), str_literals.end(), str));
        instructions +=
            "    la t0, S" + to_string(str_idx) + "\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the value to the stack\n";
        dumpInstructions(m_output_file.get(), instructions.c_str());
        printType = 1;
        return;
    }
    string value = string(p_constant_value.getConstantValueCString());
    if(value == "true" || value == "false"){
        value = (value == "true") ?"1" :"0";
    }

    instructions += 
        "    li t0, " + value + "\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
    printType = 0;
}

void CodeGenerator::visit(FunctionNode &p_function) {
    if(findString){
        p_function.visitChildNodes(*this);
        return;
    }
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_function)));
    
    string function_prologue =
        "    .align 2\n"
        "    .globl " + p_function.getName() + "\n"
        "    .type " + p_function.getName() + ", @function\n"
        "" + p_function.getName() + ":\n"
        "    addi sp, sp, -128\n"
        "    sw ra, 124(sp)\n"
        "    sw s0, 120(sp)\n"
        "    addi s0, sp, 128\n";
    string function_epilogue =
        "    lw ra, 124(sp)\n"
        "    lw s0, 120(sp)\n"
        "    addi sp, sp, 128\n"
        "    jr ra\n"
        "    .size " + p_function.getName() + ", .-" + p_function.getName() + "\n";

    dumpInstructions(m_output_file.get(), function_prologue.c_str());
    fp = -8;
    string instructions = "";
    int paramdist = 0;
    for (int i = 1 ; i <= p_function.getParametersNum(p_function.getParameters()) ; i++) {
        fp += -4;
        instructions +=
            "    sw s" + to_string(i) + ", " + to_string(fp) + "(s0)\n";
        paramdist += 4;
    }
    dumpInstructions(m_output_file.get(), instructions.c_str());
    allowReturn++;
    inFunc = true;
    p_function.visitBodyChildNodes(*this);
    inFunc = false;
    dumpInstructions(m_output_file.get(), function_epilogue.c_str());

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    if(findString){
        p_compound_statement.visitChildNodes(*this);
        return;
    }
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));

    p_compound_statement.visitChildNodes(*this);

    m_symbol_manager.popScope();
}

void CodeGenerator::visit(PrintNode &p_print) {
    if(findString){
        p_print.visitChildNodes(*this);
        return;
    }
    if(inFunc && allowReturn == 0) return;
    p_print.visitChildNodes(*this);
    string instructions =
        "    lw a0, 0(sp)     # pop the value from the stack to the first argument register 'a0'\n"
        "    addi sp, sp, 4\n";
    switch (printType)
    {
    case 0:
        instructions += "    jal ra, printInt # call function 'printInt'\n";
        break;
    case 1:
        instructions += "    jal ra, printString # call function 'printString'\n";
    default:
        break;
    }
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    if(findString){
        p_bin_op.visitChildNodes(*this);
        return;
    }
    p_bin_op.visitChildNodes(*this);
    string instructions = "";
    instructions +=
        "    lw t0, 0(sp)     # pop the val2 from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)     # pop the val1 from the stack\n"
        "    addi sp, sp, 4\n";
    string op = "";
    switch (p_bin_op.getOp()) {
        case Operator::kPlusOp:
            op = "    add t0, t1, t0\n";
            break;
        case Operator::kMinusOp:
            op = "    sub t0, t1, t0\n";
            break;
        case Operator::kMultiplyOp:
            op = "    mul t0, t1, t0\n";
            break;
        case Operator::kDivideOp:
            op = "    div t0, t1, t0\n";
            break;
        case Operator::kModOp:
            op = "    rem t0, t1, t0\n";
            break;
        case Operator::kAndOp:
            op = "    and t0, t1, t0\n";
            break;
        case Operator::kOrOp:
            op = "    or t0, t1, t0\n";
            break;
        case Operator::kLessOp:
            binCompCnt++;
            op =
                "    bge t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        case Operator::kLessOrEqualOp:
            binCompCnt++;
            op =
                "    bgt t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        case Operator::kEqualOp:
            binCompCnt++;
            op =
                "    bne t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        case Operator::kGreaterOp:
            binCompCnt++;
            op =
                "    ble t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        case Operator::kGreaterOrEqualOp:
            binCompCnt++;
            op =
                "    blt t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        case Operator::kNotEqualOp:
            binCompCnt++;
            op =
                "    beq t1, t0, bin_false_" + to_string(binCompCnt) + "\n"
                "    li t0, 1\n"
                "    j bin_true_" + to_string(binCompCnt) + "\n"
                "bin_false_" + to_string(binCompCnt) + ":\n"
                "    li t0, 0\n"
                "bin_true_" + to_string(binCompCnt) + ":\n";
            break;
        default:
            break;
    }
    instructions += op;
    instructions +=
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    if(findString){
        p_un_op.visitChildNodes(*this);
        return;
    }
    p_un_op.visitChildNodes(*this);
    string instructions = "";
    instructions +=
        "    lw t0, 0(sp)     # pop the val from the stack (unary)\n"
        "    addi sp, sp, 4\n";
    string op = "";
    switch (p_un_op.getOp()) {
    case Operator::kNegOp:
        op =
            "    li t1, -1\n"
            "    mul t0, t0, t1\n";
        break;
    case Operator::kNotOp:
        op =
            "    li t1, 1     #(not)\n"
            "    xor t0, t0, t1\n";
        break;
    default:
        break;
    }
    instructions += op +
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    if(findString){
        p_func_invocation.visitChildNodes(*this);
        return;
    }
    inFuncInvo = true;
    p_func_invocation.visitChildNodes(*this);
    inFuncInvo = false;
    int cnt = p_func_invocation.getArguments().size();
    string instructions = "";
    while(cnt > 0){
        instructions +=
            "    lw s" + to_string(cnt--) + ", 0(sp)     # load param value from stack\n"
            "    addi sp, sp, 4\n";
    }
    instructions +=
        "    jal ra, " + p_func_invocation.getName() + "\n"
        "    addi sp, sp, -4\n"
        "    sw a0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    if(findString){
        return;
    }
    string instructions = "";
    string var_name = p_variable_ref.getName();
    if(m_symbol_manager.lookup(var_name)->getLevel() == 0){
        instructions +=
            "    la t0, " + var_name + "\n";
    }else{
        int dist = fp + m_symbol_manager.lookupdist(var_name) * 4;
        if(p_variable_ref.getIndices().empty()){
            instructions +=
                "    addi t0, s0, " + to_string(dist) + "\n";
        }else{
            p_variable_ref.visitChildNodes(*this);
            auto dims = m_symbol_manager.lookup(var_name)->getTypePtr()->getDimensions();
            int dim_size = 1;
            for(auto dim : dims) dim_size *= (int)dim;
            for(int i = p_variable_ref.getIndices().size()-1 ; i >= 0  ; i--){
                instructions +=
                    "    li t0, 0\n"
                    "    lw t1, 0(sp)     # pop size from the stack\n"
                    "    addi sp, sp, 4\n"
                    "    li t2, " + ((i+1 < p_variable_ref.getIndices().size()) ?to_string(dims[i+1]) :to_string(1)) + "\n"
                    "    mul t1, t1, t2\n"
                    "    add t0, t0, t1\n";
            }
            instructions +=
                "    li t2, -4\n"
                "    mul t0, t0, t2\n"
                "    addi t1, s0, " + to_string(dist) + "\n";
            if(m_symbol_manager.lookup(var_name)->getKind() == SymbolEntry::KindEnum::kParameterKind){
                instructions +=
                    "    lw t2, 0(t1)\n"
                    "    add t0, t2, t0\n";
            }else{
                instructions +=
                    "    add t0, t1, t0\n";
            }
                
        }
    }
    instructions +=
        "    addi sp, sp, -4\n";
    if(inAssignment || inRead){
        inAssignment = false;
        instructions += "    sw t0, 0(sp)     # push the address to the stack\n";
    }else if(inFuncInvo && !m_symbol_manager.lookup(var_name)->getTypePtr()->getDimensions().empty() && p_variable_ref.getIndices().empty()){
        instructions +=
            "    sw t0, 0(sp)     # push the array addr to the stack\n";
    }else{
        instructions +=
            "    lw t1, 0(t0)\n"
            "    sw t1, 0(sp)     # push the value to the stack\n";
    }
    dumpInstructions(m_output_file.get(), instructions.c_str());
    if(m_symbol_manager.lookup(var_name)->getTypePtr()->isString()){
        printType = 1;
    }else{
        printType = 0;
    }
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    if(findString){
        p_assignment.visitChildNodes(*this);
        return;
    }
    if(inFunc && allowReturn == 0) return;
    inAssignment = true;
    p_assignment.visitChildNodes(*this);
    string instructions =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)     # pop the address from the stack\n"
        "    addi sp, sp, 4\n"
        "    sw t0, 0(t1)     # var = value\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(ReadNode &p_read) {
    if(findString){
        p_read.visitChildNodes(*this);
        return;
    }
    if(inFunc && allowReturn == 0) return;
    inRead = true;
    p_read.visitChildNodes(*this);
    inRead = false;
    string instructions =
        "    jal ra, readInt  # call function 'readInt'\n"
        "    lw t0, 0(sp)     # pop the address from the stack\n"
        "    addi sp, sp, 4\n"
        "    sw a0, 0(t0)     # save the return value to 'a'\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(IfNode &p_if) {
    if(findString){
        p_if.visitChildNodes(*this);
        return;
    }
    if(inFunc && allowReturn == 0) return;
    allowReturn++;
    int ifIdx = ++ifCnt;
    string jump;
    p_if.visitCondition(*this);
    jump =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    li t1, 0\n"
        "    beq t0, t1, if_end_" + to_string(ifIdx) + "\n"
        "if_then_" + to_string(ifIdx) + ":\n";
    dumpInstructions(m_output_file.get(), jump.c_str());
    
    p_if.visitBody(*this);
    if(!p_if.hasElse()){
        jump = "if_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());
    }else{
        jump =
            "    j if_else_end_" + to_string(ifIdx) + "\n"
            "if_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());

        allowReturn++;
        p_if.visitElse(*this);
        jump = "if_else_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());
    }
}

void CodeGenerator::visit(WhileNode &p_while) {
    if(findString){
        p_while.visitChildNodes(*this);
        return;
    }
    if(inFunc && allowReturn == 0) return;
    int whileIdx = ++whileCnt;
    string instructions = "";
    instructions = "while_begin_" + to_string(whileIdx) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
    p_while.visitCondition(*this);
    instructions =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    li t1, 0\n"
        "    beq t0, t1, while_end_" + to_string(whileIdx) + "\n"
        "while_then_" + to_string(whileIdx) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());

    p_while.visitCompound(*this);
    instructions =
        "    j while_begin_" + to_string(whileIdx) + "\n"
        "while_end_" + to_string(whileIdx) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}

void CodeGenerator::visit(ForNode &p_for) {
    if(findString){
        p_for.visitChildNodes(*this);
        return;
    }
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_for)));

    int forIdx = ++forCnt;
    string instructions = "";
    p_for.visitDecl(*this);
    int loop_var_dist = fp;

    p_for.visitAssignment(*this);
    instructions =
            "for_begin_" + to_string(forCnt) + ":\n"
            "    lw t0, " + to_string(loop_var_dist) + "(s0)        # load the value of 'i'\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)          # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());

    p_for.visitCondition(*this);
    instructions =
            "    lw t0, 0(sp)          # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    lw t1, 0(sp)          # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    bge t1, t0, for_end_" + to_string(forCnt) + "        # if i >= 13, exit the loop\n"
            "for_then_" + to_string(forCnt) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());

    p_for.visitCompound(*this);
    instructions =
        "    addi t0, s0, " + to_string(loop_var_dist) + "      # load the address of 'i'\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)          # push the address to the stack\n"
        "    lw t0, " + to_string(loop_var_dist) + "(s0)        # load the value of 'i'\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)          # push the value to the stack\n"
        "    li t0, 1\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)          # push the value to the stack\n"
        "    lw t0, 0(sp)          # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)          # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    add t0, t1, t0        # i + 1, always save the value in a certain register you choose\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)          # push the value to the stack\n"
        "    lw t0, 0(sp)          # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)          # pop the address from the stack\n"
        "    addi sp, sp, 4\n"
        "    sw t0, 0(t1)          # save the value to 'i'\n"
        "    j for_begin_" + to_string(forIdx) + "\n"
        "for_end_" + to_string(forIdx) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(ReturnNode &p_return) {
    if(findString){
        p_return.visitChildNodes(*this);
        return;
    }
    if(allowReturn == 0) return;
    p_return.visitChildNodes(*this);
    string instructions =
        "    lw a0, 0(sp)     # return value\n"
        "    addi sp, sp, 4\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
    allowReturn--;
}
