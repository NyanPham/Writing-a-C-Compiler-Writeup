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
| KeywordChar | char |
| ConstChar | '([^'\\\n]|\\['"?\\abfnrtv])' |
| StringLiteral | "([^"\\\n]|\\['"\\?abfnrtv])\*" |

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
type Array {
    elem_type: t,
    size: int,
}
type FunType {
    param_types: t[],
    ret_type: t,
}

t = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Pointer | Array | FunType
```

# Const

We add ConstChar and ConstUChar to Const.t, and extend the type_of_const.

```
type t =
    | ConstChar(Int8)
    | ConstUChar(Int8)
    | ConstInt(int32)
    | ConstLong(int64)
    | ConstUInt(int32)
    | ConstULong(int64)
    | ConstDouble(double)

int_zero = ConstInt(0)
long_zero = ConstLong(0)
```

```
type_of_const(const):
    match const
        case ConstChar: return Types.SChar
        case ConstUChar: return Types.UChar
        case ConstInt: return Types.Int
        case ConstLong: return Types.Long
        case ConstUInt: return Types.UInt
        case ConstULong: return Types.ULong
        case ConstDouble: return Types.Double
        default:
            fail("Internal error: unknown const type")
```

# ConstConvert

We cannot cast a constant to an array.

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
        default: // targetType is funType or Array
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

We already update `type` and `const` in different files.
We extend the Expression to have the `String` construct.

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer* list)
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double
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
// NEW
unescape(string s):
    result = []
    i = 0
    length = len(s)

    while i < length:
        if s[i] == '\\':
            if i + 1 >= length:
                fail("Internal error: not a valid escape sequence; should have been rejected during lexing")
            next_char = s[i + 1]
            if next_char == '\'':
                result.append('\'')
            else if next_char == '"':
                result.append('"')
            else if next_char == '?':
                result.append('?')
            else if next_char == '\\':
                result.append('\\')
            else if next_char == 'a':
                result.append(chr(7))
            else if next_char == 'b':
                result.append('\b')
            else if next_char == 'f':
                result.append(chr(12))
            else if next_char == 'n':
                result.append('\n')
            else if next_char == 'r':
                result.append('\r')
            else if next_char == 't':
                result.append('\t')
            else if next_char == 'v':
                result.append(chr(11))
            else:
                fail("Internal error: not a valid escape sequence; should have been rejected during lexing")
            i += 2
        else:
            result.append(s[i])
            i += 1

    return join(result, '')
```

```
is_type_specifier(token):
    switch token type:
        case "int":
        case "long":
        case "double":
        case "signed":
        case "unsigned":
        case "char":
            return true
        default:
            return false
```

```
parse_type(specifier_list):
    /*
        sort specifiers so we don't need to check for different
        orderings of same specifiers
    /*

    specifier_list = sort(specifier_list)

    if specifier_list == ["double]:
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
parse_constant(tokens):
    tok = take_token(tokens)

    if tok is ConstChar:
        s_prime = unescape(tok.value)
        if len(s_prime) == 1:
            return Const.ConstInt(to_int32(s_prime[0]))
        else:
            fail("Internal error: Character token contains multiple characters, lexer should have rejected this")

    if (tok is signed and tok.value > MAX_INT64) OR (tok is unsigned and tok.value > MAX_UINT64):
        fail("Constant is too large to fit in an int or long with given signedness")

    if tok is Token.Double:
        return Const.Double(tok.value)

    if tok is Token.ConstInt:
        if tok.value <= MAX_INT32:
            return Const.ConstInt(cast_to_int_32(tok.value))
        else:
            return Const.ConstLong(cast_to_int_64(tok.value))
    else if tok is Token.ConstLong:
        return Const.ConstLong(cast_to_int_64(tok.value))
    else if tok is Token.ConstUInt:
        if tok.value <= MAX_UINT32 :
            return Const.ConstUInt(cast_to_uint_32(tok.value))
        else:
            return Const.ConstULong(cast_to_uint_64(tok.value))
    else if tok is Token.ConstULong:
        return Const.ConstULong(cast_to_uint_64(tok.value))
    else:
        fail("Internal error: unknown token constant type")
```

```
const_to_dim(c):
    i;
    match c:
        case ConstInt(i): i = i
        case ConstLong(l): i = l
        case ConstUInt(u): i = u
        case ConstULong(ul): i = ul
        case ConstDouble: fail("Array dimensions must have integer type")
        case ConstChar:
        case ConstUChar: fail("Internal error, we're not using these yet")

    if i > 0:
        return i
    else:
        fail("Array dimension must be greater than zero")
```

```
// NEW
parse_string_literals(tokens):
    result = ""
    while peek(tokens) is Token.StringLiteral:
        tok = take(tokens)
        result += unescape(tok.value)

    return result
```

```
// Check case of ConstChar and StringLiteral
parse_primary_exp(tokens):
	next_token = peek(tokens)
	if next_token is Token.ConstChar
        or Token.ConstInt
        or Token.ConstLong
        or Token.ConstUInt
        or Token.ConstULong
        or Token.ConstDouble:
		return parse_constant(tokens)
	if next_token is an identifier:
        --snip--
    if next_token is StringLiteral:
        string_exp = parse_string_literals(tokens)
        return AST.String(string_exp)
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

# Initializers

```
type static_init =
    | CharInit(int8)
    | UCharInit(int8)
    | IntInit(int32 v)
    | LongInit(int64 v)
    | UIntInit(uint32 v)
    | ULongInit(uint64 v)
    | DoubleInit(double v)
    // zero out arbitrary number of bytes
    | ZeroInit(int v)
    | StringInit(string, bool null_terminated)
    | PointerInit(string name) // pointer to static variable
```

```
zero(Types.t type):
   return [ ZeroInit(TypeUtils.get_size(type)) ]
```

```
is_zero(static_init):
    switch static_init:
        case CharInit(c):
            return c == 0 (in 8-bit)
        case IntInit(i):
            return i == 0 (in 32-bit)
        case LongInit(l):
            return l == 0 (in 64-bit)
        case UCharInit(uc):
            return uc == 0 (in 8-bit)
        case UIntInit(u):
            return u == 0 (in 32-bit)
        case ULongInit(ul):
            return u == 0 (in 64-bit)
        // Note: consider all double non-zero since we don't know if it's zero or negative-zero
        case DoubleInit(d):
            return false
        case ZeroInit:
            return true
        case PointerInit:
        case StringInit:
            return false
```

# Identifier Resolution

Beside Constant, String expressions are the terminal nodes, so we return it instead of checking internal contents.

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
        case String:
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

Some new helpers to check the type

```
is_pointer(type):
    return type is Pointer
```

```
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
is_array(type):
    return type is Array
```

```
// NEW
is_character(type):
    return get_size(type) == 1
```

```
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

# Symbols

```
type identifier_attrs =
    | FunAttr(bool defined, bool global)
    | StaticAttr(initial_value init, bool global)
    | ConstAttr(Initializers.static_init init)
    | LocalAttr
```

```
// NEW
add_string(s):
    str_id = UniqueIds.make_named_temporary("string")
    t = Types.Array(elem_type=Char, size=len(s) + 1)
    symbols.add(str_id, entry(
        t,
        attrs=ConstAttr(StringInit(s, true))
    ))
    return str_id
```

```
is_global(name):
    attrs = symbols.get(name).attrs

    if attrs is LocalAttr or ConstAttr:
        return false
    else if attrs is StaticAttr:
        return attrs.global
    else if attrs is FunAttr:
        return attrs.global
```

# TypeCheck

```
// String is also lvalue
is_lvalue(AST.Expression e):
    return e == Var or e == Dereference or e == Subscript or e == String
```

```
// Promote the types of char to int right on top
get_common_type(type1, type2):
    type1 = if is_character(type1)
                then Types.Int
                else type1

    type2 = if is_character(type2)
                then Types.Int
                else type1

	if type1 == type2:
		return type1
    else if type1 is Double OR type2 is Double:
        return Double
    else if get_size(type1) == get_size(type2):
        if is_signed(type1):
            return type2
        else:
            return type1
    else if get_size(type1) > get_size(type2):
        return type1
	else:
		return type2
```

```
// NEW
typecheck_string(s):
    e = AST.String(s)
    t = Types.Array(elem_type=Char, size=len(s) + 1)
    return set_type(e, t)
```

```
// Add case for String(s)
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
```

```
// Also promote character types to int
typecheck_complement(inner):
    typed_inner = typecheck_and_convert(inner)
    if typed_inner.type is Double OR is_pointer(typed_inner.type):
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
// Also promote character types to int
typecheck_negate(inner):
    typed_inner = typecheck_and_convert(inner)
    if is_pointer(inner.type):
        fail("Can't negate a pointer")
    else:
        // promote character types to int
        typed_inner = if is_character(typed_inner.type)
                        then convert_to(typed_inner, Types.Int)
                        else typed_inner

        negate_exp = Unary(Negate, typed_inner)
        return set_type(negate_exp, get_type(typed_inner))
```

```
// Also promote character types to int
typecheck_bitshift(op, e1, e2):
    typed_e1 = typecheck_and_convert(e1)
    typed_e2 = typecheck_and_convert(e2)

    // promote both operands to from character to int type
    typed_e1 = if is_character(typed_e1.type)
                    then convert_to(typed_e1, Types.Int)
                    else typed_e1

    typed_e2 = if is_character(typed_e2.type)
                    then convert_to(typed_e2, Types.Int)
                    else typed_e2

    if NOT (is_integer(get_type(typed_e1)) AND is_integer(get_type(typed_e2))):
        fail("Both operands of bit shift operation must be an integer")
    else:
        // Don't perform usual arithmetic conversions; result has type of left operand
        typed_binexp = Binary(op, typed_e1, typed_e2)
        return set_type(typed_binexp, get_type(typed_e1))
```

```
// Apply integer type promotions to >>= and <<=, but don't convert to common type
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
// First check if SingleInit is with string and the type is an array -> string literal for array initialization
// Then we need to check if init is SingeInit with string and the type is Pointer to Char -> constant string
// if a singleInit with string is not used in any the case above, throw error
// Finally, add case for ConstChar and ConstUChar in the SingleInit(exp)
static_init_helper(var_type, init):
    if var_type is Array(elem_type, size) and init is SingleInit(String(s)):
        if is_character(elem_type):
            n = size - len(s)
            if n == 0:
                return [ Initializers.StringInit(s, False) ]
            else if n == 1:
                return [ Initializers.StringInit(s, True) ]
            else if n > 1:
                return [ Initializers.StringInit(s, True), Initializers.ZeroInit(n - 1) ]
            else:
                fail("string is too long for initialize")
        else:
            fail("Can't initailize array of non-character type with string literal")
    else if var_type is Array and init is SingleInit:
        fail("Can't initialize array from scalar value")
    else if var_type is Pointer(Char) and init is SingleInit(String(s)):
        str_id = symbols.add_string(s)
        return [ Initializers.PointerInit(str_id) ]
    else if init is SingleInit(String):
        fail("String literal can only initialize char or decay to pointer")
    else if init is SingleInit(exp) and exp is Constant(c) and is_zero_int(c):
        return Initializers.zero(var_type)
    else if var_type is Pointer:
        fail("invalid static initializer for pointer")
    else if init is SingleInit(exp):
        if exp is Constant(c):
            converted_c = ConstConvert.convert(var_type, c)
            switch (converted_c);:
                case ConstChar(c):      init_val = Initializers.CharInit(c)
                case ConstInt(i):       init_val = Initializers.IntInit(i)
                case ConstLong(l):      init_val = Initializers.LongInit(l)
                cast ConstUChar(uc):    init_val = Initializers.UCharInit(uc)
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
            fail("Internal error: can't create zero initializer with function type")
```

```
// Add the first case of SingleInit(String) and target_type is Array
typecheck_init(target_type, init):
    if target_type is Array(elem_type, size) and init is SingleInit(String(s)):
        if !is_character(elem_type):
            fail("Can't initialize non-character type with string literal")
        else if len(s) > size:
            fail("Too many characters in string literal")
        else:
            return SingleInit(set_type(String(s), target_type))
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
// Switch control expression promotes char type to int
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
        // Perform integer promotions on controlling expression
        typed_control = if is_character(typed_control.type) then convert_to(typed_control, Types.Int) else typed_control
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

# TACKY

We add StaticConstant in top_level.

```
program = Program(top_level*)
top_level =
    | Function(identifier name, bool global, identifier* params, instruction* body)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init* inits)
    | StaticConstant(identifier name, type t, Initializers.static_init init)
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
// Add a case for AST.String
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
```

```
// NEW
emit_string_init(s, dst, offset):
    insts = []
    len_s = len(s)
    i = 0

    while i < len_s:
        if len_s - i >= 8:
            l = s[i, i+8]
            insts.append(
                CopyToOffset(
                    src=Constant(ConstLong(l)),
                    dst=dst,
                    offset=offset
                )
            )
            i += 8
        else if len_s - i >= 4:
            i = s[i, i+4]
            insts.append(
                CopyToOffset(
                    src=Constant(ConstInt(i)),
                    dst=dst,
                    offset=offset
                )
            )
            i += 4
        else:
            c = s[i]
            inst.append(
                CopyToOffset(
                    src=Constant(ConstChar(c)),
                    dst=dst,
                    offset=offset
                )
            )
            i += 1

    return insts
