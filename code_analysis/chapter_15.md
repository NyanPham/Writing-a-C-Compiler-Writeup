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
| OpenBracket | [ |
| CloseBracket | ] |

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
type Array {
    elem_type: t,
    size: int,
}
type FunType {
    param_types: t[],
    ret_type: t,
}

t = Int | Long | UInt | ULong | Double | Pointer | Array | FunType
```

# Const

_No changes_

# ConstConvert

We cannot cast a constant to an array.

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
        default: // targetType is funType or Array
            fail("Internal error: cannot cast constant to non-scalar type")
```

```
// No changes
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

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer* list)
type = Int | Long | UInt | ULong | Double
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
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
```

# Parser

```
type declarator =
    | Ident(identifier)
    | PointerDeclarator(declarator)
    | ArrayDeclarator(declarator, Constant.Const)
    | FunDeclarator(param_info* params, declarator)
```

```
/* { "[" <const> "]" }+ */
parse_array_dimensions(tokens):
    dimensions = []

    while peek(tokens) == "[":
        take(tokens)
        dim = parse_constant(tokens)
        expect("]", tokens)
        dimensions.append(dim)

    return dimensions
```

```
parse_direct_declarator(tokens):
    simple_decl = parse_simple_declarator(tokens)
    if peek(tokens) is "[":
        array_dimensions = parse_array_dimensions(tokens)
        for dim in array_dimensions:
            simple_dec = ArrayDeclarator(simple_dec, dim)
        return simple_dec
    else if peek(tokens) is "(":
        params = parse_param_list(tokens):
        return FunDeclarator(params, simple_decl)
    else:
        return simple_decl
```

```
// Convert constant to int and check that it's a valid array dimension: must be an integer > 0
const_to_dim(c):
    i;
    match c:
        case ConstInt(i): i = i
        case ConstLong(l): i = l
        case ConstUInt(u): i = u
        case ConstULong(ul): i = ul
        case ConstDouble: fail("Array dimensions must have integer type")

    if i > 0:
        return i
    else:
        fail("Array dimension must be greater than zero")
```

```
process_declarator(decl, base_type):
    switch decl:
        case Ident(s):
            return (s, base_type, [])
        case PointerDeclarator(d):
            derived_type = Types.Pointer(base_type)
            return process_declarator(d, derived_type)
        case ArrayDeclarator(inner, cnst):
            size = const_to_dim(cnst)
            derived_type = Types.Array(base_type, size)
            return process_declarator(inner, derived_type)
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

```
type abstract_declarator =
    | AbstractPointer(abstract_declarator)
    | AbstractArray(abstract_declarator, Constant.Const)
    | AbstractBase
```

```
parse_abstract_declarator(tokens):
    if peek(tokens) is "*":
        // it's a pointer declarator
        take_token(tokens)

        next_tok = peek(tokens)
        if next_tok is "*" or "(" or "[":
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
/*
<direct-abstract-declarator ::= "(" <abstract-declarator> ")" { "[" <const> "]" }
        | { "[" <const> "]" }+
/*
parse_direct_abstract_declarator(tokens):
    switch peek(tokens):
        case "(":
            take(tokens)
            abstr_decl = parse_abstract_declarator(tokens)
            expect(")", tokens)
            // inner declarator is followed by possibly-empty list of aray dimension
            array_dimensions = parse_array_dimensions(tokens)
            for dim in array_dimensions:
                abstr_decl = AbstractArray(abstr_decl, dim)

            return abstr_decl
        case "[":
            array_dimensions = parse_array_dimensions(tokens)
            abstr_decl = AbstractBase()
            for dim in array_dimensions:
                abstr_decl = AbstractArray(abstr_decl, dim)
            retun abstr_decl
        default:
            raise_error("an abstract direct delclarator", peek(tokens))
```

```
process_abstract_declarator(decl, base_type):
    switch decl:
        case AbstractCase: base_type
        case AbstractArray(inner, cnst):
            dim = const_to_dim(cnst)
            dervived_type = Types.Array(base_type, dim)
            return process_abstract_declarator(inner, derived_type)
        case AbstractPointer(inner):
            derived_type = Types.Pointer(base_type)
            return process_abstract_declarator(inner, derived_type)
```

```
parse_postfix_helper(primary, tokens):
	next_token = peek(tokens)
	if next_token is "--":
		decr_exp = PostfixDecr(primary)
		return parse_postfix_helper(decr_exp, tokens)
	else if next_token is "++":
		incr_exp = PostfixDecr(primary)
		return parse_postfix_helper(incr_exp, tokens)
	else if next_token is "[":
        take(tokens)
        index = parse_exp(tokens, 0)
        expect("]", tokens)
        subscript_exp = Subscript(primary, index)
        return parse_postfix_helper(subscript_exp, tokens)
    else:
		return primary
```

```
// <initializer> ::= <exp> | "{" <initializer> { "," <initializer> } [ "," ] "}"
parse_initializer(tokens):
    if peek(tokens) is "{":
        take(tokens)
        init_list = parse_init_list(tokens)
        expect("}", tokens)
        return CompoundInit(init_list)
    else:
        e = parse_exp(tokens, 0)
        return SingleInit(e)
```

```
parse_init_list(tokens):
    init_list = []

    while true:
        next_init = parse_initializer(tokens)
        init_list.append(next_init)

        next_two_tokens = npeek(tokens, 2)

        if next_two_tokens[0] is "," and next_two_tokens[1] is "}":
            take(tokens)
            break
        else if  next_two_tokens[0] is ",":
            take(tokens)
        else:
            break

    return init_list
```

```
/*
<variable-declaration> ::= { <specifier> }+ <declarator> [ "=" <initializer> ] ";"
   we've already parsed { <specifier> }+ <declarator>
*/
// Actually, no changes
finish_parsing_variable_declaration(name, storage_class, var_type, tokens):
    next_token = take_token(tokens)
    if next_token is ";":
        return (name, init=null, var_type, storage_class)
    else if next_token is "=":
        init = parse_initializer(tokens)
        expect(";", tokens)
        return (name, init, var_type, storage_class)
    else:
        raise_error("An initializer or semicolon", next_token type)
```

# Initializers

We have the new ZeroInit type, and use it in the zero and is_zero functions.

```
type static_init =
    | IntInit(int32 v)
    | LongInit(int64 v)
    | UIntInit(uint32 v)
    | ULongInit(uint64 v)
    | DoubleInit(double v)
    // zero out arbitrary number of bytes
    | ZeroInit(int v)
