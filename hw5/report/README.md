# hw5 report

|      |                 |
| ---: | :-------------- |
| Name | yourname        |
|周世恩 | 110704010       |

## How much time did you spend on this project

4 days

## Project overview

> After spending about half day to see how RISC-V worked, down below is the implementation.

### Implementation

<details>
<summary>Click me to see implementation</summary>

### ProgramNode
First of all, iterate through all nodes to find string literals for later use. Then, visit child nodes of the ProgramNode and dump corresponding instructions before and after child nodes.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(ProgramNode &p_program) {
    ...

    // Find string literals
    findString = true; // turn on switch of string finding mode
    p_program.visitChildNodes(*this);
    findString = false;

    // DeclNode
    dumpInstructions(m_output_file.get(), ".section    .rodata       # emit rodata section\n");
    for(int i = 0 ; i < str_literals.size() ; i++){
        // Dump the string literals
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
    ...
    dumpInstructions(m_output_file.get(), compound_in_program_prologue);
    fp = -8; // init pointer location
    p_program.visitBody(*this);
    ...
    dumpInstructions(m_output_file.get(), compound_in_program_epilogue);
    ...
}
```
</details>

---
### DeclNode
If in string finding mode, push string literal into `str_literals` we defined in [CodeGenerator.hpp](../src/include/codegen/CodeGenerator.hpp).

If in global declaration, declare variable using `.comm`, constant using example's way but when encounter strings we use `.word S<str_idx>` instead.

If in local declaration, see below relation diagram.
```cpp
if(isVariable){
    if(isScalar){
        move pointer down 4
    }else{
        move pointer down 4 * array_size
    }
}else{
    if(isString){
        load string reg
    }else{
        load int value
    }
    move pointer down 4
    store in it
}
```

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(DeclNode &p_decl) {
    if(findString){
        for(auto &variable : p_decl.getVariables()){
            // if string variable
            if(variable->getTypePtr()->isString()){
                if(variable->getConstantPtr() == nullptr) continue;
                string str = string(variable->getConstantPtr()->getConstantValueCString());
                // push back if not in vector
                if(find(str_literals.begin(), str_literals.end(), str) == str_literals.end()){
                    str_literals.push_back(str);
                }
            }
        }
        return;
    }

    string decl_instructions = "";
    // global declaration
    if(inProgramDecl){
        for(auto &variable : p_decl.getVariables()){
            const SymbolEntry * entry;
            if((entry = m_symbol_manager.lookup(variable->getName())) != nullptr){
                if(entry->getKind() == SymbolEntry::KindEnum::kVariableKind){
                    // variable
                    decl_instructions += ".comm " + entry->getName() + ", 4, 4\n";
                }else{
                    // constant
                    decl_instructions +=
                    "    .align 2\n"
                    "    .globl " + entry->getName() + "\n"
                    "    .type " + entry->getName() + ", @object\n"
                    "" + entry->getName() + ":\n";

                    string str = string(entry->getAttribute().constant()->getConstantValueCString());
                    if(entry->getTypePtr()->isString()){
                        int str_idx = distance(str_literals.begin(), find(str_literals.begin(), str_literals.end(), str));
                        // string
                        decl_instructions +=
                            "    .word S" + to_string(str_idx) + "\n";
                    }else{
                        // int
                        decl_instructions +=
                            "    .word " + str + "\n";
                    }
                }
            }
        }
    // local declaration
    }else{
        for(auto &variable : p_decl.getVariables()){
            const SymbolEntry * entry;
            if((entry = m_symbol_manager.lookup(variable->getName())) != nullptr){
                if(entry->getKind() == SymbolEntry::KindEnum::kVariableKind || entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind || entry->getKind() == SymbolEntry::KindEnum::kParameterKind){
                    // variable
                    if(entry->getTypePtr()->isScalar()){
                        // scalar
                        // move pointer down 4
                        fp += -4;
                    }else{
                        auto dims = entry->getTypePtr()->getDimensions();
                        int dim_size = 1;
                        for(auto dim : dims) dim_size *= (int)dim;
                        // array
                        // move pointer down 4 * arr_size
                        fp += -(dim_size*4);
                    }
                }else{
                    // constant
                    // move pointer down 4
                    fp += -4;
                    decl_instructions +=
                        "    addi t0, s0, " + to_string(fp) + "\n";
                    string str = string(entry->getAttribute().constant()->getConstantValueCString());
                    if(entry->getTypePtr()->isString()){
                        int str_idx = distance(str_literals.begin(), find(str_literals.begin(), str_literals.end(), str));
                        // string
                        // load string register
                        decl_instructions +=
                            "    la t1, S" + to_string(str_idx) + "\n";
                    }else{
                        // int
                        // load int value
                        decl_instructions +=
                            "    li t1, " + str + "\n";
                    }
                    // store info in corresponding register
                    decl_instructions +=
                        "    sw t1, 0(t0) \n";
                }
            }
        }
    }
    dumpInstructions(m_output_file.get(), decl_instructions.c_str());
}
```
</details>

---
### VariableNode
Not modified

---
### ConstantValueNode
If in string finding mode, push string into `str_literals`.

In this node, we push __integer value__ or __string register__ to the stack. (boolean is converted to 0/1)

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    if(findString){
        // push string if not in str_literals
        if(p_constant_value.getTypePtr()->isString()){
            ...
        }
        return;
    }
    string instructions = "";
    if(p_constant_value.getTypePtr()->isString()){
        ...
        // push string address to stack
        instructions +=
            "    la t0, S" + to_string(str_idx) + "\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the value to the stack\n";
        ...
    }
    ...
    // convert boolean to integer
    if(value == "true" || value == "false"){
        value = (value == "true") ?"1" :"0";
    }

    // push integer value to stack
    instructions += 
        "    li t0, " + value + "\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    ...
}
```

</details>

---
### FunctionNode
In this node, we load parameters from specific register and then just visit its body.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(FunctionNode &p_function) {
    ...
    // init pointer
    fp = -8;
    string instructions = "";
    int paramdist = 0;
    // load parameters
    for (int i = 1 ; i <= p_function.getParametersNum(p_function.getParameters()) ; i++) {
        fp += -4;
        instructions +=
            "    sw s" + to_string(i) + ", " + to_string(fp) + "(s0)\n";
        paramdist += 4;
    }
    ...
    // visit body
    p_function.visitBodyChildNodes(*this);
    ...
}
```

