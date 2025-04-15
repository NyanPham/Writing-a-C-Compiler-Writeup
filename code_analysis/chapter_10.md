# Table of Contents

- [Token, Lexer](#token-lexer)
- [AST](#ast)
- [Parser](#parser)
- [IdentifierResolution](#identifier-resolution)
- [ValidateLabels](#validatelabels)
- [LoopLabeling](#looplabeling)
- [CollectSwitchCases](#collectswitchcases)
- [Symbols](#symbols)
- [TypeCheck](#typecheck)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly](#assembly)
- [CodeGen](#codegen)
- [ReplacePseudo](#replacepseudo)
- [Instruction Fixup](#instruction-fixup)
- [Emit](#emit)
- [Output](#output)

---

# Token, Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordStatic | static |
| KeywordExtern | extern |

# AST

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, storage_class?)
storage_class = Static | Extern
block_item = S(statement) | D(declaration)
block = Block(block_item*)
for_init = InitDecl(variable_declaration) | InitExp(exp?)
statement = Return(exp)
    | Expression(exp)
    | If(exp condition, statement then, statement else)
    | Compound(block)
    | Break
    | Continue
    | While(exp condition, statement body, identifier id)
    | DoWhile(statement body, exp condition, identifier id)
    | For(for_init init, exp? condition, exp? post, statement body, identifier id)
    | Switch(exp control, statement body, string* cases, identifier id)
    | Case(exp, statement body, identifier id)
    | Default(statement body, identifier id)
    | Null
    | LabeledStatement(indentifier label, statement)
    | Goto(identifier label)
exp = Constant(int)
    | Var(identifier)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
    | Assignment(exp, exp)
    | CompoundAssignment(binary_operator, exp, exp)
    | PostfixIncr(exp)
    | PostfixDecr(exp)
    | Conditional(exp condition, exp then, exp else)
    | FunctionCall(identifier, exp* args)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
```

# Parser

```
parse_specifier_list(tokens):
    specifiers = []
    next_token = peek(tokens)

    while next_token is "int" or "static" or "extern":
        spec = take(tokens)
        specifiers.append(spec)
        next_token = peek(tokens)

    return specifiers
```

```
parse_storage_class(spec):
    if spec is "extern":
        return Extern
    else if spec is "static":
        return Static
    else:
        fail("Internal error: bad storage class")
```

```
parse_type_and_storage_class(specifier_list):
    types = []
    storage_classes = []

    for tok in specifier_list:
        if tok is "int":
            types.append(tok)
        else:
            storage_classes.append(tok)

    if length(types) != 1:
        fail("Invalid type specifier")
    else:
        type = Int
        storage_class = null

        if storage_classes is empty:
            // do nothing, storage_class is null
        else if length(storage_classes) == 1:
            storage_class = parse_storage_class(storage_classes[0])
        else:
            fail("Invalid storage class")

        return (type, storage_class)
```

```
parse_block_item(tokens):
    match peek(tokens):
        case "int":
        case "static":
        case "extern":
            return parse_declaration(tokens)
        default:
            return parse_statement(tokens)
```

```
finish_parsing_function_declaration(name, storage_class, tokens):
    expect("(", tokens)
    params = []

    if peek(tokens) is ")":
        params = []
    else:
        params = parse_param_list(tokens)

    expect(")", tokens)
    next_token = peek(tokens)

    body = null
    if next_token is "{":
        body = parse_block(tokens)
    else if next_token is ";":
        body = null
    else:
        raise_error("function body or semicolon", next_token type)

    return (name, params, body, storage_class)
```

```
finish_parsing_variable_declaration(name, storage_class, tokens):
    next_token = take_token(tokens)
    if next_token is ";":
        return (name, init=null, storage_class)
    else if next_token is "=":
        init = parse_exp(0, tokens)
        expect(";", tokens)
        return (name, init, storage_class)
    else:
        raise_error("An initializer or semicolon", next_token type)
```

```
parse_declaration(tokens):
    // expect("int", tokens) We replace this with 2 parsing functions to support more specifier beside "int": "static" and "extern"
    specifiers = parse_specifier_list(tokens)
    type, storage_class = parse_type_and_storage_class(specifiers) // We don't actually use the returned value "type" from the function now as our compiler only supports 1 type

    name = parse_id(tokens)
    if peek(tokens) is "(":
        return finish_parsing_function_declaration(name, storage_class, tokens)
    else:
        return finish_parsing_variable_declaration(name, storage_class, tokens)
```

```
parse_for_init(tokens):
    if peek(tokens) is "int" or "static" or "extern": // Note that we choose to parse them static and extern here, and let the semantic analysis to catch this invalid usage
        return AST.InitDecl(parse_variable_declaration(tokens))
    else:
        return AST.InitExp(parse_optional_exp(";", tokens))
```

```
// Change the "parse_function_declaration_list" to "parse_declaration_list"
parse_declaration_list(tokens):
    decl_list = []

    while not tokens.is_empty():
        next_decl = parse_declaration(tokens)
        decl_list.append(next_decl)

    return decl_list
```

```
parse(tokens):
    declarations = parse_declaration_list   (tokens)
    return Program(declarations)
```

# Identifier Resolution

```
resolve_local_var_helper(id_map, name, storage_class):
    entry = id_map.find(name)

    if entry is not null && entry.from_current_scope:
        if not (entry.has_linkage && storage_class is Extern): // !entry.has_linkage or storage_class != Extern:
            // variable is present in the map and was defined in the current scope
            fail("duplicate variable declaration")
    else:
        if storage_class is Extern:
            new_map = id_map.add(
                unique_name=name,
                from_current_scope=True,
                has_linkage=True
            )

            return (new_map, name)
        else:
            // variable isn't in the map or defined in the outer scope
            // generate a unique name and add to the map/update existing entry
            unique_name = UniqueIds.make_named_temporary(name)
            new_map = id_map.add(
                unique_name,
                from_current_scope=True,
                has_linkage=False
            )

            return (new_map, unique_name)
```

```
resolve_local_var_declaration(id_map, var_decl):
    new_map, unique_name = resolve_local_var_helper(id_map, var_decl.name, var.storage_class)

    resolved_init = null
    if var_decl has init:
        ressolved_init = resolve_exp(var_decl.init)

    return (new_map, (name = unique_name, init=resolved_init, var.storage_class))
```

```
resolve_local_declaration(id_map, decl):
    if decl is VarDecl:
        new_map, resolved_vd = resolve_local_var_declaration(id_map, decl.decl)
        return (new_map, VarDecl(resolved_vd))
    else if decl is FunDecl and has body:
        fail("Nested function defintiions are not allowed")
    else if  decl is FunDecl and decl.storage_class is Static:
        fail("Static keyword not allowed on local function declarations")
    else if decl is FunDecl:
        new_map, resolved_fd = resolve_function_declaration(id_map, decl.decl)
        return (new_map, FunDecl(resolved_fd))
```

```
resolve_params(id_map, params):
    resolved_params = []
    final_map = id_map

    for param in params:
        new_map, new_param = resolve_local_var_helper(id_map, param, null)
        resolved_params.append(new_param)
        final_map = new_map

    return (final_map, resolved_params)
```

```
resolve_file_scope_variable_declaration(id_map, var_decl):
    new_map = id_map.add(var_decl.name, (
        unique_name=var_decl.name,
        from_current_scope=true,
        has_linkage=true
    ))

    return new_map, var_decl
```

```
resolve_global_declaration(id_map, decl):
    if decl is FunDecl:
        id_map1, fd = resolve_function_declaration(id_map, decl)
        return (id_map1, fd)
    else if decl is VarDecl:
        id_map1, vd = resolve_file_scope_variable_declaration(id_map, decl)
        return (id_map1, vd)
```

Update our resolve from the top-level:

```
resolve(Program prog):
    resolved_decls = []

    for decl in prog.decls:
        resolved_decls.append(
            resolve_global_declaration({}, decl)
        )

    return Program(resolved_decls)
```

# ValidateLabels

```
validate_labels_in_decl(decl):
    if decl is FunDecl:
        return validate_labels_in_fun(decl.fd)
    else:
        return decl.vd
```

```
validate_labels(AST.Program prog):
    validated_decls = []
    for decl in prog.decls:
        new_decl = validate_labels_in_decl(decl)
        validated_decls.append(new_decl)

    return Program(validated_decls)
```

# LoopLabeling

We change the name label_funtion_def into label_decl and extend the logic for variable declarations.

```
label_decl(decl):
    if decl is FunDef:
        return AST.FunctionDef(
            funDef.name,
            body= has body ? label_block(funDef.body, null, null) : null
        )
    else:
        return decl as VarDecl
```

```
label_loops(AST.Program prog):
    labeled_decls = []
    for decl in prog.decls:
        labeled_decls.append(label_decl(decl))

    return Program(labeled_decls)
```

# CollectSwitchCases

```
analyze_decl(decl):
    if decl is FunDecl:
        return analyze_function_def(decl)
    else:
        return decl as VarDecl
```

```
analyze_switches(Program prog):
    analyzed_decls = []
    for decl in prog.decls:
        analyzed_decls.append(analyze_decl(decl))

    return Program(analyzed_decls)
```

# Symbols

We added some representations to differentiate between function declarations, local and global variable declarations.

```
type initial_value = Tentative | Initial(int) | NoInitializer

type identifier_attrs =
    | FunAttr(bool defined, bool global, int stack_frame_size)
    | StaticAttr(initial_value init, bool global)
    | LocalAttr
```

Then, update our symbol entry struct:

```
type entry = {
    t: Types.t,
    attrs: identifier_attrs
}
```

The previous function add*var is now splitted into 2: \_add_automatic_var* and _add_static_var_

```
add_automatic_var(name, type t):
    symbols.add_or_assign(name, entry(
        t,
        attrs=LocalAttr()
    ))

add_static_var(name, t, global, initial_value init):
    symbols.add_or_assign(name, entry(
        t,
        attrs=StaticAttr(init, global)
    ))
```

Function can now also be either global or internal

```
add_fun(name, t, global, defined):
    symbols.add(name, entry(
        t,
        attrs=FunAttr(defined, global, stack_frame_size=0)
    ))
```

We also add 2 helper functions to check if an object is global, or is static:

```
is_global(name):
    attrs = symbols.get(name).attrs

    if attrs is LocalAttr:
        return false
    else if attrs is StaticAttr:
        return attrs.global
    else if attrs is FunAttr:
        return attrs.global

is_static(name):
    attrs = symbols.get(name).attrs

    if attrs is LocalAttr:
        return false
    else if attrs is StaticAttr:
        return true
    else if attrs is FunAttr:
        fail("Functions don't have storage duration")
    else:
        // If it's not in the symbol table, it's a TACKY temporary, so not static
        return false
```

Update the function set_bytes_required

```
set_bytes_required(name, bytes_required):
    entry = symbols.find(name)

    if entry.attrs is FunAttr:
        entry.attrs.stack_frame_size = bytes_required
    else:
        fail("Internal error: not a function")

    symbols.update(name, entry)
```

Lastly, write a helper function to get the stack frame size:

```
get_bytes_required(name):
    entry = symbols.find(name)

    if entry.attrs is FunAttr:
        return entry.attrs.stack_frame_size
    else:
        fail("Internal error: not a function)
```

# TypeCheck

```
// Block item means it's local,
// so it's either statement
// or local declaration
typecheck_block_item(block_item):
    if block_item is statement:
        return typecheck_statement(block_item)
    else if block_item is declaration:
        return typecheck_local_decl(block_item)
```

```
// Update the typecheck ForInit, specifically InitDecl,
// throws error if it has any storage class and change
// the typecheck_var_decl to typecheck_local_var_decl
typecheck_statement(stmt):
    match stmt type:
    case Return(e):
        typecheck_exp(e)
    case Expression(e):
        typecheck_exp(e)
    case If(condition, then_clause, else_clause):
        typecheck_exp(condition)
        typecheck_statement(then_clause)
        if else_clause is not null:
            typecheck_statement(else_clause)
    case LabeledStatement(lbl, s):
        typecheck_statement(s)
    case Case(val, s, id):
        typecheck_statement(s)
    case Default(s, id):
        typecheck_statement(s)
    case Switch(control, body, cases, id):
        typecheck_exp(control)
        typecheck_statement(body)
    case Compound(block):
        typecheck_block(block)
    case While(condition, body, id):
        typecheck_exp(condition)
        typecheck_statement(body)
    case DoWhile(body, condition, id):
        typecheck_statement(body)
        typecheck_exp(condition)
    case For(init, condition, post, body, id):
        // typecheck_for_init
        if init is InitDecl:
            if init.storage_class is not null:
                fail("Storage class not permitted on declaration in for loop header")
            else:
                typecheck_local_var_decl(init.d)
        else if init is InitExp and has exp:
            typecheck_exp(init.exp)

        if condition is not null:
            typecheck_exp(condition)
        if post is not null:
            typecheck_exp(post)
        typecheck_statement(body)
    case Null:
    case Break:
    case Continue:
    case Goto:
        break
```

```
// We change typecheck_decl into typecheck_local_decl
typecheck_local_decl(decl):
    if decl is VarDecl: typecheck_local_var_decl(decl)
    else if decl is FunDecl: typecheck_fn_decl(decl)
```

```
// The function typecheck_var_decl is changed to typecheck_local_var_decl
// The logic here is also more complicated as we need to check for storage class errors
typecheck_local_var_decl(var_decl):
    if var_decl.storage_class is Extern:
        if var_decl has init:
            fail("Initializer on local extern declaration")
        else:
            entry = symbols.get_opt(var_decl.name)
            if entry is not null:
                // If an external local var is already in the symbol table, don't need to add it
                if entry.t != Int:
                    fail("Function redeclared as variable")
            else:
                symbols.add_static_var(var_decl.name, t=Int, global=true, init=NoInitializer)
    else if var_decl.storage_class is Static:
        if var_decl.init is Constant(i):
            init = Initial(i)
        else if var_decl.init is null:
            init = Initial(0)
        else:
            fail("Non-constant initializer on local static variable")

        symbols.add_static_var(var_decl.name, t=Int, global=false, init)
    else: // storage_class is null
        Symbols.add_automatic_var(var_decl.name, t.Int)
        if var_decl has init:
            typecheck_exp(var_decl.init)
```

```
// Update typecheck_fn_decl to process storage_class
typecheck_fn_decl(fn_decl):
    fun_type = Types.FunType(param_count=length(fn_decl.params))
    has_body = fn_decl has body?
    global = fn_decl.storage_class != Static

    already_defined = false

    if fn_decl.name is in symbols:
		old_decl = symbols.get(fn_decl.name)
		if old_decl.type != fun_type:
			fail("Incompatiable function declaration")

        if old_decl.attrs is FunAttr:
            if old_decl.attrs.defined and has_body:
			    fail("Function is defined more than once")
            else if old_decl.attrs.global and fn_decl.storage_class == Static:
                fail("Static function declaration follows non-static")
            else:
                already_defined = has_body or old_decl.attrs.defined
                global = old_decl.attrs.global
        else:
            fail("Internal error: symbol has function type but not function attributes")

    symbols.add_fun(fn_decl.name, fun_type, global, already_defined)

	if has_body:
		for param in fn_decl.params:
			symbols.add_automatic_var(param, Int)
		typecheck_block(fn_decl.body)
```

We've updated and rename our previous logic for variable declarations to be more specific for local variable declartions.
Now we need another function to typecheck file scope variable declaration on the top-level

```
typecheck_file_scope_var_decl(var_decl):
    curr_init = null
    if var_decl.init is Constant(c)
        curr_init = Initial(c)
    else if var_decl.init is null:
        if var_decl.storage_class == Extern:
            curr_init = NoInitializer
        else:
            curr_init = Tentative
    else:
        fail("File scope variable has non-constant initializer)

    curr_global = var_decl.storage_class != Static

    if symbols has var_decl.name
        old_decl = symbols.get(var_decl.name)
        if old_decl.type != Int:
            fail("Function redclared as variable")

        if old_decl.attrs is not StaticAttr:
            fail("Internal error: file-scope variable previsouly declared as local")

        if var_decl.storage_class == Extern:
            curr_global = old_decl.attrs.global
        else if old_decl.attrs.global != curr_global:
            fail("Conflicting variable linkage")

        if old_decl.attrs.init is an Initial(x):
            if curr_init is Initial(c):
                fail("Conflicting global variable definition")
            else:
                curr_init = old_decl.attrs.init
        else if old_delc.attrs.init is Tentative and curr_init is either Tentative or NoInitializer:
            curr_init = Tentative

    symbols.add_static_var(var_decl.name, Int, curr_global, curr_init)

```

We typecheck_global_decl function for the top layer checking

```
typecheck_global_decl(decl):
    if decl is FunDec:
        return typecheck_fn_decl(decl)
    else:
        return typecheck_file_csope_var_decl(decl)
```

```
typecheck(Program prog):
    checked_decls = []

    for decl in prog.delcs:
        checked_decl = typecheck_global_decl(decl)
        checked_decls.append(checked_decl)

    return Program(checked_decls)
```

# TACKY

```
program = Program(top_level*)
top_level = Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, int init)
instruction = Return(val)
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| Jump(identifier target)
	| JumpIfZero(val condition, identifier target)
	| JumpIfNotZero(val condition, identifier target)
	| Label(identifier)
	| FunCall(identifier fun_name, val* args, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
```

# TACKYGEN

```
emit_local_declaration(decl):
    if decl is VarDecl:
        if decl.storage_class is not null:
            return []

        return emit_var_declaration(decl.decl)
    else:
        return []
```

```
emit_fun_declaration(fun_decl):
    insts = []

    if fun_decl has body:
        global = symbols.is_global(decl.name)

        for block_item in fun_def.body:
            emit_insts = emit_tacky_for_block_item(block_item)
            insts.append(...emit_insts)

        extra_return = Tacky.Return(Constant(0))
        insts.append(extra_return)

        return Tacky.Function(fun_def.name, global, fun_decl.params, insts)

    return null
```

Real declarations of static variables live on the symbol tables. We need to convert the symbols to TACKY and move all of them to the top of the program.

```
convert_symbols_to_tacky(symbols):
    static_vars = []

    for name, entry in symbols:
        if entry.attrs is StaticAttr:
            if entry.attrs.init is Initial:
                static_vars.push(StaticVariable(
                    name,
                    global=entry.attrs.global,
                    init=entry.attrs.init.value
                ))
            else if entry.attrs.init is Tentative:
                static_vars.push(StaticVariable(
                    name,
                    global=entry.attrs.global,
                    init=0
                ))
            else: // No Initializer, don't do anything
        else:
            // Don't do anything

    return static_vars
```

```
gen(AST.Program prog):
    tacky_fn_defs = []

    for fun_decl in prog.fun_decls:
        new_fun = emit_fun_declaration(fun_decl)
        if new_fun is not null:
            tacky_fn_defs.append(new_fun)

    tacky_var_defs = convert_symbols_to_tacky(symbols.entries())

    return Tacky.Program([...tacky_var_defs, ...tacky_fn_defs])
```

# Assembly

```
program = Program(top_level*)
top_level = Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int init)
instruction = Mov(operand src, operand dst)
    | Unary(unary_operator, operand dst)
    | Binary(binary_operator, operand, operand)
    | Cmp(operand, operand)
    | Idiv(operand)
    | Cdq
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | AllocateStack(int)
    | DeallocateStack(int)
    | Push(operand)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11
```

# CodeGen

```
// Top level now not only contains function but also static variable.
// Change the function convert_function to convert_top_level
convert_top_level(Tacky.top_level top_level):
    if top_level is Function:
        params_insts = pass_params(fun_def.params)
        insts = [...params_insts]

        for inst in fun_def.body:
            insts.append(...convert_instruction(inst))

        return Function(fun_def.name, fun_decl.global, insts)
    else: // top_level is StaticVariable
        return Assembly.StaticVariable(top_level.name, top_level.global, top_level.init)
```

```
gen(Tacky.Program prog):
    converted_top_levels = []

    for top_level in prog.top_levels
        insts = convert_top_level(top_level)
        converted_top_levels.append(...insts)

    return Assembly.Program(converted_top_levels)
```

# ReplacePseudo

```
replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
            if symbols.is_static(operand.name):
                return (state, Assembly.Data(operand.name))
			else:
                if state.offset_map.has(operand.name):
                    return (state, state.offset_map.get(operand.name))
                else:
                    new_offset = state.current_offset - 4
                    new_state = {
                        current_offset: new_offset,
                        offset_map: state.offset_map.add(operand.name, new_offset)
                    }
				    return (new_state, Assembly.Stack(new_offset))
		default:
			return (state, operand)
```

```
// Change the replace_pseudos_in_function to replace_pseudos_in_top_level
replace_pseudos_in_top_level(Assembly.top_level top_level):
	if top_level is Function:

        curr_state = init_state
        fixed_instructions = []

        for inst in fn.instrustions:
            new_state, new_inst = replace_pseudos_in_instruction(inst, curr_state)
            curr_state = new_state
            fixed_instructions.push(new_inst)

        Symbols.set_bytes_required(fn.name, curr_state.current_offset)

        new_fn = Assembly.Function(fn.name, fn.global, fixed_instructions)
        return new_fn, curr_state.current_offset
    else: // is StaticVariable
        return top_level
```

```
replace_pseudos(Assembly.Program prog):
    fixed_tls = []

    for top_level in prog.top_levels:
        new_tl = replace_pseudos_in_top_level(top_level)
        fixed_tls.append(new_tl)

	return Assembly.Program(fixed_tls)
```

# Instruction Fixup

```
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	case Mov:
        if inst.src is (Stack | Data) and inst.dst is (Stack | Data):
            return [
				Mov(src, Reg(R10)),
				Mov(Reg(R10), dst)
			]
        else:
			return [ inst ]
    case Idiv:
		(* Idiv can't operate on constants *)
		if inst.operand is a constant:
			return [
				Mov(Imm(inst.value), Reg(R10)),
				Idiv(Reg(R10))
			]
		else:
			return [ inst ]
    case Binary:
		(* Add/Sub can't use memory addresses for both operands *)
		if inst.op is Add Or Sub and both operands are (Stack | Data):
			return [
				Mov(inst.src, Reg(R10)),
				Binary(inst.op, Reg(R10), inst.dst)
			]
		 (* Destination of Mult can't be in memory *)
		else if inst.op is Mult and dst is (Stack | Data):
			return [
				Mov(inst.dst, Reg(R11)),
				Binary(inst.op, inst.src, Reg(R11)),
				Mov(Reg(R11), inst.dst)
			]
		else:
			return [ inst ]
    case Cmp:
		(* Both operands of cmp can't be in memory *)
		if both src and dst of cmp are (Stack | Data):
			return [
				Mov(inst.src, Reg(R10)),
				Cmp(Reg(10), inst.dst)
			]

		else if dst of cmp is an immediate:
			return [
				Mov(Imm(inst.dst.value), Reg(R11)),
				Cmp(inst.src, Reg(R11)),
			]

        else:
			return [ inst ]
    default:
		return [ other ]
```

```
// Similarly, we change fixup_function to fixup_top_level
fixup_top_level(Assembly.top_level top_level):
    if top_level is Function:
        stack_bytes = -Symbols.get_bytes_required(top_level.name)
        alloc_stack = AllocateStack(round_away_from_zero(16, stack_bytes))

        return Assembly.Function(fn_def.name, [alloc_stack, ...fn_def.instructions])
    else: // StaticVariable
        return top_level
```

```
fixup_program(Assembly.Program prog):
    fixed_tls = []

    for tl in prog.top_levels:
        fixed_tl = fixup_top_level(tl)
        fixed_tls.append(fixed_tl)

    return Assembly.Program(fixed_tls)
```

# Emit

```
For static variable, we need to align them
align_directive():
    if platform is OS_X:
        return ".balign"
    else if platform is Linux:
        return ".align"
```

```
// Operand has new type: Data
show_operand(operand):
    if operand is Reg:
        return show_reg(operand)
    else if operand is Imm:
        return "${operand.value}"
    else if operand is Stack:
        return "%{operand.offset}(%rbp)"
    else if operand is Data:
        return "{operand.name}(%rip)"
    else if operand is pseudo: // For debugging
        return operand.name
```

```
emit_global_directive(global, label):
    if global:
        return "\t.global %{label}\n"
    else:
        return ""
```

```
// Change the emit_function to emit_top_level
emit_top_level(Assembly.top_level top_level):
    if top_level is Function:
        func = top_level as Function

        label = show_label(func.name)

        return """
        {emit_global_directive(func.global, label)}
    {label}:
        pushq	%rbp
        movq	%rsp, %rbp

        {emit_instruction(inst) for inst in func.instructions}
        """
    else: // StaticVariable
        static_var = top_level as StaticVariable

        if static_var.init == 0:
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .bss
                {align_directive()} 4
            {label}:
                .zero 4
            """
        else:
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .data
                {align_directive()} 4
            {label}:
                .long {static_var.init}
            """
```

```
emit(Assembly.Program prog):
    file = open(file)

    content = ""
    for tl in prog.top_levels:
        content += emit_top_level(tl)

	content += emit_stack_note()

	write content to file
    close file
```

# Output

From C:

```C
static int foo;

int main(void)
{
    for (int i = 0; i < 10; i++)
    {
        extern int inc;
        foo += inc;

        if (foo > 10)
        {
            break;
        }

        if (foo == 5)
        {
            continue;
        }
    }
    return foo;
}

int inc = 3;
extern int foo;
```

To x64 Assembly on Linux:

```asm
	.global inc
	.data
	.align 4
inc:
	.long 3

	.bss
	.align 4
foo:
	.zero 4

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movl	$0, -4(%rbp)
.Lfor_start.2:
	movl	$10, %r11d
	cmpl	-4(%rbp), %r11d
	movl	$0, -8(%rbp)
	setl	-8(%rbp)
	cmpl	$0, -8(%rbp)
	je	.Lbreak.for.1
	movl	foo(%rip), %r10d
	movl	%r10d, foo(%rip)
	movl	inc(%rip), %r10d
	addl	%r10d, foo(%rip)
	movl	$10, %r11d
	cmpl	foo(%rip), %r11d
	movl	$0, -12(%rbp)
	setg	-12(%rbp)
	cmpl	$0, -12(%rbp)
	je	.Lif_end.3
	jmp	.Lbreak.for.1
.Lif_end.3:
	movl	$5, %r11d
	cmpl	foo(%rip), %r11d
	movl	$0, -16(%rbp)
	sete	-16(%rbp)
	cmpl	$0, -16(%rbp)
	je	.Lif_end.5
	jmp	.Lcontinue.for.1
.Lif_end.5:
.Lcontinue.for.1:
	movl	-4(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	addl	$1, -4(%rbp)
	jmp	.Lfor_start.2
.Lbreak.for.1:
	movl	foo(%rip), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