```

```
zero(Types.t type):
   return [ ZeroInit(TypeUtils.get_size(type)) ]
```

```
is_zero(static_init):
    switch static_init:
        case IntInit(i)
            return i == 0 (in 32-bit)
        case LongInit(l):
            return l == 0 (in 64-bit)
        case UIntInit(u):
            return u == 0 (in 32-bit)
        case ULongInit(ul):
            return u == 0 (in 64-bit)
        // Note: consider all double non-zero since we don't know if it's zero or negative-zero
        case DoubleInit(d):
            return false
       ccase ZeroInit:
            return true
```

# Identifier Resolution

In resolve_exp, we add the case for Subscript expressions.

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
resolve_initializer(init, id_map):
    if init is SingleInit(e):
        return SingleInit(resolve_exp(e, id_map))
    else if init is CompoundInit(inits):
        resolved_inits = []
        for init in inits:
            resolved_inits.append(resolve_initializer(init, id_map))
        return CompoundInit(resolved_inits)
```

```
// Use resolve_initializer for var declarations
resolve_local_var_declaration(id_map, var_decl):
    new_map, unique_name = resolve_local_var_helper(id_map, var_decl.name, var_decl.storage_class)

    resolved_init = null
    if var_decl has init:
        resolved_init = resolve_initializer(var_decl.init, id_map)

    return (new_map, (name = unique_name, init=resolved_init, var_decl.var_type, var_decl.storage_class))
```

# ValidateLabels

_No Changes_

# LoopLabeling

_No Changes_

# CollectSwitchCases

_No Changes_

# TypeUtils

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
        case Types.Array:
            return type.size * get_size(type.elem_type)
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
        case Types.Array:
            return get_alignment(type.elem_type)
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
            fail("Internal error: signedness doesn't make sense for function, double type, and array type")
```

Some new helpers to check the type

```
is_pointer(type):
    return type is Pointer
```

```
is_integer(type):
    switch (type):
        case Int:
        case UInt:
        case Long:
        case ULong:
            return true
        default:
            return false
```

```
is_array(type):
    return type is Array
```

```
is_arithmetic(type):
    switch (type):
        case Int:
        case UInt:
        case Long:
        case ULong:
        case Double:
            return true
        default:
            return false
```

# Symbols

Initial has a list of static_init instead of a single static_init as before.

```
type initial_value =
    | Tentative
    | Initial(Initializers.static_int* inits)
    | NoInitializer
```

# TypeCheck

The helper functions are moved to the TypeUtils:

- is_pointer
- is_arithmetic
- is_integer

```
// an lvalue is Var, or Dereference, or Subscript
is_lvalue(AST.Expression e):
    return e == Var or e == Dereference or e == Subscript
```

```
// NEW: this is used in is_null_pointer_constant and to initialize static inits
is_zero_int(cnst):
    if cnst is ConstInt(i):
        return i == 0
    if cnst is ConstLong(l):
        return l == 0
    if cnst is ConstUInt(u):
        return u == 0
    if cnst is ConstULong(ul):
        return ul == 0

    return false
```

```
is_null_pointer_constant(exp):
    if exp is AST.Const(c):
        return is_zero_int(c)
    else:
        return false
```

Checking types of binary expressions is now more complicated,
as it involve pointer arithmetic rules.
So we split the checking for addition, subtraction, multiplicative, equality and comparisons.
Also, we add a new case for type check subscript expressions.

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
```

For the rest of changes, we check if the type to cast to is Array as an invalid cast, and use typecheck_and_convert instead of typecheck_exp.
Checking the lvalue is different now, we typecheck the lhs first, before calling the is_lvalue.

```
typecheck_cast(target_type, inner):
    if target_type is Array:
        fail("Cannot cast to array type")
    else:
        typed_inner = typecheck_and_convert(inner)
        if
            (target_type is Pointer and typed_inner.type is Double)
            OR
            (target_type is Double and typed_inner.type is Pointer):
                fail("Cannot cast between pointer and double")
        else:
            cast_exp = Cast(target_type, typecheck_and_convert(inner))
            return set_type(cast_exp, target_type)
```

```
typecheck_not(inner):
    typed_inner = typecheck_and_convert(inner)
    not_exp = Unary(Not, typed_inner)
    return set_type(not_exp, Types.Int)
```

```
typecheck_complement(inner):
    typed_inner = typecheck_and_convert(inner)
    if typed_inner.type is Double OR is_pointer(typed_inner.type):
        fail("Bitwise complement only valid for integer types")
    else:
        complement_exp = Unary(Complement, typed_inner)
        return set_type(commplement_exp, get_type(typed_inner))
```

```
typecheck_negate(inner):
    typed_inner = typecheck_and_convert(inner)
    if is_pointer(inner.type):
        fail("Can't negate a pointer")
    else:
        negate_exp = Unary(Negate, typed_inner)
        return set_type(negate_exp, get_type(typed_inner))
```

```
typecheck_incr(op, inner):
    typed_inner = typecheck_and_convert(inner)
    if is_lvalue(typed_inner):
        typed_exp = Unary(op, typed_inner)
        return set_type(typed_exp, get_type(typed_inner))
    else:
        fail("Operand of ++/-- must be an lvalue")
```

```
typecheck_postfix_decr(e):
    typed_e = typecheck_and_convert(e)
    if is_lvalue(typed_e):
        // result has same value as e; no conversions required.
        // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far
        result_type = get_type(typed_e)
        return set_type(PostfixDecr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue")
```

```
typecheck_postfix_incr(e):
    typed_e = typecheck_and_convert(e)
    if is_lvalue(typed_e):
        // Same deal as postfix decrement
        result_type = get_type(typed_e)
        return set_type(PostfixIncr(typed_e), result_type)
    else:
        fail("Operand of postfix ++/-- must be an lvalue")
```

```
typecheck_logical(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)
    typed_binexp = Binary(op, typed_e1, typed_e2)
    return set_type(typed_binexp, Types.Int)
```

The typecheck_arithmetic is splitted into:

- typecheck_addition
- typecheck_subtraction
- typecheck_multiplicative
- typecheck_bitwise

