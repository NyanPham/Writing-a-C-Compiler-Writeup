# Table of Contents

- [Token, Lexer](#token-lexer)
- [Types](#types)
- [Const](#const)
- [ConstConvert](#ConstConvert)
- [AST](#ast)
- [Parser](#parser)
- [Initializers](#initializers)
- [IdentifierResolution](#identifier-resolution)
- [ValidateLabels](#validatelabels)
- [LoopLabeling](#looplabeling)
- [CollectSwitchCases](#collectswitchcases)
- [TypeUtils](#typeutils)
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

# Token, Lexer

We have already supported \* and & operators.

# Types

```
type Int {}
type Long {}
type UInt {}
type ULong {}
type Double {}
type Pointer {
    referenced_t: t,
}
type FunType {
    param_types: t[],
    ret_type: t
}

t = Int | Long | UInt | ULong | Double | Pointer | FunType
```

# Const

_No changes._

# ConstConvert

We treat pointer as ULong.

```
// create different for each type of v, better use template if your implementation language supports it
cast(int32/uint32/int64/uint64 v, Types.t targetType):
    switch targetType:
        case Types.Int: return Const.ConstInt(to_int32(v))
        case Types.UInt: return Const.ConstUInt(to_uint32(v))
        case Types.Long: return Const.ConstLong(to_int64(v))
        case Types.ULong: return Const.ConstULong(to_uint64(v))
        case Types.Pointer: return Const.ConstULong(to_uint64(v))
        case Types.Double: return Const.Double(to_double(v))
```

```
const_convert(Types.t target_type, Const.const const):
    match const:
        case ConstInt(i):
            return cast(i, target_type)
        case ConstLong(l):
            return cast(l, target_type)
        case ConstUInt(u):
            return cast(u, target_type)
        case ConstULong(ul):
            return cast(ul, target_type)
        case Double(d):
            if target_type is Types.Double:
                return Double(d)
            if target_type is Types.ULong:
                return ConstULong(to_uint64(d))
            if target_type is Pointer:
                fail("Internal error: cannot convert double to pointer")
            else:
                i64 = to_int64(d)
                return cast(d, target_type)
```

# AST

We add _Dereference_ and _AddrOf_ expressions.

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
type = Int | Long | UInt | ULong | Double | FunType(type* params, type ret)
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
    | Dereference(exp)
    | AddrOf(exp)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
```

# Parser

We need an intermediate layer to parse declarators. They will not appear in AST.

```
// Parsing declarators

// first parse declarators of this type, then convert to AST
type declarator =
    | Ident(identifier)
    | PointerDeclarator(declarator)
    | FunDeclarator(param_info* params, declarator)
```

```
param_info = Param(Types.t, declarator)
```

```
// <simple-declarator> ::= <identifier> | "(" <declarator> ")"
parse_simple_declarator(tokens):
    next_tok = take_token(tokens)
    switch next_tok:
        case "(":
            decl = parse_declarator(tokens)
            expect(")", tokens)
            return decl
        case Identifier(id):
            return Ident(id)
        default:
            raise_error("a simple declartor", "other")
```

```
// <declarator> ::= "*" <declarator> | <direct-declarator>
parse_declarator(tokens)
    switch next_token(tokens):
        case "*":
            take_token(tokens)
            inner = parse_declarator(tokens)
            return PointerDeclarator(inner)
        default:
            return parse_direct_declarator(tokens)
```

```
// <direct-declarator> ::= <simple-declarator> [ <param-list> ]
parse_direct_declarator(tokens):
    simple_decl = parse_simple_declarator(tokens)
    if peek(tokens) is "(":
        params = parse_param_list(tokens):
        return FunDeclarator(params, simple_decl)
    else:
        return simple_decl
```

```
// <param-list> ::= "(" <param> { "," <param> } ")" | "(" "void" ")"
parse_param_list(tokens):
    expect("(", tokens)

    if peek(tokens) is "void":
        params = []
    else:
        params = param_loop(tokens)

    expect(")", tokens)
    return params
```

```
param_loop(tokens):
    params = [ parse_param(tokens) ]
    while peek(tokens) == ",":
        // parse the rest of the param list
        take_token(tokens)
        params.append(parse_param(tokens))

    return params
```

```
// <param> ::= { <type-specifier> }+ <declarator>
parse_param(tokens):
    specifiers = parse_type_specifier_list(tokens)
    param_type = parse_type(specifiers)
    param_decl = parse_declarator(tokens)
    return Param(param_type, param_decl)
```

```
process_declarator(decl, base_type):
    switch decl:
        case Ident(s):
            return (s, base_type, [])
        case PointerDeclarator(d):
            derived_type = Types.Pointer(base_type)
            return process_declarator(d, derived_type)
        case FunDeclarator(params, Ident(s)):
            param_names = []
            param_types = []

            for Param(p_base_type, p_decl) in params:
				param_name, param_t, _ = process_declarator(p_decl, p_base_type)
				if param_t is a function type:
					fail("Function pointers in parameters are not supported")
				param_names.append(param_name)
				param_types.append(param_t)

            fun_type = Types.FunType(param_types, ret_type=base_type)
            return (s, fun_type, param_names)
        default:
            fail("Internal error: unknown declarator")
```

// abstract declarator

```
type abstract_declarator =
    | AbstractPointer(abstract_declarator)
    | AbstractBase
```

```
/*
<abstract-declarator> ::= "*" [ <abstract-declarator> ]
    | <direct-abstract-declarator>
*
parse_abstract_declarator(tokens):
    if peek(tokens) is "*":
        // it's a pointer declarator
        take_token(tokens)

        next_tok = peek(tokens)
        if next_tok is "*" or "(":
            // there's an inner declarator
            inner = parse_abstract_declarator(tokens)
        else if next_tok is ")":
            inner = AbstractBase()
        else:
            raise_error("an abstract declarator", "other")

        return AbstractPointer(inner)
    else:
        return parse_direct_abstract_declarator(tokens)
```

```
// <direct-abstract-declarator ::= "(" <abstract-declarator> ")"
parse_direct_abstract_declarator(tokens):
    expect("(", tokens)
    decl = parse_abstract_declarator(tokens)
    expect(")", tokens)
    return decl
```

```
process_abstract_declarator(decl, base_type):
    switch decl:
        case AbstractCase: base_type
        case AbstractPointer(inner):
            derived_type = Types.Pointer(base_type)
            return process_abstract_declarator(inner, derived_type)
```

```
// <factor> ::= <unop> <factor> | "(" { <type-specifier> }+ ")" | factor | <postfix-exp>
parse_factor(tokens):
	next_tokens = npeek(2, tokens) // We peek 2 tokens
	// Unary expression
	if next_tokens[0] is Tilde, Hyphen, Bang, DoublePlus, DoubleHyphen:
		op = parse_unop(tokens)
		inner_exp = parse_factor(tokens)
		return Unary(op, inner_exp)
    else if next_tokens[0] is "*":
        take_token(tokens)
        inner_exp = parse_factor(tokens)
        return Dereference(inner_exp)
    else if next_tokens[0] is "&" :
        take_token(tokens)
        inner_exp = parse_factor(tokens)
        return AddrOf(inner_exp)
    else if next_tokens[0] is "(" and is_type_specifier(next_token[1]):
        // It's a cast, consume the "(", then parse the type specifiers
        take_token(tokens)
        type_specifiers = parse_type_specifier_list(tokens)
        base_type = parse_type(type_specifiers)
        // check for optional abstract declarator
        if peek(tokens) is ")":
            target_type = base_type
        else:
            abstract_decl = parse_abstract_declarator(tokens)
            target_type = process_abstract_declarator(abstract_decl, base_type)
        expect(")", tokens)
        inner_exp = parse_factor(tokens)
        return Cast(target_type, inner_exp)
	else:
		return parse_postfix_exp(tokens)
```

```
/*
<function-declaration> ::= { <specifier> }+ <declarator> ( <block> | ";")
   we've already parsed { <specifier> }+ <declarator>
*/
finish_parsing_function_declaration(name, storage_class, fun_type, params, tokens):
    next_token = peek(tokens)

    body = null
    if next_token is "{":
        body = parse_block(tokens)
    else if next_token is ";":
        body = null
    else:
        raise_error("function body or semicolon", next_token type)

    return (name, fun_type, params, body, storage_class)
```

```
/*
<variable-declaration> ::= { <specifier> }+ <declarator> [ "=" <exp> ] ";"
   we've already parsed { <specifier> }+ <declarator>
*/
// Actually, no changes
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
    specifiers = parse_specifier_list(tokens)
    base_typ, storage_class = parse_type_and_storage_class(specifiers)
    declarator = parse_declarator(tokens)
    name, typ, params = process_declarator(declarator, base_typ)

    if typ is Types.FunType:
        return finish_parsing_function_declaration(name, storage_class, typ, params, tokens)
    else:
        if params is empty:
            return finish_parsing_variable_declaration(name, storage_class, typ, tokens)
        else:
            fail("Internal error: declarator has parameters but object type")
```

# Initializers

We only extend the _zero_ function to handle pointer similarly to ULong

```
type static_init =
    | IntInit(int32 v)
    | LongInit(int64 v)
    | UIntInit(uint32 v)
    | ULongInit(uint64 v)
    | DoubleInit(double v)
```

```
zero(Types.t type):
    if type is Int:
        return IntInit(0)
    if type is Long:
        return LongInit(0)
    if type is UInt:
        return UIntInit(0)
    if type is ULong or Pointer:
        return ULongInit(0)
    if type is Double:
        return Double(0.0)
    if type is FunType:
        fail("Internal error: zero doens't make sense for function type")
```

```
is_zero(static_init):
    if static_init is IntInit(i):
        return i == 0 (in 32-bit)
    else if static_init is LongInit(l):
        return l == 0 (in 64-bit)
    else if static_init is UIntInit(u):
        return u == 0 (in 32-bit)
    else if static_init is ULongInit(ul):
        return u == 0 (in 64-bit)
    // Note: consider all double non-zero since we don't know if it's zero or negative-zero
    else if static_init is DoubleInit(d):
        return false
```

# Identifier Resolution

We remove all places that we check for valid lvalue.
We add the 2 cases for Dereference and AddrOf.

```
resolve_exp(exp, id_map):
    switch exp.type:
        case Assignment:
            return Assignment(
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map),
                exp.type
            )
        case CompoundAssignment:
            return CompoundAssignment(
                exp.op,
                resolve_exp(exp.left, id_map),
                resolve_exp(exp.right, id_map),
                exp.type
            )
        case PostfixIncr:
            return PostfixIncr(
                resolve_exp(exp.inner, id_map),
                exp.type
            )
        case PostfixDecr:
            return PostfixDecr(
                resolve_exp(exp.inner, id_map),
                exp.type
            )
        case Var:
            if exp.name exists in id_map:                       // rename var from map
                return Var( id_map.get(exp.name).uniqueName, exp.type )
            else:
                fail("Undeclared variable: " + exp.name)
        // recursively process operands for other expressions
        case Cast:
            return Cast(
                exp.target_type,
                resolve_exp(exp.exp, id_map),
                exp.type
            )
        case Unary:
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
            if fun_name is in id_map:
                new_fun_name = id_map.get(fun_name).new_name
                new_args = []
                for arg in args:
                    new_args.append(resolve_exp(arg, id_map))

                return FunctionCall(new_fun_name, new_args, exp.type)
            else:
                fail("Undeclared function!")
        case Dereferene(inner) ->
            return Dereference(resolve_exp(inner, id_map))
        case AddOf(inner) ->
            return AddrOf(resolve_exp(inner, id_map))
        default:
            fail("Internal error: Unknown expression")
```

# ValidateLabels

_No Changes_

# LoopLabeling

_No Changes_

# CollectSwitchCases

_No Changes_

# TypeUtils

We treat pointer the same as Long, ULong and Double in terms of size, alignment.

Pointers cannot be signed.

```
get_size(Types.t type):
    switch type:
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8
        default:
            fail("Internal error: function type doesn't have size")
```

```
get_alignment(Types.t type):
    switch type:
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8:
        case Types.FunType:
            fail("Internal error: function type doesn't have alignment")
```

```
is_signed(Types.t type):
    switch type:
        case Types.Int:
        case Types.Long:
            return true
        case Types.UInt:
        case Types.ULong:
        case Types.Pointer:
            return false
        default:
            fail("Internal error: signedness doesn't make sense for function and double type")
```

# Symbols

_No Changes_

# TypeCheck

```
is_pointer (Types.t type):
    return type == Types.Pointer
```

```
is_arithmetic(Types.t type):
    switch type:
        case Int:
        case UInt:
        case Long:
        case ULong:
        case Double:
            return true
        case FunType:
        case Pointer:
            return false
        default:
            fail("Internal error: unknown type")
```

```
is_integer(Types.t type):
    switch type:
        case Int:
        case UInt:
        case Long:
        case ULong:
            return true
        case Double:
        case FunType:
        case Pointer:
            return false
        default:
            fail("Internal error: unknown type")
```

```
is_lvalue(AST.Expression e):
    return e == Var or e == Dereference
```

```
is_null_pointer_constant(exp):
    if exp is Const.const(c):
        if c == ConstInt(i) and i == 0:
            return true
        if c == ConstUInt(u) and u == 0:
            return true
        if c == ConstLong(l) and u == 0:
            return true
        if c == ConstULong(ul) and ul == 0:
            return true
        else:
            return false
    else:
        return false


```

```
get_common_pointer_type(e1, e2):
    if e1.type == e2.type:
        return e1.type
    else if is_null_pointer_constant(e1.e):
        return e2.type
    else if is_null_pointer_constant(e2.e):
        return e1.type
    else:
        fail("Expressions have incompatible types")
```

```
convert_by_assignment(e, target_type):
    if e.type == target_type:
        return e
    else if is_arithmetic(e.type) and is_arithmetic(target_type):
        convert_to(e, target_type)
    else if is_null_pointer_constant(e.e) and is_pointer(target_type):
        convert_to(e, target_type)
    else:
        fail("Cannot convert type for assignment")
```

Typechecking _Cast_ instructions has its own function now.
_Unary_ typechecking is separated for different operators: typecheck*not, typecheck_complement, typecheck_negate, typecheck_incr.
\_Binary* typechecking has its cases separated as well into: typecheck_logical, typecheck_arithmetic, typecheck_comparison, typecheck_bitshift.
We add the extension to typecheck Dereference and AddrOf.

```
typecheck_exp(e, symbols):
	match e with:
	case Var(v):
		return typecheck_var(v)
	case Constant(c):
        return typecheck_const(c)
	case Cast(target_type, inner):
        return typecheck_cast(target_type, inner)
	case Unary(op, inner):
        if op == Not:
            return typecheck_not(inner)
        if op == Complement:
            return typecheck_complement(inner)
        if op == Negate:
            return typecheck_negate(inner)
		return typecheck_incr(op, inner)
	case Binary(op, e1, e2):
        if op is And or Or:
            return typecheck_logical(op, e1, e2)
        if op is in (Add, Subtract, Multiply, Divide, Remainder, BitwiseAnd, BitwiseOr, BitwiseXor):
            return typecheck_arithmetic(op, e1, e2)
        if op is in (Equal, NotEqual, GreaterThan, GreaterOrEqual, LessThan, LessOrEqual):
            return typecheck_comparison(op, e1, e2)
        if op is BitshiftLeft or BitshiftRight:
            return typecheck_bitshift(op, e1, e2)
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
    case Dereference(inner):
        return typecheck_dereference(inner)
    case AddrOf(inner):
        return typecheck_addr_of(inner)
```

Now we define those functions

```
typecheck_cast(target_type, inner):
    typed_inner = typecheck_exp(inner)

    if
        (target_type is Pointer and typed_inner.type is Double)
        OR
        (target_type is Double and typed_inner.type is Pointer):
        fail("Cannot cast between pointer and double")
    else:
        cast_exp = Cast(target_type, typecheck_exp(inner))
        return set_type(cast_exp, target_type)
```

The next 4 functions should replace the typecheck_unary completely.

```
typecheck_not(inner):
    typed_inner = typecheck_exp(inner)
    not_exp = Unary(Not, typed_inner)
    return set_type(not_exp, Types.Int)
```

```
typecheck_complement(inner):
    typed_inner = typecheck_exp(inner)
    if typed_inner.type is Double OR is_pointer(typed_inner.type):
        fail("Bitwise complement only valid for integer types")
    else:
        complement_exp = Unary(Complement, typed_inner)
        return set_type(commplement_exp, get_type(typed_inner))
```

```
typecheck_negate(inner):
    typed_inner = typecheck_exp(inner)
    if is_pointer(inner.type):
        fail("Can't negate a pointer")
    else:
        negate_exp = Unary(Negate, typed_inner)
        return set_type(negate_exp, get_type(typed_inner))
```

```
typecheck_incr(op, inner):
    if is_lvalue(inner):
        typed_inner = typecheck_exp(inner)
        typed_exp = Unary(op, typed_inner)
        return set_type(typed_exp, get_type(typed_inner))
    else:
        fail("Operand of ++/-- must be an lvalue")
```

Similar to unary typechecking, the following typecheck functions handle all cases in typecheck_binary.

```
typecheck_logical(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)
    typed_binexp = Binary(op, typed_e1, typed_e2)
    return set_type(typed_binexp, Types.Int)
```

```
typecheck_arithmetic(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)
    if is_pointer(typed_e1.type) || is_pointer(typed_e2.type)
        fail("arithmetic operations not permitted on pointers")
    else:
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        binary_exp = Binary(op, converted_e1, converted_e2)

        if op is in (Remainder, BitwiseAnd, BitwiseOr, BitwiseXor) and common_type == Double:
            fail("Can't apply %, &, |, ^ to double")
        if op is in (Add, Subtract, Multiply, Divide, Remainder, BitwiseAnd, BitwiseOr, BitwiseXor):
            return set_type(binary_exp, common_type)

        fail("Internal error: {op} should be typechecked elsewhere")
```

```
typecheck_comparison(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)

    if typed_e1.type == Pointer OR typed_e2.type == Pointer:
        common_type = get_common_pointer_type(typed_e1, typed_e2)
    else:
        common_type = get_common_type(typed_e1.type, typed_e2.type)

    converted_e1 = convert_to(typed_e1, common_type)
    converted_e2 = convert_to(typed_e2, common_type)
    binary_exp = Binary(op, converted_e1, converted_e2)
    return set_type(binary_exp, Types.Int)
```

```
typecheck_bitshift(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)

    if NOT (is_integer(get_type(typed_e1)) AND is_integer(get_type(typed_e2))):
        fail("Both operands of bit shift operation must be an integer")
    else:
        // Don't perform usual arithmetic conversions; result has type of left operand
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, get_type(typed_e1))
```

We removed all the lvalue checking in Identifier Resolution pass.
So we update the typecheck_postfix_drc, typecheck_postifx_incr, typecheck_assignment, typecheck_compound_assignment,

```
typecheck_postfix_decr(e):
    if is_lvalue(e):
        // result has same value as e; no conversions required.
        // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far

        typed_e = typecheck_exp(e)
        result_type = get_type(typed_e)
        return set_type(PostfixDecr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue")
```

```
typecheck_postfix_incr(e):
    if is_lvalue(e):
        // Same deal as postfix decrement

        typed_e = typecheck_exp(e)
        result_type = get_type(typed_e)
        return set_type(PostfixIncr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue")
```

```
typecheck_assignment(lhs, rhs):
    if is_lvalue(lhs):
        typed_lhs = typecheck_exp(lhs)
        lhs_type = get_type(typed_lhs)
        typed_rhs = typecheck_exp(rhs)
        converted_rhs = convert_by_assignment(typed_rhs, lhs_type)
        assign_exp = Assignment(typed_lsh, converted_rhs)
        return set_type(assign_exp, lhs_type)
    else:
        fail("left hand side of assignment is invalid lvalue")
```

```
typecheck_compound_assignment(op, lhs, rhs):
    if is_lvalue(lhs):
        typed_lhs = typecheck_exp(lhs)
        lhs_type = get_type(typed_lhs)
        typed_rhs = typecheck_exp(rhs)
        rhs_type = get_type(typed_rhs)

        if op is in (Remainder, BitwiseAnd, BitwiseOr, BitwiseXor, BitshiftLeft, BitshiftRight) AND (NOT(is_integer(lhs_type) OR NOT(is_integer(rhs_type)))):
            fail("Operand {op} only supports integer operands")
        if op is in (Multiply, Divide) AND (is_pointer(lhs_type) OR is_pointer(rhs_type)):
            fail("Operand {op} does not support pointer operands")

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
    else:
        fail("Left-hand side of compound assignment must be an lvalue")
```

We check for pointer cases in typecheck_conditional

```
typecheck_conditional(condition, then_exp, else_exp):
    typed_condition = typecheck_exp(condition)
    typed_then = typecheck_exp(then_exp)
    typed_else = typecheck_exp(else_exp)

    if is_pointer(typed_then.type) || is_pointer(typed_else.type):
        common_type = get_common_pointer_type(typed_then, typed_else)
    else:
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

When typechecking function calls, we use convert_by_assignment for arguments instead of naive convert_to.

```
typecheck_fun_call(f, args):
    f_type = symbols.get(f).t

    if f_type is FunType(param_types, ret_type):
        if length(param_types) != length(args):
            fail("Function called with wrong number of arguments")

        converted_args = []
        for i in length(param_types):
            param_t = param_types[i]
            arg = args[i]
            new_arg = convert_by_assignment(typecheck_exp(arg), param_type)
            converted_args.append(new_arg)

        call_exp = FunctionCall(f, converted_args)
        return set_type(call_exp, ret_type)
    else:
        fail("Tried to use variable as function name")
```

The two new expressions are typechecked as the following:

```
typecheck_dereference(inner):
    typed_inner = typecheck_exp(inner)

    if get_type(typed_inner) is Pointer(referenced_t):
        deref_exp = Dereference(typed_inner)
        return set_type(deref_exp, referenced_t)
    else:
        fail("Tried to dereference non-pointer")
```

```
typecheck_addr_of(inner):
    if inner is Derefernce or Var:
        typed_inner = typecheck_exp(inner)
        inner_t = get_type(typed_inner)
        addr_exp = AddrOf(typed_inner)
        return set_type(addr_exp, Pointer(inner_t))
    else:
        fail("Cannot take address of non-lvalue")
```

For static init, we allow null pointer as the only constant to initialize static pointers.

```
to_static_init(var_type, constant):
    if constant is of type AST.Constant(c):
        Initializers.StaticInit init_val

        if is_pointer(var_type):
            if is_null_pointer_constant(constant):
                init_val = Initializers.ULongInit(0)
            else:
                fail("Static pointers can only be initilalized with null pointer constants")
        else:
            converted_c = ConstConvert.const_convert(var_type, c)
            if converted_c is ConstInt(i):
                init_val = Initializers.IntInit(i)
            else if converted_c is ConstLong(l):
                init_val = Initializers.LongInit(l)
            else if converted_c is ConstUInt(u):
                init_val = Initializers.UIntInit(u)
            else if converted_c is ConstULong(ul):
                init_val = Initializers.ULongInit(ul)
            else if converted_c is ConstDouble(d):
                init_val = Initializers.DoubleInit(d)
            else:
                fail("Internal error: invalid const type")

            return Symbols.Initial(init_val)
    else:
        fail("Non-constant initializer on static variable")
```

For return statements, we use convert_by_assignment rather than convert_to to ensure the types between the value to be returned with the function return type.
In switch statements, we don't allow pointer in the controlling expression, so we use is_integer to exclude the Double and Pointer.

```
typecheck_statement(stmt, ret_type):
    match stmt type:
    case Return(e):
        typed_e = typecheck_exp(e)
        return Return(convert_by_assignment(typed_e, ret_type))
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
        typed_e = typecheck_exp(e)
        if get_type(typed_e) == Double:
            fail("Case expression cannot be bouble")
        else:
            // Note: e must be converted to type of controlling expression in enclosing switch;
            // we do that during CollectSwitchCases pass
            return Case(typecheck_exp(e), typecheck_statement(s, ret_type), id)
    case Default(s, id):
        return Default(typecheck_statement(s, ret_type), id)
    case Switch(control, body, cases, id):
        typed_control = typecheck_exp(control)
        if NOT is_integer(get_type(typed_control)):
            fail("Controlling expression in switch must have integer type")
        return Switch(
            control=typed_control,
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

Similarly, we use convert_by_assignment, which is the a type-validate-wrapped function of convert_to in typecheck_local_var_decl.

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
            new_init = convert_by_assignment(typecheck_exp(var_decl.init), var_decl.var_type)

        return var_declaration(var_decl.name, init=new_init, storage_class=null, t=var_decl.var_type, )
```

# TACKY

We have 3 new instructions: GetAddress, Load, Store.

```
program = Program(top_level*)
top_level =
    | Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init init)
instruction =
    | Return(val)
    | SignExtend(val src, val dst)
    | Truncate(val src, val dst)
    | ZeroExtend(val src, val dst)
    | DoubleToInt(val src, val dst)
    | DoubleToUInt(val src, val dst)
    | IntToDouble(val src, val dst)
    | UIntToDouble(val src, val dst)
    | Unary(unary_operator, val src, val dst)
    | Binary(binary_operator, val src1, val src2, val dst)
    | Copy(val src, val dst)
    | GetAddress(val src, val dst)
    | Load(val src_ptr, val dst)
    | Store(val src, val dst_ptr)
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

```
// an expression result that may or may not be lvalue converted
type exp_result =
    | PlainOperand(Tacky.Val val)
    | DereferencedPointer(Tacky.Val val)
```

We modify the emit_tacky_for_exp to return exp_result instead, and call emit_tacky_and_convert instead of emit_tacky_for_exp.
In emit_assignment, we already checked the left handside to be an lvalue, so the check is redundant here.

```
// return list of instructions to evaluate expression and resulting exp_result value as a pair
emit_tacky_for_exp(exp, type):
	match exp type:
		case AST.Constant(c):
            return ([], PlainOperand(TACKY.Constant(c)))
		case AST.Var(v):
            return ([], PlainOperand(TACKY.Var(v)))
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
                return emit_assignment(exp.left, exp.right)
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
        case Dereference:
            return emit_dereference(exp.exp)
        case AddrOf:
            return emit_addr_of(type, exp.exp)
```

```
// call emit_tacky_and_convert, and return PlainOperand
emit_unary_expression(unary):
    eval_inner, v = emit_tacky_and_convert(unary.exp)
    // define a temporary variable to hold result of this expression
    dst_name = create_tmp(unary.t)
    dst = Tacky.Var(dst_name)
    tacky_op = convert_op(unary.op)
    insts = [
        ...eval_inner,
        Unary(tacky_op, src=v, dst)
    ]

    return (insts, PlainOperand(dst))
```

```
emit_postfix(op, inner):
    // define var for result - i.e. value of lval BEFORE incr or decr
	dst = TACKY.Var(create_tmp(inner.t))

    // evaluate inner to get exp_result
    insts, lval = emit_tacky_for_exp(inner)
    tacky_op = convert_binop(op)
    one = Tacky.Constant(mk_const(inner.type, 1))
    // copy result to dst and perform incr or decr

    if lval is PlainOperand(Var(name)):
        /*
            dst = v
            v = v + 1 // v - 1
        */
        oper_insts = [
            Copy(Var(name), dst),
            Binary(tacky_op, Var(name), one, Var(name)),
        ]
    else if lval is DereferencedPointer(p):
        /*
            dst = Load(p)
            tmp = dst + 1 // dst - 1
            Store(tmp, p)
        */
        tmp = Var(create_tmp(inner.type))
        oper_insts = [
            Load(src_ptr=p, dst),
            Binary(tacky_op, dst, one, tmp),
            Store(tmp, dst_ptr=p),
        ]

	return ([...insts, ...oper_insts], PlainOperand(dst))
```

```
// call emit_tacky_and_convert instead of emit_tacky_for_exp
// and return PlainOperand instead of mere Tacky.Val
emit_cast_expression(cast):
    eval_inner, result = emit_tacky_and_convert(cast.exp)
    src_type = get_type(cast.exp)
    if src_type == cast.target_type:
        return (eval_inner, PlainOperand(result))
    else:
        dst_name = create_tmp(cast.target_type)
        dst = Var(dst_name)
        cast_inst = get_cast_instruction(result, dst, src_type, cast.target_type)

        return (
            [...eval_inner, cast_inst],
            PlainOperand(result)
        )
```

```
emit_binary_expression(binary):
    eval_v1, v1 = emit_tacky_and_convert(binary.e1)
    eval_v2, v2 = emit_tacky_and_convert(binary.e2)
    dst_name = create_tmp(binary.t)
    dst = Var(dst_name)
    tacky_op = convert_op(binary.op)
    insts = [
        ...eval_v1,
        ...eval_v2,
        Binary(tacky_op, src1=v1, src2=v2, dst)
    ]

    return (insts, PlainOperand(dst))
```

```
emit_compound_expression(op, lhs, rhs, result_type):
    /*
        if LHS is var w/ same type as result:
            lhs = lhs <op> rval
        if LHS is a var w/ different type:
            tmp = cast(lhs)
            tmp = tmp <op> rval
            lhs = cast(tmp)
        if LHS is pointer w/ same type:
            tmp = load(lhs_ptr)
            tmp = tmp <op> rval
            store(tmp, lhs_ptr)
        if LHS is pointer w/ diff type:
            tmp = load(lhs_ptr)
            tmp2 = cast(tmp)
            tmp2 = tmp2 <op> rval
            tmp = cast(tmp2)
            store(tmp, rhs_ptr)
    */
    lhs_type = lhs.type
    // evaluate LHS
    eval_lhs, lhs = emit_tacky_for_exp(lhs)
    // evaluate RHS - type checker already added conversion to common type if one is needed
    eval_rhs, rhs = emit_tacky_and_convert(rhs)
    /*
        If LHS is a variable, we can update it directly.
        If it's a dereferenced pointer, we need to load it into a temporary variable, operate on that, and then store it.
    */
    if lhs is PlainOperand(dst):
        dst = dst
        load_inst = []
        store_inst = []
    else if lhs is DereferencedPointer(p):
        dst = Var(create_tmp(lhs_type))
        load_inst = [ Load(src_ptr=p, dst) ]
        store_inst = [ Store(dst, dst_ptr=p) ]

    /*
        If LHS type and result type are the same, we can operate on dst directly.
        Otherwise, we need to cast dst to correct type before operation, then cast result
        back and assign to dst
    */
    if lhs_type == result_type:
        result_var = dst
        cast_to = []
        cast_from = []
    else:
        tmp = Var(create_tmp(result_type))
        cast_lhs_to_tmp = get_cast_instruction(dst, tmp, lhs_type, resul_type)
        cast_tmp_to_lhs = get_cast_instruction(tmp, dst, result_type, lhs_type)

        result_var = tmp
        cast_to = [ cast_lhs_to_tmp ]
        cast_from = [ cast_tmp_to_lhs ]

    binary_inst = Binary(
        op=convert_binop(op),
        src1=result_var,
        src2=rhs,
        dst=result_var,
    )
    insts = [
        ...eval_lhs,
        ...eval_rhs,
        ...load_inst,
        ...cast_to,
        binary_inst,
        ...cast_from,
        ...store_inst,
    ]

    return (insts, PlainOperand(dst))
```

```
emit_and_expression(and_exp):
    eval_v1, v1 = emit_tacky_and_convert(and_exp.e1)
    eval_v2, v2 = emit_tacky_and_convert(and_exp.e2)
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

    return (insts, PlainOperand(dst))
```

```
emit_or_expression(or_exp):
    eval_v1, v1 = emit_tacky_and_convert(or_exp.e1)
    eval_v2, v2 = emit_tacky_and_convert(or_exp.e2)
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

    return (insts, PlainOperand(dst))
```

```
emit_assignment(lhs, rhs):
    lhs_insts, lval = emit_tacky_for_exp(lhs)
    rhs_insts, rval = emit_tacky_and_convert(rhs)
    insts = [
        ...lhs_insts,
        ...rhs_insts,
    ]

    if lval is PlainOperand(o):
        insts.append(
            Copy(rval, o)
        )
        return (insts, lval)
    else if lval is DereferencedPointer(ptr):
        insts.append(
            Store(rval, ptr)
        )
    return (insts, PlainOperand(rval))
```

```
emit_conditional_expression(conditional):
    eval_cond, c = emit_tacky_and_convert(conditional.condition)
    eval_v1, v1 = emit_tacky_and_convert(conditional.e1)
    eval_v2, v2 = emit_tacky_and_convert(conditional.e2)
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

    return (insts, PlainOperand(dst))
```

```
emit_fun_call(AST.FunctionCall fun_call):
    dst_name = create_tmp(fun_call.t)
    dst = Tacky.Var(dst_name)
    arg_insts = []
    arg_vals = []

    for arg in fun_call.args:
        insts, v = emit_tacky_and_convert(arg)
        arg_insts.append(...insts)
        arg_vals.append(v)

    insts = [
        ...arg_insts,
        Tacky.FunCall(fun_decl.name, arg_vals, dst)
    ]

    return (insts, PlainOperand(dst))
```

```
emit_dereference(inner):
    insts, result = emit_tacky_and_convert(inner)
    return (insts, DereferencedPointer(result))
```

```
emit_addr_of(Types.t type, inner):
    insts, result = emit_tacky_for_exp(inner)
    if result is PlainOperand(o):
        dst = Var(create_tmp(type))
        insts.append(
            GetAddress(src=o, dst)
        )
        return (insts, PlainOperand(dst))
    else if result is DereferencedPointer(ptr):
        return (insts, PlainOperand(ptr))
```

```
emit_tacky_and_convert(e):
    insts, result = emit_tacky_for_exp(e)
    if result is PlainOperand(o):
        return (insts, o)
    else if result is DereferencedPointer(ptr):
        dst = Var(create_tmp(e.t))
        return (
            [...insts, Load(src_ptr=ptr, dst)],
            dst
        )
```

```
emit_tacky_for_statement(statement):
	match statement type:
		case Return:
			val_exp, v = emit_tacky_and_convert(statement.exp)
            return [
                ...eval_exp,
                Return(v),
            ]
        case Expression:
			eval_exp, _ = emit_tacky_for_exp(statement.exp)
            return eval_exp
		case If:
			--snip--
        case LabeledStatement:
            --snip--
        case Goto:
            --snip--
		case Compound:
			--snip--
        case Break:
            --snip--
        case Continue:
            --snip--
        case DoWhile:
            --snip--
        case While:
            --snip--
        case For:
            --snip--
        case Case:
            return [
                Label(statement.id),
                ...emit_tacky_for_statement(statement.statement)
            ]
        case Default:
            return [
                Label(statement.id),
                ...emit_tacky_for_statement(statement.statement)
            ]
        case Switch:
            return emit_tacky_for_switch(statement)
		case Null:
			return []
```

```
emit_var_declaration(var_decl):
    if var_decl has init:
        // Treat var_decl with initializer as assignment
        lhs = AST.Var(var_decl.name)
        set_type(lhs, var_decl.var_type)
        eval_assignment, assign_result = emit_assignment(lhs, var_decl.init)
        return eval_assignment
    else:
        // Do not generate any instructions
        return []
```

```
emit_tacky_for_if_statement(statement):
	if statement has no else_clause:
		end_label = UniqueIds.make_label("if_end")
		eval_condition, c = emit_tacky_and_convert(statement.condition)

		return [
			...eval_condition,
			JumpIfZero(c, end_label),
			...emit_tacky_for_statement(statement.then_clause),
			Label(end_label)
		]
	else:
		else_label = UniqueIds.make_label("else")
		end_label = UniqueIds.make_label("if_end")
		eval_condition, c = emit_tacky_and_convert(statement.condition)

		return [
			...eval_condition,
			JumpIfZero(c, else_label),
			...emit_tacky_for_statement(statement.then_clause),
			Jump(end_label),
			Label(else_label),
			...emit_tacky_for_statement(statement.else_clause),
			Label(end_label)
		]
```

```
emit_tacky_for_do_loop(do_loop):
    start_label = UniqueIds.make_label("do_loop_start")
    cont_label = continue_label(do_loop.id)
    br_label = break_label(do_loop.id)
    eval_cond, c = emit_tacky_and_convert(do_loop.condition)
    body_insts = emit_tacky_for_statement(do_loop.body)

    insts = [
        Label(start_label),
        ...body_insts,
        Label(cont_label),
        ...eval_cond,
        JumpIfNotZero(c, start_label),
        Label(br_label)
    ]

    return insts
```

```
emit_tacky_for_while_loop(while_loop):
    cont_label = continue_label(while_loop.id)
    br_label = break_label(while_loop.id)
    eval_cond, c = emit_tacky_and_convert(while_loop.condition)
    body_insts = emit_tacky_for_statement(while_loop.body)

    insts = [
        Label(cont_label),
        ...eval_cond,
        JumpIfZero(c, br_label),
        ...body_insts
        Jump(cont_label),
        Label(br_label)
    ]

    return insts
```

```
emit_tacky_for_for_loop(for_loop):
    start_label = UniqueIds.make_label("for_start")
    cont_label = continue_label(for_loop.id)
    br_label = break_label(for_loop.id)
    for_init_insts = []

    if for_loop.init is declaration:
        for_init_insts = emit_declaration(for_loop.init)
    else: // for_loop.init is an opt_exp
        if for_loop.init is not null:
            for_init_insts = emit_tacky_for_exp(for_loop.init)
        else:
            for_init_insts = []

    test_condition = []

    if for_loop.condition is not null:
        inner_insts, v = emit_tacky_and_convert(for_loop.condition)

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

```
emit_tacky_for_switch(switch_stmt):
    br_label = break_label(switch_stmt.id)
    eval_control, c = emit_tacky_and_convert(switch_stmt.control)
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

# Assembly

We add the new register BP, replace Stack operand with Memory operand, and a new Lea instruction.

```
program = Program(top_level*)
asm_type = Longword | Quadword | Double
top_level =
    | Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int alignment, static_init init)
    | StaticConstant(identifier name, int alignment, static_init init)
instruction =
    | Mov(asm_type, operand src, operand dst)
    | Movsx(operand src, operand dst)
    | MovZeroExtend(operand src, operand dst)
    | Lea(operand src, operand dst)
    | Cvttsd2si(asm_type, operand src, operand dst)
    | Cvtsi2sd(asm_type, operand src, operand dst)
    | Unary(unary_operator, asm_type, operand dst)
    | Binary(binary_operator, asm_type, operand, operand)
    | Cmp(asm_type, operand, operand)
    | Idiv(asm_type, operand)
    | Div(asm_type, operand)
    | Cdq(asm_type)
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | Push(operand)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not | ShrOneOp
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor | Sal | Sar | Shr | Shl
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

# AssemblySymbols

_No changes_

# CodeGen

```
// Pointer also is translated to asm_type of Quadword
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong or Pointer:
        return asm_type.Quadword
    else if type is Double:
        return asm_type.Double
    else:
        fail("Internal error: converting function type to assembly")
```

```
// Extend for TACKY's instructions: Load, Store, and GetAddress
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
            ret_reg = t == Assembly.Double ? XMM0 : AX
			return [
				Assembly.Mov(t, asm_val, Reg(ret_reg)),
				Assembly.Ret
			]

		case Unary:
			match inst.op:
				case Not:
                    src_t = asm_type(inst.src)
                    dst_t = asm_type(inst.dst)
					asm_src = convert_val(inst.src)
					asm_dst = convert_val(inst.dst)

                    if src_t == Double:
                        insts = [
                            Binary(Xor,t=Double src=Reg(XMM0), dst=Reg(XMM0)),
                            Cmp(src_t, asm_src, Reg(XMM0)),
                            Mov(dst_t, zero(), asm_dst),
                            SetCC(E, asm_dst),

                            // cmp with NaN sets both ZF and PF, but !NaN should evaluate to 0,
                            // so we'll calculate:
                            // !x = ZF && !PF

                            SetCC(NP, Reg(R9)),
                            Binary(And, dst_t, Reg(R9), asm_dst),
                        ]

                        return insts
                    else:
                        return [
                            Cmp(src_t, Imm(0), asm_src),
                            Mov(dst_t, Imm(0), asm_dst),
                            SetCC(E, asm_dst)
                        ]
                case Negate:
                    if tacky_type(src) == Double:
                        asm_src = convert_val(inst.src)
					    asm_dst = convert_val(inst.dst)
                        negative_zero = add_constant(-0.0, 16)

                        return [
                            Mov(Double, asm_src, asm_dst),
                            Binary(op=Xor, t=Double, src=Data(negative_zero), dst=asm_dst)
                        ]
                    else:
                        // Fall through to the default case
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
                    if src_t == Double:
                        return convert_dbl_comparison(inst.op, dst_t, asm_src1, asm_src2, asm_dst)
                    else:
                        signed = src_t == Double
                            ? false
                            : TypeUtils.is_signed(tacky_type(inst.src1))

                        cond_code = convert_cond_code(inst.op, signed)

                        return [
                            Cmp(src_t, asm_src2, asm_src1),
                            Mov(dst_t, zero(), asm_dst),
                            SetCC(cond_code, asm_dst),
                        ]

                (* Division/modulo *)
                case Divide:
                case Mod:
                    if src_t != Double:
                        result_reg = op == Divide ? AX : DX

                        if (TypeUtils.is_signed(tacky_type(inst.src1))):
                            return [
                                Mov(src_t, asm_src1, Reg(AX)),
                                Cdq(src_t),
                                Idiv(src_t, asm_src2),
                                Mov(src_t, Reg(result_reg), asm_dst)
                            ]
                        else:
                            return [
                                Mov(src_t, asm_src1, Reg(AX)),
                                Mov(src_t, zero(), Reg(DX)),
                                Div(src_t, asm_src2),
                                Mov(src_t, Reg(result_reg), asm_dst)
                            ]
                    else:
                        // Jump to the default case
                        break
                case BiftShiftLeft:
				case BiftShiftRight:
                    is_signed = TypeUtils.is_signed(tacky_type(inst.src1))
					asm_op = convert_shift_op(inst.op, is_signed)
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
        case Load:
            asm_src_ptr = convert_val(inst.str_ptr)
            asm_dst = convert_val(inst.dst)
            t = asm_type(inst.dst)
            return [
                Mov(Quadword, asm_src_ptr, Reg(A9)),
                Mov(t, Memory(Reg(R9), 0), asm_dst),
            ]
        case Store:
            asm_src = convert_val(inst.src)
            t = asm_type(inst.src)
            asm_dst_ptr = convert_val(inst.dst_ptr)
            return [
                Mov(Quadword, asm_dst_ptr, Reg(R9)),
                Mov(t, asm_src, Memory(Reg(R9), 0)),
            ]
        case GetAddress:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            return [ Lea(asm_src, asm_dst) ]
		case Jump:
			return [ Jmp(inst.target) ]

		case JumpIfZero:
            t = asm_type(inst.cond)
			asm_cond = convert_val(inst.cond)

            if t == Double:
                compare_to_zero = [
                    Binary(Xor, Double, Reg(XMM0), Reg(XMM0)),
                    Cmp(t, asm_cond, Reg(XMM0)),
                ]

                lbl = UniqueIds.make_label("nan.jmp.end")
                conditional_jmp = [
                    // Comparison to NaN sets ZF and PF flags;
                    // to treat NaN as nonzero, skip over je instruction if PF flag is set
                    JmpCC(P, lbl),
                    JmpCC(E, inst.target),
                    Label(lbl),
                ]

                return [...compare_to_zero, ...conditional_jmp]
            else:
                return [
                    Cmp(t, zero(), asm_cond),
                    JmpCC(E, inst.target),
                ]
		case JumpIfNotZero:
            t = asm_type(inst.cond)
			asm_cond = convert_val(inst.cond)

            if t == Double:
                return [
                    Binary(Xor, Double, Reg(XMM0), Reg(XMM0)),
                    Cmp(t, asm_cond, Reg(XMM0)),
                    JmpCC(NE, inst.target),

                    // Also jumpt to target on Nan, which is nonzero
                    JmpCC(P, target),
                ]

            else:
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
        case ZeroExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ MovZeroExtend(asm_src, asm_dst) ]
        case IntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(src)

            return [
                Cvtsi2sd(t, asm_src, asm_dst)
            ]
        case DoubleToInt:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(asm_dst)

            return [
                Cvttsd2si(t, asm_src, asm_dst)
            ]
        case UIntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            if tacky_type(src) == Types.UInt:
                return [
                    MovZeroExtend(asm_src, Reg(R9)),
                    Cvtsi2sd(Quadword, Reg(R9), asm_dst)
                ]
            else:
                out_of_bound = UniqueIds.make_label("ulong2dbl.oob")
                end_lbl = UniqueIds.make_label("ulong2dbl.end")
                r1 = Reg(R8)
                r2 = Reg(R9)

                return [
                    Cmp(Quadword, zero(), asm_src),         // check whether asm_src is w/in range of long
                    JmpCC(L, out_of_bound),
                    Cvtsi2sd(Quadword, asm_src, asm_dst),   // it's in range, just use normal cvtsi2sd then jump to end
                    Jmp(end_lbl),
                    Label(out_of_bound),                    // it's out of bound
                    Mov(Quadword, asm_src, r1),             // halve source and round to dd
                    Mov(Quadword, r1, r2),
                    Unary(ShrOneOp, Quadword, r2),
                    Binary(And, Quadword, Imm(one), r1),
                    Binary(Or, Quadword, r1, r2),
                    Cvtsi2sd(Quadword, r2, asm_dst),        // convert to double, then double it
                    Binary(Add, Double, asm_dst, asm_dst),
                    Label(end_lbl)
                ]
        case DoubleToUInt:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            if tacky_type(dst) == Types.UInt:
                return [
                    Cvttsd2si(Quadword, asm_src, Reg(R9)),
                    Mov(Longword, Reg(R9), asm_dst)
                ]
            else:
                out_of_bounds = UniqueIds.make_label("dbl2ulong.oob")
                end_lbl = UniqueIds.make_label(dbl2ulong.end)
                upper_bound = add_constant(9223372036854775808.0)
                upper_bound_as_int = Imm(Int64.min_int)
                r = Reg(R9)
                x = Reg(XMM7)

                return [
                    Cmp(Double, Data(upper_bound), asm_src),
                    JmpCC(AE, out_of_bound),
                    Cvttsd2si(Quadword, asm_src, asm_dst),
                    Jmp(end_lbl),
                    Label(out_of_bounds),
                    Mov(Double, asm_src, x),
                    Binary(Sub, Double, Data(upper_bound), x),
                    Cvttsd2si(Quadword, x, asm_dst),
                    Mov(Quadword, upper_bound_as_int, r),
                    Binary(Add, Quadword, r, asm_dst),
                    Label(end_lbl)
                ]
```

```
pass_params(param_list):
    int_reg_params, dbl_reg_params, stack_params = classify_parameters(param_list)
    insts = []

    // pass params in registers
    reg_idx = 0
    for param_t, param in int_reg_params:
        r = int_param_passing_regs[reg_idx]
        insts.append(Mov(param_t, Reg(r), param))
        reg_idx++

    // pass params in registers
    reg_idx = 0
    for param in dbl_reg_params:
        r = dbl_param_passing_regs[reg_idx]
        insts.append(Mov(Double, Reg(r), param))
        reg_idx++

    // pass params on the stack
    // first param passed on stack has idx0 and is passed at Stack(16)
    stk_idx = 0
    for param_t, param in stack_params:
        stk = Memory(Reg(BP), 16 + (8*stk_idx))
        inst.append(Mov(param_t, stk, param))
        stk_idx++

    return insts
```

# ReplacePseudo

```
replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
            if AsmSymbols.is_static(operand.name):
                return (state, Assembly.Data(operand.name))
			else:
                if state.offset_map.has(operand.name):
                    return (state, Assembly.Memory(Reg(BP), state.offset_map.get(operand.name)))
                else:
                    size = AsmSymbols.get_size(operand.name)
                    alignment = AsmSymbols.get_alignment(operand.name)
                    new_offset = Rounding.round_away_from_zero(alignment, state.current_offset - size)
                    new_state = {
                        current_offset: new_offset,
                        offset_map: state.offset_map.add(operand.name, new_offset)
                    }
				    return (new_state, Assembly.Memory(Reg(BP), new_offset))
		default:
			return (state, operand)
```

```
// Extend for Lea instruction
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

        case MovZeroExtend:
            state1, new_src = replace_operand(inst.src, state)
            state2, new_dst = replace_operand(inst.dst, state1)
            new_movzx = MovZeroExtend(new_src, new_dst)
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
```

# Instruction Fixup

```
is_memory(operand):
    return operand.type is Data or Memory
```

```
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
        case XMM14:
        case XMM15:
            return true
        default:
            return false
```

We change anywhere we check for Stack to be Memory, add a new case for Lea, and extend the Push for pushing registers.

```
fixup_instruction(Assembly.Instruction inst):
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
        else:
            return [ inst ]
    case Movsx:
        // Movsx cannot handle immediate src or memory dst
        if inst.src is Imm && inst.dst is (Memory | Data):
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
        else if inst.dst is (Memory | Data):
            return [
                Movsx(inst.src, Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
        else:
            return [ inst ]
    case MovZeroExtend:
        // Rewrite MovZeroExtend as one or two instructions.
        if inst.dst is Reg:
            return [
                Mov(Longword, inst.src, inst.dst)
            ]
        else:
            return [
                Mov(Longword, inst.src, Reg(R11)),
                Mov(Quadword, Reg(R11), inst.dst)
            ]
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
        // Div can't operant on constants
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
    default:
        return [ other ]
```

# Emit

```
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
        case BP: fail("Internal error: no 32-bit RBP")
        default:
            fail("Internal error: can't store longword in XMM register")
```

```
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
        case BP: return "%rbp"
        default:
            fail("Internal error: can't store quadword type in XMM register")
```

```
// Stack is replaced with Memory
show_operand(asm_type, operand):
    if operand is Reg:
        if asm_type is Longword:
            return show_long_reg(operand)
        else if asm_type is Quadword:
            return show_quadword_reg(operand)
        else:
            return show_double_reg(operand)
    else if operand is Imm:
        return "${operand.value}"
    else if operand is Memory:
        if operand.offset = 0:
            return "({show_quadword_reg(operand.reg)})"
        return "%{operand.offset}({show_quadword_reg(operand.reg)})"
    else if operand is Data:
        lbl =
            AssemblySymbols.is_constant(operand.name)
                ? show_local_label(operand.name)
                : show_label(operand.name)
        return "{lbl}(%rip)"
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
        case BP: fail("Internal error: no one-byte RBP")
        default:
            fail("Internal error: can't store byte type in XMM register")
```

Extend the emit_instruction for Lea.

```
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
                fail("Internal error: can't apply cdq to double type")
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

        case Call(f):
            return "\tcall {show_fun_name(f)}\n"

        case Movsx(src, dst):
            return "\tmovslq {show_operand(Longword, src)}, {show_operand(Quadword, dst)}\n"

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

        case MovZeroExtend:
            fail("Internal error: MovZeroExtend should have been removed in instruction rewrite pass")
```

# Output

From C:

```C
int i = 0;

int putchar(int c);
int *print_A(void) {
    putchar(65);
    return &i;
}

int main(void) {
    *print_A() += 5;
    if (i != 5) {
        return 1;
    }
    return 0; // success
}
```

To x64 Assembly on Linux:

```asm
	.global i
	.bss
	.align 4
i:
	.zero 4

	.global print_A
print_A:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$65, %edi
	call	putchar@PLT
	movl	%eax, -4(%rbp)
	leaq	i(%rip), %r11
	movq	%r11, -16(%rbp)
	movq	-16(%rbp), %rax
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
	call	print_A
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	addl	$5, -12(%rbp)
	movq	-8(%rbp), %r9
	movl	-12(%rbp), %r10d
	movl	%r10d, (%r9)
	movl	$5, %r11d
	cmpl	i(%rip), %r11d
	movl	$0, -16(%rbp)
	setne	-16(%rbp)
	cmpl	$0, -16(%rbp)
	je	.Lif_end.5
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.5:
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