```

```
// Add the case of SingleInit(String(s), Array(size)) at the top
emit_compound_init(init, name, offset):
    if init is SingleInit(String(s), t=Array(size, _)):
        str_bytes = str_to_bytes(s)
        padding_bytes = to_bytes([0] * size-len(s))
        return emit_string_init([...str_bytes, ...padding_bytes], name, offset)
    else if init is SingleInit(e):
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
// Add a case to the top if var_decl has init
// As String is a single init, but we treat it like compound init.
emit_var_declaration(var_decl):
    if var_decl has init:
        if init is SingleInit(e)
            if e is String and init.type is Array as string_init:
                return emit_compound_init(string_init, var_decl.name, 0)
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

```
// Add case to convert ConstAttr to StaticConstant
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
        else if entry.attrs is ConstAttr:
            static_vars.push(StaticConstant(
                name,
                t=entry.t,
                init=entry.attrs.init.value
            ))
        else:
            // Don't do anything

    return static_vars
```

# Assembly

New asm_type: Byte.
Update the Movsx, and MovZeroExtend nodes.

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

Bytes have size and alignment of 1.

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
convert_val(Tacky.Val val):
    match val.type:
        case Tacky.Constant(ConstChar(c)):
            return Assembly.Imm((int64) c)
        case Tacky.Constant(ConstUChar(uc)):
            return Assembly.Imm((int64) uc)
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
        fail("Internal error: converting function type to assembly")