```
typecheck_addition(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1) and is_arithmetic(typed_e2):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        add_exp = Binary(Add, converted_e1, converted_e2)
        return set_type(add_exp, common_type)
    else if is_pointer(typed_e1.type) and is_integer(typed_e2.type):
        converted_e2 = convert_to(typed_e2, Types.Long)
        add_exp = Binary(Add, typed_e1, converted_e2)
        return set_type(add_exp, typed_e1.type)
    else if is_pointer(typed_e2.type) and is_integer(typed_e1.type):
        converted_e1 = convert_to(typed_e1, Types.Long)
        add_exp = Binary(Add, converted_e1, typed_e2)
        return set_type(add_exp, typed_e2.type)
    else:
        fail("invalid operands for addition")
```

```
typecheck_subtraction(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1) and is_arithmetic(typed_e2):
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        sub_exp = Binary(Subtract, converted_e1, converted_e2)
        return set_type(sub_exp, common_type)
    else if is_pointer(typed_e1.type) and is_integer(typed_e2.type):
        converted_e2 = convert_to(typed_e2, Types.Long)
        sub_exp = Binary(Subtract, typed_e1, converted_e2)
        return set_type(sub_exp, typed_e1.type)
    else if is_pointer(typed_e1.type) and typed_e1.type == typed_e2.type:
        sub_exp = Binary(Subtract, typed_e1, typed_e2)
        return set_type(sub_exp,  Types.Long)
    else:
        fail("invalid operands for subtraction")
```

```
typecheck_multiplicative(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_pointer(typed_e1.type) or is_pointer(typed_e2.type):
        fail("multiplicative operations not permitted on pointers")
    else:
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
```

```
typecheck_bitwise(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if NOT (is_integer(get_type(typed_e1)) AND is_integer(get_type(typed_e2))):
        fail("Both operands of bitwise opeation must be integers")
    else:
        common_type = get_common_type(typed_e1.type, typed_e2.type)
        converted_e1 = convert_to(typed_e1, common_type)
        converted_e2 = convert_to(typed_e2, common_type)
        bin_exp = Binary(op, converted_e1, converted_e2)
        return set_type(bin_exp, common_type)
```

And the typecheck_comparison stops handling Equal and NotEqual as they are checked in typecheck_equality.

```
// Equal and NotEqual support pointer with null pointer constants.
typecheck_equality(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_pointer(typed_e1.type) or is_pointer(typed_e2.type):
        common_type = get_common_pointer_type(typed_e1, typed_e2)
    else:
        common_type = get_common_type(typed_e1.type, typed_e2.type)

    converted_e1 = convert_to(typed_e1, common_type)
    converted_e2 = convert_to(typed_e2, common_type)
    bin_exp = Binary(op, converted_e1, converted_e2)
    return set_type(bin_exp, Int)
```

```
// We don't support null pointer constants in <, <=, >, >=
typecheck_comparison(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if is_arithmetic(typed_e1.type) and is_arithmetic(typed_e2.type)
        common_type = get_common_type(typed_e1.type, typed_e2.type)
    else if is_pointer(typed_e1.type) and typed_e1.type == typed_e2.type:
        common_type = typed_e1.type
    else:
        fail("invalid types for comparisons")

    converted_e1 = convert_to(typed_e1, common_type)
    converted_e2 = convert_to(typed_e2, common_type)
    binary_exp = Binary(op, converted_e1, converted_e2)
    return set_type(binary_exp, Types.Int)
```

```
typecheck_bitshift(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    if NOT (is_integer(get_type(typed_e1)) AND is_integer(get_type(typed_e2))):
        fail("Both operands of bit shift operation must be an integer")
    else:
        // Don't perform usual arithmetic conversions; result has type of left operand
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, get_type(typed_e1))
```

Phew, we've done with the binary expressions.
Let's move on.

```
typecheck_assignment(lhs, rhs):
    typed_lhs = typecheck_and_convert(lhs)
    if is_lvalue(typed_lhs):
        lhs_type = get_type(typed_lhs)
        typed_rhs = typecheck_and_convert(rhs)
        converted_rhs = convert_by_assignment(typed_rhs, lhs_type)
        assign_exp = Assignment(typed_lsh, converted_rhs)
        return set_type(assign_exp, lhs_type)
    else:
        fail("left hand side of assignment is invalid lvalue")
```

```
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
        if op is in (Multiply, Divide) AND (is_pointer(lhs_type) OR is_pointer(rhs_type)):
            fail("Operand {op} does not support pointer operands")

        // += and -= require either two arithmetic operators, or pointer on LHS and integer on RHS
        if op is (Add or Subtract)
            AND NOT(
                (is_arithmetic(lhs_type) and is_arithmetic(rhs_type))
                OR (is_pointer(lhs_type) and is_integer(rhs_type))
            ):
            fail("invalid types for +=/-=")

        result_t, converted_rhs
        if op is BitShiftLeft or BitShiftRight:
            // Don't perform type conversion for >>= and <<=
            result_t = lhs_type
            converted_rhs = typed_rhs
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
typecheck_conditional(condition, then_exp, else_exp):
    typed_condition = typecheck_and_convert(condition)
    typed_then = typecheck_and_convert(then_exp)
    typed_else = typecheck_and_convert(else_exp)

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
            new_arg = convert_by_assignment(typecheck_and_convert(arg), param_type)
            converted_args.append(new_arg)

        call_exp = FunctionCall(f, converted_args)
        return set_type(call_exp, ret_type)
    else:
        fail("Tried to use variable as function name")
```

```
typecheck_dereference(inner):
    typed_inner = typecheck_and_convert(inner)

    if get_type(typed_inner) is Pointer(referenced_t):
        deref_exp = Dereference(typed_inner)
        return set_type(deref_exp, referenced_t)
    else:
        fail("Tried to dereference non-pointer")
```

```
typecheck_addr_of(inner):
    typed_inner = typecheck_exp(inner)
    if is_lvalue(typed_inner):
        inner_t = get_type(typed_inner)
        addr_exp = AddrOf(typed_inner)
        return set_type(addr_exp, Pointer(inner_t))
    else:
        fail("Cannot take address of non-lvalue")
```

```
// NEW
typecheck_subscript(e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    ptr_type, converted_e1, converted_e2

    if is_pointer(typed_e1.type) and is_integer(typed_e2.type):
        ptr_type = typed_e1.type
        converted_e1 = typed_e1
        converted_e2 = convert_to(typed_e2, Types.Long)
    else if is_pointer(typed_e2.type) and is_integer(typed_e1.type):
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
typecheck_and_convert(e):
    typed_e = typcheck_exp(e)

    if typed_e.type is Types.Array(elem_type):
        addr_exp = AddrOf(typed_e)
        return set_type(addr_exp, Pointer(elem_type))
    else:
        return typed_e
```

