# Table of Contents

- [Compiler Driver](#compiler-driver)
- [Token, Lexer](#token-lexer)
- [AST](#ast)
- [Parser](#parser)
- [IdentifierResolution](#identifier-resolution)
- [ValidateLabels](#validatelabels)
- [LoopLabeling](#looplabeling)
- [CollectSwitchCases](#collectswitchcases)
- [Types](#types)
- [Symbols](#symbols)
- [Rounding](#rounding)
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

# Compiler Driver

Add one more stage to our compiler for Object.

```
assemble_and_link(src, link = true, cleanup = true):
    asm_file = replace_extension(src, '.s')
    output_file = link ? remove_extension(src) : replace_extension(src, '.o')
    gcc_args = [asm_file, '-o', output_file]
    if not link:
        gcc_args.insert(1, '-c')
    run_command('gcc', gcc_args)
    if cleanup:
        run_command('rm', [asm_file])
```

```
driver(target, debug, extra_credit, stage, src):
    set platform to target (e.g., OSX | Linux)
    preprocessed_name = preprocess(src)
    asm_name = compile(stage, preprocessed_name)

    if stage == Executable:
        // Link enabled, cleanup disabled if debugging
        assemble_and_link(asm_name, link = true, cleanup = true)
    elif stage == Obj:
        // Link disabled, cleanup depends on debug mode
        assemble_and_link(asm_name, link = false, cleanup = true)
```

# Token, Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| COMMA | , |

# AST

```
program = Program(function_declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init)
function_declaration = (identifier name, identifier* params, block? body)>
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
parse_primary_exp(tokens):
	next_token = peek(tokens)
	if next_token is an int:
		--snip--
	if  next_token is an identifier:
        id = parse_id(tokens)
        // look at next token to figure out whether this is a variable or function call
        if peek(tokens) is "(":
            args = parse_optional_arg_list(tokens)
            return FunCall(id, args)
        else:
            return Var(id)
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

```
parse_optional_arg_list(tokens):
    expect("(", tokens)
    args = peek(tokens) is ")" ? [] : parse_arg_list(tokens)

    expect(")", tokens)
    return args
```

```
parse_arg_list(tokens):
    args = []
    while tokens is not empty:
        arg = parse_exp(0, tokens)
        args.append(arg)

        next_token = peek(tokens)
        if next_token == ",":
            takeToken(tokens)
        else:
            break
    return args
```

```
finish_parsing_function_declaration(name, tokens):
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

    return (name, params, body)
```

```
parse_param_list(tokens):
    expect_token("int", tokens)
    next_param = parse_id(tokens)

    args = [next_param]
    while tokens is not empty:
        next_token = peek(tokens)
        if next_token is ",":
            take_token(tokens)
            expect("int", tokens)
            next_param = parse_id(tokens)
            args.append(next_param)  # Add the parameter to the list
        else:
            break
    return args
```

```
finish_parsing_variable_declaration(name, tokens):
    next_token = take_token(tokens)
    if next_token is ";":
        return (name, init=null)
    else if next_token is "=":
        init = parse_exp(0, tokens)
        expect(";", tokens)
        return (name, init)
    else:
        raise_error("An initializer or semicolon", next_token type)
```

```
parse_declaration(tokens):
    expect("int", tokens)
    name = parse_id(tokens)
    if peek(tokens) is "(":
        return finish_parsing_function_declaration(name, tokens)
    else:
        return finish_parsing_variable_declaration(name, tokens)
```

```
parse_variable_declaration(tokens):
    decl = parse_declaration(tokens)
    if decl is VarDecl:
        return decl
    else:
        fail("Expected variable declaration but found function declaration")
```

```
parse_for_init(tokens):
    if peek(tokens) is "int":
        return AST.InitDecl(parse_variable_declaration(tokens))
    else:
        return AST.InitExp(parse_optional_exp(";", tokens))
```

```
parse_function_declaration_list(tokens):
    function_list = []

    while not tokens.is_empty():
        next_decl = parse_declaration(tokens)

        if next_decl is a FunDecl:
            function_list.append(next_decl)

        else if next_decl is a VarDecl:
            raise_error("Expected function declaration at top level, but found variable declaration")

        else:
            raise_error("Unexpected declaration type")

    return function_list
```

```
parse(tokens):
    functions = parse_function_declaration_list(tokens)
    return Program(functions)
```

# Identifier Resolution

Note that this is the file changed its name from _Variable Resolution_. We changed every place used var_map into id_map.

Update the var_entry for the identifer_map:

```
var_entry = {
    unique_name: string,
    from_current_scope: bool,
    has_linkage: bool,
}
```

The updated _resolve_exp_ is provided in the BOOK_NOTES. I put here for easy reference.

```
resolve_exp(exp, id_map):
    match exp.type:
        case Assignment:
            if exp.left.type != Var:            // validate that lhs is an lvalue
                fail("Invalid lvalue!")
            return Assignment(                  // recursively process lhs and rhs
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map)
            )

        case CompoundAssignment:
            if exp.left.type != Var:            // validate that lhs is an lvalue
                fail("Invalid lvalue!")
            return CompoundAssignment(          // recursively process lhs and rhs
                exp.op,
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map)
            )

        case PostfixIncr:
            if exp.inner.type != Var:
                fail("Invalid lvalue!")
            return PostfixIncr(
                resolve_exp(exp.inner, id_map)
            )

        case PostfixDecr:
            if exp.inner.type != Var:
                fail("Invalid lvalue!")
            return PostfixDecr(
                resolve_exp(exp.inner, id_map)
            )

        case Var:
            if exp.name exists in id_map:                       // rename var from map
                return Var( id_map.get(exp.name).uniqueName )
            else:
                fail("Undeclared variable: " + exp.name)

        // recursively process operands for unary, binary, conditional and function calls

        case Unary:
            // For unary operators like ++/--, the operand must be a variable.
            if (exp.op == Incr or exp.op == Decr) and (exp.inner.type != Var):
                fail("Operand of ++/-- must be an lvalue!")
            return Unary(
                exp.op,
                resolve_exp(exp.inner, id_map)
            )

        case Binary:
            return Binary(
                exp.op,
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map)
            )

        case Constant:
            return exp

        case Conditional:
            return Conditional(
                resolve_exp(exp.condition, id_map),
                resolve_exp(exp.then, id_map),
                resolve_exp(exp.else, id_map)
            )

        case FunctionCall(fun_name, args) ->
            if fun_name is in identifier_map:
                new_fun_name = identifier_map.get(fun_name).new_name
                new_args = []
                for arg in args:
                    new_args.append(resolve_exp(arg, identifier_map))

                return FunctionCall(new_fun_name, new_args)
            else:
                fail("Undeclared function!")

        default:
            fail("Internal error: Unknown expression")
```

Our previous _resolve_declaration_ is only for local variable declarations. We will keep most of the logic, but as we also need to resolve parameters the same way, so we'll split the logic into 2 functions:

- _resolve_local_var_helper_
- _resolve_local_var_declaration_

```
// helper for resolving local variables and parameters; deals with validation and updating the identifier map
resolve_local_var_helper(id_map, name):
    entry = id_map.find(name)

    if entry is not null && entry.from_current_scope:
        // variable is present in the map and was defined in the current scope
        fail("duplicate variable declaration")
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
    new_map, unique_name = resolve_local_var_helper(id_map, var_decl.name)

    resolved_init = null
    if var_decl has init:
        ressolved_init = resolve_exp(var_decl.init)

    return (new_map, (name = unique_name, init=resolved_init))
```

Our AST constructs don't have FunctionDefinition anymore, as FunctionDeclaration can also play a role of a definition if it has body. And function declarations can appear in local scope as well.
We change remove the resolve_function_def and replace it with three functions:

- resolve_local_declaration
- resolve_params
- resolve_function_declaration

```
resolve_local_declaration(id_map, decl):
    if decl is VarDecl:
        new_map, resolved_vd = resolve_local_var_declaration(id_map, decl.decl)
        return (new_map, VarDecl(resolved_vd))
    else if decl is FunDecl and has body:
        fail("Nested function defintiions are not allowed")
    else if decl is FunDecl:
        new_map, resolved_fd = resolve_function_declaration(id_map, decl.decl)
        return (new_map, FunDecl(resolved_fd))
```

```
resolve_params(id_map, params):
    resolved_params = []
    final_map = id_map

    for param in params:
        new_map, new_param = resolve_local_var_helper(id_map, param)
        resolved_params.append(new_param)
        final_map = new_map

    return (final_map, resolved_params)
```

```
resolve_function_declaration(id_map, fun_decl):
    entry = id_map.find(fun_decl.name)

    if entry exists and entry.from_current_scope == True and entry.has_linkage == False:
        fail("Duplicate declaration")
    else:
        new_entry = (
            unique_name=fun_decl.name,
            from_current_scope=True,
            has_linkage=True,
        )

        new_map = id_map.add(fun_decl.name, new_entry)
        inner_map = copy_identifier_map(new_map)
        inner_map1, resolved_params = resolve_params(inner_map, fun_decl.params)
        resolved_body = null

        if fun_decl has body:
            resolved_body = resolve_block(inner_map1, fun_decl.body)

        return (
            new_map,
            fun_decl.name,
            resolved_params,
            resolved_body
        )
```

Update our resolve from the top-level:

```
resolve(Program prog):
    resolved_fun_decls = []

    for fun_decl in prog.fun_decls:
        resolved_fun_decls.append(
            resolve_function_declaration({}, fun_decl)
        )

    return Program(resolved_fun_decls)
```

# ValidateLabels

Our program supports multiple functions in top-level, so now we need to update the validate_labels_in_fun, and make sure the target of goto never be out of the enclosing functions.

When we process any label in a statement, we transform the label to contains the enclosing function name, then return the new statement with that transformed_label. This way, we ensure that in Assembly, an instruction in one subroutine cannot jumpt to an arbitrary place in another subroutine.

```
collect_labels_from_statement(Set<string> defined, Set<string> used, statement, transform_label):
    match statement type:
        case Goto:
            used.add(statement.lbl)
            return Goto(transform_label(statement.lbl))
        case LabeledStatement:
            if defined already has statement.lbl:
                fail("Duplicate label: " + statement.lbl)
            defined.add(statement.lbl)
            new_stmt = collect_labels_from_statement(defined, used, statement.statement, transform_label)
            return LabeledStatement(transform_label(statement.lbl), new_stmt)
        case If:
            new_then_clause = collect_labels_from_statement(defined, used, statement.thenClause, transform_label)
            new_else_clause = None
            if statement has elseClause:
                new_else_clause = collect_labels_from_statement(defined, used, statement.elseClause, transform_label)
            return If(statement.condition, new_then_clause, new_else_clause)
        case Compound:
            new_block = []
            for block_item in statement.block:
                new_item = collect_labels_from_block_item(defined, used, block_item, transform_label)
                new_block.append(new_item)
            return Compound(new_block)
        case While:
            new_body = collect_labels_from_statement(defined, used, statement.body, transform_label)
            return While(statement.condition, new_body, statement.id)
        case DoWhile:
            new_body = collect_labels_from_statement(defined, used, statement.body, transform_label)
            return DoWhile(statement.condition, new_body, statement.id)
        case For:
            new_body = collect_labels_from_statement(defined, used, statement.body, transform_label)
            return For(statement.init, statement.condition, statement.post, new_body, statement.id)
        case Switch:
            new_body = collect_labels_from_statement(defined, used, statement.body, transform_label)
            return Switch(statement.exp, new_body, statement.id)
        case Case:
            new_stmt = collect_labels_from_statement(defined, used, statement.statement, transform_label)
            return Case(statement.value, new_stmt, statement.id)
        case Default:
            new_stmt = collect_labels_from_statement(defined, used, statement.statement, transform_label)
            return Default(new_stmt, statement.id)
        default:
            return statement
```

```
collect_labels_from_block_item(defined, used, block_item, transform_label)
	if block_item type is a Statement:
		new_stmt = collect_labels_from_statement(defined, used, block_item, transform_label)
        return new_stmt
	else:
		return block_item
```

```
validate_labels_in_fun(fun_decl):
	labels_defined = []
	labels_used = []

    transform_label = (lbl) -> {
        return fun_decl.name + "." + lbl
    }

    if fun_decl has body:
        renamed_block = []

        for block_item in fun_decl.body:
		    new_block_item = collect_labels_from_block_item(labels_defined, labels_used, block_item, transform_label)
            renamed_block.append(new_block_item)

        undefined_labels = []
        for label in labels_used:
            if label_defined doesn't has label:
                undefined_labels.append(label)

        if undefined_labels is not empty:
            errMsg = "Found labels that are used but not defined: "

            for label in undefined_labels:
                errMsg += label + ", "

            fail(errMsg)

        return (fun_decl.name, fun_decl.params, body=renamed_block)
    else:
        return fun_decl

```

```
validate_labels(AST.Program prog):
    validated_funs = []
    for fun in prog.fun_decls:
        new_fun = validate_labels_in_fun(fun)
        validated_funs.append(new_fun)

    return Program(validated_funs)
```

# LoopLabeling

No major changes. We only change the top-level label_loops to label loops in multiple functions, not one.

```
label_loops(AST.Program prog):
    labeled_funs = []

    for fun_decl in prog.fun_decls:
        labeled_funs.append(label_function_def(fun_decl))

    return Program(labeled_funs)
```

# CollectSwitchCases

No major changes either. We also change the top-level for multiple function declarations. In each function declaration, check if it has body. If it does, analyze_block it and return new function declaration.

```
analyze_switches(Program prog):
    analyzed_funs = []
    for fun_decl in prog.fun_decls:
        analyzed_funs.append(analyze_function_def(fun_def))

    return Program(analyzed_funs)
```

# Types

```
type Int {}
type FunType {
    param_count: int,
}

t = Int | FunType
```

# Symbols

Defines our symbol tables here.

```
entry = {
    t: Types.t,
    is_defined: bool,
    stack_frame_size: int,
}

add_var(name, t):
    symbols.add(name, entry(
        t,
        is_defined=False,
        stack_frame_size=0,
    ))

add_fun(name, t, is_defined):
    symbols.add(name, entry(
        t,
        is_defined,
        stack_frame_size=0,
    ))

get(name):
    return symbols.find(name)

get_opt(name):
    if symbols.has(name):
        return symbols.find(name)
    else:
        return null

set_bytes_required(name, bytes_required):
    entry = symbols.find(name)
    symbols.add(name, (
        t=entry.t,
        is_defined=entry.is_defined,
        stack_frame_size=bytes_required
    ))
```

# Rounding

Create a new file Rounding, or a function in your util file, with the primary purpose to round the size of stack padding to the next multiple of 16.

```
// Rounds x up to the nearest multiple of n.
roundAwayFromZero(int n, int x):
    if x % n == 0: // x is already a multiple of n
        return x

    if x < 0:
        return x - n - (x % n)

    return x + n - (x % n)
```

# TypeCheck

In Part I, our Type Checker won't transform AST; it simply check for errors. But it will in Part II.

```
typecheck_exp(exp):
    match type of exp:
        case FunCall(f_name, args):
            t = symbols.get(f_name).t

            if t is Int:
                fail("Tried to use variable as function name")
            else if t is FunType:
                if length(args) != t.param_count:
                    fail("Function called with wrong number of arguments")

                for arg in args:
                    typecheck_exp(arg)
        case Var(v):
            t = Symbols.get(v)
            if t is Int:
                // Do nothing
            else if t is FunType:
                fail("Tried to use function name as variable")
        case Unary(_, inner):
            typecheck_exp(inner)
        case Binary(_, e1, e2):
            typecheck_exp(e1)
            typecheck_exp(e2)
        case Assignment(lhs, rhs):
            typecheck_exp(lsh)
            typecheck_exp(rhs)
        case CompoundAssignment(_, lhs, rhs):
            typecheck_exp(lhs)
            typecheck_exp(rhs)
        case PostfixDecr(e):
            typecheck_exp(e)
        case PostfixIncr(e):
            typecheck_exp(e)
        Conditional(condition, then_result, else_result):
            typecheck_exp(condition)
            typecheck_exp(then_result)
            typecheck_exp(else_result)
        Constant():
            // Do nothing
```

```
typecheck_block(block):
    checked_block = []

    for block_item in block:
        checked_block.append(typecheck_block_item(block_item))

    return checked_block
```

```
typecheck_block_item(block_item):
    if block_item is statement:
        return typecheck_statement(block_item)
    else if block_item is declaration:
        return typecheck_declaration(block_item)
```

```
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
            typecheck_var_decl(init.d)
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
typecheck_decl(decl):
    if decl is VarDecl: typecheck_var_decl(decl)
    else if decl is FunDecl: typecheck_fn_decl(decl)
```

```
typecheck_var_decl(var_decl):
    Symbols.add_var(var_decl.name, t.Int)
    if var_decl has init:
        typecheck_exp(var_decl.init)
```

```
typecheck_fn_decl(fn_decl):
    fun_type = Types.FunType(param_count=length(fn_decl.params))
    has_body = fn_decl has body?

    if fn_decl.name is in symbols:
		old_decl = symbols.get(fn_decl.name)
		if old_decl.type != fun_type:
			fail("Incompatiable function declaration")
		if old_decl.defined and has_body:
			fail("Function is defined more than once")

    symbols.add(fn_decl.name, fun_type, defined=(old_decl?.defined or has_body))

	if has_body:
		for param in fn_decl.params:
			symbols.add(param, Int)
		typecheck_block(fn_decl.body)
```

```
typecheck(Program prog):
    checked_funs = []
    for fn_decl in prog.fun_decls:
        checked_fun = typecheck_fn_decl(fn_decl)
        checked_funs.append(checked_fn)

    return Program(checked_funs)
```

# TACKY

As the BOOK_NOTES, we add a construct for FunctionCall in Tacky, change the top-level program to have several function declarations, and record the params in each function.

```
program = Program(function_definition*)
function_definition = Function(identifier, identifier* params, instruction* body)
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
emit_tacky_for_exp(exp):
	match exp type:
		case AST.Constant:
            return ([], TACKY.Constant)
		case AST.Var:
            return ([], TACKY.Var)
		case Unary:
			if exp.op is increment:
				if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
				return emit_compound_assignment(AST.Add, exp.exp.name, AST.Constant(1))
			else if exp.op is decrement:
				if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
				return emit_compound_assignment(AST.Subtract, exp.exp.name, AST.Constant(1))
			else:
				return emit_unary_expression(exp)
		case Binary:
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_or_expression(exp)
			else:
				return emit_binary_expression(exp)
		case Assignment:
			if exp.left is not AST.Var:
				fail("Internal Error: bad lvalue!") // We also checked in the Semantic Analysis that Left is a Var

			rhs_instructions, rhs_result = emit_tacky_for_exp(exp.right)
			insts = [
				...rhs_instructions,
				Tacky.Copy(rhs_result, Tacky.Var(exp.left.name))
			]

			return (insts, rhs_result)
		case CompoundAssignment:
			if exp.left is not AST.Var:
				fail("Internal Error: bad lvalue!")
			return emit_compound_assignment(exp.op, exp.left, exp.right)
		case PostfixIncr:
			if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
			return emit_postfix(AST.Add, exp.exp.name)
		case PostfixDecr:
			if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
			return emit_postfix(AST.Subtract, exp.exp.name)
        case FunCall:
            return emit_fun_call(exp)
```

```
emit_fun_call(AST.FunctionCall fun_call):
    dst_name = UniqueIds.make_temporary()
    dst = Tacky.Var(dst_name)
    arg_insts = []
    arg_vals = []

    for arg in fun_call.args:
        insts, v = emit_tacky_for_exp(arg)
        arg_insts.append(...insts)
        arg_vals.append(v)

    insts = [
        ...arg_insts,
        Tacky.FunCall(fun_decl.name, arg_vals, dst)
    ]

    return (insts, dst)
```

```
// Change the emit_declaration to emit_local_declaration
emit_tacky_for_block_item(block_item):
	match block_item type:
		case Statement:
			return emit_tacky_for_statement(block_item)
		case Declaration:
			return emit_local_declaration(block_item)
```

A new layer to check whether the local declaration is for variable or for function.

```
emit_local_declaration(decl):
    if decl is VarDecl:
        return emit_var_declaration(decl.decl)
    else:
        return []
```

The previous _emit_declaration_ is for variable. Update its name to _emit_var_declaration_.

```
emit_var_declaration(declaration):
    if declaration has init:
        // Treat declaration with initializer as assignment
        eval_assignment, v = emit_tacky_for_exp(AST.Assignment(AST.Var(declaration.name), declaration.init))
        return eval_assignment
    else:
        // Do not generate any instructions
        return []
```

ForInit in ForLoop support declarations of variables, not functions.

```
emit_tacky_for_for_loop(for_loop):
    start_label = UniqueIds.make_label("for_start")
    cont_label = continue_label(for_loop.id)
    br_label = break_label(for_loop.id)
    for_init_insts = []

    if for_loop.init is declaration:
        for_init_insts = emit_var_declaration(for_loop.init)
    else: // for_loop.init is an opt_exp
        if for_loop.init is not null:
            for_init_insts = emit_tacky_for_exp(for_loop.init)
        else:
            for_init_insts = []

    test_condition = []

    if for_loop.condition is not null:
        inner_insts, v = emit_tacky_for_exp(for_loop.condition)

        test_condition = [
            ...inner_insts,
            JumpIfZero(v, br_label)
        ]

    post_insts = []

    if for_loop.post is not null:
        inner_insts, v = emit_tacky_for_exp(for_loop.post)
        post_insts = inner_insts

    body_insts = emit_tacky_for_statement(for_loop.body)

    insts = [
        ...for_init_insts,
        Label(start_label),
        ...test_condition,
        ...body_insts,
        Label(cont_label),
        ...post_insts,
        Jump(start_label),
        Label(br_label)
    ]
```

Change the function name from _emit_tacky_for_function_ to _emit_fun_declaration_

```
emit_fun_declaration(fun_decl):
    insts = []

    if fun_decl has body:
        for block_item in fun_def.body:
            emit_insts = emit_tacky_for_block_item(block_item)
            insts.append(...emit_insts)

        extra_return = Tacky.Return(Constant(0))
        insts.append(extra_return)

        return Tacky.Function(fun_def.name, fun_decl.params, insts)

    return null
```

```
gen(AST.Program prog):
    tacky_fn_defs = []

    for fun_decl in prog.fun_decls:
        new_fun = emit_fun_declaration(fun_decl)
        if new_fun is not null:
            tacky_fn_defs.append(new_fun)

    return Tacky.Program(tacky_fn_defs)
```

# Assembly

We added constructs as BOOK_NOTES suggests.

```
program = Program(function_definition*)
function_definition = Function(identifier name, instruction* instructions)
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
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11
```

# CodeGen

Create global list of registers used to pass params:

```
param_passing_regs = [DI, SI, DX, CX, R8, R9]
```

The main change is function call, so let convert it assembly.

```
convert_function_call(TACKY.FunCall fun_call)
    reg_args, stack_args = fun_call.args[:6], fun_call.args[6:]

    // adjust stack alignment
    stack_padding = length(stack_args) % 2 == 0 ? 0 : 8
    insts = []

    if stack_padding != 0:
        inst.append(AllocateStack(stack_padding))

    // pass args in registers
    reg_idx = 0
	for tacky_arg in reg_args:
		r = param_passing_regs[reg_idx]
		assembly_arg = convert_val(tacky_arg)
        insts.append(Mov(assembly_arg, Reg(r)))
		reg_idx += 1

   // pass args on the stack
    for tacky_arg in reverse(stack_args):
		assembly_arg = convert_val(tacky_arg)
		if assembly_arg is a Reg or Imm operand:
			insts.append(Push(assembly_arg))
		else:
            // copy into a register before pushing
			insts.append(Mov(assembly_arg, Reg(AX)))
			insts.append(Push(Reg(AX)))

    insts.append(Call(fun_call.name))

    // adjust stack pointer
    bytes_to_remove = 8 * length(stack_args) + stack_padding
    if bytes_to_remove != 0:
        insts.append(DeallocateStack(bytes_to_remove))

    // Retrieve return value
    assembly_dst = convert_val(fun_call.dst)
    insts.append(Mov(Reg(AX), assembly_dst))

    return insts
```

Now in convert_instructions, add the case to convert from Tacky Function call to Assembly.

```
convert_instruction(Tacky.Instruction inst):
	match inst type:
		case Copy:
			asm_src = convert_val(inst.src)
			asm_dst = convert_dst(inst.dst)

			return [Mov(asm_src, asm_dst)]

		case Return:
			--snip--
		case Unary:
			match inst.op:
				case Not:
					asm_src = convert_val(inst.src)
					asm_dst = convert_val(inst.dst)

					return [
						Cmp(Imm(0), asm_src),
						Mov(Imm(0), asm_dst),
						SetCC(E, asm_dst)
					]
				default:
					--snip--
		case Binary:
			asm_src1 = convert_val(inst.src1)
			asm_src2 = convert_val(inst.src2)
			asm_dst = convert_dst(inst.dst)

			match inst.op:
				(* Relational operator *)
				case Equal:
				case NotEqual:
				case GreaterThan:
				case GreaterOrEqual:
				case LessThan:
				case LessOrEqual:
					cond_code = convert_cond_code(inst.op)

					return [
						Cmp(asm_src2, asm_src1),
						Mov(Imm(0), asm_dst),
						SetCC(cond_code, asm_dst),
					]
				default:
					--snip--
		case Jump:
			return [ Jmp(inst.target) ]

		case JumpIfZero:
			asm_cond = convert_val(inst.cond)

			return [
				Cmp(Imm(0), asm_cond),
				JmpCC(E, inst.target),
			]

		case JumpIfNotZero:
			asm_cond = convert_val(inst.cond)

			return [
				Cmp(Imm(0), asm_cond),
				JmpCC(NE, inst.target),
			]

		case Label:
			return [ Assembly.Label(inst.name) ]
        case FunCall:
            return convert_function_call(inst)
```

We're done with function calling. How about function definition?
We define how params are passed:

```
pass_params(param_list):
    reg_params, stack_params = param_list[:6], param_list[6:0]
    insts = []

    // pass params in registers
    reg_idx = 0
    for tacky_param in reg_params:
        r = param_passing_regs[reg_idx]
        assembly_param = Pseudo(tacky_param)
        insts.append(Mov(Reg(r), assembly_param))
        reg_idx++

    // pass params on the stack
    // first param passed on stack has idx0 and is passed at Stack(16)
    stk_idx = 0
    for tacky_param in stack_params:
        stk = Stack(16 + (8*stk_idx))
        assembly_param = Pseudo(tacky_param)
        inst.append(Mov(stk, assembly_param))
        stk_idx++

    return insts
```

```
convert_function(Tacky.Function fun_def):
    params_insts = pass_params(fun_def.params)

    insts = [...params_insts]

    for inst in fun_def.body:
        insts.append(...convert_instruction(inst))

    return Function(fun_def.name, insts)
```

```
gen(Tacky.Program prog):
    converted_funs = []

    for fn_def in prog.fun_defs
        insts = convert_functionI(fn_def)
        converted_funs.append(...insts)

    return Assembly.Program(insts)
```

# ReplacePseudo

We extend the pass to replace pseudo in Push instructions.
We have pseudo operand in Push now, but we will in Part II.

```
replace_pseudos_in_instruction(Assembly.Instruction inst, state):
	match inst.type:
		--snip--
		case Binary:
			--snip--
		case Cmp:
			state1, new_op1 = replace_operand(inst.src, state)
			state2, new_op2 = replace_operand(inst.dst, state1)
			new_cmp = Cmp(new_op1, new_op2)

			return (state2, new_cmp)
		case Idiv:
			--snip--
		case SetCC:
			state1, new_operand = replace_operand(inst.operand, state)
			new_setcc = SetCC(inst.cond_code, new_operand)

			return (state1, new_setcc)
        case Push:
            state1, new_op = replace_operand(state, inst.op)
            new_push = Push(new_op)

            return (state1, new_push)
		case Ret:
		case Cdq:
		case Label:
		case JmpCC:
		case Jmp:
        case DeallocateStack:
        case Call:
			return (state, inst)
		case AllocateStack:
		    return (state, inst)
```

```
replace_pseudos_in_function(Assembly.Function fn):
	curr_state = init_state
	fixed_instructions = []

	for inst in fn.instrustions:
		new_state, new_inst = replace_pseudos_in_instruction(inst, curr_state)
		curr_state = new_state
		fixed_instructions.push(new_inst)

    Symbols.set_bytes_required(fn.name, curr_state.current_offset)

	new_fn = Assembly.Function(fn.name, fixed_instructions)
	return new_fn, curr_state.current_offset

replace_pseudos(Assembly.Program prog):
    fixed_defs = []

    for fn_def in prog.fun_defs:
        new_def = replace_pseudo_in_function(fn_def)
        fixed_defs.append(new_def)

	return Assembly.Program(fixed_defs)
```

# Instruction Fixup

```
fixup_function(Assembly.Function fn_def):
    stack_bytes = -Symbols.get(fn_def.name).stack_frame_size
	alloc_stack = AllocateStack(round_away_from_zero(16, stack_bytes))

	return Assembly.Function(fn_def.name, [alloc_stack, ...fn_def.instructions])
```

```
fixup_program(Assembly.Program prog):
    fixed_defs = []

    for fn_def in prog.fn_defs:
        fixed_fun = fixup_function(fn_def)
        fixed_defs.append(fixed_fun)

    return Assembly.Program(fixed_defs)
```

# Emit

```
show_reg(reg):
    match reg:
        case AX -> %eax
        case CX -> %ecx
        case DX -> %edx
        case DI -> %edi
        case SI -> %esi
        case R8 -> %r8d
        case R9 -> %r9d
        case R10 -> %r10d
        case R11 -> %r11d
```

```
show_operand(operand):
    if operand is Reg:
        return show_reg(operand)
    else if operand is Imm:
        return "${operand.value}"
    else if operand is Stack:
        return "%{operand.offset}(%rbp)"
    else if operand is pseudo: // For debugging
        return operand.name
```

```
show_byte_reg(reg):
     match reg:
        case AX -> %al
        case CX -> %cl
        case DX -> %dl
        case DI -> %dil
        case SI -> %sil
        case R8 -> %r8b
        case R9 -> %r9b
        case R10 -> %r10b
        case R11 -> %r11b
```

```
show_byte_operand(operand):
    if operand is Reg:
        return show_byte_reg(operand)
    else:
        show_operand(operand)
```

```
show_quadword_reg(reg):
    match reg:
        case AX -> %rax
        case CX -> %rcx
        case DX -> %rdx
        case DI -> %rdi
        case SI -> %rsi
        case R8 -> %r8
        case R9 -> %r9
        case R10 -> %r10
        case R11 -> %r11
```

```
show_quadword_operand(operand):
    if operand is Reg:
        return show_quadword_reg(operand)
    else:
        return show_operand(operand)
```

```
show_fun_name(fnCall):
    if platform is OS_X:
        return "_{fnCall.name}"
    else if platfrom is Linux:
        return "{fnCall}@PLT"
```

```
emit_instruction(inst):
	match inst type:
		case Mov:
			--snip--
		case Unary:
			--snip--
		case Binary:
			--snip--
		Cmp:
			--snip--
		Idiv:
			--snip--
		Cdq:
			--snip--
		Jmp:
			--snip--
		JmpCC:
			--snip--
		SetCC:
			--snip--
		Label:
			--snip--
		AllocateStack:
            --snip--
        DeallocateStack:
            return "\taddq ${inst.value}, %rsp\n"
        Push:
            return "\tpushq {show_quadword_operand(inst.op)}\n"
        Call:
            return "\tcall {show_fun_name(inst)}\n"
        Ret:
            --snip--
```

```
emit(Assembly.Program prog):
    file = open(file)

    content = ""
    for fn_def in prog.fun_defs:
        content += emit_function(fn_def)

	content += emit_stack_note()

	write content to file
    close file
```

# Output

From C:

```C
int foo(int a, int b, int c, int d, int e, int f, int g, int h) {
    putchar(h);
    return a + g;
}

int main(void) {
    return foo(1, 2, 3, 4, 5, 6, 7, 65);
}
```

To x64 Assembly on Linux:

```asm
	.globl foo
foo:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	%edx, -12(%rbp)
	movl	%ecx, -16(%rbp)
	movl	%r8d, -20(%rbp)
	movl	%r9d, -24(%rbp)
	movl	16(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	24(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	movl	-32(%rbp), %edi
	call	putchar@PLT
	movl	%eax, -36(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -40(%rbp)
	movl	-28(%rbp), %r10d
	addl	%r10d, -40(%rbp)
	movl	-40(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$1, %edi
	movl	$2, %esi
	movl	$3, %edx
	movl	$4, %ecx
	movl	$5, %r8d
	movl	$6, %r9d
	pushq	$65
	pushq	$7
	call	foo@PLT
	addq	$16, %rsp
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret


	.section .note.GNU-stack,"",@progbits
```
