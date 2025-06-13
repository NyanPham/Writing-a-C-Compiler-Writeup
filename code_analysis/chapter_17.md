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

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordSizeOf | sizeof |

# Types

```
type Char {}
type SChar {}
type UChar {}
type Int {}
type Long {}
type UInt {}
type ULong {}
type Double {}
type Pointer {
    referenced_t: t,
}
type Void {}
type Array {
    elem_type: t,
    size: int,
}
type FunType {
    param_types: t[],
    ret_type: t,
}

t = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Pointer | Void | Array | FunType
```

# Const

_No changes_

# ConstConvert

_Actually no changes_

The default case in const_convert already accounts for the void case.

```
// create different for each type of v, better use template if your implementation language supports it
cast(int32/uint32/int64/uint64 v, Types.t targetType):
    switch targetType:
        case Types.Char:
        case Types.SChar: return Const.ConstChar(to_int8(v))
        case Types.UChar: return Const.ConstUChar(to_int8(v))
        case Types.Int: return Const.ConstInt(to_int32(v))
        case Types.UInt: return Const.ConstUInt(to_uint32(v))
        case Types.Long: return Const.ConstLong(to_int64(v))
        case Types.ULong: return Const.ConstULong(to_uint64(v))
        case Types.Pointer: return Const.ConstULong(to_uint64(v))
        case Types.Double: return Const.Double(to_double(v))
        default: // targetType is funType or Array or Void
            fail("Internal error: cannot cast constant to non-scalar type")
```

```
const_convert(Types.t target_type, Const.const const):
    match const:
        case ConstChar(c):
            return cast(c, target_type)
        case ConstUChar(uc):
            return cast(uc, target_type)
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

The Type file already record changes for Void type.
We add two new sizeof expressions: SizeOfT and SizeOf.
Return statement has optional inner expression.

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer* list)
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Void
    | Pointer(type referenced)
    | Array(type elem_type, int size)
    | FunType(type* params, type ret)
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
    | String(string)
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
    | Dereference(exp, type)
    | AddrOf(exp, type)
    | Subscript(exp, exp, type)
    | SizeOf(exp, type)
    | SizeOfT(type, type)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
    | ConstChar(int) | ConstUChar(int)
```

# Parser

```
// Void is also type specifier
is_type_specifier(token):
    switch token type:
        case "int":
        case "long":
        case "double":
        case "signed":
        case "unsigned":
        case "char":
        case "void":
            return true
        default:
            return false
```

```
// Check void type right on top
// and raise error if appears with other specifier.
parse_type(specifier_list):
    /*
        sort specifiers so we don't need to check for different
        orderings of same specifiers
    /*

    specifier_list = sort(specifier_list)

    if specifier_list == [ "void" ]:
        return Types.Void
    else if specifier_list == ["double]:
        return Types.Double
    else if specifier_list == ["char"]:
        return Types.Char
    else if specifier_list == ["char", "signed"]:
        return Types.SChar
    else if specifier_list == ["char", "unsigned"]:
        return Types.UChar

    if (len(specifier_list) == 0 or
        len(set(specifier_list)) != len(specifier_list) or
        ("double in specifier_list) or
        ("char" in specifier_list) or
        ("void" in specifier_list) or
        (("signed" in specifier_list) and ("unsigned" in specifier_list))):
        fail("Invalid type specifier")
    else if "unsigned" in specifier_list and "long" in specifier_list:
        return Types.ULong
    else if "unsigned" in specifier_list:
        return Types.UInt
    else if "long" in specifier_list:
        return Types.Long
    else:
        return Types.Int
```

```
parse_param_list(tokens):
    expect("(", tokens)

    if npeek(2, tokens) is [ "void", ")" ]:
        take(tokens)
        params = []
    else:
        params = param_loop(tokens)

    expect(")", tokens)
    return params
```

```
// NEW
// <type-name> ::= { <type-specifier> }+ [ <abstract-declarator> ]
parse_type_name(tokens):
    type_specifiers = parse_type_specifier_list(tokens)
    base_type = parse_type(type_specifiers)
    /*
    check for optional abstract declarator
    note that <type-name> is always followed by close paren,
    although that's not part of the grammar rule
    */

    if peek(tokens) is ")":
        return base_type
    else:
        abstract_decl = parse_abstract_declarator(tokens)
        return process_abstract_declarator(abstract_decl, base_type)
```

We replace completely the parse_factor with parse_cast_expression and parse_unary_expression.

```
// New
/* New<cast-exp> ::= "(" <type-name> ")" <cast-exp>Add commentMore actions
    | <unary-exp> */
parse_cast_expression(tokens):
    next_toks = npeek(2, tokens)

    if next_toks[0] == "(" && is_type_specifier(next_toks[1]):
        // this is a cast expression
        take(tokens) // consume open parenthesis
        target_type = parse_type_name(tokens)
        expect(")", tokens)
        inner_exp = parse_cast_expression(tokens)
        return Cast(target_type, inner_exp)
    else:
        return parse_unary_expression(tokens)
```