```
// NEW
static_init_helper(var_type, init):
    if var_type is Array and init is SingleInit:
        fail("Can't initialize array from scalar value")
    else if init is SingleInit(exp) and exp is Constant(c) and is_zero_int(c):
        return Initializers.zero(var_type)
    else if var_type is Pointer:
        fail("invalid static initializer for pointer")
    else if init is SingleInit(exp):
        if exp is Constant(c):
            converted_c = ConstConvert.convert(var_type, c)
            switch (converted_c);:
                case ConstInt(i):       init_val = Initializers.IntInit(i)
                case ConstLong(l):      init_val = Initializers.LongInit(l)
                case ConstUInt(ui):     init_val = Initializers.UIntInit(ui)
                case ConstULong(ul):    init_val = Initializers.ULongInit(ul)
                case ConstDouble(d):    init_val = Initializers.DoubleInit(d)
                default: fail("invalid static initializer")

            return [ init_val ]
        else:
            fail("non-constant initializer")
    else if var_type is Array(element, size) and init is CompoundInit(inits):
        static_inits = []
        for init in inits:
            static_inits.append(static_init_helper(elem_type, init))

        n = size - len(inits)

        if n == 0:
            padding = []
        if n > 0:
            zero_bytes = get_size(elem_type) * n
            padding = [ Initializers.ZeroInit(zero_bytes) ]
        else:
            fail("Too many values in static initializer")

        static_inits.append(...padding)

        return static_inits
    else if init is CompoundInit:
        fail("Can't use compound initialzier for object with scalar type")
```

```
to_static_init(var_type, init):
    init_list = static_init_helper(var_type, init)
    return Symbols.Initial(init_list)
```

```
// New
make_zero_init(type):
    scalar = (Constant.Const c) -> return SingleInit(AST.Constant(c), type)

    switch type:
        case Array(elem_type, size):
            return CompoundInit([make_zero_init(elem_type)] * size, type)
        case Int:
            return scalar(Constant.ConstInt(0))
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
            fail("Internal error: can't create zero initializer with function type")
```

```
// NEW
typecheck_init(target_type, init):
    if init is SingleInit(e):
        typedchecked_e = typecheck_and_convert(e)
        cast_exp = convert_by_assignment(typechecked_e, target_type)
        return SingleInit(cast_exp)
    else if var_type is Array(elem_type, size) and init is CompoundInit(inits):
        if inits > size:
            fail("Too many values in initializer")
        else:
            typechecked_inits = []
            for init in inits:
                typechecked_inits.append(typecheck_init(elem_type, init))
            padding = [make_zero_init(elem_type)] * (size - len(inits))
            return CompoundInit(target_type, [...typechecked_inits, ...padding])
    else:
        fail("Can't initialize scalar value from compound initializer")
```

```
typecheck_statement(stmt, ret_type):
    match stmt type:
    case Return(e):
        typed_e = typecheck_and_convert(e)
        return Return(convert_by_assignment(typed_e, ret_type))
    case Expression(e):
        return Expression(typecheck_and_convert(e))
    case If(condition, then_clause, else_clause):
        return If(
            condition=typecheck_and_convert(condition),
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
            condition=typecheck_and_convert(condition),
            body=typecheck_statement(body, ret_type),
            id
        )
    case DoWhile(body, condition, id):
        return DoWhile(
            body=typecheck_statement(body),
            condition=typecheck_and_convert(condition),
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
            condition = condition is not null ? typecheck_and_convert(condition) : null
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
// use typecheck_init instead of typecheck_exp
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
            new_init = typecheck_init(var_decl.var_type, var_decl.init)

        return var_declaration(var_decl.name, init=new_init, storage_class=null, t=var_decl.var_type, )
```

```
typecheck_fn_decl(fn_decl):
    param_ts = []
    return_t, fun_type

    if fn_decl.fun_type == FunType:
        if fn_decl.fun_type.ret_type == Array:
            fail("A function cannot return an array")
        else:
            for param_t in fn_decl.fun_type.param_types:
                if param_t is Array(elem_type):
                    param_ts.append(Pointer(elem_type))
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

# TACKY

StaticVariable recieves a list of static_init.
New instructions AddPtr and CopyToOffset are added.

```
program = Program(top_level*)
top_level =
    | Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init* inits)
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
    | Addptr(val ptr, val index, int scale, val dst)
    | CopyToOffset(val src, identifier dst, int offset)
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
get_ptr_scale(type):
    if type is Pointer(referenced):
        return TypeUtils.get_size(referenced)
    else:
        fail("Internal error: tried to get scale of non-pointer type")
```

We update the Unary increment and decrement to treat pointers as long type.
To emit binary, if we add or subtract with pointer operands, we have: emit_pointer_addition, emit_subtraction_from_pointer, emit_pointer_diff
We add a new case to emit Subscript expresions.

```
emit_tacky_for_exp(exp):
	match exp.type:
		case AST.Constant(c):
            return ([], PlainOperand(TACKY.Constant(c)))
		case AST.Var(v):
            return ([], PlainOperand(TACKY.Var(v)))
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
```

```
emit_postfix(op, inner):
    /*  If LHS is a variable:
            dst = lhs
            lhs = lhs <op> 1
        If LHS is a pointer:
            dst = load(ptr)
            tmp = dst <op> 1
            store(tmp, ptr)

        If LHS has a pointer type, we implement <op> with AddPtr
        otherwise with Binary instruction
    */

    // define var for result - i.e. value of lval BEFORE incr or decr
	dst = TACKY.Var(create_tmp(inner.t))

    // evaluate inner to get exp_result
    insts, lval = emit_tacky_for_exp(inner)
    /*  Helper to construct Binary or AddPtr instruction from operands, depending on type
        Note that dst is destination of this instruction rather than the whole expression
        (i.e. it's the lvalue we're updating, or a temporary we'll then store to that lvalue)
    */

    do_op(op, src, dst):
        if op is Add:
            index = TACKY.Constant(mk_const(Long, 1))
        else if op is Subtract:
            index = TACKY.Constant(mk_const(Long, -1))
        else:
            fail("Internal error")

        if TypeUtils.is_pointer(inner.type):
            return Addptr(src, index, get_ptr_scale(inner.type), dst)
        else:
            one = TACKY.Constant(mk_const(inner.type, 1))
            return Binary(convert_binop(op), src1=src, src2=one, dst)

    // copy result to dst and perform incr or decr
    if lval is PlainOperand(Var(name)):
        /*
            dst = v
            v = v + 1 // v - 1
        */
        oper_insts = [
            Copy(Var(name), dst),
            do_op(op, Var(name), Var(name)),
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
            do_op(op, dst, tmp)
            Store(tmp, dst_ptr=p),
        ]

	return ([...insts, ...oper_insts], PlainOperand(dst))