</details>

---
### CompoundStatementNode
Just simply visit its child nodes.

---
### PrintNode
Visit its child first to get the printed value on stack, then we call `printInt` or `printString` determined by `printType` we previously defined.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(PrintNode &p_print) {
    ...
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
```

</details>

---
### BinaryOperatorNode
Load left operand to t1, right to t0.

If it is arithmetic operator, `<op> t0, t1, t0`

If it is compare operator, go to `bin_true_<cnt>` state if true, `bin_false_<cnt>` state if false.
```
    <op> t1, t0, bin_false_<cnt>
    li t0, 1
    j bin_true_
bin_false_<cnt>:
    li t0, 0
bin_true_<cnt>:
```

Then push the result to stack.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    ...
    // push operands to the stack
    p_bin_op.visitChildNodes(*this);
    string instructions = "";
    instructions +=
        "    lw t0, 0(sp)     # pop the val2 from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)     # pop the val1 from the stack\n"
        "    addi sp, sp, 4\n";
    string op = "";
    switch (p_bin_op.getOp()) {
        // arithmetic
        case Operator::kPlusOp:
            op = "    add t0, t1, t0\n";
            break;
        ...
        // compare
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
        ...
        default:
            break;
    }
    instructions += op;
    // push result to stack
    instructions +=
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### UnaryOperatorNode
Load operand to t0 and then multiply -1 if operator is `negative`, xor 1 if operator is `not`. Then push the result to the stack.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    ...
    // push the operand to the stack
    p_un_op.visitChildNodes(*this);
    string instructions = "";
    instructions +=
        "    lw t0, 0(sp)     # pop the val from the stack (unary)\n"
        "    addi sp, sp, 4\n";
    string op = "";
    // operator
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
    // push the result to the stack
    instructions += op +
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### FunctionInvocationNode
Load parameters to specific register and then call function and push return value to the stack.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    ...
    inFuncInvo = true;
    p_func_invocation.visitChildNodes(*this);
    inFuncInvo = false;
    int cnt = p_func_invocation.getArguments().size();
    string instructions = "";
    // load parameters to specific registers
    while(cnt > 0){
        instructions +=
            "    lw s" + to_string(cnt--) + ", 0(sp)     # load param value from stack\n"
            "    addi sp, sp, 4\n";
    }
    // call function and push return value to stack
    instructions +=
        "    jal ra, " + p_func_invocation.getName() + "\n"
        "    addi sp, sp, -4\n"
        "    sw a0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### VariableReferenceNode
If global variable, `la <name>` and load value from this address then push it to the stack.

If local variable, calculate its distance from the current pointer `fp` and load value from there then push it to the stack.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    ...
    // global variable
    if(m_symbol_manager.lookup(var_name)->getLevel() == 0){
        instructions +=
            "    la t0, " + var_name + "\n";

    // local variable
    }else{
        ...
        // no subscript
        if(p_variable_ref.getIndices().empty()){
            instructions +=
                "    addi t0, s0, " + to_string(dist) + "\n";

        // has subscript
        }else{
            ...
            // load array address
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
    
    // push address to the stack if in assignment or read node
    if(inAssignment || inRead){
        inAssignment = false;
        instructions += "    sw t0, 0(sp)     # push the address to the stack\n";

    // push array address to the stack if calling function
    }else if(inFuncInvo && !m_symbol_manager.lookup(var_name)->getTypePtr()->getDimensions().empty() && p_variable_ref.getIndices().empty()){
        instructions +=
            "    sw t0, 0(sp)     # push the array addr to the stack\n";
    
    // push value to the stack if none of the above
    }else{
        instructions +=
            "    lw t1, 0(t0)\n"
            "    sw t1, 0(sp)     # push the value to the stack\n";
    }
    dumpInstructions(m_output_file.get(), instructions.c_str());

    // set type for print
    if(m_symbol_manager.lookup(var_name)->getTypePtr()->isString()){
        printType = 1;
    }else{
        printType = 0;
    }
}
```

</details>

---
### AssignmentNode
Pop the address and the value from the stack, then store the value to the address.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(AssignmentNode &p_assignment) {
    ...
    string instructions =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)     # pop the address from the stack\n"
        "    addi sp, sp, 4\n"
        "    sw t0, 0(t1)     # var = value\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### ReadNode
Call `read` function, then pop the address from the stack and store return value to it.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(ReadNode &p_read) {
    ...
    string instructions =
        "    jal ra, readInt  # call function 'readInt'\n"
        "    lw t0, 0(sp)     # pop the address from the stack\n"
        "    addi sp, sp, 4\n"
        "    sw a0, 0(t0)     # save the return value to 'a'\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### IfNode
Pop bin_op return value from the stack and determined which state to go by that value.
```
    lw t0, 0(sp)     # pop the value from the stack
    addi sp, sp, 4
    li t1, 0
    beq t0, t1, if_end_<cnt>
if_then_<cnt>:
    ...
    j if_else_end_<cnt>
if_end_<cnt>:
    ...
if_else_end_<cnt>:
```

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(IfNode &p_if) {
    ...
    // pop the bin_op return value and go to if_then state if true, if_else state if false
    jump =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    li t1, 0\n"
        "    beq t0, t1, if_end_" + to_string(ifIdx) + "\n"
        "if_then_" + to_string(ifIdx) + ":\n";
    dumpInstructions(m_output_file.get(), jump.c_str());
    
    p_if.visitBody(*this);
    // if has else
    if(!p_if.hasElse()){
        // then
        jump = "if_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());
    }else{
        // then
        jump =
            "    j if_else_end_" + to_string(ifIdx) + "\n"
            "if_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());

        // else
        allowReturn++;
        p_if.visitElse(*this);
        jump = "if_else_end_" + to_string(ifIdx) + ":\n";
        dumpInstructions(m_output_file.get(), jump.c_str());
    }
}
```

</details>

---
### WhileNode
Pop the compare value from the stack and loop if true.
```
while_begin_<cnt>:
    lw t0, 0(sp)     # pop the value from the stack
    addi sp, sp, 4
    li t1, 0
    beq t0, t1, while_end_<cnt>
while_then_<cnt>:
    ...
    j while_begin_<cnt>
while_end_<cnt>:
```

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(WhileNode &p_while) {
    ...
    int whileIdx = ++whileCnt;
    string instructions = "";

    // begin
    instructions = "while_begin_" + to_string(whileIdx) + ":\n";
    dumpInstructions(m_output_file.get(), instructions.c_str());
    p_while.visitCondition(*this);
    instructions =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    li t1, 0\n"
        "    beq t0, t1, while_end_" + to_string(whileIdx) + "\n"
        "while_then_" + to_string(whileIdx) + ":\n";
    // then
    dumpInstructions(m_output_file.get(), instructions.c_str());

    p_while.visitCompound(*this);
    instructions =
        "    j while_begin_" + to_string(whileIdx) + "\n"
        "while_end_" + to_string(whileIdx) + ":\n";
    // end
    dumpInstructions(m_output_file.get(), instructions.c_str());
}
```

</details>

---
### ForNode
Basically just follow the example and I add a begin state at the front.

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(ForNode &p_for) {
    ...
    // load i and int to compare
    instructions =
            "for_begin_" + to_string(forCnt) + ":\n"
            "    lw t0, " + to_string(loop_var_dist) + "(s0)        # load the value of 'i'\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)          # push the value to the stack\n";
    ...
    instructions =
            "    lw t0, 0(sp)          # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    lw t1, 0(sp)          # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    bge t1, t0, for_end_" + to_string(forCnt) + "        # if i >= 13, exit the loop\n"
            "for_then_" + to_string(forCnt) + ":\n";
    ...
    // middle code
    p_for.visitCompound(*this);
    // i++
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
    ...
}
```

</details>

---
### ReturnNode
Pop return value from the stack and store it into `a0`

<details>
<summary><b>Code with comments</b></summary>

```cpp
void CodeGenerator::visit(ReturnNode &p_return) {
    ...
    p_return.visitChildNodes(*this);
    string instructions =
        "    lw a0, 0(sp)     # return value\n"
        "    addi sp, sp, 4\n";
   ...
}
```

</details>

</details>

### Some problems while implementing the homework
---
1. Some RISC-V command I want to use is not provided in the tutorial
    - Solution: Google and read document.
2. String literals have to predeclare in the `.section    .rodata` but some string is local
    - Solution: Traverse all nodes twice. First time for reading strings and push to a vector. Second time for code generating.
3. If, While, and For recursive structure: I've tried using BB++ for every time a basic block was created but it caused a lot of bugs
    - Solution: Make counter for each kind of BB so that they won't interfere with each other.
4. Variable reference problem: I don't know where to access the data
    - Solution: Calculate the distance from current pointer `fp` using `lookupdist()` then simply access that address.
5. Array reference problem: `lookupdist()` only reach the arr.begin()
    - Solution: using `m_symbol_manager.lookup(var_name)->getTypePtr()->getDimensions()` to get dimensions and calculate the offset from the `arr.begin()`
6. Array as parameter and array reference in function
    - Solution: Only put `arr.begin()` as input instead of all elements and calculate offset in the function then add it to `arr.begin()` to get actual address.
7. There have some residual value in the stack
    - Solution:
        ```
        popbegin:
            mv t0, sp              # t0 = current sp
            addi t0, t0, 128       # t0 = t0 + 128
            beq s0, t0, popend     # if(s0 == t0) goto popend
            addi sp, sp, 4         # else sp = sp - 4
            j popbegin             # goto popbegin
        popend:
        ```
The above is just a few problems I got, there are still a lot more that I met with (but this time is not that hard than last time :kissing: ).
## What is the hardest you think in this project

Array handling and Recursion :anger:

## Feedback to T.A.s

哭啊我原本以為作業分數是 115 -> 11.5 這樣算，這樣我把 HW5 Bonus 寫完就可以 A+ ，抱著這個信念每天爆肝想要怎麼解，結果今天突然說每次作業上限只有 100 我直接哭死，把正在寫的 String 寫完就完全不想再折磨自己了（我討厭考試啊啊啊啊啊啊啊啊啊）