```

```
// note: this reports the type of ConstChar as SChar instead of Char, doesn't matter in this context
tacky_type(Tacky.Val operand):
    if operand is Tacky.Constant(c):
        return Const.type_of_const(c)
    else if operand is Tacky.Var(v):
        entry = symbols.get(v)
        if entry is null:
            fail("Internal error: {v} not in symbol table")
        return entry.t
    else:
        fail("Internal error: operand is not in symbol table")
```

```
// Update the case BitshiftLeft and BitshiftRight on Binary
// Also update the case SignExtend, Truncate, ZeroExtend, IntToDouble, DoubleToInt, UIntToDouble, DoubleToUInt.
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

```
// Add case for converting TACKY StaticConstant to Assembly's
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
    else if top_level is StaticVariable:
        return Assembly.StaticVariable(
            name=top_level.name,
            global=top_level.global,
            alignment=get_var_alignment(top_level.t),
            init=top_level.inits
        )
    else: // is StaticConstant
        return Assembly.StaticConstant(top_level.name, TypeUtils.get_elignment(top_level.t), top_level.init)
```

```
// We add case for ConstAttr
convert_symbol(name, symbol):
    if symbol.attrs is Types.FunAttr:
        return asmSymbols.add_fun(name, defined)
    else if symbol.attrs is ConstAttr:
        return asmSymbols.add_constant(name, convert_type(symbol.t))
    else if symbol.attrs is Types.StaticAttr:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), true)
    else:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), false)
```