```

```
// NEW
emit_pointer_addition(type, e1, e2):
    eval_v1, v1 = emit_tacky_and_convert(e1)
    eval_v2, v2 = emit_tacky_and_convert(e2)
    dst_name = create_tmp(type)
    dst = Var(dst_name)
    ptr, index =
        if type == e1.type
        then (v1, v2)
        else (v2, v1)
    scale = get_ptr_scale(type)
    insts = [
        ...eval_v1,
        ...eval_v2,
        AddPtr(ptr, index, scale, dst)
    ]

    return (insts, PlainOperand(dst))
```

```
// NEW
emit_subtraction_from_pointer(type, ptr_e, idx_e):
    eval_v1, ptr = emit_tacky_and_convert(ptr_e)
    eval_v2, index = emit_tacky_and_convert(idx_e)
    dst_name = create_tmp(type)
    dst = Var(dst_name)
    negated_index = Var(create_tmp(Types.Long))
    scale = get_ptr_scale(type)

    insts = [
        ...eval_v1,
        ...eval_v2,
        Unary(Negate, src=index, dst=negated_index),
        AddPtr(ptr, negated_index, scale, dst),
    ]

    return (insts, PlainOperand(dst))
```

```
// NEW
emit_pointer_diff(type, e1, e2):
    eval_v1, v1 = emit_tacky_and_convert(e1)
    eval_v2, v2 = emit_tacky_and_convert(e2)
    ptr_diff = Var(create_tmp(Types.Long))
    dst_name = create_tmp(type)
    dst = Var(dst_name)
    scale = Constant(ConstLong(to_int64(get_ptr_scale(e1.type))))

    insts = [
        ...eval_v1,
        ...eval_v2,
        Binary(Subtract, v1, v2, ptr_diff),
        Binary(Divide, ptr_diff, scale, dst)
    ]

    return (insts, PlainOperand(dst))
```

```
// NEW
emit_subscript(type, e1, e2):
    insts, result = emit_pointer_addition(Type.Pointer(type), e1, e2)

    if result is PlainOperand(dst):
        return (insts, DereferencedPointer(dst))
    else:
        fail("Internal error: expected result of pointer addition to be lvalue converted")
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

    do_operation():
        if TypeUtils.is_pointer(result_t):
            scale = get_ptr_scale(result_t)
            if op is Add:
                return [ AddPtr(result_var, rhs, scale, result_var) ]
            else if op is Subtract:
                negated_index = Var(create_tmp(Types.Long))
                return [
                    Unary(Negae, rhs, negated_index),
                    AddPtr(result_var, negated_index, scale, result_var),
                ]
            else: fail("Internal error in compound assignment")
        else:
            return [
                Binary(
                op=convert_binop(op),
                src1=result_var,
                src2=rhs,
                dst=result_var)
            ]

    insts = [
        ...eval_lhs,
        ...eval_rhs,
        ...load_inst,
        ...cast_to,
        ...do_operation(),
        ...cast_from,
        ...store_inst,
    ]

    return (insts, PlainOperand(dst))
```

```
// NEW
emit_compound_init(init, name, offset):
    if init is SingleInit(e):
        eval_init, v = emit_tacky_and_convert(e)
        return [...eval_init, CopyToOffset(src=v, dst=name, offset)]
    else if init is CompoundInit:
        new_inits = []

        if init.type is Array(elem_type):
            for idx, elem_init in init.inits:
                new_ofset = offset + (idx + TypeUtils.get_size(elem_type))
                new_inits.append(...emit_compound_init(elem_init, name, new_offset))

            return new_inits
        else:
            fail("Internal error: compound init has non-array type!")
```

```
emit_var_declaration(var_decl):
    if var_decl has init:
        if init is SingleInit(e)
            // Treat var_decl with initializer as assignment
            lhs = AST.Var(var_decl.name)
            set_type(lhs, var_decl.var_type)
            eval_assignment, assign_result = emit_assignment(lhs, e)
            return eval_assignment
        else if init is CompoundInit:
            return emit_compound_init(init, var_decl.name, 0)
    else:
        // Do not generate any instructions
        return []
```

# Assembly

We add PseudoMem and Indexed.
The asm_type has a new ByteArray type.
StaticVariable receives a list of static_init, not a single init.

```
program = Program(top_level*)
asm_type =
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
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier) | PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

# AssemblySymbols

```
get_size(var_name):
    if symbol_table has var_name:
        if symbol_table[var_name] is Obj:
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
// In case of TACKY.Var has Array type, we convert it to PseudoMem instead of Pseudo
convert_val(Tacky.Val val):
	match val.type:
        case Tacky.Constant(ConstInt(i)):
            return Assembly.Imm((int64) i)
		case Tacky.Constant(ConstLong(l)):
			return Assembly.Imm(l)
        case Tacky.Constant(ConstUInt(u)):
            return Assembly.Imm((uint64) u)
        case Tacky.Constant(ConstULong(ul)):
            return Assembly.Imm((uint64) ul)
        case Tacky.Double(ConstDouble(d)):
            return Assembly.Data(add_constant(d))
		case Tacky.Var:
            if TypeUtils.is_array(symbols.get(val.name)):
                return Assembly.PseudoMem(val.name, 0)
			return Assembly.Pseudo(val.name)
```

```
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong or Pointer:
        return asm_type.Quadword
    else if type is Double:
        return asm_type.Double
    else if type is Array:
        return ByteArray(
            size=TypeUtils.get_size(type),
            alignment=TypeUtils.get_alignment(type)
        )
    else:
        fail("Internal error: converting function type to assembly")
```

```
// extend for CopyToOffset and Addptr
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

```
// NEW
// Special-case logic to get type/alignment of array; array variables w/ size >=16 bytes have alignment of 16
get_var_alignment(type):
    if type is Array and TypeUtils.get_size(type) >= 16:
        return 16
    else:
        return TypeUtils.get_alignment(type)