```
// New
/* <unary-exp> ::= <unop> <cast-exp>
   | "sizeof" <unary-exp>
   | "sizeof" "(" <type-name> ")"
   | <postfix-exp>*/
parse_unary_expression(tokens):
    next_tokens = npeek(3, tokens)
	// unary expression
	if next_tokens[0] is Tilde, Hyphen, Bang, DoublePlus, DoubleHyphen:
		op = parse_unop(tokens)
		inner_exp = parse_cast_expression(tokens)
		return Unary(op, inner_exp)
    else if next_tokens[0] is "*":
        take_token(tokens)
        inner_exp = parse_cast_expression(tokens)
        return Dereference(inner_exp)
    else if next_tokens[0] is "&" :
        take_token(tokens)
        inner_exp = parse_cast_expression(tokens)
        return AddrOf(inner_exp)
    else if next_tokens[0] is "sizeof" and next_tokens[1] is "(" and is_type_specifier(next_tokens[2]):
        // this is a size of a type name
        take(tokens)
        take(tokens)
        target_type = parse_type_name(tokens)
        expect(")", tokens)
        return SizeOfT(target_type)
    else if next_tokens[0] is "sizeof":
        // size of an expression
        take(tokens)
        inner_exp = parse_unary_expression(tokens)
        return SizeOf(inner_exp)
	else:
		return parse_postfix_exp(tokens)
```

```
// call parse_cast_expression instead of parse_factor
parse_exp(tokens, min_prec):
	left = parse_cast_expression(tokens)
	next_token = peek(tokens)

	while next_token is a binary operator and precedence(next_token) >= min_prec:
		if is_assignment(next_token):
			take_token(tokens) // remove "=" from list of tokens
			right = parse_exp(tokens, precedence(next_token))
			op = get_compound_operator(next_token)
			if op is null:
				left = Assignment(left, right)
			else:
				left = CompoundAssignment(op, left, right)
		else:
			operator = parse_binop(tokens)
			right = parse_exp(tokens, precedence(next_token) + 1)
			left = Binary(operator, left, right)
		next_token = peek(tokens)
	return left
```

```
parse_statement(tokens):
	token1, token2 = npeek(2, tokens)
	match token1 type:
		case "return":
			take(tokens)
            exp = parse_optional_expression(";", tokens)
            return Return(exp)
        --snip--
```

# Initializers

_No changes_

# Identifier Resolution

We add cases to resolve SizeOf, and leave SizeOfT untouched.

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
        case SizeOf:
            return SizeOf(resolve_exp(exp.exp, id_map))
        case Constant:
        case String:
        case SizeOfT:
            return exp
        case Conditional:
            return Conditional(
                resolve_exp(exp.condition, id_map),
                resolve_exp(exp.then, id_map),
                resolve_exp(exp.else, id_map),
                exp.type
            )
        case FunctionCall(fun_name, args):
            if fun_name is in id_map:
                new_fun_name = id_map.get(fun_name).new_name
                new_args = []
                for arg in args:
                    new_args.append(resolve_exp(arg, id_map))

                return FunctionCall(new_fun_name, new_args, exp.type)
            else:
                fail("Undeclared function!")
        case Dereferene(inner):
            return Dereference(resolve_exp(inner, id_map))
        case AddOf(inner):
            return AddrOf(resolve_exp(inner, id_map))
        case Subscript(ptr, index):
            return Subscript(resolve_exp(ptr, id_map), resolve_exp(index, id_map))
        default:
            fail("Internal error: Unknown expression")
```

```
resolve_statement(statement, var_map):
	match statement with
        case Return:
            resolved_e = if statement has value
                            then resolve_exp(statement.value, id_map)
                            else: null
            return Return(resolved_e)
        --snip--
```

# ValidateLabels

_No Changes_

# LoopLabeling

_No Changes_

# CollectSwitchCases

_No Changes_

# TypeUtils

Void type is invalid to get size, alignment, signedness

```
// default case already handles Void
get_size(Types.t type):
    switch type:
        case Types.Char:
        case Types.SChar:
        case Types.UChar:
            return 1
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8
        case Types.Array:
            return type.size * get_size(type.elem_type)
        default:
            fail("Internal error: function type doesn't have size")
```

```
// default case already handles Void
get_alignment(Types.t type):
    switch type:
        case Types.Char:
        case Types.UChar:
        case Types.SChar:
            return 1
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8:
        case Types.Array:
            return get_alignment(type.elem_type)
        case Types.FunType:
            fail("Internal error: function type doesn't have alignment")
```

```
// default case already handles Void
is_signed(Types.t type):
    switch type:
        case Types.Int:
        case Types.Long:
        case Types.Char:
        case Types.SChar:
            return true
        case Types.UInt:
        case Types.ULong:
        case Types.Pointer:
        case Types.UChar:
            return false
        default:
            fail("Internal error: signedness doesn't make sense for function, double type, and array type")
```

```
// default case already handles Void
is_integer(type):
    switch (type):
        case Char:
        case UChar:
        case SChar:
        case Int:
        case UInt:
        case Long:
        case ULong:
            return true
        default:
            return false
```

```
// default case already handles Void
is_arithmetic(type):
    switch (type):
        case Int:
        case UInt:
        case Long:
        case ULong:
        case Double:
        case Char:
        case SChar:
        case UChar:
            return true
        default:
            return false