# ReplacePseudo

```
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
```

# Instruction Fixup

```
// NEW
is_larger_than_byte(imm):
    return imm >= 256L || imm < -128L
```

```
// Extend the case for Mov with byte type
// Update Movsx, MovZeroExtend
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
// Add case for Byte
suffix(asm_type type):
    if type is Byte:
        return "b"
    else if type is Longword:
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
        if asm_type is Byte:
            return show_byte_reg(operand)
        else if asm_type is Longword:
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
// Update the case of Movsx, MovZeroExtend, Cdq
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

```
// NEW
escape(s):
    result = ''
    for c in s:
        if c a digit or alpha:
            result += c
        else:
            result += ()"\\%03o") c // to octal

    return result
```

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
    else if init is CharInit(c):
        return "\t.byte ${to_int8(c)}\n"
    else if init is UCharInit(uc):
        return "\t.byte ${to_uint8(uc)}\n"
    else if init is DoubleInit(d):
        return "\t.quad {to_double(d)}\n"

    // a partly-initialized array can include a mix of zero and non-zero initializaers
    else if init is ZeroInit(byte_count):
        return "\t.zero {byte_count}\n"
    else if init is StringInit(s, true):
        return "\t.asciz {escape(s)}\n"
    else if init is StringInit(s, false):
        return "\t.ascii {escape(s)}\n"
    else if init is PointerInit(lbl):
        return "\t.quad {show_local_label(lbl)}\n"
```