```

```
// NEW
convert_var_type(type):
    if type is Array:
        return Assembly.ByteArray(size=TypeUtils.get_size(type), alignment=get_var_alignment(type))
    else:
        return convert_type(type)
```

```
// call the get_var_alignment
convert_top_level(Tacky.top_level top_level):
    if top_level is Function:
        params_as_tacky = []
        for param in fun_def.params:
            params_as_tacky.append(Tacky.Var(param))

        params_insts = pass_params(params_as_tacky)
        insts = [...params_insts]

        for inst in fun_def.body:
            insts.append(...convert_instruction(inst))

        return Function(fun_def.name, fun_decl.global, insts)
    else: // top_level is StaticVariable
        return Assembly.StaticVariable(
            name=top_level.name,
            global=top_level.global,
            alignment=get_var_alignment(top_level.t),
            init=top_level.inits
        )
```

```
// call convert_var_type
convert_symbol(name, symbol):
    if symbol.attrs is Types.FunAttr:
        return asmSymbols.add_fun(name, defined)
    else if symbol.attrs is Types.StaticAttr:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), true)
    else:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), false)
```

# ReplacePseudo

```
// NEW
calculate_offset(state, name):
    size = asmSymbols.get_size(name)
    alignment = asmSymbols.get_alignment(name)
    new_offset = Rounding.round_away_from_zero(alignment, state.current_offset - size)
    new_state = {
        current_offset = new_offset,
        offset_map = state.offset_map.add(operand.name, new_offset)
    }

    return (new_state, new_offset)
```

```
// call the calculate_offset in Pseudo case, and add cases for PseudoMem
replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
            if AsmSymbols.is_static(operand.name):
                return (state, Assembly.Data(operand.name))
			else:
                if state.offset_map.has(operand.name):
                    return (state, Assembly.Memory(Reg(BP), state.offset_map.get(operand.name)))
                else:
                    new_state, new_offset = calculate_offset(state, operand.name)
				    return (new_state, Assembly.Memory(Reg(BP), new_offset))
		case PseudoMem:
            if AsmSymbols.is_static(operand.name):
                if operand.offset == 0:
                    return (state, Data(operand.name))
                else:
                    fail("Internal error: shouldn't have static variables with non-zero offset at this point")
            else:
                if state.offset_map.has(operand.name):
                    // We've already assigned this operand a stack slot
                    v = state.offset_map.get(operand.name)
                    return (state, Assembly.Memory(Reg(BP), operand.offset + v.var_offset))
                else:
                    // assign operand name a stack slot, and add its offset to the offset w/tin operand.name to get new operand
                    new_state, new_var_offset = calculate_offset(state, operand.name)
                    return (new_state,  Assembly.Memory(Reg(BP), operand.offset + new_var_offset))
        default:
			return (state, operand)
```

# Instruction Fixup

```
// Indexed operand indicates indexed-addressing mode
is_memory(operand):
    return operand.type is Data or Memory or Indexed
```

# Emit

```
suffix(asm_type type):
    if type is Longword:
        return "l"
    else if type is Quadword:
        return "q"
    else if type is Double:
        return "sd"
    else if type is ByteArray:
        fail("Internal error: found instruction w/ non-scalar operand type")
```

```
show_operand(asm_type, operand):
    if operand is Reg:
        if asm_type is Longword:
            return show_long_reg(operand)
        else if asm_type is Quadword:
            return show_quadword_reg(operand)
        else if asm_type is Double:
            return show_double_reg(operand)
        else: // is ByteArray
            fail("Internal error: can't store non-scalar operand in register")
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
    else if operand is Indexed:
        return "({show_quadword_reg(operand.base)}, {show_quadword_reg(operand.index)}, {operand.scale})"
    else if operand is Pseudo: // For debugging
        return operand.name
    else if operand is PseudoMem:
        return "{operand.offset}(%{operand.name})"