```

```
// NEW
is_scalar(type):
    switch (type):
        case Array:
        case Void:
        case FunType:
            return false
        default:
            return true
```

```
// NEW
is_complete(type):
    return type != Void
```

```
// NEW
is_complete_pointer(type):
    if type is Pointer(t):
        return is_complete(t)
    else:
        return false
```

# Symbols

_No changes_

# TypeCheck

```
// NEW
validate_type(type):
    if type is Array(elem_type, _):
        if is_complete(elem_type):
            validate_type(elem_type)
        else:
            fail("Array of incomplete type")

    else if type is Pointer(t):
        validate_type(t)

    else if type is FunType(param_types, ret_type):
        for param_t in param_types:
            validate_type(param_t)
        validate_type(ret_type)

    else if type is in (Char, SChar, UChar, Int, Long, UInt, ULong, Double, Void):
        return
```

```
get_common_pointer_type(e1, e2):
    if e1.type == e2.type:
        return e1.type
    else if is_null_pointer_constant(e1.e):
        return e2.type
    else if is_null_pointer_constant(e2.e):
        return e1.type
    else if
        (e1.type is Pointer(Void) and is_pointer(e2))
        OR
        (e2.type is Pointer(Void) and is_pointer(e1)):
        return Pointer(Void)
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
    else if
        (target_type is Pointer(Void) and is_pointer(e.type))
        OR
        (is_pointer(target_type) and e.type == Pointer(Void)):
        convert_to(e, target_type)
    else:
        fail("Cannot convert type for assignment")
```

```
// Add cases for SizeOfT and SizeOf
typecheck_exp(e, symbols):
	match e with:
	case Var(v):
		return typecheck_var(v)
	case Constant(c):
        return typecheck_const(c)
    case String(s):
        return typecheck_string(s)
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
        switch op:
            case And:
            case Or:
                return typecheck_logical(op, e1, e2)
            case Add:
                return typecheck_addition(e1, e2)
            case Subtract:
                return typecheck_subtraction(e1, e2)
            case Multiply:
            case Divide:
            case Remainder:
                return typecheck_multiplicative(op, e1, e2)
            case Equal:
            case NotEqual:
                return typecheck_equality(op, e1, e2)
            case GreaterThan:
            case GreaterOrEqual:
            case LessThan:
            case LessOrEqual:
                return typecheck_comparison(op, e1, e2)
            case BitwiseAnd:
            case BitwiseOr:
            case BitwiseXor:
                return typecheck_bitwise(op, e1, e2)
            case BitshiftLeft:
            case BitshiftRight:
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
    case Subscript():
        return typecheck_subscript(exp)
    case SizeOfT(t):
        return typecheck_size_of_t(t)
    case SizeOf(e):
        return typecheck_size_of(e)
```

```
typecheck_cast(target_type, inner):
    validate_type(target_type)
    typed_inner = typecheck_and_convert(inner)

    if
        (target_type is Pointer and typed_inner.type is Double)
        OR
        (target_type is Double and typed_inner.type is Pointer):
            fail("Cannot cast between pointer and double")

    if target_type is Void:
        cast_exp = Cast(Void, typed_inner)
        return set_type(cast_exp, Void)

    if not is_scalar(target_type):
        fail("Can only cast to scalar types or void")
    else if not is_scalar(typed_inner.type):
        fail("Can only cast scalar expressions to non-void type")

    cast_exp = Cast(target_type, typecheck_and_convert(inner))
    return set_type(cast_exp, target_type)
```

```
// NEW
typecheck_scalar(e):
    typed_e = typecheck_and_convert(e)
    if is_scalar(typed_e.type):
        return typed_e
    else:
        fail("A scalar operand is required")
```

```
//  call typecheck_scalar that wraps typecheck_and_convert
typecheck_not(inner):
    typed_inner = typecheck_scalar(inner)
    not_exp = Unary(Not, typed_inner)
    return set_type(not_exp, Types.Int)
```

```
typecheck_complement(inner):
    typed_inner = typecheck_and_convert(inner)
    if not is_integer(typed_inner.type):
        fail("Bitwise complement only valid for integer types")
    else:
        // promote character types to int
        typed_inner = if is_character(typed_inner.type)
                        then convert_to(typed_inner, Types.Int)
                        else typed_inner

        complement_exp = Unary(Complement, typed_inner)
        return set_type(commplement_exp, get_type(typed_inner))
```

```
// call is_arithmetic
typecheck_negate(inner):
    typed_inner = typecheck_and_convert(inner)
    if is_arithmetic(typed_inner.type):
        // promote character types to int
        typed_inner = if is_character(typed_inner.type)
                        then convert_to(typed_inner, Types.Int)
                        else typed_inner

        negate_exp = Unary(Negate, typed_inner)
        return set_type(negate_exp, get_type(typed_inner))
    else:
        fail("Can only negate arithmetic types")
```

```
// needs check the lvalue is arithmetic or complete pointer.
typecheck_incr(op, inner):
    typed_inner = typecheck_and_convert(inner)
    if is_lvalue(typed_inner) and (is_arithmetic(typed_inner.type) OR is_complete_pointer(typed_inner.type)):
        typed_exp = Unary(op, typed_inner)
        return set_type(typed_exp, get_type(typed_inner))
    else:
        fail("Operand of ++/-- must be an lvalue with arithmetic or pointer type")
