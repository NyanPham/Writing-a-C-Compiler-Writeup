Table of Contents

- [Phase 1](#phase-1-basic-register-allocator)
  - [Compiler Driver](#compiler-driver)
  - [BackwardDataflow](#backwarddataflow)
  - [CFG](#cfg)
  - [AddressTaken](#addresstaken)
  - [DeadStoreElimination](#deadstoreelimination)
  - [Assembly](#assembly)
  - [AssemblySymbols](#assemblysymbols)
  - [CodeGen](#codegen)
  - [Instruction Fixup](#instruction-fixup)
  - [RegAlloc](#regalloc)
  - [ReplacePseudo](#replacepseudo)
  - [Emit](#emit)
  - [Output](#output)
- [Phase 2](#phase-2-register-allocator-with-coalescing)
  - [Compiler Driver](#compiler-driver-1)
  - [DisjointSets](#disjointsets)
  - [RegAlloc](#regalloc-1)
  - [Output](#output-1)

---

# Phase 1 (Basic Register Allocator)

---

## Compiler Driver

```
Settings = {
    --snip--
    spill_info: bool;
    interference_ncol: bool;
    interference_graphviz: bool;
    liveness: bool;
}
```

## BackwardDataflow

We define a generic backward dataflow analysis framework, parameterized by a control-flow graph (CFG) module.

```
Dataflow(G):
    // G is a CFG module (see previous section)

    annotation = Set<var>
    annotated_block = G.basic_block<annotation>
    annotated_graph = G.t<annotation>

    analyze(debug_printer, meet_fn, transfer_fn, cfg):
        // Initialize all block annotations to the empty set
        current_cfg = G.initialize_annotation(cfg, empty_set)
        worklist = reverse(current_cfg.basic_blocks)  // list of (block_idx, block)

        process_worklist(current_cfg, worklist):
            debug_printer(current_cfg)
            if worklist is empty:
                return current_cfg  // analysis is done
            else:
                (block_idx, blk), rest = head(worklist), tail(worklist)
                old_annotation = blk.value
                live_vars_at_exit = meet_fn(current_cfg, blk)
                block_prime = transfer_fn(blk, live_vars_at_exit)
                updated_cfg = current_cfg with
                    basic_blocks = update block_idx to block_prime in current_cfg.basic_blocks

                if old_annotation == block_prime.value:
                    new_worklist = rest
                else:
                    // Add all predecessors of this block to the worklist (if not already present)
                    new_worklist = rest
                    for pred in block_prime.preds:
                        if pred is G.Entry:
                            continue
                        else if pred is G.Exit:
                            fail("Internal error: malformed CFG")
                        else if pred is G.Block(n) and n not in new_worklist:
                            new_worklist = [(n, updated_cfg.basic_blocks[n])] + new_worklist

                return process_worklist(updated_cfg, new_worklist)

        return process_worklist(current_cfg, worklist)
```

- `debug_printer(cfg)` is called at each iteration for debugging.
- `meet_fn(cfg, blk)` computes the meet (e.g., union/intersection) of successor annotations for the block.
- `transfer_fn(blk, annotation)` computes the new annotation for the block given the annotation at its exit.
- The analysis iterates until a fixed point is reached (no annotation changes).

## CFG

We define a generic control-flow graph (CFG) module, parameterized by an instruction type. This allows us to build CFGs for both TACKY and assembly instructions.

### Simple Instruction Kind

A simplified instruction type is used to classify instructions for basic block boundaries:

```
simple_instr =
    | Label(string)
    | ConditionalJump(string)
    | UnconditionalJump(string)
    | Return
    | Other
```

### Instruction Interface

We require an interface for instructions:

```
INSTR = {
    type instr
    simplify(instr) -> simple_instr
    pp_instr(formatter, instr)
}
```

### CFG Types

```
node_id = Entry | Block(int) | Exit

basic_block<'v> = {
    id: node_id,
    instructions: list of (annotation: 'v, instr: INSTR.instr),
    preds: list of node_id,
    succs: list of node_id,
    value: 'v
}

cfg<'v> = {
    basic_blocks: list of (int, basic_block<'v>),
    entry_succs: list of node_id,
    exit_preds: list of node_id,
    ctx: Context
}
```

### CFG Construction

- **Partition into Basic Blocks:**  
  Split a list of instructions into basic blocks at labels and after jumps/returns.

- **Build Label Map:**  
  Map label names to block IDs for jump resolution.

- **Add Edges:**  
  For each block, add edges to successors:

  - After a `Return`, edge to `Exit`.
  - After an unconditional jump, edge to the target label.
  - After a conditional jump, edge to both the target label and the next block.
  - Otherwise, edge to the next block.

- **Entry/Exit:**  
  Add an edge from `Entry` to the first block.

### CFG API

```
instructions_to_cfg(ctx, instructions) -> cfg<unit>
cfg_to_instructions(cfg<'v>) -> list of instr
get_succs(node_id, cfg) -> list of node_id
get_block_value(blocknum, cfg) -> 'v
add_edge(pred, succ, cfg)
remove_edge(pred, succ, cfg)
initialize_annotation(cfg<'a>, dummy: 'b) -> cfg<'b>
strip_annotations(cfg<'a>) -> cfg<unit>
print_graphviz(tag, pp_val, cfg<'v>)
```

### Example: TACKY and Assembly CFGs

We instantiate the generic CFG for TACKY and assembly instructions:

```
TackyCfg = Cfg({
    type instr = Tacky.instruction
    simplify(instr):
        match instr:
            case Label(l): Label(l)
            case Jump(target): UnconditionalJump(target)
            case JumpIfZero(_, target): ConditionalJump(target)
            case JumpIfNotZero(_, target): ConditionalJump(target)
            case Return(_): Return
            else: Other
    pp_instr = Tacky_print.pp_instruction
})

AsmCfg = Cfg({
    type instr = Assembly.instruction
    simplify(instr):
        match instr:
            case Label(l): Label(l)
            case Jmp(target): UnconditionalJump(target)
            case JmpCC(_, target): ConditionalJump(target)
            case Ret: Return
            else: Other
    pp_instr = Assembly.pp_instruction
})
```

---

This abstraction allows us to build, analyze, and transform control-flow graphs for any instruction set that implements the `INSTR` interface.

## AddressTaken

Given a TACKY program, we want to find all variables whose address is taken (i.e., variables that are potentially aliased).

```
// NEW
analyze_program(program):
    // program is Tacky.Program(toplevels)
    define analyze_toplevel(tl):
        if tl is Tacky.Function(f):
            return analyze(f.body)  // analyze function body for address-taken vars
        else:
            return empty_set

    aliased_vars_per_fun = map(analyze_toplevel, program.toplevels)
    return union_all(aliased_vars_per_fun)
```

Where:

- `union_all(list_of_sets)` unions all sets in the list into

## DeadStoreElimination

We now implement dead store elimination using the generic backward dataflow framework.

### Liveness Analysis with Backward Dataflow

We instantiate the generic `Dataflow` module for liveness analysis on the TACKY CFG:

```
Liveness = Dataflow(G)
    // G is the TackyCfg module (see CFG section)
```

### Transfer Function

The transfer function computes the set of live variables before each instruction, given the set of live variables at the end of the block:

```
transfer(static_and_aliased_vars, block, end_live_vars):
    // Helper functions
    remove_var(var, set):
        if var is Var(name): return set - {name}
        else: error

    add_var(val, set):
        if val is Var(name): return set ∪ {name}
        else: return set

    add_vars(vals, set):
        for v in vals: set = add_var(v, set)
        return set

    current_live = end_live_vars
    annotated = []

    // Process instructions in reverse order
    for (annot, instr) in reverse(block.instructions):
        switch instr:
            case Binary(dst, src1, src2):
                live1 = remove_var(dst, current_live)
                new_live = add_vars([src1, src2], live1)
            case Unary(dst, src):
                live1 = remove_var(dst, current_live)
                new_live = add_var(src, live1)
            case JumpIfZero(cond, _):
                new_live = add_var(cond, current_live)
            case JumpIfNotZero(cond, _):
                new_live = add_var(cond, current_live)
            case Copy(dst, src):
                live1 = remove_var(dst, current_live)
                new_live = add_var(src, live1)
            case Return(Some v):
                new_live = add_var(v, current_live)
            case FunCall(dst = opt_dst, args):
                if opt_dst is Some(d):
                    live1 = remove_var(d, current_live)
                else:
                    live1 = current_live
                live2 = add_vars(args, live1)
                new_live = live2 ∪ static_and_aliased_vars
            case Store(src, dst_ptr):
                new_live = add_vars([src, dst_ptr], current_live)
            // ... handle all other instructions as in previous code ...
            case Load(dst, src_ptr):
                live1 = remove_var(dst, current_live)
                live2 = add_var(src_ptr, live1)
                new_live = live2 ∪ static_and_aliased_vars
            case Jump(_), Label(_), Return(None):
                new_live = current_live
            // ... handle other instructions similarly ...
        annotated.prepend((new_live, instr))
        current_live = new_live

    // Reconstruct block
    return block.with(
        instructions = reverse(annotated),
        value = current_live
    )
```

### Meet Function

The meet function computes the union of live variable sets from all successors:

```
meet(static_vars, cfg, block):
    live = empty_set
    for succ in block.succs:
        switch succ:
            case Entry:
                fail("malformed CFG")
            case Exit:
                live = live ∪ static_vars
            case Block(n):
                live = live ∪ cfg.get_block_value(n)
    return live
```

### Running Liveness Analysis

```
find_live_variables(static_vars, aliased_vars, cfg):
    static_and_aliased_vars = static_vars ∪ aliased_vars
    meet_f = meet(static_vars)
    transfer_f = transfer(static_and_aliased_vars)
    debug_printer = debug_print(extra_tag="in_progress")
    return Liveness.analyze(debug_printer, meet_f, transfer_f, cfg)
```

### Removing Dead Stores

After liveness analysis, we remove instructions that write to variables which are not live:

```
is_dead_store((live_vars, instr)):
    if instr is FunCall or instr is Store:
        return false
    switch OptimizeUtils.get_dst(instr):
        case Some(Var(name)) when name not in live_vars:
            return true
        case _:
            return false
```

### Full Optimization Pipeline

```
optimize(aliased_vars, cfg):
    // Collect all static/global variables (always live)
    static_vars = { v | v is static in Symbols }
    annotated_cfg = find_live_variables(static_vars, aliased_vars, cfg)
    // Remove dead stores
    for (idx, block) in annotated_cfg.basic_blocks:
        block.instructions = filter(not is_dead_store, block.instructions)
    // Strip liveness annotations
    return G.strip_annotations(annotated_cfg)
```

### Summary of Changes

- Liveness analysis is now implemented using the generic `Dataflow` framework.
- The transfer and meet functions are passed to `Liveness.analyze`.
- The code is more modular and reuses the generic backward dataflow engine.
- The rest of the dead store elimination logic (removing dead stores, handling statics, etc.) is unchanged in structure, but now operates on the output of the generic analysis.

This approach makes it easy to implement other backward dataflow analyses in the future.

## Assembly

Add all the registers left: BX, R12, R13, R14, R15, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13
And also add Pop instruction, which only takes a register as operand.

```
program = Program(top_level*)
asm_type =
    | Byte
    | Longword
    | Quadword
    | Double
    | ByteArray(int size, int alignment)
top_level =
    | Function(string name, bool global, instruction* instructions)
    | StaticVariable(string name, bool global, int alignment, static_init* inits)
    | StaticConstant(string name, int alignment, static_init init)
instruction =
    | Mov(asm_type, operand src, operand dst)
    | Movsx(asm_type src_type, asm_type, dst_type, operand src, operand dst)
    | MovZeroExtend(asm_type src_type, asm_type, operand src, operand dst)
    | Lea(operand src, operand dst)
    | Cvttsd2si(asm_type, operand src, operand dst)
    | Cvtsi2sd(asm_type, operand src, operand dst)
    | Unary(unary_operator, asm_type, operand dst)
    | Binary(binary_operator, asm_type, operand, operand)
    | Cmp(asm_type, operand, operand)
    | Idiv(asm_type, operand)
    | Div(asm_type, operand)
    | Cdq(asm_type)
    | Jmp(string)
    | JmpCC(cond_code, string)
    | SetCC(cond_code, operand)
    | Label(string)
    | Push(operand)
    | Pop(Reg)
    | Call(string)
    | Ret
unary_operator = Neg | Not | ShrOneOp
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor | Sal | Sar | Shr | Shl
operand = Imm(int64) | Reg(reg) | Pseudo(string) | Memory(reg, int) | Data(string, int offset) | PseudoMem(string, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | BX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | R12 | R13 | R14 | R15 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM8 | XMM9 | XMM10 | XMM11 | XMM12 | XMM13 | XMM14 | XMM15
```

## AssemblySymbols

```
// Fun entry has 3 more fields: param_regs, return_args, callee_saved_regs_used
type entry =
    | Fun(bool defined, int bytes_required, bool return_on_stack, Assembly.Reg* param_regs, Assembly.Reg* return_args, Set<Assembly.Reg> callee_saved_regs_used)
    | Obj(Assembly.asm_type t, bool is_static, bool is_constant)
```

```
// param_regs, return_regs parameters are added
add_fun(fun_name, defined, return_on_stack, param_regs, return_regs):
    symbol_table[fun_name] = Fun(defined, 0, return_on_stack, param_regs, return_regs, {})
```

We add a function to record which callee-saved registers are used by a function.

```
// NEW
add_callee_saved_regs_used(fun_name, regs):
    set_regs(entry):
        if entry is Fun(f):
            return Fun({
                ...f,
                callee_saved_regs_used = f.callee_saved_regs_used ∪ regs
            })
        else if entry is Obj(_):
            fail("Internal error: not a function")

    symbol_table[fun_name] = set_regs(symbol_table[fun_name])
```

```
// NEW
get_callee_saved_regs_used(fun_name):
    entry = symbol_table[fun_name]
    if entry is Fun(f):
        return f.callee_saved_regs_used
    else if entry is Obj(_):
        fail("Internal error: not a function")
```

```
// NEW
param_regs_used(fun_name):
    entry = symbol_table[fun_name]
    if entry is Fun(f):
        return f.param_regs
    else if entry is Obj(_):
        fail("Internal error: not a function")
```

```
// NEW
return_regs_used(fun_name):
    entry = symbol_table[fun_name]
    if entry is Fun(f):
        return f.return_regs
    else if entry is Obj(_):
        fail("Internal error: not a function")
```

## CodeGen

Remove the function `classify_tacky_val`.

```
// NEW
get_tag(type):
    if type is Types.Structure(tag):
        return tag
    else if type is Types.Union(tag):
        return tag
    else:
        fail("Internal error: trying to get tag for non-structure or union type")
```

```
// NEW
classify_params_helper(typed_asm_vals, return_on_stack):
    if return_on_stack == true:
        int_regs_available = 5
    else:
        int_regs_available = 6

    int_reg_args = []
    dbl_reg_args = []
    stack_args   = []

    for each (tacky_t, operand) in typed_asm_vals:
        t = convert_type(tacky_t)
        typed_operand = (t, operand)

        if tacky_t is Structure or Union:
            // extract the base name of the struct/union
            if operand is PseudoMem(var_name, 0):
                // OK
            else:
                fail("Bad structure operand")

            var_size = get_size(tacky_t)
            classes  = classify_type(get_tag(tacky_t))

            // decide whether to spill all eightbytes
            if classes[0] == Mem:
                updated_int = int_reg_args
                updated_dbl = dbl_reg_args
                use_stack   = true
            else:
                // try placing each eightbyte in regs
                tentative_ints = copy(int_reg_args)
                tentative_dbls = copy(dbl_reg_args)

                for i, cls in enumerate(classes):
                    eb_op = PseudoMem(var_name, i * 8)
                    if cls == SSE:
                        tentative_dbls.append(eb_op)
                    else if cls == Integer:
                        eb_type = get_eightbyte_type(i, var_size)
                        tentative_ints.append((eb_type, eb_op))
                    else:
                        fail("Internal error: found eightbyte in Mem class")

                if length(tentative_ints) ≤ int_regs_available
                   and length(tentative_dbls) ≤ 8:
                    updated_int = tentative_ints
                    updated_dbl = tentative_dbls
                    use_stack   = false
                else:
                    updated_int = int_reg_args
                    updated_dbl = dbl_reg_args
                    use_stack   = true

            // if spilling to stack, push all eightbytes there
            if use_stack == true:
                for i from 0 to length(classes)-1:
                    eb_type = get_eightbyte_type(i, var_size)
                    eb_op   = PseudoMem(var_name, i * 8)
                    stack_args.append((eb_type, eb_op))
            else:
                int_reg_args = updated_int
                dbl_reg_args = updated_dbl

        else if tacky_t == Double:
            if length(dbl_reg_args) < 8:
                dbl_reg_args.append(operand)
            else:
                stack_args.append(typed_operand)

        else:
            // integer-like types
            if length(int_reg_args) < int_regs_available:
                int_reg_args.append(typed_operand)
            else:
                stack_args.append(typed_operand)

    return (int_reg_args, dbl_reg_args, stack_args)
```

```
classify_parameters(params, return_on_stack):
    typed_asm_vals = []
    for each v in params:
        typed_asm_vals.append((type_of_val(v), convert_val(v)))
    return classify_params_helper(typed_asm_vals, return_on_stack)
```

```
classify_param_types(type_list, return_on_stack):
    // build a list of (type, dummy_operand) pairs
    typed_asm_vals = []
    for each t in type_list:
        if is_scalar(t):
            operand = Pseudo("dummy")
        else:
            operand = PseudoMem("dummy", 0)
        typed_asm_vals.append((t, operand))

    // reuse the helper to figure out how many int and double regs would be used
    (ints, dbls, _) = classify_params_helper(typed_asm_vals, return_on_stack)

    // pick that many registers from the predefined calling‐convention lists
    int_regs = int_param_passing_regs[0 .. length(ints)−1]
    dbl_regs = dbl_param_passing_regs[0 .. length(dbls)−1]

    // return the concatenation of int- and double-register lists
    return [...int_reg, ...dbl_regs]
```

```
// NEW
classify_return_helper(ret_type, asm_retval):
    // Handle struct or union return
    if ret_type is Structure or Union:
        classes = classify_type(get_tag(ret_type))

        // extract the base name from the return slot
        if asm_retval is PseudoMem(var_name, 0):
            // ok
        else:
            fail("Invalid assembly operand for structure return")

        // if the first eightbyte is classified as memory, spill all to stack
        if classes[0] == Mem:
            return ([], [], true)

        // otherwise, assign each eightbyte to registers
        ints = []
        dbls = []
        for i, cls in enumerate(classes):
            operand = PseudoMem(var_name, i * 8)
            if cls == SSE:
                dbls.append(operand)
            else if cls == Integer:
                eb_type = get_eightbyte_type(i, total_var_size = size_of(ret_type))
                ints.append((eb_type, operand))
            else:
                fail("Internal error: unexpected Mem class in eightbyte")

        return (ints, dbls, false)

    // Handle double return
    else if ret_type == Double:
        return ([], [asm_retval], false)

    // Handle all other scalar returns
    else:
        typed_operand = (convert_type(ret_type), asm_retval)
        return ([typed_operand], [], false)
```

```
// call classify_return_helper
classify_return_value(retval):
    return classify_return_helper(type_of_val(retval), convert_val(retval))
```

```
// NEW
classify_return_type(ret_type):
    if ret_type == Void:
        return ([], false)

    // build a dummy asm operand for sizing
    if is_scalar(ret_type):
        asm_val = Pseudo("dummy")
    else:
        asm_val = PseudoMem("dummy", 0)

    (ints, dbls, return_on_stack) = classify_return_helper(ret_type, asm_val)

    if return_on_stack:
        // spill struct/union on stack: use AX to carry address
        return ([AX], true)
    else:
        // select the needed integer and SSE registers
        int_regs = [AX, DX][0 .. length(ints)−1]
        dbl_regs = [XMM0, XMM1][0 .. length(dbls)−1]

        return ([...int_regs, ...dbl_regs], false)
```

```
convert_symbol(name, symbol):
    // Case 1: a fully‐defined function with complete return and parameter types
    if symbol.t is FunType(param_types, ret_type)
       and symbol.attrs is FunAttr { defined }
       and (is_complete(ret_type) or ret_type is Void)
       and for all t in param_types: is_complete(t):

        // classify how the return value is passed
        (ret_regs, return_on_stack) = classify_return_type(ret_type)

        // classify how the parameters are passed
        param_regs = classify_param_types(param_types, return_on_stack)

        return asmSymbols.add_fun(
            name,
            defined,
            returns_on_stack(name),
            param_regs,
            ret_regs
        )

    // Case 2: a function with incomplete return or parameter types
    else if symbol.t is FunType(_)
         and symbol.attrs is FunAttr { defined }:
        // If this function has incomplete return type besides void, or any incomplete
        // param type (implying we don't define or call it in this translation unit)
        // use dummy values
        assert(not symbol.attrs.defined)
        return asmSymbols.add_fun(name, symbol.attrs.defined, false, [], [])

    // Case 3: a constant
    else if symbol.attrs is ConstAttr:
        return asmSymbols.add_constant(name, convert_type(symbol.t))

    // Case 4a: a static variable of incomplete type
    else if symbol.attrs is StaticAttr
         and not is_complete(symbol.t):
        // use dummy type for static variables of incomplete type
        return asmSymbols.add_var(name, Byte, true)

    // Case 4b: a static variable of complete type
    else if symbol.attrs is StaticAttr:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), true)

    // Case 5: any other global symbol (e.g., non‐static variable)
    else:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), false)
```

## Instruction Fixup

```
// extend the added registers
is_xmm(reg):
    switch reg:
        case XMM0:
        case XMM1:
        case XMM2:
        case XMM3:
        case XMM4:
        case XMM5:
        case XMM6:
        case XMM7:
        case XMM8:
        case XMM9:
        case XMM10:
        case XMM11:
        case XMM12:
        case XMM13:
        case XMM14:
        case XMM15:
            return true
        default:
            return false
```

```
// parameter callee_saved_regs added.
// add the case for Ret to fix up.
fixup_instruction(Assembly.Instruction inst, callee_saved_regs):
    match inst.type:
    case Mov:
        // Mov can't move a value from one memory address to another
        if inst.src is (Memory | Data) and inst.dst is (Memory | Data):
            scratch = inst.t == Double
                ? Reg(XMM14)
                : Reg(R10)

            return [
                Mov(inst.t, src, scratch),
                Mov(inst.t, scratch, dst)
            ]
        // Mov can't move a large constant to a memory address
        else if inst.t is Quadword and inst.src is Imm and inst.dst is (Memory | Data) and is_large(inst.src.value):
            return [
                Mov(Quadword, inst.src, Reg(R10)),
                Mov(Quadword, Reg(R10), inst.dst),
            ]
        // Moving a quadword-size constant with a longword operand size produces assembler warning
        else if inst.t is Longword and inst.src is Imm and is_larger_than_uint(inst.src.value):
            // reduce modulo 2^32 by zeroing out upper 32 bits
            bitmask = convert_to_64bit("0xffffffff")
            reduced = i & bitmask
            return [
                Mov(Longword, Imm(reduced), inst.dst)
            ]
        // Moving a longword-size constant with a byte operand size produces assembler warning
        else if inst.t is Byte and inst.src is Imm and is_larger_than_byte(inst.src.value):
            reduced = int64(inst.src.value)
            return [
                Mov(Byte, Imm(reduced), inst.dst)
            ]
        else:
            return [ inst ]
    case Movsx:
        // Movsx cannot handle immediate src or memory dst
        if inst.src is Imm && inst.dst is (Memory | Data):
            return [
                Mov(inst.src_type, inst.src, Reg(R10)),
                Movsx(inst.src_type, inst.dst_type, Reg(R10), Reg(R11)),
                Mov(inst.dst_type, Reg(R11), inst.dst)
            ]
        else if inst.src is Imm:
            return [
                Mov(inst.src_type, inst.src, Reg(R10)),
                Movsx(inst.src_type, inst.dst_type, Reg(R10), inst.dst)
            ]
        else if inst.dst is (Memory | Data):
            return [
                Movsx(inst.src_type, inst.dst_type, inst.src, Reg(R11)),
                Mov(dst_type, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]
    case MovZeroExtend:
        if inst.src_type is Byte and inst.src is Imm:
            // MovZeroExtend src can't be an immediate.
            if is_memory(inst.dst):
                return [
                    Mov(Byte, Imm(inst.src.value), Reg(R10)),
                    MovZeroExtend(Byte, inst.dst_type, Reg(R10), Reg(R11)),
                    Mov(inst.dst_type, Reg(R11), dst)
                ]
            else
                return [
                    Mov(Byte, Imm(inst.src.value), Reg(R10)),
                    MovZeroExtend(Byte, inst.dst_type, Reg(R10), inst.dst)
                ]

        else if inst.src_type is Byte and is_memory(inst.dst):
            // MovZeroExtend destination must be a register
            return [
                MovZeroExtend(Byte, inst.dst_type, inst.src, Reg(R11)),
                Mov(inst.dst_type, Reg(R11), inst.dst)
            ]
        else if inst.src_type is Longword and isMemory(dst):
            // to zero-extend longword to quadword, first copy into register, then move to destination
            return [
                Mov(Longword, inst.src, Reg(R11)),
                Mov(inst.dst_type, Reg(R11), dst)
            ]
        else if inst.src_type is Longword:
            return [
                Mov(Longword, inst.src, inst.dst)
            ]
        else:
            return [ inst ]
    case Idiv:
        // Idiv can't operate on constants
        if inst.operand is Imm:
            return [
                Mov(inst.t, Imm(inst.operand.val), Reg(R10)),
                Idiv(inst.t, Reg(R10))
            ]
        else:
            return [ inst ]
    case Div:
        // Div can't operate on constants
        if inst.operand is Imm:
            return [
                Mov(inst.t, Imm(inst.operand.val), Reg(R10)),
                Div(inst.t, Reg(R10))
            ]
        else:
            return [ inst ]
    case Lea:
        // dst of lea must be a register
        if is_memory(inst.dst):
            return [
                Lea(inst.src, Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]

    case Binary:
        // Binary operations on double require register as destination
        if inst.t == Double:
            if inst.dst is Reg:
                return [ inst ]
            else:
                return [
                    Mov(Double, dst, Reg(XMM15)),
                    Binary(inst.op, Double, src, Reg(XMM15)),
                    Mov(Double, Reg(XMM15), dst)
                ]
        // Add/Sub/And/Or/Xor can't take large immediates as source operands
        else if (inst.op is Add | Sub | And | Or | Xor)
            and (inst.t is Quadword)
            and (inst.src is Imm)
            and is_large(inst.src.val):
            return [
                Mov(Quadword, inst.src, Reg(R10)),
                Binary(inst.op, Quadword, Reg(R10), inst.dst)
            ]

        // Add/Sub can't use memory addresses for both operands
        if inst.op is (Add | Sub | And | Or | Xor) and both operands are (Memory | Data):
            return [
                Mov(inst.t, inst.src, Reg(R10)),
                Binary(inst.op, inst.t, Reg(R10), inst.dst)
            ]
         // Destination of Mult can't be in memory, src can't be a big operand
        else if (inst.op is Mult)
                    and (dst is (Memory | Data))
                    and (t is Quadword)
                    and (src is Imm)
                    and (is_large(src.val)):
            // rewrite both operands
            return [
                Mov(Quadword, inst.src, Reg(R10)),
                Mov(Quadword, inst.dst, Reg(R11)),
                Binary(Mult, Quadword, src=Reg(R10), dst=Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
        else if (inst.op is Mult)
                    and (t is Quadword)
                    and (src is Imm)
                    and (is_large(src.val)):
            // just rewrite src
            return [
                Mov(Quadword, inst.src, Reg(R10)),
                Binary(mult, Quadword, src=Reg(R10), inst.dst)
            ]
        else if inst.op is Mult and dst is (Memory | Data):
            return [
                Mov(inst.t, inst.dst, Reg(R11)),
                Binary(inst.op, inst.t, inst.src, Reg(R11)),
                Mov(inst.t, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]
    case Cmp:
        // destination of comisd must be a register
        if inst.t == Double:
            if inst.dst is Reg:
                return [ inst ]
            else:
                return [
                    Mov(Double, dst, Reg(XMM15)),
                    Cmp(Double, src, Reg(XMM15))
                ]
        // Both operands of cmp can't be in memory
        else if both src and dst of cmp are (Memory | Data):
            return [
                Mov(inst.t, inst.src, Reg(R10)),
                Cmp(inst.t, Reg(10), inst.dst)
            ]
        // First operand of Cmp can't be a large constant, second can't be a constant at all
        else if (inst.t is Quadword)
            and (src is Imm)
            and (dst is Imm)
            and is_large(src.val):
            return [
                Mov(Quadword, src, Reg(R10)),
                Mov(Quadword, dst, Reg(R11)),
                Cmp(Quadword, Reg(R10), Reg(R11))
            ]
        else if (inst.t is Quadword)
            and (src is Imm)
            and is_large(src.val):
            return [
                Mov(Quadword, src, Reg(R10)),
                Cmp(Quadword, Reg(r10), inst.dst)
            ]
        else if dst of cmp is an immediate:
            return [
                Mov(inst.t, Imm(inst.dst.val), Reg(R11)),
                Cmp(inst.t, inst.src, Reg(R11)),
            ]

        else:
            return [ inst ]
    case Push:
        if inst.operand is Reg and is_xmm(inst.operand):
            return [
                Binary(Sub, Quadword, Imm(8), Reg(SP)),
                Mov(Double, Reg(inst.operand.reg_name), Memory(Reg(SP), 0))
            ]

        if src is Imm and is_large(src.val):
            return [
                Mov(Quadword, src, Reg(R10)),
                Push(Reg(R10))
            ]
        else:
            return [ inst ]
    case Cvttsd2si:
        // destination of cvttsd2si must be a register
        if inst.dst is Memory or Data:
            return [
                Cvttsd2si(inst.t, inst.src, Reg(R11)),
                Mov(inst.t, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]
    case Cvtsi2sd:
        if is_constant(inst.src) and is_memory(inst.dst):
            return [
                Mov(inst.t, inst.src, Reg(R10)),
                Cvtsi2sd(inst.t, Reg(R10), Reg(XMM15)),
                Mov(Double, Reg(XMM15), dst)
            ]
        else if is_constant(inst.src):
            return [
                Mov(inst.t, src, Reg(R10)),
                Cvtsi2sd(inst.t, Reg(R10), dst)
            ]
        else if is_memory(inst.dst):
            return [
                Cvtsi2sd(inst.t, src, Reg(XMM15)),
                Mov(Double, Reg(XMM15), dst)
            ]
        else:
            return [ inst ]
    case Ret:
        // On return, restore each callee-saved register in reverse order,
        // then emit the Ret instruction.
        restores = []
        for r in reverse(callee_saved_regs):
            restores.append(Pop(r))
        restores.append(Assembly.Ret)
        return restores
    default:
        return [ other ]
```

```
// NEW
emit_stack_adjustment(bytes_for_locals, callee_saved_count):
    callee_saved_bytes = callee_saved_count * 8
    total_stack_bytes = callee_saved_bytes + bytes_for_locals
    adjusted_stack_bytes = Rounding.round_away_from_zero(16, total_stack_bytes)
    stack_adjustment = to_int64(adjusted_stack_bytes - callee_saved_bytes)
    return Binary(op=Sub, t=Quadword, src=Imm(stack_adjustment), dst=Reg(SP))
```

```
fixup_top_level(Assembly.top_level top_level):
    if top_level is Function:
        // TODO bytes_required should be positive (fix this in replace_pseudos)
        stack_bytes = -AsmSymbols.get_bytes_required(top_level.name)
        callee_saved_regs = list(AsmSymbols.get_callee_saved_regs_used(top_level.name))

        save_reg = (r):
        {
            return Push(Reg(r))
        }

        adjust_rsp = emit_stack_adjustment(stack_bytes, length(callee_saved_regs))
        setup_instructions = [...adjust_rsp]
        for reg in callee_saved_regs:
            setup_instructions.append(save_reg(reg))

        return Assembly.Function(
            fn_def.name,
            [
                setup_instructions,
                ...[fn_def.instructions for each inst -> fixup_instruction(inst, callee_saved_regs)]
            ]
        )
    else: // StaticVariable
        return top_level
```

## RegAlloc

```
get_operands(instr):
    match instr:
        case Mov(_, src, dst): return [src, dst]
        case Movsx(i): return [i.src, i.dst]
        case MovZeroExtend(zx): return [zx.src, zx.dst]
        case Lea(src, dst): return [src, dst]
        case Cvttsd2si(_, src, dst): return [src, dst]
        case Cvtsi2sd(_, src, dst): return [src, dst]
        case Unary(_, _, op): return [op]
        case Binary(b): return [b.src, b.dst]
        case Cmp(_, v1, v2): return [v1, v2]
        case Idiv(_, op): return [op]
        case Div(_, op): return [op]
        case SetCC(_, op): return [op]
        case Push(op): return [op]
        case Label(_), Call(_), Ret, Cdq(_), JmpCC(_), Jmp(_): return []
        case Pop(_): fail("Internal error")
```

```
// Replace all operands in instr using function f
replace_ops(f, instr):
    match instr:
        case Mov(t, src, dst): return Mov(t, f(src), f(dst))
        case Movsx(i): return Movsx({ ...i, src: f(i.src), dst: f(i.dst) })
        case MovZeroExtend(zx): return MovZeroExtend({ ...zx, src: f(zx.src), dst: f(zx.dst) })
        case Lea(src, dst): return Lea(f(src), f(dst))
        case Cvttsd2si(t, src, dst): return Cvttsd2si(t, f(src), f(dst))
        case Cvtsi2sd(t, src, dst): return Cvtsi2sd(t, f(src), f(dst))
        case Unary(op, t, operand): return Unary(op, t, f(operand))
        case Binary(b): return Binary({ ...b, src: f(b.src), dst: f(b.dst) })
        case Cmp(code, v1, v2): return Cmp(code, f(v1), f(v2))
        case Idiv(t, op): return Idiv(t, f(op))
        case Div(t, op): return Div(t, f(op))
        case SetCC(code, op): return SetCC(code, f(op))
        case Push(op): return Push(f(op))
        case Label(_), Call(_), Ret, Cdq(_), Jmp(_), JmpCC(_): return instr
        case Pop(_): fail("We shouldn't use this yet")
```

```
cleanup_movs(instructions):
    is_redundant_mov(instr):
        match instr:
            case Mov(_, src, dst): return src == dst
            default: return false
    return filter(lambda instr: not is_redundant_mov(instr), instructions)
```

```
REG_TYPE = {
    suffix: string,
    all_hardregs: list of reg,
    caller_saved_regs: list of reg,
    pseudo_is_current_type: fn(string) -> bool,
    debug_settings: fn() -> debug_options
}
```

```
namespace Allocator(R: REG_TYPE):
    regs_to_operands(regs):
        return set(map(Reg, regs))

    all_hardregs = regs_to_operands(R.all_hardregs)
    caller_saved_regs = regs_to_operands(R.caller_saved_regs)

    node = {
        id: operand,
        neighbors: set of operand,
        spill_cost: float,
        color: int or None,
        pruned: bool
    }

    graph = map from operand to node

    k = number of hardregs

    add_edge(graph, id1, id2):
        // Add id2 to the neighbors of id1
        graph[id1].neighbors = graph[id1].neighbors ∪ {id2}
        // Add id1 to the neighbors of id2
        graph[id2].neighbors = graph[id2].neighbors ∪ {id1}

    regs_used_and_written(instr):
        // Determine operands used and written based on instruction type
        match instr:
            case Mov(_, src, dst): ops_used = [src]; ops_written = [dst]
            case MovZeroExtend(zx): ops_used = [zx.src]; ops_written = [zx.dst]
            case Movsx(i): ops_used = [i.src]; ops_written = [i.dst]
            case Cvtsi2sd(_, src, dst): ops_used = [src]; ops_written = [dst]
            case Cvttsd2si(_, src, dst): ops_used = [src]; ops_written = [dst]
            // dst of binary or unary instruction is both read and written
            case Binary(b): ops_used = [b.src, b.dst]; ops_written = [b.dst]
            case Unary(_, _, op): ops_used = [op]; ops_written = [op]
            case Cmp(_, v1, v2): ops_used = [v1, v2]; ops_written = []
            case SetCC(_, op): ops_used = []; ops_written = [op]
            case Push(v): ops_used = [v]; ops_written = []
            case Idiv(_, op): ops_used = [op, Reg(AX), Reg(DX)]; ops_written = [Reg(AX), Reg(DX)]
            case Div(_, op): ops_used = [op, Reg(AX), Reg(DX)]; ops_written = [Reg(AX), Reg(DX)]
            case Cdq(_): ops_used = [Reg(AX)]; ops_written = [Reg(DX)]
            case Call(f):
                // function call updates caller-saved regs, uses param-passing registers
                used = param_regs_used(f) filtered to hardregs, as Reg(r)
                ops_used = used
                ops_written = caller_saved_regs
            // if src is a pseudo, lea won't actually generate it,
            // but we've excluded it from the graph anyway
            // if it's a memory address or indexed operand, we _do_ want to generate
            // hardregs used in address calculations
            case Lea(src, dst): ops_used = [src]; ops_written = [dst]
            case Jmp(_), JmpCC(_), Label(_), Ret: ops_used = []; ops_written = []
            case Pop(_): fail("Internal error")

        // convert list of operands read into list of hard/pseudoregs read
        regs_used_to_read(opr):
            if opr is Pseudo or Reg:
                return [opr]
            else if opr is Memory(r, _):
                return [Reg(r)]
            else if opr is Indexed(x):
                return [Reg(x.base), Reg(x.index)]
            else if opr is Imm or Data or PseudoMem:
                return []

        regs_read1 = concat_map(regs_used_to_read, ops_used)

        // now convert list of operands written into lists of hard/pseudoregs
        // read _or_ written, accounting for the fact that writing to a memory address
        // may require reading a pointer
        regs_used_to_update(opr):
            if opr is Pseudo or Reg:
                return ([], [opr])
            else if opr is Memory(r, _):
                return ([Reg(r)], [])
            else if opr is Indexed(x):
                return ([Reg(x.base), Reg(x.index)], [])
            else if opr is Imm or Data or PseudoMem:
                return ([], [])

        regs_read2, regs_written = unzip(map(regs_used_to_update, ops_written))
        regs_read2 = concat(regs_read2)
        regs_written = concat(regs_written)

        return (set(regs_read1 + regs_read2), set(regs_written))

    class DumpGraph:
        // Helper to print the interference graph in various formats
        dump_helper(start_graph, end_graph, file_ext, edge_printer, post_processor, ctx, g):
            filename = make_filename(R.suffix + ".interference", ctx, file_ext)
            path = join_path(get_cwd(), filename)
            chan = open_file_for_write(path)

            print_edges(nd, node):
                if node.pruned:
                    return
                else:
                    print_edge(nghbor_id):
                        edge_printer(chan, show_node_id(nd), show_node_id(nghbor_id))
                    // Only print edges to neighbors with higher id and not pruned
                    _, later_neighbors = split_le(nd, node.neighbors)
                    not_pruned(nd_id) = not g[nd_id].pruned
                    unpruned_later_neighbors = filter(not_pruned, later_neighbors)
                    for nghbor_id in unpruned_later_neighbors:
                        print_edge(nghbor_id)

            if start_graph is not None:
                chan.write(start_graph)
            for nd, node in g:
                print_edges(nd, node)
            if end_graph is not None:
                chan.write(end_graph)
            close_file(chan)
            post_processor(filename)

        dump_graphviz(ctx, g):
            if R.debug_settings().interference_graphviz and is_dump_target(ctx):
                start_graph = "graph {\n"
                end_graph = "\t}\n"
                edge_printer = lambda chan, n1, n2: chan.write("\t" + n1 + " -- " + n2 + "\n")
                post_processor = lambda filename:
                    // convert DOT file to png
                    cmd = "circo -Tpng " + filename + " -o " + remove_extension(filename) + ".png"
                    if system(cmd) != 0:
                        fail("graphviz fail: " + cmd)
                    else if system("rm " + filename) != 0:
                        fail("failed to remove DOT file")
                dump_helper(start_graph, end_graph, ".dot", edge_printer, post_processor, ctx, g)

        dump_ncol(ctx, g):
            if R.debug_settings().interference_ncol and is_dump_target(ctx):
                edge_printer = lambda chan, n1, n2: chan.write(n1 + " " + n2 + "\n")
                post_processor = lambda filename: None
                dump_helper(None, None, ".ncol", edge_printer, post_processor, ctx, g)

    class LivenessAnalysis:
        meet(cfg, block):
            all_return_regs = regs_to_operands(return_regs_used(cfg.ctx.fun_name))
            live_at_exit = intersection(all_hardregs, all_return_regs)
            for succ in block.succs:
                if succ is Entry: error
                else if succ is Exit: live = live ∪ live_at_exit
                else if succ is Block(n): live = live ∪ get_block_value(n, cfg)
            return live

        transfer(block, end_live_regs):
            // Process instructions in reverse order for backward dataflow
            current_live = end_live_regs
            annotated = []

            for (annot, instr) in reverse(block.instructions):
                (regs_used, regs_written) = regs_used_and_written(instr)
                without_killed = current_live - regs_written
                new_live = without_killed ∪ regs_used
                annotated.prepend((new_live, instr))
                current_live = new_live

            // Reconstruct block with updated liveness annotations
            return block.with(
                instructions = reverse(annotated),
                value = current_live
            )

        analyze(cfg):
            return Iterative.analyze(debug_print_cfg("in_progress"), meet, transfer, cfg)

    mk_base_graph():
        // Create nodes for all hardregs
        graph = empty_map()
        for r in all_hardregs:
            node = {
                id: r,
                neighbors: all_hardregs - {r},
                spill_cost: infinity,
                color: None,
                pruned: false
            }
            graph[r] = node
        return graph

    get_pseudo_nodes(aliased_pseudos, instructions):
        // Extract all pseudo-registers used in instructions (excluding statics and aliased)
        operands_to_pseudos(op):
            if op is Pseudo(r):
                if R.pseudo_is_current_type(r)
                and not (is_static(r) or r in aliased_pseudos):
                    return r
                else:
                    return None
            else:
                return None

        get_pseudos(instr):
            return filter_map(operands_to_pseudos, get_operands(instr))

        initialize_node(pseudo):
            return {
                id: Pseudo(pseudo),
                neighbors: empty_set,
                spill_cost: 0.0,
                color: None,
                pruned: false
            }

        pseudos = concat_map(get_pseudos, instructions)
        unique_pseudos = unique(pseudos)
        return map(initialize_node, unique_pseudos)

    add_pseudo_nodes(aliased_pseudos, graph, instructions):
        nds = get_pseudo_nodes(aliased_pseudos, instructions)
        for nd in nds:
            graph[nd.id] = nd
        return graph

    add_edges(liveness_cfg, graph):
        // For each instruction, add edges between live registers and those written
        handle_instr((live_after_instr, instr)):
            _, updated_regs = regs_used_and_written(instr)

            handle_livereg(l):
                if instr is Mov(_, src, _) and src == l:
                    // Do nothing for moves where the source is the same as the live reg
                    return
                else:
                    handle_update(u):
                        if u != l and l in graph and u in graph:
                            add_edge(graph, l, u)
                    for u in updated_regs:
                        handle_update(u)

            for l in live_after_instr:
                handle_livereg(l)

        // Flatten all annotated instructions from all blocks
        all_instructions = []
        for (_, blk) in liveness_cfg.basic_blocks:
            all_instructions += blk.instructions

        for pair in all_instructions:
            handle_instr(pair)

    build_interference_graph(ctx, aliased_pseudos, instructions):
        base_graph = mk_base_graph()
        graph = add_pseudo_nodes(aliased_pseudos, base_graph, instructions)
        cfg = instructions_to_cfg(ctx, instructions)
        liveness_cfg = LivenessAnalysis.analyze(cfg)
        add_edges(liveness_cfg, graph)
        return graph

    add_spill_costs(graph, instructions):
        // given map from pseudo names to counts, increment map entry for pseudo, or set to 1 if not already present
        incr_count(counts, pseudo):
            if pseudo in counts:
                counts[pseudo] = counts[pseudo] + 1
            else:
                counts[pseudo] = 1
            return counts

        // Get list of all operands in the function, filter out all but pseudoregs
        operands = concat_map(get_operands, instructions)
        get_pseudo(op):
            if op is Pseudo(r):
                return r
            else:
                return None
        pseudos = filter_map(get_pseudo, operands)

        // Create map from pseudoregs to counts
        count_map = {}
        for pseudo in pseudos:
            count_map = incr_count(count_map, pseudo)

        // Set each node's spill cost to the count from count_map
        set_spill_cost(nd):
            if nd.id is Pseudo(r):
                nd.spill_cost = float(count_map.get(r, 0))
            return nd

        return map_values(set_spill_cost, graph)

    color_graph(ctx, graph):
        // While there are unpruned nodes, prune and color the graph recursively
        remaining = [nd for nd in graph.values() if not nd.pruned]
        if remaining is empty:
            // we've pruned the whole graph, so we're done
            return graph
        else:
            not_pruned(nd_id) = not graph[nd_id].pruned

            // Compute degree (number of unpruned neighbors)
            degree(nd):
                unpruned_neighbors = [n for n in nd.neighbors if not_pruned(n)]
                return length(unpruned_neighbors)

            // Find a node with degree < k, or pick a spill candidate
            is_low_degree(nd) = degree(nd) < k

            next_node = find(remaining, is_low_degree)
            if next_node is None:
                // Need to choose a spill candidate!
                spill_metric(nd) = nd.spill_cost / degree(nd)
                cmp(nd1, nd2) = spill_metric(nd1) < spill_metric(nd2)
                spilled = min(remaining, key=spill_metric)

                // Debug printing
                if R.debug_settings().spill_info and is_dump_target(ctx):
                    print("================================")
                    for nd in sorted(remaining, key=spill_metric):
                        print(f"Node {show_node_id(nd.id)} has degree {degree(nd)}, spill cost {nd.spill_cost}, and spill metric {spill_metric(nd)}")
                    print(f"Spill candidate: {show_node_id(spilled.id)}")
                next_node = spilled

            // Prune the selected node
            pruned_graph = copy(graph)
            pruned_graph[next_node.id].pruned = True

            partly_colored = color_graph(ctx, pruned_graph)

            all_colors = range(0, k)
            // Remove colors used by neighbors
            remove_neighbor_color(neighbor_id, remaining_colors):
                neighbor_nd = partly_colored[neighbor_id]
                if neighbor_nd.color is not None:
                    return [c for c in remaining_colors if c != neighbor_nd.color]
                else:
                    return remaining_colors

            available_colors = all_colors
            for neighbor_id in next_node.neighbors:
                available_colors = remove_neighbor_color(neighbor_id, available_colors)

            if available_colors is empty:
                // no available colors, leave this node uncolored
                return partly_colored
            else:
                // If this is a callee-saved reg, give it the highest-numbered color; otherwise give it the lowest
                if next_node.id is Reg(r) and r not in R.caller_saved_regs:
                    c = max(available_colors)
                else:
                    c = min(available_colors)
                partly_colored[next_node.id].pruned = False
                partly_colored[next_node.id].color = c
                return partly_colored

    make_register_map(ctx, graph):
        // First, build map from colors to hardregs
        add_color(nd_id, node, color_map):
            if nd_id is Reg(r):
                color_map[node.color] = r
            return color_map

        colors_to_regs = {}
        for nd_id, node in graph.items():
            colors_to_regs = add_color(nd_id, node, colors_to_regs)

        // Then, build map from pseudoregisters to hard registers
        add_mapping(nd, (used_callee_saved, reg_map)):
            if nd.id is Pseudo(p) and nd.color is not None:
                hardreg = colors_to_regs[nd.color]
                if hardreg in R.caller_saved_regs:
                    // Do not add to used_callee_saved
                    pass
                else:
                    used_callee_saved.add(hardreg)
                reg_map[p] = hardreg
            return (used_callee_saved, reg_map)

        used_callee_saved = set()
        reg_map = {}
        for nd in graph.values():
            used_callee_saved, reg_map = add_mapping(nd, (used_callee_saved, reg_map))

        fn_name = ctx.fun_name
        add_callee_saved_regs_used(fn_name, used_callee_saved)
        return reg_map

    replace_pseudoregs(instructions, reg_map):
        // Replace all pseudo operands with assigned hardregs
        f(op):
            if op is Pseudo(p):
                if p in reg_map:
                    return Reg(reg_map[p])
                else:
                    return op
            else:
                return op

        replaced = map(lambda instr: replace_ops(f, instr), instructions)
        return cleanup_movs(replaced)

    allocate(ctx, aliased_pseudos, instructions):
        graph = build_interference_graph(ctx, aliased_pseudos, instructions)
        graph_with_spill_costs = add_spill_costs(graph, instructions)
        colored_graph = color_graph(ctx, graph_with_spill_costs)
        reg_map = make_register_map(ctx, colored_graph)
        return replace_pseudoregs(instructions, reg_map)
```

```
GP = Allocator({
    suffix = "gp",
    all_hardregs = [AX, BX, CX, DX, DI, SI, R8, R9, R12, R13, R14, R15],
    caller_saved_regs = [AX, CX, DX, DI, SI, R8, R9],
    pseudo_is_current_type = (p) => get_type(p) != Double,
    debug_settings = () => debug_gp_regalloc
})

XMM = Allocator({
    suffix = "xmm",
    all_hardregs = [XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13],
    caller_saved_regs = all_hardregs,
    pseudo_is_current_type = (p) => get_type(p) == Double,
    debug_settings = () => debug_xmm_regalloc
})
```

```
allocateRegisters(src_file, aliased_pseudos, Program(toplevels)):
    allocate_regs_for_fun(fn_name, instructions):
        instructions = GP.allocate(fn_name, aliased_pseudos, instructions)
        instructions = XMM.allocate(fn_name, aliased_pseudos, instructions)
        return instructions

    alloc_in_tl(tl):
        if tl is Function(f):
            ctx = { filename: src_file, fun_name: f.name, params: [] }
            f.instructions = allocate_regs_for_fun(ctx, f.instructions)
            return Function(f)
        else:
            return tl

    return Program(map(alloc_in_tl, toplevels))
```

## ReplacePseudo

```
// Add a case for Pop
// Update case for Movsx, MovZeroExtend
replace_pseudos_in_instruction(Assembly.Instruction inst, state):
    match inst.type:
        case Mov:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_mov = Mov(inst.t, new_src, new_dst)
            return (state2, new_mov)

        case Movsx:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_movsx = Movsx(inst.src_type, inst.dst_type, new_src, new_dst)
            return (state2, new_movsx)

        case MovZeroExtend:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_movzx = MovZeroExtend(inst.src_type, inst.dst_type, new_src, new_dst)
            return (state2, new_movzx)

        case Lea:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_lea = Lea(new_src, new_dst)
            return (state2, new_lea)

        case Unary:
            state1, new_dst = replace_operand(inst.dst, state)
            new_unary = Unary(inst.op, inst.t, new_dst)
            return (state1, new_unary)

        case Binary:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_binary = Binary(inst.op, inst.t, new_src, new_dst)
            return (state2, new_binary)

        case Cmp:
            state1, new_op1 = replace_operand(inst.src, state)
            state2, new_op2 = replace_operand(inst.dst, state1)
            new_cmp = Cmp(inst.t, new_op1, new_op2)
            return (state2, new_cmp)

        case Idiv:
            state1, new_operand = replace_operand(inst.operand, state)
            new_idiv = Idiv(inst.t, new_operand)
            return (state1, new_idiv)

        case Div:
            state1, new_operand = replace_operand(inst.operand, state)
            new_div = Div(inst.t, new_operand)
            return (state1, new_div)

        case SetCC:
            state1, new_operand = replace_operand(inst.operand, state)
            new_setcc = SetCC(inst.cond_code, new_operand)
            return (state1, new_setcc)

        case Push:
            /* Note: ensure the operand is replaced correctly (argument order same as elsewhere) */
            state1, new_op = replace_operand(inst.op, state)
            new_push = Push(new_op)
            return (state1, new_push)

        case Cvttsd2si:
            state1, new_src = replace_operand(state, inst.src)
            state2, new_dst = replace_operand(state1, inst.dst)
            new_cvt = Cvttsd2si(inst.t, new_src, new_dst)
            return (state2, new_cvt)
        case Cvtsi2sd:
            state1, new_src = replace_operand()

        /* For instructions that do not require operand replacement, simply pass them through */
        case Ret:
        case Cdq:
        case Label:
        case JmpCC:
        case Jmp:
        case Call:
            return (state, inst)
        case Pop:
            fail("Internal error")
```

## Emit

```
// Add BX, R12, R13, R14, and R15
show_long_reg(reg):
    match reg:
        case AX: return "%eax"
        case BX: return "%ebx"
        case CX: return "%ecx"
        case DX: return "%edx"
        case DI: return "%edi"
        case SI: return "%esi"
        case R8: return "%r8d"
        case R9: return "%r9d"
        case R10: return "%r10d"
        case R11: return "%r11d"
        case R12: return "%r12d"
        case R13: return "%r13d"
        case R14: return "%r14d"
        case R15: return "%r15d"
        case SP: fail("Internal error: no 32-bit RSP")
        case BP: fail("Internal error: no 32-bit RBP")
        default:
            fail("Internal error: can't store longword in XMM register")
```

```
// Add BX, R12, R13, R14, and R15
show_quadword_reg(reg):
    match reg:
        case AX: return "%rax"
        case BX: return "%rbx"
        case CX: return "%rcx"
        case DX: return "%rdx"
        case DI: return "%rdi"
        case SI: return "%rsi"
        case R8: return "%r8"
        case R9: return "%r9"
        case R10: return "%r10"
        case R11: return "%r11"
        case R12: return "%r12"
        case R13: return "%r13"
        case R14: return "%r14"
        case R15: return "%r15"
        case SP: return "%rsp"
        case BP: return "%rbp"
        default:
            fail("Internal error: can't store quadword type in XMM register")
```

```
// Add xmm8, xmm9, xmm10, xmm11, xmm12, xmm13
show_double_reg(reg):
    match reg:
        case XMM0: return "%xmm0"
        case XMM1: return "%xmm1"
        case XMM2: return "%xmm2"
        case XMM3: return "%xmm3"
        case XMM4: return "%xmm4"
        case XMM5: return "%xmm5"
        case XMM6: return "%xmm6"
        case XMM7: return "%xmm7"
        case XMM8: return "%xmm8"
        case XMM9: return "%xmm9"
        case XMM10: return "%xmm10"
        case XMM11: return "%xmm11"
        case XMM12: return "%xmm12"
        case XMM13: return "%xmm13"
        case XMM14: return "%xmm14"
        case XMM15: return "%xmm15"
        default:
            fail("Internal error: can't store double type in general-purpose register")
```

```
// Add BX, R12, R13, R14, and R15
show_byte_reg(reg):
     match reg:
        case AX: return "%al"
        case BX: return "%bl"
        case CX: return "%cl"
        case DX: return "%dl"
        case DI: return "%dil"
        case SI: return "%sil"
        case R8: return "%r8b"
        case R9: return "%r9b"
        case R10: return "%r10b"
        case R11: return "%r11b"
        case R12: return "%r12b"
        case R13: return "%r13b"
        case R14: return "%r14b"
        case R15: return "%r15b"
        case SP: fail("Internal error: no one-byte RSP")
        case BP: fail("Internal error: no one-byte RBP")
        default:
            fail("Internal error: can't store byte type in XMM register")
```

```
// add case for Pop after Push
emit_instruction(inst):
    match inst.type:
        case Mov(t, src, dst):
            return "\tmov{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Unary(op, t, dst):
            return "\t{show_unary_operator(op)}{suffix(t)} {show_operand(t, dst)}\n"

        case Binary(op, t, src, dst):
            // special logic: emit CX reg as %cl
            if op is Sal or Sar or Shl or Shr:
                return "\t{show_binary_operator(op)}{suffix(t)} {show_byte_operand(src)}, {show_operand(t, dst)}\n"
            else if op is Xor and t is Double:
                return "\txorpd {show_operand(Double, src)}, {show_operand(Double, dst)}\n"
            else if op is Mult and t is Double:
                return "\tmulsd {show_operand(Double, src)}, {show_operand(Double, dst)}\n"
            else:
                return "\t{show_binary_operator(op)}{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Cmp(t, src, dst):
            if t is Double:
                return "\tcomisd {show_operand(Double, src)}, {show_operand(Double, dst)}\n"
            else:
                return "\tcmp{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Idiv(t, operand):
            return "\tidiv{suffix(t)} {show_operand(t, operand)}\n"

        case Div(t, operand):
            return "\tdiv{suffix(t)} {show_operand(t, operand)}\n"

        case Lea(src, dst):
            return "\tleaq {show_operand(Quadword, src)}, {show_operand_Quadword, dst}\n"

        case Cdq(t):
            if t is Longword:
                return "\tcdq\n"
            else if t is Quadword:
                return "\tcdo\n"
            else:
                fail("Internal error: can't apply cdq to a byte or non-integer type)
        case Jmp(lbl):
            return "\tjmp ${show_local_label(lbl)}\n"

        case JmpCC(code, lbl):
            return "\tj{show_cond_code(code)} {show_local_label(lbl)}\n"

        case SetCC(code, operand):
            return "\tset{show_cond_code(code)} {show_byte_operand(operand)}\n"

        case Label(lbl):
            return "{show_local_label(lbl)}:\n"

        case Push(operand):
            try:
                return "\tpushq {show_operand(Quadword, operand)}\n"
            catch:
                // For intermediate/debug output only
                return "\tpushq {show_operand(Double, operand)}\n"

        case Pop(r):
            return "\tpopq {show_quadword_reg(r)}\n"

        case Call(f):
            return "\tcall {show_fun_name(f)}\n"

        case Movsx(src_type, dst_type, src, dst):
            return "\tmovs{suffix(src_type)}{suffix(dst_type)} {show_operand(src_type, src)}, {show_operand(dst_type, dst)}\n"

        case MovZeroExtend(src_type, dst_type, src, dst):
            return "\tmovz{suffix(src_type)}{suffix(dst_type)} {show_operand(src_type, src)}, {show_operand(dst_type, dst)}\n"

        case Cvtsi2sd(t, src, dst):
            return "\tcvtsi2sd{suffix(t)} {show_operand(t, src)}, {show_operand(Double, dst)}\n"

        case Cvttsd2si(t, src, dst):
            return "\tcvttsd2si{suffix(t)} {show_operand(Double, src)}, {show_operand(t, dst)}\n"

        case Ret:
            return """
                movq %rbp, %rsp
                pop %tbp
                ret
            """
        default:
            fail("Internal error: unknown instruction")
```

## Output

```C
// Each variable below should be assigned a register

int add(int a, int b) {
    int c = a + b;  // c: sum of inputs
    int d = c * 2;  // d: double the sum
    int e = d - a;  // e: difference between double-sum and first input
    int f = e + b;  // f: add second input to previous result
    int g = f * c;  // g: multiply previous result by sum

    // Register coalescing test: chain of assignments
    int h = g;      // h should ideally share register with g
    int i = h;      // i should ideally share register with h/g
    int j = i + 1;  // j depends on i, coalescing possible

    // More coalescing: copy propagation
    int k = j;
    int l = k;
    int m = l * 2;

    // Return a value that depends on the coalesced chain
    return m;
}

int main(void) {
    int x = 5;
    int y = 7;
    int result = add(x, y);
    return 0;
}
```

To x64 Assembly on Linux:

```asm
	.global add
add:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$8, %rsp
	pushq	%rbx
	movl	%edi, %edx
	movl	%esi, %eax
	movl	%edx, %ecx
	addl	%eax, %ecx
	movl	%ecx, %ebx
	imull	$2, %ebx
	subl	%edx, %ebx
	addl	%eax, %ebx
	movl	%ebx, %eax
	imull	%ecx, %eax
	addl	$1, %eax
	imull	$2, %eax
	popq	%rbx
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$0, %rsp
	movl	$5, %edi
	movl	$7, %esi
	call	add
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```

---

# Phase 2 (Register Allocator with Coalescing)

---

## Compiler Driver

```
// spill_info is renamed to debug_msg
Settings = {
    --snip--
    spill_info: bool;
    interference_ncol: bool;
    interference_graphviz: bool;
    liveness: bool;
}
```

## DisjointSets

```
// Create an empty disjoint-set map.
init():
    return {}
```

```
/*
Return a new map where x’s parent is set to y.
disj_sets: current parent map
x, y:      elements to link (x → y)
copy-on-write: build a fresh map with the new binding
*/
union(disj_sets, x, y):
    new_map = copy(disj_sets)
    new_map[x] = y
    return new_map
```

```
// Recursively follow parent pointers until reaching a root.
// If x is not in disj_sets, it’s its own root.
find(disj_sets, x):
    if x not in disj_sets:
        return x
    else:
        parent = disj_sets[x]
        return find(disj_sets, parent)
```

```
// True if no links have been made yet.
is_empty(disj_sets):
    return size(disj_sets) == 0
```

-- Optional: pure functional find with path compression --

```
// Find with path compression in pure functional style.
// Returns (root, new_map).
find_pc(disj_sets, x):
    if x not in disj_sets:
        // x is its own root; no change to the map
        return (x, disj_sets)
    parent = disj_sets[x]
    (root, updated_map) = find_pc(disj_sets, parent)
    // redirect x straight to root in the updated map
    new_map = copy(updated_map)
    new_map[x] = root
    return (root, new_map)
```

## RegAlloc

```
// updated comment, and pop case message
// map function f over all the operands in an instruction
replace_ops(f, instr):
    match instr:
        case Mov(t, src, dst): return Mov(t, f(src), f(dst))
        case Movsx(i): return Movsx({ ...i, src: f(i.src), dst: f(i.dst) })
        case MovZeroExtend(zx): return MovZeroExtend({ ...zx, src: f(zx.src), dst: f(zx.dst) })
        case Lea(src, dst): return Lea(f(src), f(dst))
        case Cvttsd2si(t, src, dst): return Cvttsd2si(t, f(src), f(dst))
        case Cvtsi2sd(t, src, dst): return Cvtsi2sd(t, f(src), f(dst))
        case Unary(op, t, operand): return Unary(op, t, f(operand))
        case Binary(b): return Binary({ ...b, src: f(b.src), dst: f(b.dst) })
        case Cmp(code, v1, v2): return Cmp(code, f(v1), f(v2))
        case Idiv(t, op): return Idiv(t, f(op))
        case Div(t, op): return Div(t, f(op))
        case SetCC(code, op): return SetCC(code, f(op))
        case Push(op): return Push(f(op))
        case Label(_), Call(_), Ret, Cdq(_), Jmp(_), JmpCC(_): return instr
        case Pop(_): fail("Shouldn't use this yet")
```

```
// NEW
get_node_by_id(graph, node_id):
    return graph[node_id]
```

```
// NEW
remove_edge(g, nd_id1, nd_id2):
    nd1 = get_node_by_id(g, nd_id1)
    nd2 = get_node_by_id(g, nd_id2)

    // won’t raise error if the neighbor isn’t present
    nd1.neighbors.remove(nd_id2)
    nd2.neighbors.remove(nd_id1)
```

```
// NEW
degree(graph, nd_id):
    nd = get_node_by_id(graph, nd_id)
    return length(nd.neighbors)
```

```
// NEW
are_neighbors(g, nd_id1, nd_id2):
    nd1 = g[nd_id1]

    if n1.neighbors has nd_id2:
        return true
    else:
        return false
```

```
// NEW
george_test(graph, hardreg, pseudo):
    pseudoreg_neighbors = get_node_by_id(graph, pseudo).neighbors

    for pseudo_nghbor in pseudoneighbors:
        /*
            a neighbor of the pseudo won't interfere with coalescing
            if it has insignificant degree or it already interferes with hardreg
        */
        ok = are_neighbors(graph, pseudo_nghbor, hardreg)
                  OR degree(graph, pseudo_nghbor) < k;

        if (!ok):
            return false;

    return true;
```

```
// NEW
briggs_test(graph, x, y):
    x_nd = get_node_by_id(graph, x)
    y_nd = get_node_by_id(graph, y)
    neighbors = [...x_nd.neighbors, ...y_nd.neighbors]

    significant_count = 0
    for nghborId in neighbors:
        deg = degree(graph, nghborId)

        if are_neighbors(graph, x, y) AND are_neighbors(graph, y, nghborId):
            deg -= 1

        if def >= k:
            significant_count += 1
            if significant_count >= k:
                return false

    return true
```

```
// NEW
conservative_coalescable(graph, src, dst):
    if briggs_test(graph, src, dst):
        return true

    if src is Reg:
        return george_test(graph, src, dst)

    if dst is Reg:
        return george_test(graph, dst, src)

    return false
```

```
// NEW
debug output
print_coalesce_msg(ctx, src, dst):
    if R.debug_settings().debug_msg AND Debug.is_dump_target(ctx):
        print("Coalescing {show_node_id(src)} into {show_node_id(dst)}\n")
```

```
// NEW
update_graph(ctx, g, to_merge, to_keep):
    print_coalesce_msg(ctx, to_merge, to_keep)

    for nghbor_id in get_node_by_id(g, to_merge).neighbors:
        // update neighbor
        add_edge(g, nghbor_id, to_keep)
        remove_edge(g, nghbor_id, to_merge)

    g.remove(to_merge)
```

```
// NEW
coalesce(ctx, graph, insts):
    if R.debug_settings().debug_msg AND Debug.is_dump_target(ctx):
        print("Coalescing round\n=============\n")

    reg_map = DisjointSets.init()
    for inst in insts:
        if inst is Move(_, src, dst):
            src' = DisjointSets.find(reg_map, src)
            dst' = DisjointSets.find(reg_map, dst)

            if (
                graph has src'
                AND g has dst'
                AND src' != dst'
                AND not are_neighbors(g, src', dst')
                AND conservative_coalescable(g, src', dst')
            ):
                if src' is Reg:
                    update_graph(ctx, g, to_merge=dst', to_keep=src')
                    DisjoinSets.union(reg_map, dst', src')
                else:
                    update_graph(ctx, g, to_merge=src', to_keep=dst')
                    DisjoinSets.union(reg_map, src', dst')
            else:
                // do nothing
        else:
            /// do nothing

    return reg_map
```

```
// NEW
rewrite_coalesced(insts, coalesced_regs):
    new_insts = []

    f = (r):
        return DisjointSets.find(coalesced_regs, r)

    for inst in insts:
        if inst is Mov(t, src, dst):
            new_src = f(src)
            new_dst = f(dst)

            if new_src == new_dst:
                // do nothing
            else:
                new_insts.append(Mov(t, new_src, new_dst))
        else:
            new_insts.append(replace_ops(f, inst))
```

```
// Move the build_interference_graph and add coalescing logic
allocate(ctx, aliased_pseudos, insts):
    curr_insts = [...insts]
    while true:
        graph = build_interference_graph(ctx, aliased_pseudos, curr_insts)

        DumpGraph.dump_graphviz(ctx, graph)
        DumpGraph.dump_ncol(ctx, graph)

        coalesced_regs = coalesce(ctx, graph, curr_insts)
        if DisjoinSets.is_empty(coalesced_regs):
            break
        else:
            curr_insts = rewrite_coalesced(curr_insts, coalesced_regs)

    coalesced_insts = curr_insts
    coalesced_graph = graph

    graph_with_spill_costs = add_spill_costs(coalesced_graph, coalesced_insts)
    colored_graph = color_graph(ctx, graph_with_spill_costs)
    register_map = make_register_map(ctx, colored_graph)
    return replace_pseudoregs(coalesced_insts, register_map)
```

## Output

From C:

```C
// Test that we can use coalescing to prevent spills. In particular,
// this tests that we rebuild the interference graph after each coalescing
// loop, allowing us to find new coalescing opportunities and ultimately
// color an up-to-date graph.
int glob = 5;
int flag = 0;

int validate(int a, int b, int c, int d, int e, int f, int g, int h, int i,
             int j, int k, int l, int m);

int target(int arg) {
    // declare some pseudoregisters but don't initialize them yet
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int h;
    int i;
    int j;
    int k;
    int l;
    int m;
    // a-m all appear to conflict, which would require us to spill one of them,
    // but they actually have the same value.
    if (flag) {
        // This branch isn't taken; only included to prevent copy propagation.
        // It creates conflict between a-m, which will go away as we perform coalescing.
        // the same value
        a = arg;
        b = arg;
        c = arg;
        d = arg;
        e = arg;
        f = arg;
        g = arg;
        h = arg;
        i = arg;
        j = arg;
        k = arg;
        l = arg;
        m = arg;
    } else {
        // This branch creates conflicts between a-m, which will go away as we perform coalescing.
        a = glob * 2;  // 10
        b = a;
        c = a;
        d = a;
        e = a;
        f = a;
        g = a;
        h = a;
        i = a;
        j = a;
        k = a;
        l = a;
        m = a;
    }
    // We'll first coalesce a-f into param-passing registers.
    // After rebuilding interference graph, we'll recognize that g-m don't
    // conflict with these registers despite originally conflicting with a-f,
    // so we'll be able to coalesce them into DI.
    return validate(a, b, c, d, e, f, g, h, i, j, k, l, m);
}
```

Without Coalescing
To x64 Assembly on Linux:

```asm
	.global glob
	.data
	.align 4
glob:
	.long 5

	.global flag
	.bss
	.align 4
flag:
	.zero 4

	.global target
target:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$8, %rsp
	pushq	%rbx
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	movl	%edi, %eax
	cmpl	$0, flag(%rip)
	je	.Lif_else.27
	movl	%eax, %ebx
	movl	%eax, %edi
	movl	%eax, %r15d
	movl	%eax, %r12d
	movl	%eax, %esi
	movl	%eax, %ecx
	movl	%eax, -4(%rbp)
	movl	%eax, %edx
	movl	%eax, %r8d
	movl	%eax, %r9d
	movl	%eax, %r14d
	movl	%eax, %r13d
	jmp	.Lif_end.28
.Lif_else.27:
	movl	glob(%rip), %eax
	imull	$2, %eax
	movl	%eax, %ebx
	movl	%eax, %edi
	movl	%eax, %r15d
	movl	%eax, %r12d
	movl	%eax, %esi
	movl	%eax, %ecx
	movl	%eax, -4(%rbp)
	movl	%eax, %edx
	movl	%eax, %r8d
	movl	%eax, %r9d
	movl	%eax, %r14d
	movl	%eax, %r13d
.Lif_end.28:
	subq	$8, %rsp
	movl	%ebx, %edi
	movl	%edi, %esi
	movl	%r15d, %edx
	movl	%r12d, %ecx
	movl	%esi, %r8d
	movl	%ecx, %r9d
	movl	%r13d, %eax
	pushq	%rax
	movl	%r14d, %eax
	pushq	%rax
	movl	%r9d, %eax
	pushq	%rax
	movl	%r8d, %eax
	pushq	%rax
	movl	%edx, %eax
	pushq	%rax
	movl	-4(%rbp), %eax
	pushq	%rax
	pushq	%rax
	call	validate@PLT
	addq	$64, %rsp
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbx
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```

With Coalescing
To x64 Assembly on Linux:

```asm
	.global glob
	.data
	.align 4
glob:
	.long 5

	.global flag
	.bss
	.align 4
flag:
	.zero 4

	.global target
target:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$0, %rsp
	cmpl	$0, flag(%rip)
	je	.Lif_else.27
	movl	%edi, %esi
	movl	%edi, %edx
	movl	%edi, %ecx
	movl	%edi, %r8d
	movl	%edi, %r9d
	movl	%edi, %eax
	jmp	.Lif_end.28
.Lif_else.27:
	movl	glob(%rip), %edi
	imull	$2, %edi
	movl	%edi, %esi
	movl	%edi, %edx
	movl	%edi, %ecx
	movl	%edi, %r8d
	movl	%edi, %r9d
	movl	%edi, %eax
.Lif_end.28:
	subq	$8, %rsp
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	movl	%edi, %eax
	pushq	%rax
	call	validate@PLT
	addq	$64, %rsp
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