```

```
// Cdq doesn't work on either Double or ByteArray
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
                fail("Internal error: can't apply cdq to non-integer type)
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

We remove the function emit_zero_init, and extend the emit_init to use the ZeroInit construct.

```
emit_init(Initializers.static_init init):
    if init is IntInit(i):
        return "\t.long {to_int32(i)}\n"
    else if init is LongInit(l):
        return "\t.quad {to_int64(l)}\n"
    else if init is UIntInit(u):
        return "\t.long {to_uint32(u)}\n"
    else if init is ULongInit(ul):
        return "\t.quad {to_uint64(ul)}\n"
    else if init is DoubleInit(d):
        return "\t.quad {to_double(d)}\n"

    // a partly-initialized array can include a mix of zero and non-zero initializaers
    else if init is ZeroInit(byte_count):
        return "\t.zero {byte_count}\n"
```

```
// For static variables, check if the whole list of static_init is zero
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
    else if top_level is StaticVariable:
        // is_zero returns False for all doubles
        static_var = top_level as StaticVariable

        if every init in static_var.inits meets Initializers.is_zero(init):
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .bss
                {align_directive()} {static_var.alignment}
            {label}:
                {for init in static_var.inits, emit_init(init)}
            """
        else:
            label = show_label(static_var.name)
            return """
                {emit_global_directive(static_var.global, label)}
                .data
                {align_directive()} {static_var.alignment}
            {label}:
                {for init in static_var.inits, emit_init(init)}
            """
    else: // StaticConstant
        return emit_constant(top_level.name, top_level.alignment, top_level.init)
```

# Output

From C:

```C
int get_call_count(void)
{
    static int count = 0;
    count += 1;
    return count;
}

int i = 2;
int j = 1;
int k = 0;

int main(void)
{
    int arr[3][2][2] = {
        {{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}, {{9, 10}, {11, 12}}};

    if (arr[i][j][k]++ != 11)
    {
        return 1;
    }

    if (arr[i][j][k] != 12)
    {
        return 2;
    }

    if (++arr[--i][j--][++k] != 9)
    {
        return 3;
    }

    arr[1][2][1] &= arr[3][2][1];
    arr[1][3][1] |= arr[1][2][1];
    arr[2][1][3] ^= arr[1][2][1];
    arr[2][1][3] >>= 3;

    double x[5] = {0.0, 1.0, 2.0, 3.0, 4.0};
    double *ptr = x;

    if (*ptr != 1.0)
    {
        return 2;
    }

    if (ptr++ != x + 1)
    {
        return 3;
    }

    double *end_ptr = x + 1;
    end_ptr[1] += 3;
}
```

To x64 Assembly on Linux:

```asm
	.section .rodata
	.align 8
.Ldbl.114:
	.quad 4

	.section .rodata
	.align 8
.Ldbl.113:
	.quad 3

	.section .rodata
	.align 8
.Ldbl.112:
	.quad 2

	.section .rodata
	.align 8
.Ldbl.111:
	.quad 1

	.section .rodata
	.align 8
.Ldbl.110:
	.quad 0

	.global k
	.bss
	.align 4
k:
	.zero 4
	

	.bss
	.align 4
count.0:
	.zero 4
	

	.global j
	.data
	.align 4
j:
	.long 1
	

	.global i
	.data
	.align 4
i:
	.long 2
	

	.global get_call_count
get_call_count:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$0, %rsp
	movl	count.0(%rip), %r10d
	movl	%r10d, count.0(%rip)
	addl	$1, count.0(%rip)
	movl	count.0(%rip), %eax
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
	subq	$3136, %rsp
	movl	$1, -48(%rbp)
	movl	$2, -92(%rbp)
	movl	$3, -136(%rbp)
	movl	$4, -180(%rbp)
	movl	$5, -224(%rbp)
	movl	$6, -268(%rbp)
	movl	$7, -312(%rbp)
	movl	$8, -356(%rbp)
	movl	$9, -400(%rbp)
	movl	$10, -444(%rbp)
	movl	$11, -488(%rbp)
	movl	$12, -532(%rbp)
	leaq	-624(%rbp), %r11
	movq	%r11, -632(%rbp)
	movslq 	i(%rip), %r11
	movq	%r11, -640(%rbp)
	movq	-648(%rbp), %r8
	movq	-656(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -664(%rbp)
	movslq 	j(%rip), %r11
	movq	%r11, -672(%rbp)
	movq	-680(%rbp), %r8
	movq	-688(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -696(%rbp)
	movslq 	k(%rip), %r11
	movq	%r11, -704(%rbp)
	movq	-712(%rbp), %r8
	movq	-720(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -728(%rbp)
	movq	-736(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -740(%rbp)
	movl	-744(%rbp), %r10d
	movl	%r10d, -748(%rbp)
	addl	$1, -752(%rbp)
	movq	-760(%rbp), %r9
	movl	-764(%rbp), %r10d
	movl	%r10d, (%r9)
	movl	$11, %r11d
	cmpl	-768(%rbp), %r11d
	movl	$0, -772(%rbp)
	setne	-776(%rbp)
	cmpl	$0, -780(%rbp)
	je	.Lif_end.5
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.5:
	leaq	-828(%rbp), %r11
	movq	%r11, -840(%rbp)
	movslq 	i(%rip), %r11
	movq	%r11, -848(%rbp)
	movq	-856(%rbp), %r8
	movq	-864(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -872(%rbp)
	movslq 	j(%rip), %r11
	movq	%r11, -880(%rbp)
	movq	-888(%rbp), %r8
	movq	-896(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -904(%rbp)
	movslq 	k(%rip), %r11
	movq	%r11, -912(%rbp)
	movq	-920(%rbp), %r8
	movq	-928(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -936(%rbp)
	movq	-944(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -948(%rbp)
	movl	$12, %r11d
	cmpl	-952(%rbp), %r11d
	movl	$0, -956(%rbp)
	setne	-960(%rbp)
	cmpl	$0, -964(%rbp)
	je	.Lif_end.16
	movl	$2, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.16:
	leaq	-1012(%rbp), %r11
	movq	%r11, -1024(%rbp)
	movl	i(%rip), %r10d
	movl	%r10d, i(%rip)
	subl	$1, i(%rip)
	movslq 	i(%rip), %r11
	movq	%r11, -1032(%rbp)
	movq	-1040(%rbp), %r8
	movq	-1048(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -1056(%rbp)
	movl	j(%rip), %r10d
	movl	%r10d, -1060(%rbp)
	movl	j(%rip), %r10d
	movl	%r10d, j(%rip)
	subl	$1, j(%rip)
	movslq 	-1064(%rbp), %r11
	movq	%r11, -1072(%rbp)
	movq	-1080(%rbp), %r8
	movq	-1088(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -1096(%rbp)
	movl	k(%rip), %r10d
	movl	%r10d, k(%rip)
	addl	$1, k(%rip)
	movslq 	k(%rip), %r11
	movq	%r11, -1104(%rbp)
	movq	-1112(%rbp), %r8
	movq	-1120(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -1128(%rbp)
	movq	-1136(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -1140(%rbp)
	movl	-1144(%rbp), %r10d
	movl	%r10d, -1148(%rbp)
	addl	$1, -1152(%rbp)
	movq	-1160(%rbp), %r9
	movl	-1164(%rbp), %r10d
	movl	%r10d, (%r9)
	movl	$9, %r11d
	cmpl	-1168(%rbp), %r11d
	movl	$0, -1172(%rbp)
	setne	-1176(%rbp)
	cmpl	$0, -1180(%rbp)
	je	.Lif_end.26
	movl	$3, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.26:
	leaq	-1228(%rbp), %r11
	movq	%r11, -1240(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1248(%rbp)
	movq	-1256(%rbp), %r8
	movq	-1264(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -1272(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1280(%rbp)
	movq	-1288(%rbp), %r8
	movq	-1296(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -1304(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1312(%rbp)
	movq	-1320(%rbp), %r8
	movq	-1328(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -1336(%rbp)
	leaq	-1384(%rbp), %r11
	movq	%r11, -1392(%rbp)
	movl	$3, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1400(%rbp)
	movq	-1408(%rbp), %r8
	movq	-1416(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -1424(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1432(%rbp)
	movq	-1440(%rbp), %r8
	movq	-1448(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -1456(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1464(%rbp)
	movq	-1472(%rbp), %r8
	movq	-1480(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -1488(%rbp)
	movq	-1496(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -1500(%rbp)
	movq	-1512(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -1516(%rbp)
	movl	-1520(%rbp), %r10d
	movl	%r10d, -1524(%rbp)
	movl	-1528(%rbp), %r10d
	andl	%r10d, -1532(%rbp)
	movq	-1544(%rbp), %r9
	movl	-1548(%rbp), %r10d
	movl	%r10d, (%r9)
	leaq	-1596(%rbp), %r11
	movq	%r11, -1608(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1616(%rbp)
	movq	-1624(%rbp), %r8
	movq	-1632(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -1640(%rbp)
	movl	$3, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1648(%rbp)
	movq	-1656(%rbp), %r8
	movq	-1664(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -1672(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1680(%rbp)
	movq	-1688(%rbp), %r8
	movq	-1696(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -1704(%rbp)
	leaq	-1752(%rbp), %r11
	movq	%r11, -1760(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1768(%rbp)
	movq	-1776(%rbp), %r8
	movq	-1784(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -1792(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1800(%rbp)
	movq	-1808(%rbp), %r8
	movq	-1816(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -1824(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1832(%rbp)
	movq	-1840(%rbp), %r8
	movq	-1848(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -1856(%rbp)
	movq	-1864(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -1868(%rbp)
	movq	-1880(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -1884(%rbp)
	movl	-1888(%rbp), %r10d
	movl	%r10d, -1892(%rbp)
	movl	-1896(%rbp), %r10d
	orl	%r10d, -1900(%rbp)
	movq	-1912(%rbp), %r9
	movl	-1916(%rbp), %r10d
	movl	%r10d, (%r9)
	leaq	-1964(%rbp), %r11
	movq	%r11, -1976(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -1984(%rbp)
	movq	-1992(%rbp), %r8
	movq	-2000(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -2008(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2016(%rbp)
	movq	-2024(%rbp), %r8
	movq	-2032(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -2040(%rbp)
	movl	$3, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2048(%rbp)
	movq	-2056(%rbp), %r8
	movq	-2064(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -2072(%rbp)
	leaq	-2120(%rbp), %r11
	movq	%r11, -2128(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2136(%rbp)
	movq	-2144(%rbp), %r8
	movq	-2152(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -2160(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2168(%rbp)
	movq	-2176(%rbp), %r8
	movq	-2184(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -2192(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2200(%rbp)
	movq	-2208(%rbp), %r8
	movq	-2216(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -2224(%rbp)
	movq	-2232(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -2236(%rbp)
	movq	-2248(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -2252(%rbp)
	movl	-2256(%rbp), %r10d
	movl	%r10d, -2260(%rbp)
	movl	-2264(%rbp), %r10d
	xorl	%r10d, -2268(%rbp)
	movq	-2280(%rbp), %r9
	movl	-2284(%rbp), %r10d
	movl	%r10d, (%r9)
	leaq	-2332(%rbp), %r11
	movq	%r11, -2344(%rbp)
	movl	$2, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2352(%rbp)
	movq	-2360(%rbp), %r8
	movq	-2368(%rbp), %r9
	imulq	$16, %r9
	leaq	(%r8, %r9, 1), %r11
	movq	%r11, -2376(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2384(%rbp)
	movq	-2392(%rbp), %r8
	movq	-2400(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -2408(%rbp)
	movl	$3, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2416(%rbp)
	movq	-2424(%rbp), %r8
	movq	-2432(%rbp), %r9
	leaq	(%r8, %r9, 4), %r11
	movq	%r11, -2440(%rbp)
	movq	-2448(%rbp), %r9
	movl	(%r9), %r10d
	movl	%r10d, -2452(%rbp)
	movl	-2456(%rbp), %r10d
	movl	%r10d, -2460(%rbp)
	sarl	$3, -2464(%rbp)
	movq	-2472(%rbp), %r9
	movl	-2476(%rbp), %r10d
	movl	%r10d, (%r9)
	movsd	.Ldbl.110(%rip), %xmm14
	movsd	%xmm14, -2520(%rbp)
	movsd	.Ldbl.111(%rip), %xmm14
	movsd	%xmm14, -2552(%rbp)
	movsd	.Ldbl.112(%rip), %xmm14
	movsd	%xmm14, -2584(%rbp)
	movsd	.Ldbl.113(%rip), %xmm14
	movsd	%xmm14, -2616(%rbp)
	movsd	.Ldbl.114(%rip), %xmm14
	movsd	%xmm14, -2648(%rbp)
	leaq	-2720(%rbp), %r11
	movq	%r11, -2728(%rbp)
	movq	-2736(%rbp), %r10
	movq	%r10, -2744(%rbp)
	movq	-2752(%rbp), %r9
	movsd	(%r9), %xmm14
	movsd	%xmm14, -2760(%rbp)
	movsd	-2768(%rbp), %xmm15
	comisd	.Ldbl.111(%rip), %xmm15
	movl	$0, -2772(%rbp)
	setne	-2776(%rbp)
	movl	$0, %r9d
	setp	%r9b
	orl	%r9d, -2780(%rbp)
	cmpl	$0, -2784(%rbp)
	je	.Lif_end.94
	movl	$2, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.94:
	movq	-2792(%rbp), %r10
	movq	%r10, -2800(%rbp)
	movq	-2808(%rbp), %r9
	leaq	8(%r9), %r11
	movq	%r11, -2816(%rbp)
	leaq	-2856(%rbp), %r11
	movq	%r11, -2864(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2872(%rbp)
	movq	-2880(%rbp), %r8
	movq	-2888(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -2896(%rbp)
	movq	-2904(%rbp), %r10
	cmpq	%r10, -2912(%rbp)
	movl	$0, -2916(%rbp)
	setne	-2920(%rbp)
	cmpl	$0, -2924(%rbp)
	je	.Lif_end.97
	movl	$3, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.97:
	leaq	-2968(%rbp), %r11
	movq	%r11, -2976(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -2984(%rbp)
	movq	-2992(%rbp), %r8
	movq	-3000(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -3008(%rbp)
	movq	-3016(%rbp), %r10
	movq	%r10, -3024(%rbp)
	movl	$1, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -3032(%rbp)
	movq	-3040(%rbp), %r8
	movq	-3048(%rbp), %r9
	leaq	(%r8, %r9, 8), %r11
	movq	%r11, -3056(%rbp)
	movl	$3, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -3064(%rbp)
	movq	-3072(%rbp), %r9
	movsd	(%r9), %xmm14
	movsd	%xmm14, -3080(%rbp)
	movsd	-3088(%rbp), %xmm14
	movsd	%xmm14, -3096(%rbp)
	movsd	-3112(%rbp), %xmm15
	addsd	-3104(%rbp), %xmm15
	movsd	%xmm15, -3112(%rbp)
	movq	-3120(%rbp), %r9
	movsd	-3128(%rbp), %xmm14
	movsd	%xmm14, (%r9)
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