```

```
// needs check the lvalue is arithmetic or complete pointer.
typecheck_postfix_decr(e):
    typed_e = typecheck_and_convert(e)
    if is_lvalue(typed_e) and (is_arithmetic(typed_e.type) OR is_complete_pointer(typed_e.type)):
        // result has same value as e; no conversions required.
        // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far
        result_type = get_type(typed_e)
        return set_type(PostfixDecr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue with arithmetic or pointer type")
```

```
typecheck_postfix_incr(e):
    typed_e = typecheck_and_convert(e)
    if is_lvalue(typed_e) and (is_arithmetic(typed_e.type) OR is_complete_pointer(typed_e.type)):
        // Same deal as postfix decrement
        result_type = get_type(typed_e)
        return set_type(PostfixIncr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue with arithmetic or pointer type")
```

```
// call typecheck_scalar instead of typecheck_and_convert
typecheck_logical(op, e1, e2):
    typed_e1 = typecheck_scalar(e1)
    typed_e2 = typecheck_scalar(e2)
    typed_binexp = Binary(op, typed_e1, typed_e2)
    return set_type(typed_binexp, Types.Int)
```

```
// check if pointer is complete, not just a sole pointer.
typecheck_addition(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1) and is_arithmetic(typed_e2):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        add_exp = Binary(Add, converted_e1, converted_e2)
        return set_type(add_exp, common_type)
    else if is_complete_pointer(typed_e1.type) and is_integer(typed_e2.type):
        converted_e2 = convert_to(typed_e2, Types.Long)
        add_exp = Binary(Add, typed_e1, converted_e2)
        return set_type(add_exp, typed_e1.type)
    else if is_complete_pointer(typed_e2.type) and is_integer(typed_e1.type):
        converted_e1 = convert_to(typed_e1, Types.Long)
        add_exp = Binary(Add, converted_e1, typed_e2)
        return set_type(add_exp, typed_e2.type)
    else:
        fail("invalid operands for addition")
```

```
// check if pointer is complete, not just a sole pointer.
typecheck_subtraction(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1) and is_arithmetic(typed_e2):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        sub_exp = Binary(Subtract, converted_e1, converted_e2)
        return set_type(sub_exp, common_type)
    else if is_complete_pointer(typed_e1.type) and is_integer(typed_e2.type):
        converted_e2 = convert_to(typed_e2, Types.Long)
        sub_exp = Binary(Subtract, typed_e1, converted_e2)
        return set_type(sub_exp, typed_e1.type)
    else if is_complete_pointer(typed_e1.type) and typed_e1.type == typed_e2.type:
        sub_exp = Binary(Subtract, typed_e1, typed_e2)
        return set_type(sub_exp,  Types.Long)
    else:
        fail("invalid operands for subtraction")
```

```
// check if the typed_e1, and typed_e2 are arithmetic directly
typecheck_multiplicative(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1.type) and is_arithmetic(typed_e2.type):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        bin_exp = Binary(op, converted_e1, converted_e2)

        if op is Remainder and common_type is Double:
            fail("Can't apply % to double")
        else if op is Multiply or Divide or Remainder:
            return set_type(bin_exp, common_type)
        else:
            fail("The op is not a multiplacative operator")
    else:
        fail("Can only multiply arithmetic types")
```

```
// before calling get_common_type, check if both operands are arithmetic
typecheck_equality(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_pointer(typed_e1.type) or is_pointer(typed_e2.type):
        common_type = get_common_pointer_type(typed_e1, typed_e2)
    else if is_arithmetic(typed_e1.type) and is_arithmetic(typed_e2.type):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
    else:
        fail("Invalid operands for equality")

    converted_e1 = convert_to(typed_e1, common_type)
    converted_e2 = convert_to(typed_e2, common_type)
    bin_exp = Binary(op, converted_e1, converted_e2)
    return set_type(bin_exp, Int)
```

```
// Move the integer promotions in the else branch.
typecheck_bitshift(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if NOT (is_integer(get_type(typed_e1)) AND is_integer(get_type(typed_e2))):
        fail("Both operands of bit shift operation must be an integer")
    else:
        // promote both operands to from character to int type
        typed_e1 = if is_character(typed_e1.type)
                        then convert_to(typed_e1, Types.Int)
                        else typed_e1

        typed_e2 = if is_character(typed_e2.type)
                        then convert_to(typed_e2, Types.Int)
                        else typed_e2

        // Don't perform usual arithmetic conversions; result has type of left operand
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, get_type(typed_e1))
```

```
// Be stricter while checking *=, /=.
// And pointer in +=, -= must be complete pointers
typecheck_compound_assignment(op, lhs, rhs):
    typed_lhs = typecheck_and_convert(lhs)
    if is_lvalue(typed_lhs):
        lhs_type = get_type(typed_lhs)
        typed_rhs = typecheck_and_convert(rhs)
        rhs_type = get_type(typed_rhs)

        // %= and compound bitwise ops only permit integer types
        if op is in (Remainder, BitwiseAnd, BitwiseOr, BitwiseXor, BitshiftLeft, BitshiftRight) AND (NOT(is_integer(lhs_type) OR NOT(is_integer(rhs_type)))):
            fail("Operand {op} only supports integer operands")

        // *= and /= only support arithmetic types
        if op is in (Multiply, Divide) AND (not is_arithmetic(lhs_type) OR not is_arithmetic(rhs_type)):
            fail("Operand {op} only supports arithmetic operands")

        // += and -= require either two arithmetic operators, or pointer on LHS and integer on RHS
        if op is (Add or Subtract)
            AND NOT(
                (is_arithmetic(lhs_type) and is_arithmetic(rhs_type))
                OR (is_complete_pointer(lhs_type) and is_integer(rhs_type))
            ):
            fail("invalid types for +=/-=")

        result_t, converted_rhs
        if op is BitShiftLeft or BitShiftRight:
            // Apply integer type promotions to >>= and <<=, but don't convert to common type
            lhs_type = if is_character(lhs_type) then Types.Int else lhs_type
            result_t = lhs_type
            converted_rhs = if is_character(typed_rhs.type) then convert_to(typed_rhs, Types.Int) else typed_rhs
        else if is_pointer(lhs_type):
            // For += and -= with pointers, convert RHS to Long and leave LHS type as result type
            result_t = lhs_type
            converted_rhs = convert_to(typed_rhs, Long)
        else:
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

```
// for condition, we call typecheck_scalar.
// change the common_type to result_type
// check if both operands are Void, either is pointer, or both are arithmetic
typecheck_conditional(condition, then_exp, else_exp):
    typed_condition = typecheck_scalar(condition)
    typed_then = typecheck_and_convert(then_exp)
    typed_else = typecheck_and_convert(else_exp)

    if typed_then.type is Void and typed_else.type is Void:
        result_type = Void
    else if is_pointer(typed_then.type) or is_pointer(typed_else.type):
        result_type = get_common_pointer_type(typed_then, typed_else)
    else if is_arithmetic(typed_then.type) and is_arithemtic(typed_else.type):
        result_type = get_common_type(typed_then.type, typed_else.type)
    else:
        fail("Invalid operands for conditional")

    converted_then = convert_to(typed_then, result_type)
    converted_else = convert_to(typed_else, result_type)

    conditionl_exp = Conditional(
        typed_condition,
        converted_then,
        converted_else
    )

    return set_type(conditional_exp, result_type)
```

```
// reject dereference pointer to void
typecheck_dereference(inner):
    typed_inner = typecheck_and_convert(inner)
    t = get_type(typed_inner)

    if t is Pointer(void):
        fail("Can't dereference pointer to void")
    else if t is Pointer(referenced_t):
        deref_exp = Dereference(typed_inner)
        return set_type(deref_exp, referenced_t)
    else:
        fail("Tried to dereference non-pointer")
```

```
// call is_complete_pointer instead of mere is_pointer
typecheck_subscript(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    ptr_type, converted_e1, converted_e2

    if is_complete_pointer(typed_e1.type) and is_integer(typed_e2.type):
        ptr_type = typed_e1.type
        converted_e1 = typed_e1
        converted_e2 = convert_to(typed_e2, Types.Long)
    else if is_complete_pointer(typed_e2.type) and is_integer(typed_e1.type):
        ptr_type = typed_e2.type
        converted_e1 = convert_to(typed_e1, Types.Long)
        converted_e2 = typed_e2
    else:
        fail("Invalid types for subscript operation")

    result_type

    if ptr_type is Pointer(referenced):
        result_type = referenced
    else:
        fail("Internal error: typechecking subscript")

    subscript_exp = Subscript(converted_e1, index = converted_e2)
    return set_type(subscript_exp, result_type)
```

```
// NEW
typecheck_size_of_t(type):
    validate_type(type)
    if is_complete(type):
        sizeof_exp = SizeOfT(type)
        return set_type(sizeof_exp, ULong)
    else:
        fail("Can't apply sizeof to incomplete type")
```

```
// NEW
typecheck_size_of(inner):
    typed_inner = typecheck_exp(inner)
    if is_complete(typed_inner.type):
        sizeof_exp = SizeOf(typed_inner)
        return set_type(sizeof_exp, ULong)
    else:
        fail("Can't apply sizeof to incomplete type")
```

```
// add case for Void the same as FunType
make_zero_init(type):
    scalar = (Constant.Const c) -> return SingleInit(AST.Constant(c), type)

    switch type:
        case Array(elem_type, size):
            return CompoundInit([make_zero_init(elem_type)] * size, type)
        case Char:
        case SChar:
            return scalar(Constant.ConstChar(0))
        case Int:
            return scalar(Constant.ConstInt(0))
        case UChar:
            return scalar(Constant.ConstUChar(0))
        case UInt:
            return scalar(Constant.ConstUInt(0))
        case Long:
            return scalar(Constant.ConstLong(0))
        case ULong:
        case Pointer:
            return scalar(Constant.ConstULong(0))
        case Double:
            return scalar(Constant.ConstDouble(0))
        case FunType:
        case Void:
            fail("Internal error: can't create zero initializer with function or void type")
```

```
// Extend more checks on Return.
// Call the typecheck_scalar for condition expressions in If, While, DoWhile, ForLoop
// Move the integer promotion of Switch into the else branch.
typecheck_statement(stmt, ret_type):
    match stmt type:
    case Return(e):
        if e is not null:
            if ret_type == Void:
                fail("function with void return type cannot return a value")
            else:
                typed_e = convert_by_assignment(typecheck_and_convert(e), ret_type)
                return Return(typed_e)
        else:
            if ret_type is Void:
                return Return()
            else:
                fail("function with non-void return type must return a value")
    case Expression(e):
        return Expression(typecheck_and_convert(e))
    case If(condition, then_clause, else_clause):
        return If(
            condition=typecheck_scalar(condition),
            then_clause=typecheck_statement(then_clause, ret_type),
            else_clause= else_clause == null ? null : typecheck_statement(else_clause, ret_type)
        )
    case LabeledStatement(lbl, s):
        return LabeledStatement(lbl, typecheck_statement(s, ret_type))
    case Case(e, s, id):
        typed_e = typecheck_and_convert(e)
        if get_type(typed_e) == Double:
            fail("Case expression cannot be bouble")
        else:
            // Note: e must be converted to type of controlling expression in enclosing switch;
            // we do that during CollectSwitchCases pass
            return Case(typecheck_and_convert(e), typecheck_statement(s, ret_type), id)
    case Default(s, id):
        return Default(typecheck_statement(s, ret_type), id)
    case Switch(control, body, cases, id):
        typed_control = typecheck_and_convert(control)

        if NOT is_integer(get_type(typed_control)):
            fail("Controlling expression in switch must have integer type")

        // Perform integer promotions on controlling expression
        typed_control = if is_character(typed_control.type) then convert_to(typed_control, Types.Int) else typed_control

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
            condition=typecheck_scalar(condition),
            body=typecheck_statement(body, ret_type),
            id
        )
    case DoWhile(body, condition, id):
        return DoWhile(
            body=typecheck_statement(body),
            condition=typecheck_scalar(condition),
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
            typechecked_for_init = InitExp(typecheck_and_convert(init.exp))

        return For(
            init = typechecked_for_init
            condition = condition is not null ? typecheck_scalar(condition) : null
            post = post is not null ? typecheck_and_convert(post) : null
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
// check on top if we declare with void type, and validate_type
typecheck_local_var_decl(var_decl):
    if var_decl.var_type is Void:
        fail("No void declarations")
    else:
        validate_type(var_decl.var_type)

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
            new_init = typecheck_init(var_decl.var_type, var_decl.init)

        return var_declaration(var_decl.name, init=new_init, storage_class=null, t=var_decl.var_type, )
```

```
// call validate_type on top
// and reject any parameter of void type
typecheck_fn_decl(fn_decl):
    validate_type(fn_decl.fun_type)

    param_ts = []
    return_t, fun_type

    if fn_decl.fun_type == FunType:
        if fn_decl.fun_type.ret_type == Array:
            fail("A function cannot return an array")
        else:
            for param_t in fn_decl.fun_type.param_types:
                if param_t is Array(elem_type):
                    param_ts.append(Pointer(elem_type))
                else if param_t is Void:
                    fail("No void params allowed")
                else:
                    param_ts.append(param_t)
        return_t = fn_decl.fun_type.ret_type
        fun_type = FunType(param_ts, fn_decl.fun_type.ret_type)
    else:
        fail("Internal error: function has non function type")

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
// similar to typecheck_local_var_decl,
// we reject declaration of void and call validate_type
typecheck_file_scope_var_decl(var_decl):
    if var_decl.var_type is Void:
        fail("void variables not allowed")
    else:
        validate_type(var_decl.var_type)

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

# TACKY

Return has optional val, and dst of FunCall is also optional.

```
program = Program(top_level*)
top_level =
    | Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init* inits)
    | StaticConstant(identifier name, type t, Initializers.static_init init)
instruction =
    | Return(val?)
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
    | Addptr(val ptr, val index, int scale, val dst)
    | CopyToOffset(val src, identifier dst, int offset)
    | Jump(identifier target)
    | JumpIfZero(val condition, identifier target)
    | JumpIfNotZero(val condition, identifier target)
    | Label(identifier)
    | FunCall(identifier fun_name, val* args, val? dst)
val = Constant(const) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
                | LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
```

# TACKYGEN

```
// NEW
// use this as the "result" of void expressions that don't return a result
dummy_operand = Constant(Const.int_zero)
```

```
// NEW
eval_size(type):
    size = TypeUtils.get_size(type)
    return Tacky.Constant(Const.ConstLong(int64(size)))
```

```
// Add cases for SizeOf and SizeOfT
emit_tacky_for_exp(exp):
	match exp.type:
		case AST.Constant(c):
            return ([], PlainOperand(TACKY.Constant(c)))
		case AST.Var(v):
            return ([], PlainOperand(TACKY.Var(v)))
		case AST.String(s):
            str_id = symbols.add_string(s)
            return ([], PlainOperand(Var(str_id)))
        case Unary:
			if exp.op is increment:
                const_t = if TypeUtils.is_pointer(exp.type) then Types.Long else type
				return emit_compound_expression(AST.Add, exp.exp, mk_ast_const(const_t, 1), exp.type)
			else if exp.op is decrement:
                const_t = if TypeUtils.is_pointer(exp.type) then Types.Long else type
				return emit_compound_expression(AST.Subtract, exp.exp, mk_ast_const(const_t, 1), exp.type)
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
                if exp.op is Add and TypeUtils.is_pointer(exp.type):
                    return emit_pointer_addition(exp.type, exp.e1, exp.e2)
                else if exp.op is Subtract and TypeUtils.is_pointer(exp.type):
                    return emit_subtraction_from_pointer(exp.type, exp.e1, exp.e2)
                else if exp.op is Subtract and TypeUtils.is_pointer(exp.e1.type):
                    // at least one operand is pointer but result isn't, must be subtracting one pointer from another
                    return emit_pointer_diff(exp.type, exp.e1, exp.e2)

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
        case Subscript:
            return emit_subscript(exp.type, exp.e1, exp.e2)
        case SizeOfT:
            return ([], PlainOperand(eval_size(exp.type)))
        case SizeOf:
            return ([], PlainOperand(eval_size(exp.exp.type)))
```

```
// update the if condition
emit_cast_expression(cast):
    eval_inner, result = emit_tacky_and_convert(cast.exp)
    src_type = get_type(cast.exp)
    if src_type == cast.target_type OR target_type == Void:
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
// to get correct dst, check if conditional.t is Void or not
emit_conditional_expression(conditional):
    eval_cond, c = emit_tacky_and_convert(conditional.condition)
    eval_v1, v1 = emit_tacky_and_convert(conditional.e1)
    eval_v2, v2 = emit_tacky_and_convert(conditional.e2)
    e2_label = UniqueIds.make_label("conditional_else")
    end_label = UniqueIds.make_label("conditional_end")

    if conditional.t is Void:
        dst = dummy_operand
    else:
        dst_name = create_tmp(conditional.t)
        dst = Var(dst_name)

    common_insts = [
        ...eval_cond,
        JumpIfZero(c, e2_label),
        ...eval_v1,
    ]

    remaining_insts = []

    if conditional.t is Void:
        remaining_insts = [
            Jump(end_label),
            Label(e2_label),
            ...eval_v2,
            Label(end_label)
        ]
    else:
        remaining_insts = [
            Copy(v1, dst),
            Jump(end_label),
            Label(e2_label),
            ...eval_v2,
            Copy(v2, dst),
            Label(end_label)
        ]

    insts = [...common_insts, ...remaining_insts]
    return (insts, PlainOperand(dst))
```

```
// get dst based on the type
emit_fun_call(AST.FunctionCall fun_call):
    if fun_call.type is Void:
        dst = null
    else:
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

    dst_val = if dst then dst else dummy_operand
    return (insts, PlainOperand(dst_val))
```

```
// Update for Return
emit_tacky_for_statement(statement):
	match statement type:
		case Return:
            if statement has exp:
			    eval_exp, v = emit_tacky_and_convert(statement.exp)
            else:
                eval_exp = []
                v = null

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

# Assembly

_No changes_

```
program = Program(top_level*)
asm_type =
    | Byte
    | Longword
    | Quadword
    | Double
    | ByteArray(int size, int alignment)
top_level =
    | Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int alignment, static_init* inits)
    | StaticConstant(identifier name, int alignment, static_init init)
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
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | Push(operand)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not | ShrOneOp
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor | Sal | Sar | Shr | Shl
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier) | PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

# AssemblySymbols

_No changes_

```
get_size(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
            if symbol_table[var_name] has asm_type of Byte:
                return 1
            if symbol_table[var_name] has asm_type of Longword:
                return 4
            if symbol_table[var_name] has asm_type of Quadword or Double:
                return 8
            if symbol_table[var_name] has asm_type of ByteArray(size, _):
                return size
        else:
            fail("Internal error: this is a function, not an object")
    else:
        fail("Internal error: not found")
```

```
get_alignment(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
            if symbol_table[var_name] has asm_type of Byte:
                return 1
            if symbol_table[var_name] has asm_type of Longword:
                return 4
            if symbol_table[var_name] has asm_type of Quadword or Double:
                return 8
            if symbol_table[var_name] has asm_type of ByteArray(_, alignment):
                return alignment
        else:
            fail("Internal error: this is a function, not an object")
    else:
        fail("Internal error: not found")
```

# CodeGen

```
// The else branch already handles the case of type void
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong or Pointer:
        return asm_type.Quadword
    else if type is Char or SChar or UChar:
        return asm_type.Byte
    else if type is Double:
        return asm_type.Double
    else if type is Array:
        return ByteArray(
            size=TypeUtils.get_size(type),
            alignment=TypeUtils.get_alignment(type)
        )
    else:
        fail("Internal error: converting type to assembly")
```

```
// Update the end of the function to retrieve result or not
convert_function_call(TACKY.FunCall fun_call)
    int_reg_args, dbl_reg_args, stack_args = classify_parameters(fun_call.args)

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
	for arg_t, arg in int_reg_args:
		r = int_param_passing_regs[reg_idx]
        insts.append(Mov(arg_t, arg, Reg(r)))
		reg_idx += 1

    // pass args in registers
    reg_idx = 0
	for arg in dbl_reg_args:
		r = dbl_param_passing_regs[reg_idx]
        insts.append(Mov(Double, arg, Reg(r)))
		reg_idx += 1

   // pass args on the stack
    for arg_t, arg in reverse(stack_args):
		if arg is a Reg or Imm operand:
			insts.append(Push(arg))
		else:
            if arg_t is Quadword || Double:
                insts.append(Push(arg))
            else:
                // copy into a register before pushing
                insts.append(Mov(arg_t, arg, Reg(AX)))
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
    retrieve_result = []
    if fun_call.dst is not null:
        assembly_dst = convert_val(fun_call.dst)
        return_reg = null
        if asm_type(dst) == Double:
            return_reg = XMM0
        else:
            return_reg = AX
        retrieve_result = Mov(asm_type(fun_call.dst), Reg(return_reg), assembly_dst)
    else:
        retrieve_result = []

    return [...insts, ...retrieve_result]
```

```
// Converting Tacky.Return is split into 2 cases of having value or not.
convert_instruction(Tacky.Instruction inst):
    match inst type:
        case Copy:
            t = asm_type(inst.src)
            asm_src = convert_val(inst.src)
            asm_dst = convert_dst(inst.dst)

            return [Mov(t, asm_src, asm_dst)]

        case Return:
            if inst.val is not null:
                t = asm_type(inst.val)
                asm_val = convert_val(inst.val)
                ret_reg = t == Assembly.Double ? XMM0 : AX
                return [
                    Assembly.Mov(t, asm_val, Reg(ret_reg)),
                    Assembly.Ret
                ]
            else:
                return [ Assembly.Ret ]
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
                            // NOTE: only lower byte of cx is used.
                            return [
                                Mov(asm_t, asm_src1, asm_dst),
                                Mov(Byte, asm_src2, Reg(CX)),
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

            return [ Movsx(asm_type(src), asm_type(dst), asm_src, asm_dst) ]
        case Truncate:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ Mov(asm_type(dst), asm_src, asm_dst) ]
        case ZeroExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ MovZeroExtend(asm_type(src), asm_type(dst), asm_src, asm_dst) ]
        case IntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(src)

            if t is Byte:
                return [
                    Movsx(Byte, Longword, asm_src, Reg(R9)),
                    Cvtsi2sd(Longword, Reg(R9), asm_dst)
                ]

            return [ Cvtsi2sd(t, asm_src, asm_dst) ]
        case DoubleToInt:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(asm_dst)

            if t is Byte:
                return [
                    Cvttsd2si(Longword, asm_src, Reg(R9)),
                    Mov(Byte, Reg(R9), asm_dst)
                ]

            return [
                Cvttsd2si(t, asm_src, asm_dst)
            ]
        case UIntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            if tacky_type(src) == Types.UChar:
                return [
                    MovZeroExtend(Byte, Longword, asm_src, Reg(R9)),
                    Cvtsi2sd(Longword, Reg(R9), asm_dst)
                ]
            else if tacky_type(src) == Types.UInt:
                return [
                    MovZeroExtend(Longword, Quadword, asm_src, Reg(R9)),
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

            if tacky_type(dst) == Types.UChar:
                return [
                    Cvttsd2si(Longword, asm_src, Reg(R9)),
                    Mov(Byte, Reg(R9), asm_dst)
                ]
            else if tacky_type(dst) == Types.UInt:
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
        case CopyToOffset:
            return [
                Mov(asm_type(inst.src), convert_val(inst.src), PseudoMem(inst.dst, inst.offset))
            ]
        case AddPtr:
            if inst.index is Constant(ConstLong(c)):
                // note that typechecker converts index to long
                // QUESTION: what's the largest offset we should support?
                i = to_int64(c)
                return [
                    Mov(Quadword, convert_val(inst.ptr), Reg(R9)),
                    Lea(Memory(Reg(R9), i*inst.scale), convert_val(inst.dst)),
                ]
            else:
                if inst.scale is 1, or 2, or 4, or 8:
                    return [
                        Mov(Quadword, convert_val(inst.ptr), Reg(R8)),
                        Mov(Quadword, convert_val(inst.index), Reg(R9)),
                        Lea(Indexed(base=Reg(R8), index=Reg(R9), scale), convert_val(dst)),
                    ]
                else:
                    return [
                        Mov(Quadword, convert_val(inst.ptr), Reg(R8)),
                        Mov(Quadword, convert_val(inst.index), Reg(R9)),
                        Binary(op=Mult, type=Quadword, src=Imm(to_in64(scale), dst=Reg(R9))),
                        Lea(Indexed(base=Reg(R8), index=Reg(R9), scale=1), convert_val(dst)),
                    ]
```

# ReplacePseudo

_No changes_

# Instruction Fixup

_No changes_

# Emit

_No changes_

# Output

From C:

```C

```

To x64 Assembly on Linux:

```asm

```