```
emit_constant(name, alignment, init):
    contents = ""
    constant_section_name = ""

    if Settings.platform is Linux:
        constant_section_name = ".section .rodata"
    else if Settings.platform is OS_X:
        if init is Initializers.StringInit:
            return ".cstrign"
        else:
            if alignment == 8:
                constant_section_name = ".literal8"
            else if  alignment == 16:
                constant_section_name = ".literal16"
            else:
                fail("Internal error: found constant with bad alignment")

    contents = "\t{constant_section_name}\n\t{align_directive()} {show_local_label(name)}\n{}:"
    contents += emit_init(init)
    // macOS linker gets cranky if you write only 8 bytes to .literal16 section
    if constant_section.name == ".literal16":
        contents += emit_init(Initializers.LongInit(0))

    return contents
```

# Output

From C:

```C
signed char static_array[3][4] = {{'a', 'b', 'c', 'd'}, "efgh", "ijk"};

int main(void)
{
    unsigned char auto_array[2][3] = {"lmn", {'o', 'p'}};

    for (int i = 0; i < 3; i = i + 1)
        for (int j = 0; j < 4; j = j + 1)
            if (static_array[i][j] != "abcdefghijk"[i * 4 + j])
                return 1;

    unsigned char uc = 255;

    // uc is promoted to int, then shifted
    if ((uc >> 3) != 31)
    {
        return 2;
    }

    char c = -56;
    // make sure c << 3ul is promoted to int, not unsigned long
    if (((-(c << 3ul)) >> 3) != -5)
    {
        return 4;
    }

    switch (c)
    {
    // if we reduced this to a char it would be -56
    // but we won't, so this case shouldn't be taken
    case 33554632:
        return 1;
    default:
        return 0;
    }

    char c2 = 100;
    c += c2;

    return 0;
}
```

To x64 Assembly on Linux:

```asm

```
