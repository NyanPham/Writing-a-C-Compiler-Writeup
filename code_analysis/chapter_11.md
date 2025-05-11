# Table of Contents

- [Token, Lexer](#token-lexer)
- [Types](#types)
- [AST](#ast)
- [Const](#const)
- [Parser](#parser)
- [ConstConvert](#ConstConvert)
- [IdentifierResolution](#identifier-resolution)
- [ValidateLabels](#validatelabels)
- [LoopLabeling](#looplabeling)
- [CollectSwitchCases](#collectswitchcases)
- [TypeUtils](#typeutils)
- [Initializers](#initializers)
- [Symbols](#symbols)
- [TypeCheck](#typecheck)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly](#assembly)
- [AssemblySymbols](#assemblysymbols)
- [CodeGen](#codegen)
- [ReplacePseudo](#replacepseudo)
- [Instruction Fixup](#instruction-fixup)
- [Emit](#emit)
- [Output](#output)

---

We will have some new files in the source code:

- Const
- ConstConvert
- Initializer
- TypeUtils

# Token, Lexer

We remove the Constant type as we need 2 types to represent Int and Long.
Ensure both values in Int and Long is the maximum number a 2<sup>63</sup> bit can hold.

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordLong | long |
| ConstInt | [0-9]+\b |
| ConstLong | [0-9]+[lL]\b |

In Lexer, as shown in the very first chapter, we convert string into and Int Token as following:

```
convert_int(n):
    return Token(TypeToken.Constant, n)
```

We'll change the token type in this TypeToken.ConstInt, and create the same convert function for long integers:

```
convert_int(n):
    return Token(TypeToken.ConstInt, n)
```

```
// str includes the suffic l/L, chop it of
convert_long(str):
    n = Number(str.chop(-1))
    return Token(TypeToken.ConstLong, n)
```

# Types

```
type Int {}
type Long {}
type FunType {
    param_types: t[],
    ret_type: t
}

t = Int | Long | FunType
```

# AST

We reuse the type system defined in Types (We'll extend it to including _long_ and update the _FunType_).
Variable declaration and function declarations have their types added.
Cast expression is needed for both implicit and explicit type casting.
Every expression node needs to have type field.

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
type = Int | Long | FunType(type* params, type ret)
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
exp = Constant(const, type)
    | Cast(target_type, exp, type)
    | Var(identifier, type)
    | Unary(unary_operator, exp, type)
    | Binary(binary_operator, exp, exp, type)
    | Assignment(exp, exp, type)
    | CompoundAssignment(binary_operator, exp, exp, type)
    | PostfixIncr(exp, type)
    | PostfixDecr(exp, type)
    | Conditional(exp condition, exp then, exp else, type)
    | FunctionCall(identifier, exp* args, type)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int)
```

# Const

_const_ in Constant expression is defined in a separate file named Const as we'll use them both in AST and TACKY AST.
It's up to you to have the same approach, or you can just create separate ConstInt and ConstLong nodes for each stage.

In either case, it should look like this:

```
type t = ConstInt(int32) | ConstLong(int64)

int_zero = ConstInt(0)
long_zero = ConstLong(0)
```

# Parser

```
// Our type system is growing to support Long, so we need a separate function to classify "int" and "long" to be types list.
parse_type_specifier_list(tokens):
    type_specifiers = []
    next_token = peek(tokens)

    while next_token is "int" or "long":
        spec = take(tokens)
        type_specifiers.append(spec)
        next_token = peek(tokens)

    return type_specifiers
```

```
// "long" is also a specifier
parse_specifier_list(tokens):
    specifiers = []
    next_token = peek(tokens)

    while next_token is "int" or "long" or "static" or "extern":
        spec = take(tokens)
        specifiers.append(spec)
        next_token = peek(tokens)

    return specifiers
```

```
// A new function to deduce the tokens of types to be the real type
parse_type(type_list):
    if length(type_list) == 1:
        if type_list[0] is "int":
            return Type.Int
        if type_list[0] is "long":
            return Type.Long
        else:
            goto invalid
    else if length(type_list) == 2:
        if (type_list[0] is "int" and type_list[1] is "long") or (type_list[0] is "long" and type_list[1] is "int"):
            return Long
        else:
            goto invalid
invalid:
    fail("Invalid type specifier")
```

```
// Call parse_type instead of assuming the type is Int
parse_type_and_storage_class(specifier_list):
    types = []
    storage_classes = []

    for tok in specifier_list:
        if tok is "int" or "long":
            types.append(tok)
        else:
            storage_classes.append(tok)

    type = parse_type(types)

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
parse_constant(tokens):
    MAX_INT64 = (BigInt(1) << 63) - 1
    MAX_INT32 = (BigInt(1) << 31) - 1

    tok = take_token(tokens)

    if tok.value > MAX_INT64:
        fail("Constant is too large to fit in an int or long")

    if tok is "int" and tok.value <= MAX_INT32:
        return Const.ConstInt(cast_to_int_32(tok.value))
    else:
        return Const.ConstLong(cast_to_int_64(tok.value))
```

```
// Constants are primary expressions
parse_primary_exp(tokens):
	next_token = peek(tokens)
	if next_token is "int" or "long":
		return parse_constant(tokens)
	if  next_token is an identifier:
        --snip--
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

```
// Cast is an expression, at the factor level. So we'll extend the parse_factor
// by adding one more token case check: the 1st token is "(" and the 2nd token is "int" or "long"
parse_factor(tokens):
	next_tokens = npeek(2, tokens) // We peek 2 tokens
	// Unary expression
	if next_token[0] is Tilde, Hyphen, Bang, DoublePlus, DoubleHyphen:
		op = parse_unop(tokens)
		inner_exp = parse_factor(tokens)
		return Unary(op, inner_exp)
    else if next_token[0] is "(" and next_token[1] is "int" or "long":
        // It's a cast, consume the "(", then parse the type specifiers
        take_token(tokens)
        type_specifiers = parse_type_specifier_list(tokens)
        target_type = parse_type(type_specifiers)
        expect(")", tokens)
        inner_exp = parse_factor(tokens)
        return Cast(target_type, inner_exp)
	else:
		return parse_postfix_exp(tokens)
```

```
// Long keyword signals a declaration
parse_block_item(tokens):
    match peek(tokens):
        case "int":
        case "long":
        case "static":
        case "extern":
            return parse_declaration(tokens)
        default:
            return parse_statement(tokens)
```

```
// Function is no longer assumed to return _int_, and their params are not only _int_ either.
// We pass in the ret_type to be able to construct FunType
finish_parsing_function_declaration(name, storage_class, ret_type, tokens):
    expect("(", tokens)
    params_with_types = []

    if peek(tokens) is ")":
        params_with_types = []
    else:
        params_with_types = parse_param_list(tokens) // parse_param_list is extended to return not only params' names, but also types

    param_types, params = split(params_with_types)

    expect(")", tokens)
    next_token = peek(tokens)

    body = null
    if next_token is "{":
        body = parse_block(tokens)
    else if next_token is ";":
        body = null
    else:
        raise_error("function body or semicolon", next_token type)

    fun_type = FunType(params_types, ret_type)
    return (name, fun_type, params, body, storage_class)
```

```
parse_param_list(tokens):
    specifiers = parse_type_specifier_list(tokens)
    param_type = parse_type(specifiers)
    next_param = parse_id(tokens)

    params_with_types = [(param_type, next_param)]

    while tokens is not empty:
        next_token = peek(tokens)
        if next_token is ",":
            take_token(tokens)
            specifiers = parse_type_specifier_list(tokens)
            param_type = parse_type(specifiers)
            next_param = parse_id(tokens)
            params_with_types.append((param_type, next_param))
        else:
            break

    return params_with_types
```

```
// Variables also have types now, so we pass down the type from the common declaration parsing function
finish_parsing_variable_declaration(name, storage_class, var_type, tokens):
    next_token = take_token(tokens)
    if next_token is ";":
        return (name, init=null, var_type, storage_class)
    else if next_token is "=":
        init = parse_exp(0, tokens)
        expect(";", tokens)
        return (name, init, var_type, storage_class)
    else:
        raise_error("An initializer or semicolon", next_token type)
```

```
parse_declaration(tokens):
    // expect("int", tokens) We replace this with 2 parsing functions to support more specifier beside "int": "static" and "extern"
    specifiers = parse_specifier_list(tokens)
    type, storage_class = parse_type_and_storage_class(specifiers) // We do use the type now, dealing with function and variable declaration.

    name = parse_id(tokens)
    if peek(tokens) is "(":
        return finish_parsing_function_declaration(name, storage_class, type, tokens)
    else:
        return finish_parsing_variable_declaration(name, storage_class, type, tokens)
```

```
// "long" signals a declaration in for_init as well
parse_for_init(tokens):
    if peek(tokens) is "int" or "long" or "static" or "extern": // Note that we choose to parse them static and extern here, and let the semantic analysis to catch this invalid usage
        return AST.InitDecl(parse_variable_declaration(tokens))
    else:
        return AST.InitExp(parse_optional_exp(";", tokens))
```

# ConstConvert

Before jumping to our familiar territory of Semantic Analysis, we'll have new helper to convert the int value to be in the range of the target type.

```
const_convert(Types.t target_type, Const.const const):
    match const:
        case ConstInt(i):
            if target_type is Types.Int:
                return const
            else:
                return ConstLong(cast_to_int64(i))
        case ConstLong(i):
            if target_type is Types.Long:
                return const
            else:
                return ConstInt(cast_to_int32(i))
```

# Identifier Resolution

```
// Add the case for Cast expression
resolve_exp(exp, id_map):
    match exp.type:
        case Assignment:
            if exp.left.type != Var:            // validate that lhs is an lvalue
                fail("Invalid lvalue!")
            return Assignment(                  // recursively process lhs and rhs
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map),
                exp.type
            )
        case CompoundAssignment:
            if exp.left.type != Var:            // validate that lhs is an lvalue
                fail("Invalid lvalue!")
            return CompoundAssignment(          // recursively process lhs and rhs
                exp.op,
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map),
                exp.type
            )
        case PostfixIncr:
            if exp.inner.type != Var:
                fail("Invalid lvalue!")
            return PostfixIncr(
                resolve_exp(exp.inner, id_map),
                exp.type
            )
        case PostfixDecr:
            if exp.inner.type != Var:
                fail("Invalid lvalue!")
            return PostfixDecr(
                resolve_exp(exp.inner, id_map),
                exp.type
            )
        case Var:
            if exp.name exists in id_map:                       // rename var from map
                return Var( id_map.get(exp.name).uniqueName, exp.type )
            else:
                fail("Undeclared variable: " + exp.name)
        // recursively process operands for casts, unary, binary, conditional and function calls
        case Cast:
            return Cast(
                exp.target_type,
                resolve_exp(exp.exp, id_map),
                exp.type
            )
        case Unary:
            // For unary operators like ++/--, the operand must be a variable.
            if (exp.op == Incr or exp.op == Decr) and (exp.inner.type != Var):
                fail("Operand of ++/-- must be an lvalue!")
            return Unary(
                exp.op,
                resolve_exp(exp.inner, id_map),
                exp.type
            )
        case Binary:
            return Binary(
                exp.op,
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map),
                exp.type
            )
        case Constant:
            return exp
        case Conditional:
            return Conditional(
                resolve_exp(exp.condition, id_map),
                resolve_exp(exp.then, id_map),
                resolve_exp(exp.else, id_map),
                exp.type
            )
        case FunctionCall(fun_name, args) ->
            if fun_name is in identifier_map:
                new_fun_name = identifier_map.get(fun_name).new_name
                new_args = []
                for arg in args:
                    new_args.append(resolve_exp(arg, identifier_map))

                return FunctionCall(new_fun_name, new_args, exp.type)
            else:
                fail("Undeclared function!")
        default:
            fail("Internal error: Unknown expression")
```

```
// var_type field exists in var_declaration
resolve_local_var_declaration(id_map, var_decl):
    new_map, unique_name = resolve_local_var_helper(id_map, var_decl.name, var_decl.storage_class)

    resolved_init = null
    if var_decl has init:
        ressolved_init = resolve_exp(var_decl.init)

    return (new_map, (name = unique_name, init=resolved_init, var_decl.var_type, var_decl.storage_class))
```

Note that function_declaration also has a new field fun_type. Update your code to reflect the AST node changes.

# ValidateLabels

_We have no changes for this pass_

# LoopLabeling

_We have no changes for this pass_

# CollectSwitchCases

Currently, we pass around the optional map as we traverse the AST. The map is present when we're in the switch statement.
Now, we pass an optional pair of (type, map) as we traverse the AST.
The map is, as before, for cases, and the type is for the type of the switch statement's controlling expression.

Instead of opt_case_map, we call it opt_switch_ctx.

```
analyze_case_or_default(key, opt_switch_ctx, lbl, inner_stmt):
    // First, make sure we're in a switch statement
    if opt_switch_ctx is null:
        fail("Found case statement outside of switch")

    [switch_t, case_map] = opt_switch_ctx

    // Convert key to type of controlling switch statement
    if key is not null:
        key = ConstConvert.const_convert(switch_t, key)

    // Check for duplicates
    if case_map already has key:
        if key is not null:
            fail("Duplicate case in switch statement: {key}")
        else:
            fail("Duplicate default in switch statement")

    // Generate new ID - lbl should be "case" or "default"
    case_id = UniqueIds.make_label(lbl)
    updated_ctx = (switch_t, case_map.set(key, case_id))

    // Analyze inner statement
    final_ctx, new_inner_statement = analyze_statement(updated_ctx, inner_statement)

    return final_ctx, new_inner_statement, case_id
```

```
analyze_statement(stmt, opt_switch_ctx):
    switch stmt type:
        case Default(stmt, id):
            new_switch_ctx, new_stmt, default_id = analyze_case_or_default(null, opt_switch_ctx, "default", stmt)
            return (new_switch_ctx, Default(new_stmt, default_id))
        case Case(v, stmt, id):
            // Get integer value of this case
            if v is not a Constant(c):
                fail("Non-constant label in case statement")

            key = v.c
            new_switch_ctx, new_stmt, case_id = analyze_case_or_default(key, opt_switch_ctx, "case", stmt)

            return (new_switch_ctx, Case(v, new_stmt, case_id))
        case Switch(control, body, cases, id):
            // Use fresh map/expression type when traversing body
            switch_t = get_type(control)
            new_ctx, new_body = analyze_statement(body, {})
            // annotate switch with new case map; dont' pass new case map to caller
            // first item in new_ctx is controlling switch_t, which we don't need at this point
            return (opt_switch_ctx, Switch(control, new_body, cases=new_ctx.case_map, id))

        // Just pass case_map through to substatements
        case If(condition, then_clause, else_clause):
            opt_switch_ctx1, new_then_clause = analyze_statement(then_clause, opt_switch_ctx)

            opt_switch_ctx2 = opt_switch_ctx1
            new_else_clause = null

            if else_clause is not null:
                opt_switch_ctx2, new_else_clause = analyze_statement(else_clause, opt_switch_ctx1)

            return (opt_switch_ctx2, If(condition, new_then_clause, new_else_clause))
        case Compound(block):
            new_ctx, new_block = analyze_block(opt_switch_ctx, block)
            return (new_ctx, Compound(new_block))
        case While(condition, body, id):
            new_ctx, new_body = analyze_statement(opt_switch_ctx, body)
            return (new_ctx, While(condition, new_body, id))
        case DoWhile(body, condition, id):
            new_ctx, new_body = analyze_statement(opt_switch_ctx, body)
            return (new_ctx, DoWhile(new_body, condition, id))
        case For(init, condition, post, body, id):
            new_ctx, new_body = analyze_statement(opt_switch_ctx, body)
            return (new_ctx, For(init, condition, post, new_body, id))
        case LabeledStatement(lbl, stmt):
            new_case_map, new_stmt = analyze_statement(opt_switch_ctx, stmt)
            return (new_case_map, LabeledStatement(lbl, new_stmt))
        // These don't include sub-statements
        case Return:
        case Null:
        case Expression:
        case Break:
        case Continue:
        case Goto:
            return stmt
```

```
analyze_block_item(opt_switch_ctx, blk_item):
    if blk_item is a statement:
        new_opt_switch_ctx, new_stmt = analyze_statement(opt_switch_ctx, blk_item)

        return (new_opt_switch_ctx, new_stmt)
    else:
        return (opt_switch_ctx, blk_item) // don't need to change or traverse declaration

analyze_block(opt_switch_ctx, blk):
    new_opt_switch_ctx = opt_switch_ctx
    new_blk = []

    for item in blk:
        updated_ctx, new_item = analyze_block_item(new_opt_switch_ctx, item)
        new_opt_switch_ctx = updated_ctx
        new_blk.append(new_item)

    return (new_opt_switch_ctx, new_blk)

analyze_function_def(FunctionDef fun_def):
    _, blk = analyze_block(null, fun_def.body)
    return FunctionDef(fun_def.name, blk)

analyze_switches(Program fun_def):
    return Program(analyze_function_def(fun_def))
```

# TypeUtils

A simple util file for types. If you already defined the Type system in a different file, not in AST, define the following functions in that file instead.

```
get_type(AST.Expression exp):
    return exp.type

set_type(AST.Expression exp, Types.t type):
    return exp.type = type

get_alignment(Types.t type):
    case Int:
        return 4
    case Long:
        return 8:
    case FunType:
        fail("Internal error: function type doesn't have alignment")
```

# Initializers

We create a new file as an util namespace to initialize the value of static variables as they're initialized at compile time while the automatic variables get their values initialized at runtime.

```
type static_init =
    | IntInit(int32 v)
    | LongInit(int64 v)

zero(Types.t type):
    if type is Int:
        return IntInit(0)
    if type is Long:
        return LongInit(0)
    if type is FunType:
        fail("Internal error: zero doens't make sense for function type")

is_zero(static_init):
    if static_init is IntInit(i):
        return i == 0 (in 32-bit)
    else if static_init is LongInit(i):
        return i == 0 (in 64-bit)
```

# Symbols

We extend the **Initial(int)** to be **Initial(static_init)**, and the new layer **static_int** is a variant that can be either Long or Int.
We decide to not store the numbers of bytes required for the function to allocate in the FunAttr anymore, and we move unnecessary helper functions into Backend Symbol Table at [AssemblySymbols](#assemblysymbols)

```
type initial_value =
    | Tentative
    | Initial(Initializers.static_int)
    | NoInitializer

type identifier_attrs =
    | FunAttr(bool defined, bool global)
    | StaticAttr(initial_value init, bool global)
    | LocalAttr
```

```
type entry = {
    t: Types.t,
    attrs: identifier_attrs
}
```

```
add_fun(name, t, global, defined):
    symbols.add(name, entry(
        t,
        attrs=FunAttr(defined, global)
    ))
```

```
is_global(name):
    attrs = symbols.get(name).attrs

    if attrs is LocalAttr:
        return false
    else if attrs is StaticAttr:
        return attrs.global
    else if attrs is FunAttr:
        return attrs.global
```

# TypeCheck

Firstly, we define 2 helper functions on top:

- convert_to
- get_common_type

```
// A helper function that makes implicit conversions explicit
// If an expression already has the result type, return it unchanged
// Otherwise, wrap the expression in a Cast node.
convert_to(e, target_type):
	if get_type(e) == target_type:
		return e
	cast_exp = Cast(target_type, e)
	return set_type(cast_exp, target_type)
```

```
get_common_type(type1, type2):
	if type1 == type2:
		return type1
	else:
		return Types.Long
```

We refactor and extend several typechecking expressions cases:

- typecheck_var
- typecheck_const
- typecheck_unary
- typecheck_binary
- typecheck_assignment
- typecheck_compound_assignment
- typecheck_postfix_decr
- typecheck_postfix_incr
- typecheck_conditional
- typecheck_fun_call

```
typecheck_var(string v):
    v_type = symbols.get(v).t
    e = AST.Var(v)

    if v_type is FunType:
        fail("Tried to use function name as variable")
    else if v_type is Types.Int or Types.Long:
        return set_type(e, v_type)
```

```
typecheck_const(int c):
    e = Constant(c)

    if c is ConstInt:
        return set_type(e, Int)
    else if c is ConstLong:
        return set_type(e, Long)
    else:
        fail("Internal error: invalid constant type")
```

```
typecheck_unary(op, inner):
    typed_inner = typecheck_exp(inner)
    unary_exp = Unary(op, typed_inner)
    if op is "Not":
        return set_type(unary_exp, Int)
    else:
        return set_type(unary_exp, get_type(typed_inner))
```

```
typecheck_binary_(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)

    if op is BitShiftLeft or BitShiftRight:
        // Don't perform usual arithmetic conversions; result has type of left operand
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, get_type(typed_e1))

    if op is And or Or:
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, Int)

    t1 = get_type(typed_e1)
    t2 = get_type(typed_e2)
    common_type = get_common_type(t1, t2)
    converted_e1 = convert_to(typed_e1, common_type)
    converted_e2 = convert_to(typed_e2, common_type)
    binary_exp = Binary(op, converted_e1, converted_e2)

    if op is Add or Subtract or Multiply or Mod | BitwiseAnd | BitwiseOr | BitwiseXor:
        return set_type(binary_exp, common_type)
    else:
        return set_type(binary_exp, Int)
```

```
typecheck_assignment(lhs, rhs):
    typed_lhs = typecheck_exp(lhs)
    lhs_type = get_type(typed_lhs)
    typed_rhs = typecheck_exp(rhs)
    converted_rhs = convert_to(typed_rhs, lhs_type)
    assign_exp = Assignment(typed_lsh, converted_rhs)
    return set_type(assign_exp, lhs_type)
```

```
typecheck_compound_assignment(op, lhs, rhs):
    typed_lhs = typecheck_exp(lhs)
    lhs_type = get_type(typed_lhs)
    typed_rhs = typecheck_exp(rhs)
    rhs_type = get_type(typed_rhs)

    result_t, converted_rhs

    if op is BitShiftLeft or BitShiftRight:
        result_t = lhs_type
        converted_rhs = typed_rhs
    else:
        // We perform usual arithmetic conversions for every compound assignment operator
        // EXCEPT left/right bitshift
        common_type = get_common_type(lhs_type, rhs_type)
        result_t = common_type
        converted_rhs = convert_to(typed_rhs, common_type)

    // IMPORTANT: this may involve several implicit casts:
    // from RHS type to common type (represented w/ explicit convert_to)
    // from LHS type to common type (NOT directly represented in AST)
    // from common_type back to LHS type on assignment (NOT directly represented in AST)
    // We cannot add Cast expressions for the last two because LHS should be evaluated only once,
    // so we don't have two separate places to put Cast expression in this AST node. But we have
    // enough type information to allow us to insert these casts during TACKY generation
    compound_assign_exp = CompoundAssignment(op, typed_lhs, converted_rhs, result_t)

    return set_type(compound_assign_exp, lhs_type)
```

```
typecheck_postfix_decr(e):
    // result has same value as e; no conversions required.
    // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far

    typed_e = typecheck_exp(e)
    result_type = get_type(typed_e)
    return set_type(PostfixDecr(typed_e), result_type)
```

```
typecheck_postfix_incr(e):
    // Same deal as postfix decrement

    typed_e = typecheck_exp(e)
    result_type = get_type(typed_e)
    return set_type(PostfixIncr(typed_e), result_type)
```

```
typecheck_conditional(condition, then_exp, else_exp):
    typed_condition = typecheck_exp(condition)
    typed_then = typecheck_exp(then_exp)
    typed_else = typecheck_exp(else_exp)
    common_type = get_common_type(get_type(typed_then), get_type(typed_else))
    converted_then = convert_to(typed_then, common_type)
    converted_else = convert_to(typed_else, common_type)

    conditionl_exp = Conditional(
        typed_condition,
        converted_then,
        converted_else
    )

    return set_type(conditional_exp, common_type)
```

```
typecheck_fun_call(f, args):
    f_type = symbols.get(f).t

    if f_type is Int or Long:
        fail("Tried to use variable as function name")
    else if f_type is FunType(param_types, ret_type):
        if length(param_types) != length(args):
            fail("Function called with wrong number of arguments")

        converted_args = []
        for i in length(param_types):
            param_t = param_types[i]
            arg = args[i]
            new_arg = convert_to(typecheck_exp(arg), param_type)
            converted_args.append(new_arg)

        call_exp = FunctionCall(f, converted_args)
        return set_type(call_exp, ret_type)
```

Now, let's rewrite our typecheck_exp to use the refactored functions.

```
typecheck_exp(e, symbols):
	match e with:
	case Var(v):
		return typecheck_var(v)
	case Constant(c):
        return typecheck_const(c)
	case Cast(target_type, inner):
		cast_exp = Cast(target_type, typecheck_exp(inner))
        return set_type(cast_exp, target_type)
	case Unary(op, inner):
		return typecheck_unary(op, inner)
	case Binary(op, e1, e2):
		return typecheck_binary(op, e1, e2)
	case Assignment(lhs, rhs):
        return typecheck_assignment(lhs, rhs)
    case CompoundAssignment(op, lhs, rhs):
        return typecheck_compound_assignment(op, lhs, rhs)
    case PostfixDecr(e):
        return typecheck_postfix_decr(e)
    case PostfixIncr(e):
        return typecheck_postfix_incr(e)
	case Conditional(control, then_result, else_result):
		return typecheck_conditional(control, then_result, else_result)
	case FunctionCall(f, args):
		return typecheck_fun_call(f, args)
```

Eh eh! We're not done yet. We write another helper function.

```
// convert a constant to a static initializer, performing type conversion if needed.
to_static_init(var_type, constant):
    if constant is of type AST.Constant(c):
        Initializers.StaticInit init_val

        converted_c = ConstConvert.const_convert(var_type, c)
        if converted_c is ConstInt(i):
            init_val = Initializers.IntInit(i)
        else if converted_c is ConstLong(l):
            init_val = Initializers.LongInit(l)

        return Symbols.Initial(init_val)
    else:
        fail("Non-constant initializer on static variable")
```

When we are traversing the body of a function, we pass down the ret_type of the enclosing function to handle the correct conversions for return statement.

```
typecheck_block(block, ret_type):
    checked_block = []

    for block_item in block:
        checked_block.append(typecheck_block_item(block_item, ret_type))

    return checked_block
```

```
typecheck_block_item(block_item, ret_type):
    if block_item is statement:
        return typecheck_statement(block_item, ret_type)
    else if block_item is declaration:
        return typecheck_local_decl(block_item)
```

```
typecheck_statement(stmt, ret_type):
    match stmt type:
    case Return(e):
        typed_e = typecheck_exp(e)
        return Return(convert_to(typed_e, ret_type))
    case Expression(e):
        return Expression(typecheck_exp(e))
    case If(condition, then_clause, else_clause):
        return If(
            condition=typecheck_exp(condition),
            then_clause=typecheck_statement(then_clause, ret_type),
            else_clause= else_clause == null ? null : typecheck_statement(else_clause, ret_type)
        )
    case LabeledStatement(lbl, s):
        return LabeledStatement(lbl, typecheck_statement(s, ret_type))
    case Case(e, s, id):
        // Note: e must be converted to type of controlling expression in enclosing switch;
        // we do that during CollectSwitchCases pass
        return Case(typecheck_exp(e), typecheck_statement(s, ret_type), id)
    case Default(s, id):
        return Default(typecheck_statement(s, ret_type), id)
    case Switch(control, body, cases, id):
        return Switch(
            control=typecheck_exp(control),
            body=typecheck_statement(body, ret_type),
            cases,
            id
        )
    case Compound(block):
        return Compound(typecheck_block(block, ret_type))
    case While(condition, body, id):
        return While(
            condition=typecheck_exp(condition),
            body=typecheck_statement(body, ret_type),
            id
        )
    case DoWhile(body, condition, id):
        return DoWhile(
            body=typecheck_statement(body),
            condition=typecheck_exp(condition),
            id
        )
    case For(init, condition, post, body, id):
        typechecked_for_init = null

        if init is InitDecl:
            if init.storage_class is not null:
                fail("Storage class not permitted on declaration in for loop header")
            else:
                typechecked_for_init = InitDecl(typecheck_local_var_decl(init.d))
        else if init is InitExp and has exp:
            typechecked_for_init = InitExp(typecheck_exp(init.exp))

        return For(
            init = typechecked_for_init
            condition = condition is not null ? typecheck_exp(condition) : null
            post = post is not null ? typecheck_exp(post) : null
            body = typecheck_statement(body, ret_type)
            id = id
        )
    case Null:
    case Break:
    case Continue:
    case Goto:
        break
```

```
typecheck_local_decl(decl):
    if decl is VarDecl(vd):
        return VarDecl(typecheck_local_var_decl(vd))
    else if decl is FunDecl(fd):
        return FunDecl(typecheck_fn_decl(fd))
```

```
typecheck_local_var_decl(var_decl):
    if var_decl.storage_class is Extern:
        if var_decl has init:
            fail("Initializer on local extern declaration")
        else:
            entry = symbols.get_opt(var_decl.name)
            if entry is not null:
                // If an external local var is already in the symbol table, don't need to add it
                if entry.t != var_decl.var_type:
                    fail("Variable redeclared with different type")
            else:
                symbols.add_static_var(var_decl.name, t=var_decl.var_type, global=true, init=NoInitializer)

            return var_declaration(var_decl.name, init=null, var_decl.storage_class, var_decl.var_type)
    else if var_decl.storage_class is Static:
        zero_init = Symbols.Initial(Initializer.zero(var_decl.var_type))
        static_init = if var_decl has init
                        then to_static_init(var_decl.var_type, var_decl.init)
                        else zero_init

        symbols.add_static_var(var_decl.name, t=var_decl.var_type, global=false, static_init)

        // Note: we won't actually use init inb subsequent passes so we can drop it
        return var_declaration(var_decl.name, init = null, var_decl.storage_class, var_decl.var_type)
    else: // storage_class is null, so it's automatic storage duration
        Symbols.add_automatic_var(var_decl.name, var_decl.var_type)
        new_init = null
        if var_decl has init:
            new_init = convert_to(typecheck_exp(var_decl.init), var_decl.var_type)

        return var_declaration(var_decl.name, init=new_init, storage_class=null, t=var_decl.var_type, )
```

```
// Previously, we create the fun_type in when we typecheck_fn_decl,
// now we handle the fun_type at parse stage, so fn_decl already has FunType.
typecheck_fn_decl(fn_decl):
    has_body = fn_decl has body?
    global = fn_decl.storage_class != Static

    already_defined = false

    if fn_decl.name is in symbols:
		old_decl = symbols.get(fn_decl.name)
		if old_decl.type != fn_decl.fun_type:
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

    symbols.add_fun(fn_decl.name, fn_decl.fun_type, global, already_defined)

    if fn_decl.fun_type is not FunType:
        fail("Internal error: function has non-function type")

    param_ts, return_t = fun_type
    new_body = null

	if has_body:
		for i in length(param_ts):
            param = params[i]
            t = param_ts[i]

			symbols.add_automatic_var(param, t)
		new_body = typecheck_block(fn_decl.body, return_t)

    return function_declaration(fn_decl.name, fn_decl.fun_type, new_body, fn_decl.storage_class)
```

```
typecheck_file_scope_var_decl(var_decl):
    default_init = if var_decl.storage_class exists and is Extern
                    then Symbols.NoInitializer
                    else Symbols.Tentative

    static_init = if var_decl has init
                    then to_static_init(var_decl.var_type, var_decl.init)
                    else default_init

    curr_global = var_decl.storage_class != Static

    if symbols has var_decl.name
        old_decl = symbols.get(var_decl.name)
        if old_decl.type != var_decl.var_type:
            fail("Variable redeclared with different type")

        if old_decl.attrs is not StaticAttr:
            fail("Internal error: file-scope variable previsouly declared as local")

        if var_decl.storage_class == Extern:
            curr_global = old_decl.attrs.global
        else if old_decl.attrs.global != curr_global:
            fail("Conflicting variable linkage")

        if old_decl.attrs.init is an Initial(x):
            if static_init is Initial(y):
                fail("Conflicting global variable definition")
            else:
                static_init = old_decl.attrs.init
        else if old_delc.attrs.init is Tentative and static_init is either Tentative or NoInitializer:
            static_init = Tentative

    symbols.add_static_var(var_decl.name, t=var_decl.var_type, curr_global, static_init)

    // Ok to drop the initializer as it's never used after this pass
    return var_declaration(var_decl.name, init=null, var_decl.storage_class, var_decl.var_type)
```

```
typecheck_global_decl(decl):
    if decl is FunDecl(fd):
        return FunDecl(typecheck_fn_decl(fd))
    else if decl is VarDecl(vd):
        return VarDecl(typecheck_file_csope_var_decl(vd))
    else:
        fail("Internal error: Unknown declaration type")
```

# TACKY

- Firstly, use the same node _const_ (from Const source file) as in AST to help determine value type of _int_ or _long_ in the node _Constant_.
- Secondly, update _StaticVariable_ to have a new field "type" and the _init_ field not to be fixed with _int_, but with _static_init_.
- Lastly, add two new instructions to convert values between _int_ and _long_.

```
program = Program(top_level*)
top_level =
    | Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init init)
instruction =
    | Return(val)
    | SignExtend(val src, val dst)
    | Truncate(val src, val dst)
    | Unary(unary_operator, val src, val dst)
    | Binary(binary_operator, val src1, val src2, val dst)
    | Copy(val src, val dst)
    | Jump(identifier target)
    | JumpIfZero(val condition, identifier target)
    | JumpIfNotZero(val condition, identifier target)
    | Label(identifier)
    | FunCall(identifier fun_name, val* args, val dst)
val = Constant(const) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
                | LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
```

# TACKYGEN

Now, as types are more complicated, we cannot assume that the type of all local variables are always _int_.
That means whenever we create a temporary variable in TACKY to store the result of an expression, we need to look up the Symbol Table, add the temporary name to the Symbol Table with the same type as the expression.
Let's create a helper function and use that any place that we generate temporary variables in our TackyGen.

```
create_tmp(Types.t type):
    name = UniqueIds.make_temporary()
    symbols.add_automatic_var(name, type)
    return name
```

```
mk_const(Types.t t, int i):
    if t is Int:
        return Const.ConstInt((int32) i)
    else if t is Long:
        return Const.ConstLong((int64) i)
    else:
        fail("Internal error: can't make constant of fun type")
```

```
mk_ast_const(Types.t t,  int i):
    return AST.Constant(mk_const(t, i))
```

```
// Previously, we check the Var if it's and valid l-value, and pass down the var's name to emit_postfix.
// Now, we pass the whole inner expression of Postfix expression, and enclose the check for the value,
// then, make sure we use create_tmp, and mk_const to have correct nodes for each data type.
emit_postfix(op, inner):
	if inner is not of type AST.Var(name):
		fail("Invalid lvalue")

	name = inner.name
	dst = TACKY.Var(create_tmp(inner.t))

	op = convert_binop(op)

	insts = [
		Copy(Tacky.Var(name), dst),
		Binary(op, Var(name), Constant(mk_const(inner.t, 1)), Var(name))
	]

	return (insts, dst)
```

```
// Tough part, we replace any place where we create temporary TACKY variable with using the corret function _create_tmp_.
// To this point, each expression node should have their _type_ field populated.
// Also, for any expression that change the value of its own such as incr, decr, compound assignment, we move the check of valid lvalue into a function emit_compound_assignment, whose name is changed into emit_compound_expression.
emit_tacky_for_exp(exp):
	match exp type:
		case AST.Constant(c):
            return ([], TACKY.Constant(c))
		case AST.Var(v):
            return ([], TACKY.Var(v))
		case Unary:
			if exp.op is increment:
				return emit_compound_expression(AST.Add, exp.exp, mk_ast_const (exp.type, 1), exp.type)
			else if exp.op is decrement:
				return emit_compound_expression(AST.Subtract, exp.exp, mk_ast_const (exp.type, 1), exp.type)
			else:
				return emit_unary_expression(exp)
        case Cast:
            return emit_cast_expression(exp)
		case Binary:
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_or_expression(exp)
			else:
				return emit_binary_expression(exp)
		case Assignment:
                if exp.left is not AST.Var:
                    fail("bad lvalue")
                return emit_assignment(exp.left.var_name, exp.right)
		case CompoundAssignment:
			return emit_compound_expression(exp.op, exp.left, exp.right, exp.type)
		case PostfixIncr:
			return emit_postfix(AST.Add, exp.exp)
		case PostfixDecr:
			return emit_postfix(AST.Subtract, exp.exp)
        case Conditional:
            return emit_conditional_expression(exp)
        case FunCall:
            return emit_fun_call(exp)
```

The below are helper functions for individual expression, plus some new ones.

```
emit_unary_expression(unary):
    eval_inner, v = emit_tacky_for_exp(unary.exp)
    // define a temporary variable to hold result of this expression
    dst_name = create_tmp(unary.t)
    dst = Tacky.Var(dst_name)
    tacky_op = convert_op(unary.op)
    insts = [
        ...eval_inner,
        Unary(tacky_op, src=v, dst)
    ]

    return (insts, dst)
```

```
emit_cast_expression(cast):
    eval_inner, result = emit_tacky_for_exp(cast.exp)
    src_type = get_type(cast.exp)
    if src_type == cast.target_type:
        return (eval_inner, result)
    else:
        dst_name = create_tmp(cast.target_type)
        dst = Var(dst_name)
        cast_inst = get_cast_instruction(result, dst, cast.target_type)

        return (
            [...eval_inner, cast_inst],
            dst
        )
```

```
get_cast_instruction(src, dst, dst_t):
    // Note: assumes src and dst have different types
    if dst_t is Long:
        return Tacky.SignExtend(src, dst)
    else if dst_t is Int:
        return Tacky.Truncate(src, dst)
    else if dst_t is FunType:
        fail("Internal error: cast to function type")
```

```
emit_binary_expression(binary):
    eval_v1, v1 = emit_tacky_for_exp(binary.e1)
    eval_v2, v2 = emit_tacky_for_exp(binary.e2)
    dst_name = create_tmp(binary.t)
    dst = Var(dst_name)
    tacky_op = convert_op(binary.op)
    insts = [
        ...eval_v1,
        ...eval_v2,
        Binary(tacky_op, src1=v1, src2=v2, dst)
    ]

    return (insts, dst)
```

```
// Rechanged the name from emit_compound_assignment to emit_compound_expression.
// We pass the whole lhs expression (expected AST.Var) instead of prechecked string var_name anymore.
emit_compound_expression(op, lhs, rhs, result_t):
    // Make sure it's an lvalue
    if lhs is not of type AST.Var:
        fail("bad lvalue in compound assignment or prefix incr/decr")

    // evaluate RHS - typechecker already added conversion to common type if one is needed

	eval_rhs, rhs = emit_tacky_for_exp(rhs)
	dst = Tacky.Var(lhs.var_name)
	tacky_op = convert_binop(op)

	insts = [ ...eval_rsh ]

    if result_t == lhs.type:
        // result of binary operation already has correct destination type
        insts.append(
            TACKY.Binary(tacky_op, src1=dst, src2=rhs, dst)
        )
    else:
        // must convert LHS to op type, then convert result back, so we'll have
        // tmp = <cast v to result_type>
        // tmp = tmp op rhs
        // lhs = <cast tmp to lhs_type>

        tmp = Tacky.Var(create_tmp(result_t))
        cast_lhs_to_tmp = get_cast_instruction(dst, tmp, result_t)
        binary_inst = Tacky.Binary(tacky_op, src1=tmp, src2=rhs, dst=tmp)
        cast_tmp_to_lhs = get_cast_instruction(tmp, dst, lhs.type)

        insts.append(
            cast_lhs_to_tmp,
            binary_inst,
            cast_tmp_to_lhs,
        )

	return (insts, dst)
```

```
emit_and_expression(and_exp):
    eval_v1, v1 = emit_tacky_for_exp(and_exp.e1)
    eval_v2, v2 = emit_tacky_for_exp(and_exp.e2)
    false_label = UniqueIds.make_label("and_false")
    end_label = UniqueIds.make_label("and_end")
    dst_name = create_tmp(Int)
    dst = Var(dst_name)
    ints = [
        ...eval_v1,
        JumpIfZero(v1, false_label),
        ...eval_v2,
        JumpIfZero(v2, false_label),
        Copy(src=Constant(ConstInt(1)), dst),
        Jump(end_label),
        Label(false_label),
        Copy(src=Constant(ConstInt(0)), dst),
        Label(end_label)
    ]
```

```
emit_or_expression(or_exp):
    eval_v1, v1 = emit_tacky_for_exp(or_exp.e1)
    eval_v2, v2 = emit_tacky_for_exp(or_exp.e2)
    true_label = UniqueIds.make_label("or_true")
    end_label = UniqueIds.make_label("or_end")
    dst_name = create_tmp(Int)
    dst = Var(dst_name)
    ints = [
        ...eval_v1,
        JumpIfNotZero(v1, true_label),
        ...eval_v2,
        JumpIfNotZero(v2, true_label),
        Copy(src=Constant(ConstInt(0)), dst),
        Jump(end_label),
        Label(true_label),
        Copy(src=Constant(ConstInt(1)), dst),
        Label(end_label)
    ]
```

```
emit_assignment(var_name, rhs):
    rhs_insts, rhs_result = emit_tacky_for_exp(rhs)
    insts = [
        ...rhs_insts,
        Copy(rhs_result, Var(var_name))
    ]

    return (insts, Var(var_name))
```

```
emit_conditional_expression(conditional):
    eval_cond, c = emit_tacky_for_exp(conditional.condition)
    eval_v1, v1 = emit_tacky_for_exp(conditional.e1)
    eval_v2, v2 = emit_tacky_for_exp(conditional.e2)
    e2_label = UniqueIds.make_label("conditional_else")
    end_label = UniqueIds.make_label("conditional_end")
    dst_name = create_tmp(conditional.t)
    dst = Var(dst_name)
    insts = [
        ...eval_cond,
        JumpIfZero(c, e2_label),
        ...eval_v1,
        Copy(v1, dst),
        Jump(end_label),
        Label(e2_label),
        ...eval_v2,
        Copy(v2, dst),
        Label(end_label)
    ]

    return (insts, dst)
```

```
emit_fun_call(AST.FunctionCall fun_call):
    dst_name = create_tmp(fun_call.t)
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
emit_var_declaration(var_decl):
    if var_decl has init:
        // Treat var_decl with initializer as assignment
        eval_assignment, assign_result = emit_assignment(var_decl.name, var_decl.init)
        return eval_assignment
    else:
        // Do not generate any instructions
        return []
```

```
emit_tacky_for_switch(switch_stmt):
    br_label = break_label(switch_stmt.id)
    eval_control, c = emit_tacky_for_exp(switch_stmt.control)
    cmp_result = Tacky.Var(create_tmp(switch_stmt.control.t))

    jump_to_cases = []
    for case in switch_stmt.cases:
        if case.key is not null:    // It's a case statement
            insts = [
                Binary(Equal, src1=Constant(case.key), src2=c, dst=cmp_result),
                JumpIfNotZero(cmp_result, case.id)
            ]

            jump_to_cases.append(...insts)
        else: // It's a default statement, handle later
            // do nothing

    default_tacky = []
    if switch_stmt.cases has key that is null:
        default_id = switch_stmt.cases.find(null).id
        default_tacky = [
            Jump(default_id)
        ]

    body_tacky = emit_tacky_for_statement(switch_stmt.body)

    insts = [
        ...eval_control,
        ...jump_to_cases,
        ...default_tacky,
        Jump(br_label),
        ...body_tacky,
        Label(br_label),
    ]
```

```
emit_fun_declaration(fun_decl):
    insts = []

    if fun_decl has body:
        global = symbols.is_global(decl.name)

        for block_item in fun_def.body:
            emit_insts = emit_tacky_for_block_item(block_item)
            insts.append(...emit_insts)

        extra_return = Tacky.Return(Constant(ConstInt(0)))
        insts.append(extra_return)

        return Tacky.Function(fun_def.name, global, fun_decl.params, insts)

    return null
```

```
convert_symbols_to_tacky(symbols):
    static_vars = []

    for name, entry in symbols:
        if entry.attrs is StaticAttr:
            if entry.attrs.init is Initial:
                static_vars.push(StaticVariable(
                    name,
                    global=entry.attrs.global,
                    t=entry.t,
                    init=entry.attrs.init.value
                ))
            else if entry.attrs.init is Tentative:
                static_vars.push(StaticVariable(
                    name,
                    global=entry.attrs.global,
                    t=entry.t,
                    init=Initializers.zero(entry.t)
                ))
            else: // No Initializer, don't do anything
        else:
            // Don't do anything

    return static_vars
```

# Assembly

```
program = Program(top_level*)
asm_type = Longword | Quadword
top_level =
    | Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int alignment, static_init)
instruction =
    | Mov(asm_type, operand src, operand dst)
    | Movsx(operand src, operand dst)
    | Unary(unary_operator, asm_type, operand dst)
    | Binary(binary_operator, asm_type, operand, operand)
    | Cmp(asm_type, operand, operand)
    | Idiv(asm_type, operand)
    | Cdq(asm_type)
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | Push(operand)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP
```

- New enum: asm_type
- New register: SP
- Imm(int) is changed the value type to be Imm(int64)
- All instructions that takes operand(s), except for the list below, have a new field, which is asm_type
- Add new instruction Movsx
- StaticVariable has a new field - alignment; and use the Initializers.static_init
- Remove AllocateStack and DeallocateStack

Exceptions:

- _SetCC_ only takes 1-byte operands.
- _Push_ only takes 8-byte operands.
- _Movsx_ right now only works on the source type of _int_ to destination type of _long_.
- Cdq needs type to determine to use _cdq_ (for 4-byte operand) or _cdo_ (for 8-byte operand).

# AssemblySymbols

```
type entry =
    | Fun(bool defined, int bytes_required)
    | Obj (Assembly.asm_type t, bool is_static)
```

```
add_fun(fun_name, defined):
    symbol_table[fun_name] = Fun(defined, 0)
```

```
add_var(var_name, t, is_static):
    symbol_table[var_name] = Obj(t, is_static)
```

```
set_bytes_required(fun_name, bytes_required):
    if symbol_table has fun_name:
        // Note: we only set bytes_required if function is defined in this translation unit
        symbol_table[fun_name] = Fun(defined=true, bytes_required)
    else:
        fail("Internal error: function is not defined")
```

```
get_bytes_required(fun_name):
    if symbol_table has fun_name and symbol_table[fun_name] is Fun:
        return symbol_table[fun_name].bytes_required
    else:
        fail("Internal error: not a function")
```

```
get_size(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
            if symbol_table[var_name] has asm_type of Longword:
                return 4
            if symbol_table[var_name] has asm_type of Quadword:
                return 8
        else:
            fail("Internal error: this is a function, not an object")
    else:
        fail("Internal error: not found")
```

```
get_alignment(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
            if symbol_table[var_name] has asm_type of Longword:
                return 4
            if symbol_table[var_name] has asm_type of Quadword:
                return 8
        else:
            fail("Internal error: this is a function, not an object")
    else:
        fail("Internal error: not found")
```

```
is_defined(fun_name):
    if symbol_table has fun_name and is Fun:
        return symbol_table[fun_name].defined
    else:
        fail("Internal error: not a function")
```

```
is_static(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
            return symbol_table[var_name].is_static:
        else:
            fail("Internal error: functions don't have storage duration")
    else:
        fail("Internal error: not found")
```

# CodeGen

We created a function Initializers.zero to creates a corresponding node of the correct type with the value 0: IntInit(0) or LongInit(0)
In assembly, Immediate operands don't have type attached to themselves, we only care about the size of operand, and alignment through suffixes of instructions such as _l_ in _movl_, _q_ in _movq_.
So we create a helper function for Assembly only to create a node Imm((int64) 0)

```
zero():
    return Assembly.Imm((int64) 0)
```

```
convert_val(Tacky.Val val):
	match val.type:
        case Tacky.Constant(ConstInt(i)):
            return Assembly.Imm((int64) i)
		case Tacky.Constant(ConstLong(l)):
			return Assembly.Imm(l)
		case Tacky.Var:
			return Assembly.Pseudo(val.name)
```

We add 2 more functions; one is to convert Types into Asm_type, and one is to get the asm_type of tacky val operand.

```
convert_type(Types.t type):
    if type is Int:
        return asm_type.Longword
    else if type is Long:
        return asm_type.Quadword
    else:
        fail("Internal error: converting function type to assembly")
```

```
asm_type(Tacky.Val operand):
    if operand is Constant:
        if operand is Constant(ConstInt(i)) return asm_type.Longword
        else if operand is Constant(ConstLong(l)) return asm_type.Quadword
    else: // is Var
        entry = symbols.get(operand.name)
        if entry is null:
            fail("Internal error: {operand.name} not in symbol table")
        return convert_type(entry.t)
```

```
convert_function_call(TACKY.FunCall fun_call)
    reg_args, stack_args = fun_call.args[:6], fun_call.args[6:]

    // adjust stack alignment
    stack_padding = length(stack_args) % 2 == 0 ? 0 : 8
    insts = []

    if stack_padding != 0:
        inst.append(
            Assembly.Binary(
                op=Sub,
                t=Quadword,
                src=Imm((int64) stack_padding)
                dst=Reg(SP)
            )
        )

    // pass args in registers
    reg_idx = 0
	for tacky_arg in reg_args:
		r = param_passing_regs[reg_idx]
		assembly_arg = convert_val(tacky_arg)
        insts.append(Mov(asm_type(tacky_arg), assembly_arg, Reg(r)))
		reg_idx += 1

   // pass args on the stack
    for tacky_arg in reverse(stack_args):
		assembly_arg = convert_val(tacky_arg)
		if assembly_arg is a Reg or Imm operand:
			insts.append(Push(assembly_arg))
		else:
            asm_type = asm_type(tacky_arg)
            if asm_type is Quadword:
                insts.append(Push(assembly_arg))
            else:
                // copy into a register before pushing
                insts.append(Mov(asm_type, assembly_arg, Reg(AX)))
                insts.append(Push(Reg(AX)))

    insts.append(Call(fun_call.name))

    // adjust stack pointer
    bytes_to_remove = 8 * length(stack_args) + stack_padding
    if bytes_to_remove != 0:
        insts.append(
            Assembly.Binary(
                op=Add,
                t=Quadword,
                src=Imm((int64) bytes_to_remove),
                dst=Reg(SP)
            )
        )

    // Retrieve return value
    assembly_dst = convert_val(fun_call.dst)
    insts.append(Mov(asm_type(fun_call.dst), Reg(AX), assembly_dst))

    return insts
```

```
convert_instruction(Tacky.Instruction inst):
	match inst type:
		case Copy:
            t = asm_type(inst.src)
			asm_src = convert_val(inst.src)
			asm_dst = convert_dst(inst.dst)

			return [Mov(t, asm_src, asm_dst)]

		case Return:
            t = asm_type(inst.val)
			asm_val = convert_val(inst.val)
			return [
				Assembly.Mov(t, asm_val, Reg(AX)),
				Assembly.Ret
			]

		case Unary:
			match inst.op:
				case Not:
                    src_t = asm_type(inst.src)
                    dst_t = asm_type(inst.dst)
					asm_src = convert_val(inst.src)
					asm_dst = convert_val(inst.dst)

					return [
						Cmp(src_t, Imm(0), asm_src),
						Mov(dst_t, Imm(0), asm_dst),
						SetCC(E, asm_dst)
					]
				default:
                    t = asm_t(inst.src)
					asm_op = convert_op(inst.op)
                    asm_src = convert_val(inst.src)
                    asm_dst = convert_val(inst.dst)

                    return [
                        Assembly.Mov(t, asm_src, asm_dst),
                        Assembly.Unary(asm_op, t, asm_dst)
                    ]
		case Binary:
            src_t = asm_type(inst.src1)
            dst_t = asm_type(inst.dst)
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
						Cmp(src_t, asm_src2, asm_src1),
						Mov(dst_t, zero(), asm_dst),
						SetCC(cond_code, asm_dst),
					]

                (* Division/modulo *)
                case Divide:
                case Mod:
                    result_reg = op == Divide ? AX : DX

                    return [
                        Mov(src_t, asm_src1, Reg(AX)),
                        Cdq(src_t),
                        Idiv(src_t, asm_src2),
                        Mov(src_t, Reg(result_reg), asm_dst)
                    ]

                case BiftShiftLeft:
				case BiftShiftRight:
					asm_op = convert_binop(inst.op)
                    asm_t = asm_type(inst.src1)

					match type of asm_src2:
						case Imm:
							return [
								Mov(asm_t, asm_src1, asm_dst),
								Binary(asm_op, t=asm_t, asm_src2, asm_dst)
							]
						default:
							return [
								Mov(asm_t, asm_src1, asm_dst),
								Mov(Longword, asm_src2, Reg(CX)), // Note, only lower byte of CX is used. Can use Byte instead of Longword here once we add it to assembly AST
								Binary(asm_op, asm_t, Reg(CX), asm_dst)
							]
                (* Addition/subtraction/mutliplication/and/or/xor *)
                default:
                    asm_op = convert_binop(inst.op)

                    return [
                        Mov(src_t, asm_src1, asm_dst),
                        Binary(asm_op, src_t, asm_src2, asm_dst)
                    ]
		case Jump:
			return [ Jmp(inst.target) ]

		case JumpIfZero:
            t = asm_type(inst.cond)
			asm_cond = convert_val(inst.cond)

			return [
				Cmp(t, zero(), asm_cond),
				JmpCC(E, inst.target),
			]

		case JumpIfNotZero:
            t = asm_type(inst.cond)
			asm_cond = convert_val(inst.cond)

			return [
				Cmp(t, zero(), asm_cond),
				JmpCC(NE, inst.target),
			]

		case Label:
			return [ Assembly.Label(inst.name) ]
        case FunCall:
            return convert_function_call(inst)
        case SignExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ Movsx(asm_src, asm_dst) ]
        case Truncate:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ Mov(Longword, asm_src, asm_dst) ]
```

```
pass_params(param_list):
    reg_params, stack_params = param_list[:6], param_list[6:0]
    insts = []

    // pass params in registers
    reg_idx = 0
    for tacky_param in reg_params:
        r = param_passing_regs[reg_idx]
        assembly_param = Pseudo(tacky_param)
        param_t = asm_type(Tacky.Var(tacky_param))
        insts.append(Mov(param_t, Reg(r), assembly_param))
        reg_idx++

    // pass params on the stack
    // first param passed on stack has idx0 and is passed at Stack(16)
    stk_idx = 0
    for tacky_param in stack_params:
        stk = Stack(16 + (8*stk_idx))
        assembly_param = Pseudo(tacky_param)
        param_t = asm_type(Tacky.Var(tacky_param))
        inst.append(Mov(param_t, stk, assembly_param))
        stk_idx++

    return insts
```

```
convert_top_level(Tacky.top_level top_level):
    if top_level is Function:
        params_insts = pass_params(fun_def.params)
        insts = [...params_insts]

        for inst in fun_def.body:
            insts.append(...convert_instruction(inst))

        return Function(fun_def.name, fun_decl.global, insts)
    else: // top_level is StaticVariable
        return Assembly.StaticVariable(
            name=top_level.name,
            global=top_level.global,
            alignment=TypeUtils.get_alignment(top_level.t),
            init=top_level.init
        )
```

We create a new backend symbol to make the subsequent passes after CodeGen easier.

```
convert_symbol(name, symbol):
    if symbol.attrs is Types.FunAttr:
        return asmSymbols.add_fun(name, defined)
    else if symbol.attrs is Types.StaticAttr:
        return asmSymbols.add_var(name, convert_type(symbol.t), true)
    else:
        return asmSymbols.add_var(name, convert_type(symbol.t), false)
```

```
gen(Tacky.Program prog):
    converted_top_levels = []

    for top_level in prog.top_levels
        insts = convert_top_level(top_level)
        converted_top_levels.append(...insts)

    for name, symbol in symbols:
        convert_symbol(name, symbol)

    return Assembly.Program(converted_top_levels)
```

# ReplacePseudo

We will rely on the Assembly symbol to check if a variable is static, get its alignment to compute stack offset.

```
replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
            if AsmSymbols.is_static(operand.name):
                return (state, Assembly.Data(operand.name))
			else:
                if state.offset_map.has(operand.name):
                    return (state, state.offset_map.get(operand.name))
                else:
                    size = AsmSymbols.get_size(operand.name)
                    alignment = AsmSymbols.get_alignment(operand.name)
                    new_offset = Rounding.round_away_from_zero(alignment, state.current_offset - size)
                    new_state = {
                        current_offset: new_offset,
                        offset_map: state.offset_map.add(operand.name, new_offset)
                    }
				    return (new_state, Assembly.Stack(new_offset))
		default:
			return (state, operand)
```

```
// We add new instruction _Mosvx_, remove the cases of AllocateStack and DeallocateStack.
// Make sure when we replace pseudos, attach the styles when returning new instructions.
replace_pseudos_in_instruction(Assembly.Instruction inst, state):
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
            new_movsx = Movsx(new_src, new_dst)
            return (state2, new_movsx)

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

        case SetCC:
            state1, new_operand = replace_operand(inst.operand, state)
            new_setcc = SetCC(inst.cond_code, new_operand)
            return (state1, new_setcc)

        case Push:
            /* Note: ensure the operand is replaced correctly (argument order same as elsewhere) */
            state1, new_op = replace_operand(inst.op, state)
            new_push = Push(new_op)
            return (state1, new_push)

        /* For instructions that do not require operand replacement, simply pass them through */
        case Ret:
        case Cdq:
        case Label:
        case JmpCC:
        case Jmp:
        case Call:
            return (state, inst)
```

```
replace_pseudos_in_top_level(Assembly.top_level top_level):
	if top_level is Function:

        curr_state = init_state
        fixed_instructions = []

        for inst in fn.instrustions:
            new_state, new_inst = replace_pseudos_in_instruction(inst, curr_state)
            curr_state = new_state
            fixed_instructions.push(new_inst)

        AsmSymbols.set_bytes_required(fn.name, curr_state.current_offset)

        new_fn = Assembly.Function(fn.name, fn.global, fixed_instructions)
        return new_fn, curr_state.current_offset
    else: // is StaticVariable
        return top_level
```

# Instruction Fixup

We'll need to fix the immediate values at compile time to avoid warnings from Assemblers.

```
Mov(Longword, Imm(4294967299), Reg(R10))
```

to

```
Mov(Longword, Imm(3), Reg(R10))
```

So let's define some constants and helpers:

```
int32_max = convert_to_64bit(int32.max)
int32_min = convert_to_64bit(int32.min)

is_large(imm):
    return imm > int32_max || imm < int32_min

is_larger_than_uint(imm):
    // use unsigned upper-bound for positives
    max_i = convert_to_64bit(2**32 - 1)
    // use signed 32-bit lower bound for negatives
    imm > max_i || imm < int32_min
```

```
// Similarly, make sure we don't leave out the asm_type when rewriting instructions.
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	case Mov:
        // Mov can't move a value from one memory address to another
        if inst.src is (Stack | Data) and inst.dst is (Stack | Data):
            return [
				Mov(inst.t, src, Reg(R10)),
				Mov(inst.t, Reg(R10), dst)
            ]
        // Mov can't move a large constant to a memory address
        else if inst.t is Quadword and inst.src is Imm and inst.dst is (Stack | Data) and is_large(inst.src.value):
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
        else:
			return [ inst ]
    case Movsx:
        // Movsx cannot handle immediate src or memory dst
        if inst.src is Imm && inst.dst is (Stack | Data):
            return [
                Mov(Longword, Imm(inst.src.val), Reg(R10)),
                Movsx(Reg(R10), Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
        else if inst.src is Imm:
            return [
                Mov(Longword, Imm(inst.src.val), Reg(R10)),
                Movsx(Reg(R10), inst.dst)
            ]
        else if inst.dst is (Stack | Data):
            return [
                Movsx(inst.src, Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]
    case Idiv:
		(* Idiv can't operate on constants *)
		if inst.operand is a constant:
			return [
				Mov(inst.t, Imm(inst.operand.val), Reg(R10)),
				Idiv(inst.t, Reg(R10))
			]
		else:
			return [ inst ]
    case Binary:
        // Add/Sub/And/Or/Xor can't take large immediates as source operands
        if (inst.op is Add | Sub | And | Or | Xor)
            and (inst.t is Quadword)
            and (inst.src is Imm)
            and is_large(inst.src.val):
            return [
                Mov(Quadword, inst.src, Reg(R10)),
                Binary(inst.op, Quadword, Reg(R10), inst.dst)
            ]

		// Add/Sub can't use memory addresses for both operands
		if inst.op is (Add | Sub | And | Or | Xor) and both operands are (Stack | Data):
			return [
				Mov(inst.t, inst.src, Reg(R10)),
				Binary(inst.op, inst.t, Reg(R10), inst.dst)
			]
		 // Destination of Mult can't be in memory, src can't be a big operand
        else if (inst.op is Mult)
                    and (dst is (Stack | Data))
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
		else if inst.op is Mult and dst is (Stack | Data):
			return [
				Mov(inst.t, inst.dst, Reg(R11)),
				Binary(inst.op, inst.t, inst.src, Reg(R11)),
				Mov(inst.t, Reg(R11), inst.dst)
			]
		else:
			return [ inst ]
    case Cmp:
		// Both operands of cmp can't be in memory
		if both src and dst of cmp are (Stack | Data):
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
        if src is Imm and is_large(src.val):
            return [
                Mov(Quadword, src, Reg(R10)),
                Push(Reg(R10))
            ]
        else:
            return [ inst ]
    default:
		return [ other ]
```

```
fixup_top_level(Assembly.top_level top_level):
    if top_level is Function:
        stack_bytes = Rounding.round_away_from_zero(16, -AsmSymbols.get_bytes_required(top_level.name))
        stack_byte_op = Imm(to_64bit(stack_bytes))

        return Assembly.Function(
            fn_def.name,
            [
                Binary(Sub, Quadword, stack_byte_op, Reg(SP))
                , ...fn_def.instructions
            ]
        )
    else: // StaticVariable
        return top_level
```

# Emit

```
suffix(asm_type type):
    if type is Longword:
        return "l"
    else if type is Quadword:
        return "q"
```

```
// Update to use AsmSymbols
show_fun_name(fnCall):
    if platform is OS_X:
        return "_{fnCall.name}"
    else if platfrom is Linux:
        return if AsmSymbols.is_defined(fnCall.name)
                then fnCall.name
                else "{fnCall}@PLT"
```

```
// Change the show_reg name to be show_long_reg
show_long_reg(reg):
    match reg:
        case AX: return "%eax"
        case CX: return "%ecx"
        case DX: return "%edx"
        case DI: return "%edi"
        case SI: return "%esi"
        case R8: return "%r8d"
        case R9: return "%r9d"
        case R10: return "%r10d"
        case R11: return "%r11d"
        case SP: fail("Internal error: no 32-bit RSP")
```

```
// Update show_quadword_reg
show_quadword_reg(reg):
    match reg:
         case AX: return "%rax"
        case CX: return "%rcx"
        case DX: return "%rdx"
        case DI: return "%rdi"
        case SI: return "%rsi"
        case R8: return "%r8"
        case R9: return "%r9"
        case R10: return "%r10"
        case R11: return "%r11"
        case SP: return "%rsp"
```

```
// Update to show the corresponding registers with asm_type
show_operand(asm_type, operand):
    if operand is Reg:
        if asm_type is Longword:
            return show_long_reg(operand)
        else:
            return show_quadword_reg(operand)
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
        case AX: return "%al"
        case CX: return "%cl"
        case DX: return "%dl"
        case DI: return "%dil"
        case SI: return "%sil"
        case R8: return "%r8b"
        case R9: return "%r9b"
        case R10: return "%r10b"
        case R11: return "%r11b"
        case SP: fail("Internal error: no one-byte RSP")
```

```
show_byte_operand(operand):
    if operand is Reg:
        return show_byte_reg(operand)
    else:
        show_operand(Longword, operand)
```

```
// Remove any hardcoded suffix
show_unary_operator(op):
    match op:
        case Neg: return "neg"
        case Not : return "not"
```

```
show_binary_operator(op):
    match op:
        case Add: return "add"
        case Sub: return "sub"
        case Mul: return  "imul"
        case Xor: return "xor"
        case And: return "and"
        case Or : return "or"
        case Sal: return "sal"
        case Sar: return "sar"
```

```
emit_instruction(inst):
    match inst.type:
        case Mov(t, src, dst):
            return "\tmov{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Unary(op, t, dst):
            return "\t{show_unary_operator(op)}{suffix(t)} {show_operand(t, dst)}\n"

        case Binary(op, t, src, dst):
            // special logic: emit CX reg as %cl
            if op is Sal or Sar:
                return "\t{show_binary_operator(op)}{suffix(t)} {show_byte_operand(src)}, {show_operand(t, dst)}\n"
            else:
                return "\t{show_binary_operator(op)}{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Cmp(t, src, dst):
            return "\tcmp{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Idiv(t, operand):
            return "\tidiv{suffix(t)} {show_operand(t, operand)}\n"

        Cdq(t):
            if t is Longword:
                return "\tcdq\n"
            else if t is Quadword:
                return "\tcdo\n"
        case Jmp(lbl):
            return "\tjmp ${show_local_label(lbl)}\n"

        case JmpCC(code, lbl):
            return "\tj{show_cond_code(code)} {show_local_label(lbl)}\n"

        case SetCC(code, operand):
            return "\tset{show_cond_code(code)} {show_byte_operand(operand)}\n"

        case Label(lbl):
            return "{show_local_label(lbl)}:\n"

        case Push(operand):
            return "\tpushq {show_operand(Quadword, operand)}\n"

        case Call(f):
            return "\tcall {show_fun_name(f)}\n"

        case Movsx(src, dst):
            return "\tmovslq {show_operand(Longword, src)}, {show_operand(Quadword, dst)}\n"

        case Ret:
            return """
                movq %rbp, %rsp
                pop %tbp
                ret
            """
```

```
emit_zero_init(Initializers.static_init init):
    if init is IntInit:
        return "\t.zero 4\n"
    else if init is LongInit:
        return "\t.zero 8\n"
```

```
emit_init(Initializers.static_init init):
    if init is IntInit(i):
        return "\t.long {to_int32(i)}\n"
    else if init is LongInit(l):
        return "\t.quad {to_int64(l)}\n"
```

```
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

        if Initializers.is_zero(static_var.init):
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .bss
                {align_directive()} {static_var.alignment}
            {label}:
                {emit_zero_init(static_var.init)
            """
        else:
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .data
                {align_directive()} {static_var.alignment}
            {label}:
                {emit_init}(static_var.init)}
            """
```

# Output

From C:

```C
int main(void)
{
    int x = 100;
    x <<= 22l;

    long y = 10000;
    y += x;

    if (x != 419430400)
    {
        return 1;
    }

    return 0;
}
```

To x64 Assembly on Linux:

```asm
	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movl	$100, -4(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	sall	$22, -4(%rbp)
	movl	$10000, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -16(%rbp)
	movq	-16(%rbp), %r10
	movq	%r10, -24(%rbp)
	movslq 	-4(%rbp), %r11
	movq	%r11, -32(%rbp)
	movq	-24(%rbp), %r10
	movq	%r10, -24(%rbp)
	movq	-32(%rbp), %r10
	addq	%r10, -24(%rbp)
	movl	$419430400, %r11d
	cmpl	-4(%rbp), %r11d
	movl	$0, -36(%rbp)
	setne	-36(%rbp)
	cmpl	$0, -36(%rbp)
	je	.Lif_end.4
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.4:
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
