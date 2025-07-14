# Table of Contents

- [Optimizations Folder Structure](#optimizations-folder-structure)
- [Compiler Driver](#compiler-driver)
  - [Main driver functions](#main-driver-functions)
- [CFG](#cfg)
- [AddressTaken](#addresstaken)
- [Optimize](#optimize)
- [OptimizeUtils](#optimizeutils)
- [ConstantFolding](#constantfolding)
- [UnreachableCodeElimination](#unreachablecodeelimination)
- [CopyPropagation](#copypropagation)
- [DeadStoreElimination](#deadstoreelimination)
- [TACKY](#tacky)
- [CodeGen](#codegen)
- [Output](#output)

We add a new folder Optimizations:

- Optimize
- OptimizeUtils
- ConstantFolding
- UnreachableCodeElimination
- CopyPropagation
- DeadStoreElimination

# Compiler Driver

Update the compiler main to accept flags for different
optimization options

## Main driver functions

```
// add optimizations
compile(stage, optimizations, preprocessed_src):
    call Compiler.compile(stage, optimizations, preprocessed_src)
    run_command('rm', [preprocessed_src]) // Remove the preprocessed source file
    return replace_exension(preprocessed_src, '.s')
```

```
// add optimizations
driver(target, debug, stage, optimizations, src):
    set platform to target (OX_X | Linux)
    preprocessed_name = preprocess(src)
    asm_name = compile(stage, optimizations, preprocessed_name)
    if stage === Executable
        assemble_and_link(asm_name, true) // cleanup should be false for debugging
```

```
Settings = {
    --snip--
    constant_folding: boolean,
    dead_store_elimination boolean,
    copy_propagation boolean,
    unreachable_code_elimination boolean,
}

set_options(inputs):
    if inputs.optimize is true:
        all optimization options are set to true
    else:
        Settings.constant_folding = inputs.fold_constants
        Settings.dead_store_elimination = inputs.eliminate_dead_stores
        Settings.copy_propagation = inputs.propagate_copies
        Settings.unreachable_code_elimination = inputs.eliminate_unreachable_code

    return Settings
```

# CFG

We create a new file, called CFG, at the same level as our main files.

```
class SimpleInstrKind:
    LABEL = "Label"
    CJUMP = "ConditionalJump"
    UJUMP = "UnconditionalJump"
    RETURN = "Return"
    OTHER = "Other"

class SimpleInstr:
    init(kind, target = None):
        this.kind   = kind
        this.target = target

class NodeId:
    init(kind, idx = None):
        this.kind = kind
        this.idx  = idx

    Entry():
        return new NodeId("Entry")

    Exit():
        return new NodeId("Exit")

    Block(n):
        return new NodeId("Block", n)

class BasicBlock:
    init(node_id, instructions, value):
        // instructions is list of (annotation, instr)
        this.id           = node_id
        this.instructions = instructions
        this.value        = value
        this.preds        = []
        this.succs        = []

class Cfg:
    init(simplify, pp_instr, debug_label = ""):
        // simplify: Instr -> SimpleInstr
        // pp_instr: (Instr, annotation) -> string
        this.simplify    = simplify
        this.pp_instr    = pp_instr
        this.debug_label = debug_label
        this.basic_blocks = {}      // map block_index -> BasicBlock
        this.entry_succs  = []      // successors of Entry
        this.exit_preds   = []      // predecessors of Exit

    get_succs(node):
        if node.kind == "Entry":
            return this.entry_succs
        if node.kind == "Exit":
            return []
        return this.basic_blocks[node.idx].succs

    get_block_value(idx):
        return this.basic_blocks[idx].value

    update_successors(node, f):
        if node.kind == "Entry":
            this.entry_succs = f(this.entry_succs)
        elif node.kind == "Block":
            this.basic_blocks[node.idx].succs = f(this.basic_blocks[node.idx].succs)
        else:
            error("cannot update successors of Exit")

    update_predecessors(node, f):
        if node.kind == "Block":
            this.basic_blocks[node.idx].preds = f(this.basic_blocks[node.idx].preds)
        elif node.kind == "Exit":
            this.exit_preds = f(this.exit_preds)
        else:
            error("cannot update predecessors of Entry")

    add_edge(pred, succ):
        // add succ to pred.succs if missing
        this.update_successors(pred,
            lambda ss: ss + [succ] if succ not in ss else ss)
        // add pred to succ.preds if missing
        this.update_predecessors(succ,
            lambda ps: ps + [pred] if pred not in ps else ps)

    remove_edge(pred, succ):
        this.update_successors(pred,
            lambda ss: [s for s in ss if s != succ])
        this.update_predecessors(succ,
            lambda ps: [p for p in ps if p != pred])

    partition_into_basic_blocks(instructions):
        finished_blocks = []
        current_block   = []

        for instr in instructions:
            simp = this.simplify(instr)

            if simp.kind == SimpleInstrKind.LABEL:
                if current_block ≠ []:
                    finished_blocks.append(reverse(current_block))
                current_block = [instr]

            elif simp.kind in {SimpleInstrKind.CJUMP,
                               SimpleInstrKind.UJUMP,
                               SimpleInstrKind.RETURN}:
                current_block.append(instr)
                finished_blocks.append(reverse(current_block))
                current_block = []

            else:  // OTHER
                current_block.append(instr)

        if current_block ≠ []:
            finished_blocks.append(reverse(current_block))

        // restore original order of blocks and instructions
        return [reverse(block) for block in reverse(finished_blocks)]

    instructions_to_cfg(instructions):
        // clear any existing blocks/edges
        this.basic_blocks.clear()
        this.entry_succs.clear()
        this.exit_preds.clear()

        blocks = this.partition_into_basic_blocks(instructions)
        for idx, instrs in enumerate(blocks):
            annotated = [(None, instr) for instr in instrs]
            bb = new BasicBlock(NodeId.Block(idx), annotated, value = None)
            this.basic_blocks[idx] = bb

        this._add_all_edges()
        return self

    _add_all_edges():
        // map labels to block ids
        label_map = {}
        for idx, bb in this.basic_blocks:
            first_instr = bb.instructions[0][1]
            simp = this.simplify(first_instr)
            if simp.kind == SimpleInstrKind.LABEL:
                label_map[simp.target] = bb.id

        add_edge(NodeId.Entry(), NodeId.Block(0))

        max_idx = max(this.basic_blocks.keys())
        for idx in sorted(this.basic_blocks.keys()):
            last_instr = this.basic_blocks[idx].instructions[-1][1]
            simp = this.simplify(last_instr)
            next_node = NodeId.Exit() if idx == max_idx else NodeId.Block(idx + 1)

            if simp.kind == SimpleInstrKind.RETURN:
                this.add_edge(NodeId.Block(idx), NodeId.Exit())

            elif simp.kind == SimpleInstrKind.UJUMP:
                tgt = label_map[simp.target]
                this.add_edge(NodeId.Block(idx), tgt)

            elif simp.kind == SimpleInstrKind.CJUMP:
                tgt = label_map[simp.target]
                this.add_edge(NodeId.Block(idx), next_node)
                this.add_edge(NodeId.Block(idx), tgt)

            else:  // OTHER
                this.add_edge(NodeId.Block(idx), next_node)

    cfg_to_instructions():
        result = []
        for idx in sorted(this.basic_blocks.keys()):
            for (_, instr) in this.basic_blocks[idx].instructions:
                result.append(instr)
        return result

    initialize_annotation(dummy):
        for bb in this.basic_blocks.values():
            bb.value = dummy
            for pair in bb.instructions:
                pair.annotation = dummy

        return self

    strip_annotations():
        return this.initialize_annotation(None)

    print_graphviz(out_dot = None):
        dotname = out_dot or (this.debug_label + ".dot")
        pngname = replace_extension(dotname, ".png")

        open dotfile for write as f:
            write "digraph {"
            write node declarations for entry and exit

            for idx, bb in sorted(this.basic_blocks):
                write node block idx with HTML table:
                    row: block idx
                    one row per (val, instr): pp_instr(instr, val)
                    row: bb.value

            for pred in all nodes:
                for succ in pred.succs:
                    write "pred -> succ"

            write "}"
        run "dot -Tpng dotname -o pngname"
        delete dotname
```

# AddressTaken

```
analyze(instrs):
    // helper to detect if an instruction takes the address of a variable
    addr_taken(instr):
        switch instr:
            case Tacky.GetAddress { src = Var(v), _ }:
                return Some(v)
            else:
                return None

    vars = instrs
           .map(addr_taken)
           .filter_map(identity)  // discard None
    return StringSet.of_list(vars)
```

# Optimize

```
optimize_fun(debug_label, opts, instructions):
    if instructions is empty:
        return []

    // 1) find alias‐takers for copy/DS elimination
    aliased_vars = AddressTaken.analyze(instructions)

    // 2) constant folding
    if opts.constant_folding:
        folded = Constant_folding.optimize(debug_label, instructions)
    else:
        folded = instructions

    // 3) build CFG over folded code
    cfg = TackyCfg.instructions_to_cfg(debug_label, folded)

    // 4) unreachable‐code elimination
    if opts.unreachable_code_elimination:
        cfg1 = Unreachable_code_elim.optimize(cfg)
    else:
        cfg1 = cfg

    // 5) copy propagation
    if opts.copy_propagation:
        cfg2 = Copy_prop.optimize(aliased_vars, cfg1)
    else:
        cfg2 = cfg1

    // 6) dead‐store elimination
    if opts.dead_store_elimination:
        cfg3 = Dead_store_elim.optimize(aliased_vars, cfg2)
    else:
        cfg3 = cfg2

    // 7) back to linear instructions
    optimized = TackyCfg.cfg_to_instructions(cfg3)

    // 8) repeat until fixed point
    if equal_lists(instructions, optimized, eq=Tacky.equal_instruction):
        return optimized
    else:
        return optimize_fun(debug_label, opts, optimized)


optimize(opts, src_file, program):
    // Apply optimize_fun to each function body in the TACKY program
    new_toplevels = []
    for tl in program.toplevels:
        if tl is Function(name, global_flag, params, body):
            base = remove_extension(src_file)
            debug_label = base + "_" + name
            new_body = optimize_fun(debug_label, opts, body)
            new_toplevels.append(
                Function(name, global_flag, params, new_body)
            )
        else:
            // leave static vars/constants untouched
            new_toplevels.append(tl)

    return Program(new_toplevels)
```

# OptimizeUtils

```
get_dst(instr):
    switch instr:
        case Tacky.Copy { dst, _ }:
            return Some(dst)
        case Tacky.FunCall { dst, _ }:
            // dst is already an option
            return dst
        case Tacky.Unary { dst, _ }:
            return Some(dst)
        case Tacky.Binary { dst, _ }:
            return Some(dst)
        case Tacky.SignExtend { dst, _ }:
            return Some(dst)
        case Tacky.ZeroExtend { dst, _ }:
            return Some(dst)
        case Tacky.DoubleToInt { dst, _ }:
            return Some(dst)
        case Tacky.DoubleToUInt { dst, _ }:
            return Some(dst)
        case Tacky.UIntToDouble { dst, _ }:
            return Some(dst)
        case Tacky.IntToDouble { dst, _ }:
            return Some(dst)
        case Tacky.Truncate { dst, _ }:
            return Some(dst)
        case Tacky.GetAddress { dst, _ }:
            return Some(dst)
        case Tacky.Load { dst, _ }:
            return Some(dst)
        case Tacky.AddPtr { dst, _ }:
            return Some(dst)
        case Tacky.CopyToOffset { dst, _ }:
            // dst is a string name, wrap it in Var
            return Some(Var(dst))
        case Tacky.CopyFromOffset { dst, _ }:
            return Some(dst)
        case Tacky.Store _:
            return None
        case Tacky.Return _
           | Tacky.Jump _
           | Tacky.JumpIfZero _
           | Tacky.JumpIfNotZero _
           | Tacky.Label _:
            return None

is_static(var_name):
    attrs = Symbols.get(var_name).attrs
    if attrs is StaticAttr(_):
        return true
    else:
        return false
```

# ConstantFolding

```
evaluate_cast(src_const, dst):
    dst_type       = Tacky.type_of_val(dst)
    try:
        converted_src = Const_convert.const_convert(dst_type, src_const)
    except Overflow or Failure:
        // undefined behavior; cast zero instead
        converted_src = Const_convert.const_convert(dst_type, Const.int_zero)
    return Some( Tacky.Copy { src = Constant(converted_src); dst = dst } )


int_of_bool(b):
    if b then return Const.int_one else return Const.int_zero


# ────── Generic constant‐evaluator interface ──────

interface Evaluatable:
    zero(): t
    compare(x: t, y: t): int
    lognot(x: t): t
    neg(x: t): t
    add(x: t, y: t): t
    sub(x: t, y: t): t
    div(x: t, y: t): t
    mul(x: t, y: t): t
    rem(x: t, y: t): t
    to_const(x: t): Const.t


class ConstEvaluator<E implements Evaluatable>:
    eq(v1, v2):  return E.compare(v1, v2) == 0
    neq(v1, v2): return E.compare(v1, v2) != 0
    gt(v1, v2):  return E.compare(v1, v2) >  0
    ge(v1, v2):  return E.compare(v1, v2) >= 0
    lt(v1, v2):  return E.compare(v1, v2) <  0
    le(v1, v2):  return E.compare(v1, v2) <= 0

    eval_unop(v, op):
        switch op:
            case Tacky.Not:
                return int_of_bool( eq(v, E.zero()) )
            case Tacky.Complement:
                return E.to_const( E.lognot(v) )
            case Tacky.Negate:
                return E.to_const( E.neg(v) )

    eval_binop(v1, v2, op):
        switch op:
            case Tacky.Add:
                return E.to_const( E.add(v1, v2) )
            case Tacky.Subtract:
                return E.to_const( E.sub(v1, v2) )
            case Tacky.Multiply:
                return E.to_const( E.mul(v1, v2) )
            case Tacky.Divide:
                try:
                    return E.to_const( E.div(v1, v2) )
                except Division_by_zero:
                    return E.to_const( E.zero() )
            case Tacky.Remainder:
                try:
                    return E.to_const( E.rem(v1, v2) )
                except Division_by_zero:
                    return E.to_const( E.zero() )
            case Tacky.Equal:
                return int_of_bool( eq(v1, v2) )
            case Tacky.NotEqual:
                return int_of_bool( neq(v1, v2) )
            case Tacky.GreaterThan:
                return int_of_bool( gt(v1, v2) )
            case Tacky.GreaterOrEqual:
                return int_of_bool( ge(v1, v2) )
            case Tacky.LessThan:
                return int_of_bool( lt(v1, v2) )
            case Tacky.LessOrEqual:
                return int_of_bool( le(v1, v2) )


# ────── instantiate evaluators for each numeric kind ──────

IntEvaluator    = ConstEvaluator<Int32>( to_const = Const.ConstInt )
LongEvaluator   = ConstEvaluator<Int64>( to_const = Const.ConstLong )
UIntEvaluator   = ConstEvaluator<UInt32>(
                     neg = (x) ⇒ UInt32.sub(UInt32.zero, x),
                     to_const = Const.ConstUInt )
ULongEvaluator  = ConstEvaluator<UInt64>(
                     neg = (x) ⇒ UInt64.sub(UInt64.zero, x),
                     to_const = Const.ConstULong )
DoubleEvaluator = ConstEvaluator<Float>(
                     lognot = error,  // unsupported
                     rem    = error,  // unsupported
                     to_const = Const.ConstDouble )


evaluate_unop(op, c):
    switch c:
      case Const.ConstInt(i):
        return IntEvaluator.eval_unop(i, op)
      case Const.ConstUInt(u):
        return UIntEvaluator.eval_unop(u, op)
      case Const.ConstLong(l):
        return LongEvaluator.eval_unop(l, op)
      case Const.ConstULong(ul):
        return ULongEvaluator.eval_unop(ul, op)
      case Const.ConstDouble(d):
        return DoubleEvaluator.eval_unop(d, op)
      case Const.ConstChar(c) when op == Tacky.Not:
        return int_of_bool(c == Int8.zero)
      case Const.ConstUChar(uc) when op == Tacky.Not:
        return int_of_bool(uc == UInt8.zero)
      else:
        error("~ and - on chars must be promoted")


evaluate_binop(op, v1, v2):
    switch (v1, v2):
      case (Const.ConstInt(i1), Const.ConstInt(i2)):
        return IntEvaluator.eval_binop(i1, i2, op)
      case (Const.ConstUInt(u1), Const.ConstUInt(u2)):
        return UIntEvaluator.eval_binop(u1, u2, op)
      case (Const.ConstLong(l1), Const.ConstLong(l2)):
        return LongEvaluator.eval_binop(l1, l2, op)
      case (Const.ConstULong(ul1), Const.ConstULong(ul2)):
        return ULongEvaluator.eval_binop(ul1, ul2, op)
      case (Const.ConstDouble(d1), Const.ConstDouble(d2)):
        return DoubleEvaluator.eval_binop(d1, d2, op)
      case (_, _) where both are char‐types:
        error("chars must be integer promoted")
      else:
        error("Internal: misswitched types")


is_zero(c):
    return evaluate_unop(Tacky.Not, c) == Const.ConstInt(Int32.one)


optimize_instruction(instr):
    switch instr:
      case Tacky.Unary { op; src = Constant(c); dst }:
        new_c = evaluate_unop(op, c)
        return Some( Tacky.Copy { src = Constant(new_c); dst } )

      case Tacky.Binary { op; src1 = Constant(c1);
                         src2 = Constant(c2); dst }:
        new_c = evaluate_binop(op, c1, c2)
        return Some( Tacky.Copy { src = Constant(new_c); dst } )

      case Tacky.JumpIfZero(Constant(c), target):
        if is_zero(c) then return Some( Tacky.Jump(target) )
        else              return None

      case Tacky.JumpIfNotZero(Constant(c), target):
        if is_zero(c) then return None
        else              return Some( Tacky.Jump(target) )

      case Tacky.Truncate { src = Constant(c); dst }
         | Tacky.SignExtend { src = Constant(c); dst }
         | Tacky.ZeroExtend { src = Constant(c); dst }
         | Tacky.DoubleToInt { src = Constant(c); dst }
         | Tacky.DoubleToUInt { src = Constant(c); dst }
         | Tacky.IntToDouble { src = Constant(c); dst }
         | Tacky.UIntToDouble { src = Constant(c); dst }
         | Tacky.Copy { src = Constant(c); dst }:
        return evaluate_cast(c, dst)

      else:
        // no folding possible
        return Some(instr)


debug_print(debug_label, instructions):
    filename   = debug_label + "_const_fold"
    dummy_prog = Tacky.Program(
      [ Tacky.Function(name=debug_label,
                       global=false,
                       params=["UNKNOWN"],
                       body=instructions) ]
    )
    Tacky_print.debug_print_tacky(filename, dummy_prog)


optimize(debug_label, instructions):
    debug_print(debug_label, instructions)
    return List.filter_map(optimize_instruction, instructions)
```

# UnreachableCodeElimination

```
eliminate_unreachable_blocks(cfg):
    // perform DFS from Entry to find reachable nodes
    function dfs(explored, node):
        if node in explored:
            return explored
        explored.add(node)
        for succ in cfg.get_succs(node):
            explored = dfs(explored, succ)
        return explored

    reachable = dfs(empty_set, Entry)

    // filter out unreachable blocks and remove their edges
    new_blocks = []
    for (idx, blk) in cfg.basic_blocks:
        if blk.id in reachable:
            new_blocks.append((idx, blk))
        else:
            for p in blk.preds:
                cfg.remove_edge(p, blk.id)
            for s in blk.succs:
                cfg.remove_edge(blk.id, s)
    return cfg with basic_blocks = new_blocks


eliminate_useless_jumps(cfg):
    n = length(cfg.basic_blocks)
    for idx in 0 … n-1:
        (i, blk) = cfg.basic_blocks[idx]
        // skip last block
        if idx < n-1:
            last_instr = blk.instructions[-1]
            if last_instr is Jump or JumpIfZero or JumpIfNotZero:
                default_succ = cfg.basic_blocks[idx+1].blk.id
                // if every successor is fall-through, drop the jump
                if all(succ == default_succ for succ in blk.succs):
                    blk.instructions = blk.instructions[0 : -1]
    return cfg


eliminate_useless_labels(cfg):
    for idx, (i, blk) in enumerate(cfg.basic_blocks):
        if blk.instructions[0] is Label(lbl):
            if idx == 0:
                default_pred = Entry
            else:
                default_pred = cfg.basic_blocks[idx-1].blk.id
            // if every predecessor is the default, drop the label
            if all(pred == default_pred for pred in blk.preds):
                blk.instructions = blk.instructions[1:]
    return cfg


remove_empty_blocks(cfg):
    new_blocks = []
    for (i, blk) in cfg.basic_blocks:
        if blk.instructions is empty:
            // must have exactly one pred and one succ
            if length(blk.preds) == 1 and length(blk.succs) == 1:
                pred = blk.preds[0]
                succ = blk.succs[0]
                cfg.remove_edge(pred, blk.id)
                cfg.remove_edge(blk.id, succ)
                cfg.add_edge(pred, succ)
                // drop this block
            else:
                fail("Empty block should have exactly one predecessor and one successor")
        else:
            new_blocks.append((i, blk))
    return cfg with basic_blocks = new_blocks


debug_print(cfg):
    if Settings.debug:
        // no‐op instruction printer
        function nop_printer(fmt, v): return
        label = cfg.debug_label + "_unreachable"
        cfg2 = cfg with debug_label = label
        cfg2.print_graphviz(nop_printer)


optimize(cfg):
    debug_print(cfg)
    cfg1 = eliminate_unreachable_blocks(cfg)
    cfg2 = eliminate_useless_jumps(cfg1)
    cfg3 = eliminate_useless_labels(cfg2)
    return remove_empty_blocks(cfg3)
```

# CopyPropagation

```
# Copy Propagation on TACKY CFG

type CP = {
    src: Val,
    dst: Val
}

ReachingCopies = Set<CP>

debug_print(cfg, extra_tag = ""):
    if Settings.debug:
        copies_printer(fmt, copies):
            fmt.open_box()
            fmt.print_list(copies.elements(), sep=", ", pp=pp_cp)
            fmt.close_box()
        label = cfg.debug_label + "_copy_prop" + extra_tag
        cfg2 = cfg.with(debug_label = label)
        G.print_graphviz(copies_printer, cfg2)

same_type(v1, v2):
    t1 = Tacky.type_of_val(v1)
    t2 = Tacky.type_of_val(v2)
    return t1 == t2 or (TypeUtils.is_signed(t1) == TypeUtils.is_signed(t2))

var_is_aliased(string* aliased_vars, v):
    switch v:
      case Constant(_):
        return false
      case Var(name):
        return name in aliased_vars or OptimizeUtils.is_static(name)
      default:
        return false

filter_updated(ReachingCopies copies, Tacky.Val updated):
    // kill any CP whose src or dst was updated
    return copies.filter(cp -> cp.src != updated and cp.dst != updated)

transfer(string* aliased_vars, block, ReachingCopies incoming):
    is_aliased = (v) -> var_is_aliased(aliased_vars, v)
    current = incoming
    annotated = []
    for (ann, instr) in block.instructions:
        new_copies = []
        switch instr:
            case Tacky.Copy(src, dst):
                if current.contains({src=dst, dst=src}):
                    new_copies = current
                else if same_type(src, dst):
                    new_copies = filter_updated(current, dst).add({src, dst})
                else:
                    new_copies = filter_updated(current, dst)

            case Tacky.FunCall(_, dst):
                if dst exist as d:
                    tmp = filter_updated(current, d)
                    new_copies = tmp.filter(cp -> not (is_aliased(cp.src) or is_aliased(cp.dst)))
                else: // dst is null
                    new_copies = current

            case Tacky.Store:
                new_copies = current.filter(cp -> not (is_aliased(cp.src) or is_aliased(cp.dst)))

            else:
                d = OptimizeUtils.get_dst(instr):
                if d is not null:
                    new_copies = filter_updated(current, d)
                else:
                    new_copies = current

        annotated.append((new_copies, instr))
        current = new_copies

    return (
        ...block,
        instructions = annotated,
        value = current
    )

meet(ident, cfg, block):
    ReachingCopies acc = ident
    for pred in block.preds:
        switch pred:
            case NodeID.Entry:
                acc = {}
            case NodeID.Exit:
                error("malformed CFG")
            case NodeID.Block(n):
                v = G.get_block_value(n, cfg)
                acc = acc.intersect(v)
    return acc

collect_all_copies(cfg):
    instrs = G.cfg_to_instructions(cfg)
    ReachingCopies cps = []
    for instr in instrs:
        if instr is Tacky.Copy(src, dst) and same_type(src, dst):
            cps.append({src, dst})
    return cps

find_reaching_copies(aliased_vars, cfg):
    ident = collect_all_copies(cfg)
    cfg0  = G.initialize_annotation(cfg, ident)
    worklist = cfg0.basic_blocks[:]  // list of (idx, block)

    process(cfg_cur, worklist):
        debug_print(cfg_cur, extra_tag="_in_progress_")
        if worklist empty:
            return cfg_cur
        (idx, blk) = worklist.pop_front()
        old_val = blk.value
        in_copies = meet(ident, cfg_cur, blk)
        blk_prime = transfer(aliased_vars, blk, in_copies)
        cfg1 = G.update_basic_block(idx, blk_prime, cfg_cur)
        if old_val != blk_prime.value:
            for succ in blk_prime.succs:
                if succ is Block(n) and not worklist.contains(n):
                    worklist.append((n, cfg1.basic_blocks[n]))
        return process(cfg1, worklist)

    return process(cfg0, worklist)

rewrite_instruction(pair):
    (reaching, instr) = pair

    // drop identity copies
    if instr is Tacky.Copy(src, dst):
        if reaching.contains({src, dst}) or reaching.contains({src=dst, dst=src}):
            return None

    replace(op):
        if op is Constant: return op
        if op is Var:
            for cp in reaching.elements():
                if cp.dst == op:
                    return cp.src
            return op

    new_i = null

    switch instr:
        case Tacky.Copy(src, dst):
            new_i = Tacky.Copy{src=replace(src), dst}
        case Tacky.Unary(u):
            new_i = Tacky.Unary{u with src = replace(u.src)}
        case Tacky.Binary(b):
            new_i = Tacky.Binary{b with src1=replace(b.src1), src2=replace(b.src2)}
        case Tacky.Return(v_opt):
            new_i = Tacky.Return(v_opt.map(replace))
        case Tacky.JumpIfZero(v, tgt):
            new_i = Tacky.JumpIfZero(replace(v), tgt)
        case Tacky.JumpIfNotZero(v, tgt):
            new_i = Tacky.JumpIfNotZero(replace(v), tgt)
        case Tacky.FunCall(f):
            new_i = Tacky.FunCall{f with args = f.args.map(replace)}
        case Tacky.SignExtend(sx):
            new_i = Tacky.SignExtend{sx with src=replace(sx.src)}
        case Tacky.ZeroExtend(zx):
            new_i = Tacky.ZeroExtend{zx with src=replace(zx.src)}
        case Tacky.DoubleToInt(d2i):
            new_i = Tacky.DoubleToInt{d2i with src=replace(d2i.src)}
        case Tacky.UIntToDouble(u2d):
            new_i = Tacky.UIntToDouble{u2d with src=replace(u2d.src)}
        case Tacky.Truncate(t):
            new_i = Tacky.Truncate{t with src=replace(t.src)}
        case Tacky.Load(l):
            new_i = Tacky.Load{l with src_ptr=replace(l.src_ptr)}
        case Tacky.Store(s):
            new_i = Tacky.Store{ s with src = replace(s.src) }
        case Tacky.Addptr(a):
            new_i = Tacky.Addptr{ a with ptr=replace(a.ptr), index=replace(a.index) }
        case Tacky.CopyToOffset(c2o):
            new_i = Tacky.CopyToOffset{ c2o with src=replace(c2o.src) }
        case Tacky.CopyFromOffset(cfo):
            let v = replace(Var(cfo.src))
            if v is Var(name):
                new_i = CopyFromOffset{cfo with src=name}
            else:
                fail("Internal error")
        case Label:
        case Jump:
        case GetAddress:
            new_i = instr

    return new_i

optimize(aliased_vars, cfg):
    annotated = find_reaching_copies(aliased_vars, cfg)
    debug_print(annotated)
    new_blocks = []
    instrs = []
    for (idx, blk) in annotated.basic_blocks:
        for pair in blk.instructions:
            new_i = rewrite_instruction(pair)
            if new_i is not null:
                instrs.append(new_i)

        new_blocks.append((idx, blk.with(instructions = instrs)))
    cfg2 = annotated.with(basic_blocks = new_blocks)
    return G.strip_annotations(cfg2)
```

# DeadStoreElimination

```
debug_print(cfg, extra_tag = ""):
    if Settings.debug:
        function livevar_printer(fmt, live_vars):
            fmt.open_box()
            fmt.print_list(live_vars.to_list(), sep=", ", pp=fmt.print_string)
            fmt.close_box()

        label = cfg.debug_label + "_dse" + extra_tag
        cfg2   = cfg.with(debug_label = label)
        G.print_graphviz(livevar_printer, cfg2)

transfer(static_and_aliased_vars, block, end_live_vars):
    // helpers to remove/add variables
    remove_var(var, set):
        if var is Var(name): return set.remove(name)
        else error

    add_var(val, set):
        if val is Var(name): return set.add(name)
        else return set

    add_vars(vals, set):
        for v in vals: set = add_var(v, set)
        return set

    current_live = end_live_vars
    annotated = []

    // process instructions in reverse order
    for (annot, instr) in reverse(block.instructions):
        switch instr:
        case Binary(dst, src1, src2):
            live1 = remove_var(dst, current_live) // kill the defined var
            new_live = add_vars([src1, src2], live1) // add uses

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
            else
                live1 = current_live
            live2 = add_vars(args, live1)
            // external uses and aliased vars are live
            new_live = union(live2, static_and_aliased_vars)

        case Store(src, dst_ptr):
            new_live = add_vars([src, dst_ptr], current_live)

        case SignExtend(dst, src)
            | ZeroExtend(dst, src)
            | DoubleToInt (dst, src)
            | IntToDouble (dst, src)
            | DoubleToUInt(dst, src)
            | UIntToDouble(dst, src)
            | Truncate    (dst, src):
            live1 = remove_var(dst, current_live)
            new_live = add_var(src, live1)

        case AddPtr(dst, ptr, index, _):
            live1 = remove_var(dst, current_live)
            new_live = add_vars([ptr, index], live1)

        case GetAddress(dst, _):
            new_live = remove_var(dst, current_live)

        case Load(dst, src_ptr):
            live1 = remove_var(dst, current_live)
            live2 = add_var(src_ptr, live1)
            new_live = union(live2, static_and_aliased_vars)

        case CopyToOffset(src, _):
            new_live = add_var(src, current_live)

        case CopyFromOffset(dst, src_name, _):
            live1 = remove_var(dst, current_live)
            new_live = add_var(Var(src_name), live1)

        case Jump(_)
            | Label(_)
            | Return(None):
            new_live = current_live

        // record the live‐set before this instr
        annotated.prepend((new_live, instr))
        // thread state
        current_live = new_live

    // reconstruct block
    return block.with(
    instructions = annotated.reverse(),
    value        = current_live
    )


meet(static_vars, cfg, block):
    live = empty_set
    for succ in block.succs:
        switch succ.kind:
            case NodeID.Entry:
                error("malformed CFG")
            case NodeID.Exit:
                live = live ∪ static_vars
            case NodeID.Block(n):
                live = live ∪ cfg.basic_blocks[n].value
    return live


find_live_variables(static_vars, aliased_vars, cfg):
    start_cfg    = G.initialize_annotation(cfg, empty_set)
    static_plus  = static_vars ∪ aliased_vars
    worklist     = reverse(start_cfg.basic_blocks)  // list of (idx, block)

    while worklist not empty:
        (idx, blk) = worklist.pop_front()
        old_live   = blk.value
        live_exit  = meet(static_vars, start_cfg, blk)
        blk2       = transfer(static_plus, blk, live_exit)
        cfg2       = G.update_basic_block(idx, blk2, start_cfg)

        if blk2.value != old_live:
            for pred in blk2.preds:
                if pred.kind == Block and
                   not worklist.contains(pred.idx):
                    worklist.prepend((pred.idx, cfg2.basic_blocks[pred.idx]))

        start_cfg = cfg2

    return start_cfg


is_dead_store((live_vars, instr)):
    if instr is FunCall or instr is Store:
        return false
    switch OptimizeUtils.get_dst(instr):
      case Some(Var(name)) when name not in live_vars:
        return true
      case _:
        return false


optimize(aliased_vars, cfg):
    // collect all globals/static so they're always live
    static_vars = []
    for (v, info) in Symbols.bindings():
        if info.attrs is StaticAttr:
            static_vars.append(v)
    static_set = Set.of_list(static_vars)

    annotated_cfg = find_live_variables(static_set, aliased_vars, cfg)

    new_blocks = []
    for (idx, blk) in annotated_cfg.basic_blocks:
        filtered_instrs =
            blk.instructions
            .filter(lambda pair: not is_dead_store(pair))
        new_blocks.append((idx, blk.with(instructions = filtered_instrs)))

    cfg2 = annotated_cfg.with(basic_blocks = new_blocks)
    return G.strip_annotations(cfg2)
```

# TACKY

```
// we need a custom comparison function for constants to make sure that 0.0 and
// -0.0 don't compare equal
const_compare(a, b):
    if a is Double(d1) and d2 is Double(d2) and d1 == d2:
        return <compare value and signedness>
    else:
        return a == b
```

```
// TODO maybe this should be in a separate module?
type_of_val(val):
    // note: this reports the type of ConstChar as SChar instead of Char, doesn't matter in this context
    if val is Constant(c):
        return type_of_const(c)

    if val is Var(v):
        return symbols.get(v).type
```

# CodeGen

Remove tacky_type function, and replace it with Tacky.type_of_val function:

- classify_tacky_val
- classify_parameters
- classify_return_value
- convert_return_instruction
- convert_instruction

---

# Output

**C**
The test program is:

```C
/* Original C */
void printf(char *str);

int compute(int x) {
    int a = 2 + 3;            // constant folding: 2+3 → 5
    int b = a + x;            // copy propagation candidate
    int c = b * 1;            // dead store: c is never used

    if (0) {                  // unreachable code
        printf("This will never be printed.\n");
    }

    int d = b;                // copy propagation: d = b
    int e = d + 0;            // constant folding: d + 0 → d
    int f = e;                // dead store: f is never used

    int result = d * 2;       // used value
    return result;
}

int main(void) {
    int x = 10;
    int output = compute(x);
    return output;
}
```

The optimization should produce code equivalent to this:

```C
/* Optimized C */
int compute(int x) {
    return (5 + x) * 2;
}

int main(void) {
    return compute(10);
}
```

**TACKY**
Here is the TACKY before optimizations:

```plain
/* Original TACKY */
Program(
    Function(
        name="compute",
        global=1,
        params=[
        	"x.1",
        ],
        instructions=
            Binary(
                op=Add,
                src1=Constant(ConstInt(2)),
                src2=Constant(ConstInt(3)),
                dst=Var(tmp.11)
            ),
            Copy(
                src=Var(tmp.11)
                dst=Var(a.2)
            ),
            Binary(
                op=Add,
                src1=Var(a.2)
                src2=Var(x.1)
                dst=Var(tmp.12)
            ),
            Copy(
                src=Var(tmp.12)
                dst=Var(b.3)
            ),
            Binary(
                op=Multiply,
                src1=Var(b.3)
                src2=Constant(ConstInt(1)),
                dst=Var(tmp.13)
            ),
            Copy(
                src=Var(tmp.13)
                dst=Var(c.4)
            ),
            JumpIfZero(
                cond=Constant(ConstInt(0)),
                target=if_end.14
            ),
            GetAddress(
                src=Var(string.15)
                dst=Var(tmp.16)
            ),
            FunCall(
                name=printf
                args=[
                    Var(tmp.16)
                ],
                dst=None
            ),
            Label(name=if_end.14)
            Copy(
                src=Var(b.3)
                dst=Var(d.5)
            ),
            Binary(
                op=Add,
                src1=Var(d.5)
                src2=Constant(ConstInt(0)),
                dst=Var(tmp.17)
            ),
            Copy(
                src=Var(tmp.17)
                dst=Var(e.6)
            ),
            Copy(
                src=Var(e.6)
                dst=Var(f.7)
            ),
            Binary(
                op=Multiply,
                src1=Var(d.5)
                src2=Constant(ConstInt(2)),
                dst=Var(tmp.18)
            ),
            Copy(
                src=Var(tmp.18)
                dst=Var(result.8)
            ),
            Return(
                Var(result.8)
            ),
            Return(
                Constant(ConstInt(0)),
            ),
    ),
    Function(
        name="main",
        global=1,
        params=[
        ],
        instructions=
            Copy(
                src=Constant(ConstInt(10)),
                dst=Var(x.9)
            ),
            FunCall(
                name=compute
                args=[
                    Var(x.9)
                ],
                dst=Var(tmp.19)

            ),
            Copy(
                src=Var(tmp.19)
                dst=Var(output.10)
            ),
            Return(
                Var(output.10)
            ),
            Return(
                Constant(ConstInt(0)),
            ),
    ),
)
```

And the result optimized version:

```plain
/* Optimized TACKY */
Program(
    Function(
        name="compute",
        global=1,
        params=[
        	"x.1",
        ],
        instructions=
            Binary(
                op=Add,
                src1=Constant(ConstInt(5)),
                src2=Var(x.1)
                dst=Var(tmp.12)
            ),
            Binary(
                op=Multiply,
                src1=Var(tmp.12)
                src2=Constant(ConstInt(2)),
                dst=Var(tmp.18)
            ),
            Return(
                Var(tmp.18)
            ),
    ),
    Function(
        name="main",
        global=1,
        params=[
        ],
        instructions=
            FunCall(
                name=compute
                args=[
                    Constant(ConstInt(10)),
                ],
                dst=Var(tmp.19)

            ),
            Return(
                Var(tmp.19)
            ),
    ),
)
```

**Assembly**

Original x64 Assembly on Linux:

```asm
	.section .rodata
	.align 16
.Lstring.15:
	.asciz "This\040will\040never\040be\040printed\056\012"

	.global compute
compute:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$64, %rsp
	movl	%edi, -4(%rbp)
	movl	$2, -8(%rbp)
	addl	$3, -8(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -16(%rbp)
	movl	-4(%rbp), %r10d
	addl	%r10d, -16(%rbp)
	movl	-16(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-20(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-24(%rbp), %r11d
	imull	$1, %r11d
	movl	%r11d, -24(%rbp)
	movl	-24(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	$0, %r11d
	cmpl	$0, %r11d
	je	.Lif_end.14
	leaq	.Lstring.15(%rip), %r11
	movq	%r11, -40(%rbp)
	movq	-40(%rbp), %rdi
	call	printf@PLT
.Lif_end.14:
	movl	-20(%rbp), %r10d
	movl	%r10d, -44(%rbp)
	movl	-44(%rbp), %r10d
	movl	%r10d, -48(%rbp)
	addl	$0, -48(%rbp)
	movl	-48(%rbp), %r10d
	movl	%r10d, -52(%rbp)
	movl	-52(%rbp), %r10d
	movl	%r10d, -56(%rbp)
	movl	-44(%rbp), %r10d
	movl	%r10d, -60(%rbp)
	movl	-60(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -60(%rbp)
	movl	-60(%rbp), %r10d
	movl	%r10d, -64(%rbp)
	movl	-64(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$10, -4(%rbp)
	movl	-4(%rbp), %edi
	call	compute
	movl	%eax, -8(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```

Optimized x64 Assembly on Linux:

```asm
	.section .rodata
	.align 16
.Lstring.15:
	.asciz "This\040will\040never\040be\040printed\056\012"

	.global compute
compute:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	$5, -8(%rbp)
	movl	-4(%rbp), %r10d
	addl	%r10d, -8(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -12(%rbp)
	movl	-12(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$10, %edi
	call	compute
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
