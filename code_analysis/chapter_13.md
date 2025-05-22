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
- [Extra Credit: Bitwise Operators](#extra-credit-bitwise-operators)
  - [Assembly](#assembly-1)
  - [CodeGen](#codegen-1)
  - [Emit](#emit-1)
  - [Output](#output-1)

---

# Token, Lexer

Old tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| ConstInt | ([0-9]+)[^\w.] |
| ConstUInt | ([0-9]+[uU])[^\w.] |
| ConstLong | ([0-9]+[lL])[^\w.] |
| ConstULong | ([0-9]+([lL][uU]|[uU][lL]))[^\w.] |

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordDouble | double |
| ConstDouble | (([0-9]_\.[0-9]+|[0-9]+\.?)[Ee][+-]?[0-9]+|[0-9]_\.[0-9]+|[0-9]+\.)[^\w.] |

The changes for integer will match `100;` for example. Make sure you process it to remove the trailing `;`.

# Types

```
type Int {}
type Long {}
type UInt {}
type ULong {}
type Double {}
type FunType {
    param_types: t[],
    ret_type: t
}

t = Int | Long | UInt | ULong | Double | FunType
```

# Const

```
type t = ConstInt(int32) | ConstLong(int64) | ConstUInt(int32) | ConstULong(int64) | ConstDouble(double)

int_zero = ConstInt(0)
long_zero = ConstLong(0)
```

```
type_of_const(const):
    match const
        case ConstInt: return Types.Int
        case ConstLong: return Types.Long
        case ConstUInt: return Types.UInt
        case ConstULong: return Types.ULong
        case ConstDouble: return Types.Double
        default:
            fail("Internal error: unknown const type")
```

# ConstConvert

```
// create different for each type of v, better use template if your implementation language supports it
cast(int32/uint32/int64/uint64 v, Types.t targetType):
    switch targetType:
        case Types.Int: return Const.ConstInt(to_int32(v))
        case Types.UInt: return Const.ConstUInt(to_uint32(v))
        case Types.Long: return Const.ConstLong(to_int64(v))
        case Types.ULong: return Const.ConstULong(to_uint64(v))
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
            else:
                i64 = to_int64(d)
                return cast(d, target_type)
```

# AST

We already have changes the Types and Const files.

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
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
```

# Parser

```
is_type_specifier(token):
    if token is "int"
        or "long"
        or "double"
        or "signed"
        or "unsigned":
        return true
    else:
        return false
```

```
parse_type(specifier_list):
    if specifier_list == ["double"]:
        return Types.Double

    if (
        specifier_list.empty
        OR length(Set(specifier_list)) != length(specifier_list)
        OR specifier_list contains "double"
        OR specifier_list contains both "signed" and "unsigned"
    ):
        fail("Invalid type specifier")
    else if specifier_list has "unsigned" and "long":
        return Types.ULong
    else if specifier_list has "unsigned":
        return Types.UInt
    else if specifier_list has "long":
        return Types.Long
    else:
        return Types.Int
```

```
parse_constant(tokens):
    MAX_INT64 = (BigInt(1) << 63) - 1
    MAX_INT32 = (BigInt(1) << 31) - 1
    MAX_UINT32 =
    MAX_UINT64 =

    tok = take_token(tokens)

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
parse_primary_exp(tokens):
	next_token = peek(tokens)
	if next_token is Token.ConstInt
        or Token.ConstLong
        or Token.ConstUInt
        or Token.ConstULong
        or Token.ConstDouble:
		return parse_constant(tokens)
	if  next_token is an identifier:
        --snip--
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

# Initializers

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
    if type is ULong:
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

_No Changes_

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
            return false
        default:
            fail("Internal error: signedness doesn't make sense for function and double type")
```

# Symbols

_No Changes_

# TypeCheck

```
get_common_type(type1, type2):
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
// Floating-point numbers cannot be complemented.
typecheck_unary(op, inner):
    typed_inner = typecheck_exp(inner)
    unary_exp = Unary(op, typed_inner)
    if op is "Not":
        return set_type(unary_exp, Int)
    if op is "Complement" and get_type(typed_inner) is Types.Double:
        fail("Can't apply bitwise complement to double")
    else:
        return set_type(unary_exp, get_type(typed_inner))
```

```
// Biftshifting cannot work on double either.
typecheck_binary_(op, e1, e2):
    typed_e1 = typecheck_exp(e1)
    typed_e2 = typecheck_exp(e2)

    if op is BitShiftLeft or BitShiftRight:
        if get_type(typed_e1) == Double || get_type(typed_e2) == Double:
            fail("Both operands of bit shift must have integer type")
        else:
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

    if (op is Mod or BitwiseAnd or BitwiseOr or BitwiseXor) and common_type is Double:
        fail("Can't apply % or bitwise operation to double")

    if op is Add or Subtract or Multiply or Divide or Mod or BitwiseAnd or BitwiseOr or BitwiseXor:
        return set_type(binary_exp, common_type)
    else:
        return set_type(binary_exp, Int)
```

```
// We also do the check before getting result_t and converted_rhs
typecheck_compound_assignment(op, lhs, rhs):
    typed_lhs = typecheck_exp(lhs)
    lhs_type = get_type(typed_lhs)
    typed_rhs = typecheck_exp(rhs)
    rhs_type = get_type(typed_rhs)

    if op is of (Remainder | BitwiseAnd | BitwiseOr | BitwiseXor | BitshiftLeft | BitshiftRight)
        and (lhs_type == Double or rhs_type == Double):
        fail("Operand {op} doesn't support double operands")

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
to_static_init(var_type, constant):
    if constant is of type AST.Constant(c):
        Initializers.StaticInit init_val

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

```
// For switch and case statements, the value cannot be float.
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
        if get_type(typed_control) == Double:
            fail("Controlling expression in switch cannot be a double")
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

# TACKY

We have 4 new casting instruction: DoubleToInt, DoubleToUInt, IntToDouble, and UIntToDouble.

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
get_cast_instruction(src, dst, src_t, dst_t):
    if dst_t == Types.Double:
        if TypeUtils.is_signed(src_t):
            return IntToDouble(src, dst)
        else:
            return UIntToDouble(src, dst)
    else if src_t == Types.Double:
        if TypeUtils.is_signed(dst_t):
            return DoubleToInt(src, dst)
        else:
            return DoubleToUInt(src, dst)

    // Cast between int types. Note: assumes src and dst have different types
    if TypeUtils.get_size(dst_t) == TypeUtils.get_size(src_t):
        return Copy(src, dst)
    else if TypeUtils.get_size(dst_t) < TypeUtils.get_size(src_t):
        return Truncate(src, dst)
    else if TypeUtils.is_signed(src_t):
        return SignExtend(src, dst)
    else:
        return ZeroExtend(src, dst)
```

# Assembly

Assembly type has a new type for `double`.
The top_level has `StaticConstant`.
The two new instructions for double conversions.
We add `ShrOneOp`, `DivDouble`, `shr`, `shl` and 16 XMM registers.

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
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

# AssemblySymbols

We change Obj entry, add_var, get_size, get_alignment; and add 2 new function: add_constant, is_constant

```
type entry =
    | Fun(bool defined, int bytes_required)
    | Obj (Assembly.asm_type t, bool is_static, bool is_constant)
```

```
add_fun(fun_name, defined):
    symbol_table[fun_name] = Fun(defined, 0)
```

```
add_var(var_name, t, is_static):
    symbol_table[var_name] = Obj(t, is_static, false)
```

```
add_constant(const_name, t):
    symbol_table[const_name] = Obj(t, true, true)
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
            if symbol_table[var_name] has asm_type of Quadword or Double:
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
            if symbol_table[var_name] has asm_type of Quadword or Double:
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

```
is_constant(const_name):
    if symbol_table has const_name:
        if symbol_table[const_name] is Obj:
            return symbol_table[const_name].is_constant
        else:
            fail("Internal error: is_constant doesn't make sense for functions")
    else:
        fail("Internal error: not found")
```

# CodeGen

```
int_param_passing_regs = [DI, SI, DX, CX, R8, R9]
dbl_param_passing_regs = [XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7]
```

```
constants = {}

add_constant(dbl, alignment=8):
    // See if we've defined this double already
    if constants has dbl:
        name, old_alignment = constants[dbl]
        // Update aligment to max of current and new
        constants[dbl] = (name, max(alignment, old_alignment))
        return name
    else:
        // We haven't defined it yet, add it to the table
        name = UniqueIds.make_label("dbl")
        constants[dbl] = (name, alignment)
        return name
```

```
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
			return Assembly.Pseudo(val.name)
```

```
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong:
        return asm_type.Quadword
    else if type is Double:
        return asm_type.Double
    else:
        fail("Internal error: converting function type to assembly")
```

```
convert_binop(Tacky.Binop op):
	match op:
		case Add:
			return Assembly.Add
		case Subtract:
			return Assembly.Sub
		case Multiply:
			return Assembly.Mult
		case Divide:
            return Assembly.DivDouble // NB should only be called for operands on doubles
        case BitwiseAnd:
            return Assembly.And
        case BitwiseOr:
            return Assembly.Or
        case BitwiseXor:
            return Assembly.Xor
		case Mod:
		case Equal:
		case NotEqual:
		case GreaterThan:
		case GreaterOrEqual:
		case LessThan:
		case LessOrEqual:
        case BitshiftLeft:
        case BitshiftRight:
			fail("Internal error: Not a binary assembly instruction!")
```

```
classify_parameters(tacky_vals):
    int_reg_args = []
	dbl_reg_args = []
	stack_args = []

	for v in tacky_vals:
		operand = convert_val(v)
		t = asm_type(v)
		typed_operand = (t, operand)
		if t == Double:
			if length(dbl_reg_args) < 8:
				dbl_reg_args.append(operand)
			else:
				stack_args.append(typed_operand)
		else:
			if length(int_reg_args) < 6:
				int_reg_args.append(typed_operand)
			else:
				stack_args.append(typed_operand)

	return (int_reg_args, dbl_reg_args, stack_args )
```

```
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
    assembly_dst = convert_val(fun_call.dst)

    return_reg = null
    if asm_type(dst) == Double:
        return_reg = XMM0
    else:
        return_reg = AX

    insts.append(Mov(asm_type(fun_call.dst), Reg(return_reg), assembly_dst))

    return insts
```

We extend several instructions:

- Return
- Unary (Not, negate)
- Binary (Relational operator, Division)
- JumpIfZero
- JumpIfNotZero
- IntToDouble
- DoubleToInt
- UIntToDouble
- DoubleTouInt

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
                        return [
                            Binary(Xor,t=Double src=Reg(XMM0), dst=Reg(XMM0)),
                            Cmp(src_t, asm_src, Reg(XMM0)),
                            Mov(dst_t, zero(), asm_dst),
                            SetCC(E, asm_dst)
                        ]
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
		case Jump:
			return [ Jmp(inst.target) ]

		case JumpIfZero:
            t = asm_type(inst.cond)
			asm_cond = convert_val(inst.cond)

            if t == Double:
                return [
                    Binary(Xor, Double, Reg(XMM0), Reg(XMM0)),
                    Cmp(t, asm_cond, Reg(XMM0)),
                    JmpCC(e, inst.target)
                ]

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
                    JmpCC(NE, inst.target)
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
        stk = Stack(16 + (8*stk_idx))
        inst.append(Mov(param_t, stk, param))
        stk_idx++

    return insts
```

```
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
            alignment=TypeUtils.get_alignment(top_level.t),
            init=top_level.init
        )
```

```
convert_constant(key, (name, alignment)):
    dbl = key
    AssemblySymbols.add_constant(name, Double)
    return Assembly.StaticConstant(
        name,
        alignment,
        init=Initializers.DoubleInit(dbl)
    )
```

```
gen(Tacky.Program prog):
    // clear the hashtable (necessary if we're compiling multiple sources)
    constants.clear()
    converted_top_levels = []

    for top_level in prog.top_levels
        new_top_level = convert_top_level(top_level)
        converted_top_levels.append(new_top_level)

    converted_constants = []
    for constant in constants:
        converted_constants.append(convert_constant(constant))

    for name, symbol in symbols:
        convert_symbol(name, symbol)

    return Assembly.Program([...converted_constants, ...converted_top_levels])
```

# ReplacePseudo

```
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

We define 2 helper functions to quickly check if an operand is Constant, or Memory

```
is_constant(operand):
    return operand.type is Imm

is_memory(operand):
    return operand.type is Data or Stack
```

We update the fixup_instruction in Mov, Binary, Cmp, and the 2 new convert instructions to and from doubles.

```
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	case Mov:
        // Mov can't move a value from one memory address to another
        if inst.src is (Stack | Data) and inst.dst is (Stack | Data):
            scratch = inst.t == Double
                ? Reg(XMM14)
                : Reg(R10)

            return [
				Mov(inst.t, src, scratch),
				Mov(inst.t, scratch, dst)
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
		else if both src and dst of cmp are (Stack | Data):
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
    case Cvttsd2si:
        // destination of cvttsd2si must be a register
        if inst.dst is Stack or Data:
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
suffix(asm_type type):
    if type is Longword:
        return "l"
    else if type is Quadword:
        return "q"
    else if type is Double:
        return "sd"
```

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
        default:
            fail("Internal error: can't store quadword type in XMM register")
```

```
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
        case XMM14: return "%xmm14"
        case XMM15: return "%xmm15"
        default:
            fail("Internal error: can't store double type in general-purpose register")
```

```
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
    else if operand is Stack:
        return "%{operand.offset}(%rbp)"
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
        default:
            fail("Internal error: can't store byte type in XMM register")
```

```
show_unary_operator(op):
    match op:
        case Neg: return "neg"
        case Not : return "not"
        case ShrOneOp: return "shr"
```

```
show_binary_operator(op):
    match op:
        case Add: return "add"
        case Sub: return "sub"
        case Mul: return  "imul"
        case DivDouble: return "div"
        case Xor: return "xor"
        case And: return "and"
        case Or : return "or"
        case Sal: return "sal"
        case Sar: return "sar"
        case Shl: return "shl"
        case Shr: return "shr"
```

Extend the emit_instruction for Xor and Mult on double operands, Cmp for `comisd`, Push, Cvtsi2sd, Cvttsd2si, Cdq for double

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

        Cdq(t):
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
```

```
emit_constant(name, alignment, init):
    contents = ""
    constant_section_name = ""

    if Settings.platform is Linux:
        constant_section_name = ".section .rodata"
    else if Settings.platform is OS_X:
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
    else if top_level is StaticVariable:
        // is_zero returns False for all doubles
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
    else: // StaticConstant
        return emit_constant(top_level.name, top_level.alignment, top_level.init)
```

# Output

From C:

```C
int main(void) {
    double d = 1000.5;
    /* When we perform compound assignment, we convert both operands
     * to their common type, operate on them, and convert the result to the
     * type of the left operand */
    d += 1000;
    if (d != 2000.5) {
        return 1;
    }

    unsigned long ul = 18446744073709551586ul;
    /* We'll promote e to the nearest double,
     * which is 18446744073709551616,
     * then subtract 1.5 * 10^19, which
     * results in 3446744073709551616.0,
     * then convert it back to an unsigned long
     */
    ul -= 1.5E19;
    if (ul != 3446744073709551616ul) {
        return 2;
    }
    /* We'll promote i to a double, add .99999,
     * then truncate it back to an int
     */
    int i = 10;
    i += 0.99999;
    if (i != 10) {
        return 3;
    }

    return 0;
}
```

To x64 Assembly on Linux:

```asm
	.section .rodata
	.align 8
.Ldbl.20:
	.quad 0.99999

	.section .rodata
	.align 8
.Ldbl.19:
	.quad 9223372036854775808

	.section .rodata
	.align 8
.Ldbl.16:
	.quad 1.5e+19

	.section .rodata
	.align 8
.Ldbl.13:
	.quad 2000.5

	.section .rodata
	.align 8
.Ldbl.12:
	.quad 1000.5

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$64, %rsp
	movsd	.Ldbl.12(%rip), %xmm14
	movsd	%xmm14, -8(%rbp)
	movl	$1000, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -16(%rbp)
	movsd	-8(%rbp), %xmm14
	movsd	%xmm14, -8(%rbp)
	movsd	-8(%rbp), %xmm15
	addsd	-16(%rbp), %xmm15
	movsd	%xmm15, -8(%rbp)
	movsd	.Ldbl.13(%rip), %xmm15
	comisd	-8(%rbp), %xmm15
	movl	$0, -20(%rbp)
	setne	-20(%rbp)
	cmpl	$0, -20(%rbp)
	je	.Lif_end.4
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.4:
	movq	$18446744073709551586, -32(%rbp)
	cmpq	$0, -32(%rbp)
	jl	.Lulong2dbl.oob.14
	cvtsi2sdq	-32(%rbp), %xmm15
	movsd	%xmm15, -40(%rbp)
	jmp	.Lulong2dbl.end.15
.Lulong2dbl.oob.14:
	movq	-32(%rbp), %r8
	movq	%r8, %r9
	shrq	%r9
	andq	$1, %r8
	orq	%r8, %r9
	cvtsi2sdq	%r9, %xmm15
	movsd	%xmm15, -40(%rbp)
	movsd	-40(%rbp), %xmm15
	addsd	-40(%rbp), %xmm15
	movsd	%xmm15, -40(%rbp)
.Lulong2dbl.end.15:
	movsd	-40(%rbp), %xmm14
	movsd	%xmm14, -40(%rbp)
	movsd	-40(%rbp), %xmm15
	subsd	.Ldbl.16(%rip), %xmm15
	movsd	%xmm15, -40(%rbp)
	movq	.Ldbl.19(%rip), %r10
	cmpq	%r10, -40(%rbp)
	jae	.Ldbl2ulong.oob.17
	cvttsd2siq	-40(%rbp), %r11
	movq	%r11, -32(%rbp)
	jmp	.Ldbl2ulong.end.18
.Ldbl2ulong.oob.17:
	movsd	-40(%rbp), %xmm7
	subsd	.Ldbl.19(%rip), %xmm7
	cvttsd2siq	%xmm7, %r11
	movq	%r11, -32(%rbp)
	movq	$9223372036854775808, %r9
	addq	%r9, -32(%rbp)
.Ldbl2ulong.end.18:
	movq	$3446744073709551616, %r11
	cmpq	-32(%rbp), %r11
	movl	$0, -44(%rbp)
	setne	-44(%rbp)
	cmpl	$0, -44(%rbp)
	je	.Lif_end.7
	movl	$2, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.7:
	movl	$10, -48(%rbp)
	cvtsi2sdl	-48(%rbp), %xmm15
	movsd	%xmm15, -56(%rbp)
	movsd	-56(%rbp), %xmm14
	movsd	%xmm14, -56(%rbp)
	movsd	-56(%rbp), %xmm15
	addsd	.Ldbl.20(%rip), %xmm15
	movsd	%xmm15, -56(%rbp)
	cvttsd2sil	-56(%rbp), %r11d
	movl	%r11d, -48(%rbp)
	movl	$10, %r11d
	cmpl	-48(%rbp), %r11d
	movl	$0, -60(%rbp)
	setne	-60(%rbp)
	cmpl	$0, -60(%rbp)
	je	.Lif_end.10
	movl	$3, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.10:
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

# Extra Credit: NaN

Working with NaN in float comparisons, we check the parity flags.

## Assembly

We add two new condition code: P and NP.

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
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE | P | NP
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

## CodeGen

While converting instruction that is binary, specifically the relational operators, we'll use the new helper function if the source operand has Double type.

```
// Helper function for double comparisons w/ support for NaN
convert_dbl_comparison(op, dst_t, asm_src1, asm_src2, asm_dst):
    cond_code = convert_cond_code(false, op)
    /*
        If op is A or AE, can perform usual comparisons;
            these are true if only some flags are 0, so they'll be false for unordered results.
            If op is B or BE, just flip operands and use A or AE instead.
        If op is E or NE, need to check for parity afterwards.
    */
    switch cond_code:
        case B:
            cond_code = A
            asm_src1, asm_src2 = asm_src2, asm_src1
        case BE:
            cond_code = AE
            asm_src1, asm_src2 = asm_src2, asm_src1
        default:
            // Do nothing

    insts = [
        Cmp(Double, asm_src2, asm_src1),
        Mov(dst_t, zero(), asm_dst),
        SetCC(cond_code, asm_dst),
    ]

    parity_insts = []
    if cond_code == E:
        /*
            zero out destination if parity flag is set,
            indicating unordered result
        */
        parity_insts = [
            Mov(dst_t, zero(), Reg(R9)),
            SetCC(NP, Reg(R9)),
            Binary(And, dst_t, Reg(R19), asm_dst),
        ]
    else if cond_code == NE:
        // set destination to 1 if parity flag is set, indicating ordered result
        parity_insts = [
            Mov(dst_t, zero, Reg(R9)),
            SetCC(P, Reg(R9)),
            Binary(Or, dst_t, Reg(R9), asm_dst),
        ]

    return [...insts, ...parity_insts]
```

In convert_instruction, we update where we have any comparisons with Double types:

- Unary (not)
- Binary (relational operator)
- JumpIfZero
- JumpIfNotZero

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

## Emit

```
show_cond_code(cond_code):
	match cond_code:
		case E: return 'e'
		case NE: return 'ne'
		case G: return 'g'
		case GE: return 'ge'
		case L: return 'l'
		case LE: return 'le'
        case A: return 'a'
        case AE: return 'ae'
        case B: return 'b'
        case BE: return 'be'
        case P: return 'p'
        case NP: return 'np'
```

## Output

From C:

```C
int double_isnan(double d); // should be defined and call the macro isnan in C math library.

// This should return zero, because all comparisons with NaN are false
int main(void) {
    static double zero = 0.0;
    double nan = 0.0 / zero;
    if (nan < 0.0 || nan == 0.0 || nan > 0.0 || nan <= 0.0 || nan >= 0.0)
        return 1;

    if (1 > nan || 1 == nan || 1 > nan || 1 <= nan || 1 >= nan)
        return 2;

    if (nan == nan)
        return 3;

    if (!(nan != nan)) { // != should evaluate to true
        return 4;
    }

    if (!double_isnan(nan)) {
        return 5;
    }

    if (!double_isnan(4 * nan)) {
        return 6;
    }

    if (!double_isnan(22e2 / nan)) {
        return 7;
    }

    if (!double_isnan(-nan)) {
        return 8;
    }

    return 0;
}
```

To x64 Assembly on Linux:

```asm
	.section .rodata
	.align 8
.Ldbl.67:
	.quad 2200

	.section .rodata
	.align 16
.Ldbl.66:
	.quad 0

	.data
	.align 8
zero.1:
	.quad 0

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$224, %rsp
	movsd	.Ldbl.66(%rip), %xmm14
	movsd	%xmm14, -8(%rbp)
	movsd	-8(%rbp), %xmm15
	divsd	zero.1(%rip), %xmm15
	movsd	%xmm15, -8(%rbp)
	movsd	-8(%rbp), %xmm14
	movsd	%xmm14, -16(%rbp)
	movsd	.Ldbl.66(%rip), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -20(%rbp)
	seta	-20(%rbp)
	cmpl	$0, -20(%rbp)
	je	.Lor_true.7
	movsd	-16(%rbp), %xmm15
	comisd	.Ldbl.66(%rip), %xmm15
	movl	$0, -24(%rbp)
	sete	-24(%rbp)
	movl	$0, %r9d
	setnp	%r9b
	andl	%r9d, -24(%rbp)
	cmpl	$0, -24(%rbp)
	je	.Lor_true.7
	movl	$0, -28(%rbp)
	jmp	.Lor_end.8
.Lor_true.7:
	movl	$1, -28(%rbp)
.Lor_end.8:
	cmpl	$0, -28(%rbp)
	je	.Lor_true.11
	movsd	-16(%rbp), %xmm15
	comisd	.Ldbl.66(%rip), %xmm15
	movl	$0, -32(%rbp)
	seta	-32(%rbp)
	cmpl	$0, -32(%rbp)
	je	.Lor_true.11
	movl	$0, -36(%rbp)
	jmp	.Lor_end.12
.Lor_true.11:
	movl	$1, -36(%rbp)
.Lor_end.12:
	cmpl	$0, -36(%rbp)
	je	.Lor_true.15
	movsd	.Ldbl.66(%rip), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -40(%rbp)
	setae	-40(%rbp)
	cmpl	$0, -40(%rbp)
	je	.Lor_true.15
	movl	$0, -44(%rbp)
	jmp	.Lor_end.16
.Lor_true.15:
	movl	$1, -44(%rbp)
.Lor_end.16:
	cmpl	$0, -44(%rbp)
	je	.Lor_true.19
	movsd	-16(%rbp), %xmm15
	comisd	.Ldbl.66(%rip), %xmm15
	movl	$0, -48(%rbp)
	setae	-48(%rbp)
	cmpl	$0, -48(%rbp)
	je	.Lor_true.19
	movl	$0, -52(%rbp)
	jmp	.Lor_end.20
.Lor_true.19:
	movl	$1, -52(%rbp)
.Lor_end.20:
	cmpl	$0, -52(%rbp)
	je	.Lif_end.4
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.4:
	movl	$1, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -64(%rbp)
	movsd	-64(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -68(%rbp)
	seta	-68(%rbp)
	cmpl	$0, -68(%rbp)
	je	.Lor_true.27
	movl	$1, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -80(%rbp)
	movsd	-80(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -84(%rbp)
	sete	-84(%rbp)
	movl	$0, %r9d
	setnp	%r9b
	andl	%r9d, -84(%rbp)
	cmpl	$0, -84(%rbp)
	je	.Lor_true.27
	movl	$0, -88(%rbp)
	jmp	.Lor_end.28
.Lor_true.27:
	movl	$1, -88(%rbp)
.Lor_end.28:
	cmpl	$0, -88(%rbp)
	je	.Lor_true.32
	movl	$1, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -96(%rbp)
	movsd	-96(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -100(%rbp)
	seta	-100(%rbp)
	cmpl	$0, -100(%rbp)
	je	.Lor_true.32
	movl	$0, -104(%rbp)
	jmp	.Lor_end.33
.Lor_true.32:
	movl	$1, -104(%rbp)
.Lor_end.33:
	cmpl	$0, -104(%rbp)
	je	.Lor_true.37
	movl	$1, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -112(%rbp)
	movsd	-16(%rbp), %xmm15
	comisd	-112(%rbp), %xmm15
	movl	$0, -116(%rbp)
	setae	-116(%rbp)
	cmpl	$0, -116(%rbp)
	je	.Lor_true.37
	movl	$0, -120(%rbp)
	jmp	.Lor_end.38
.Lor_true.37:
	movl	$1, -120(%rbp)
.Lor_end.38:
	cmpl	$0, -120(%rbp)
	je	.Lor_true.42
	movl	$1, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -128(%rbp)
	movsd	-128(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -132(%rbp)
	setae	-132(%rbp)
	cmpl	$0, -132(%rbp)
	je	.Lor_true.42
	movl	$0, -136(%rbp)
	jmp	.Lor_end.43
.Lor_true.42:
	movl	$1, -136(%rbp)
.Lor_end.43:
	cmpl	$0, -136(%rbp)
	je	.Lif_end.22
	movl	$2, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.22:
	movsd	-16(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -140(%rbp)
	sete	-140(%rbp)
	movl	$0, %r9d
	setnp	%r9b
	andl	%r9d, -140(%rbp)
	cmpl	$0, -140(%rbp)
	je	.Lif_end.45
	movl	$3, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.45:
	movsd	-16(%rbp), %xmm15
	comisd	-16(%rbp), %xmm15
	movl	$0, -144(%rbp)
	setne	-144(%rbp)
	movl	$0, %r9d
	setp	%r9b
	orl	%r9d, -144(%rbp)
	cmpl	$0, -144(%rbp)
	movl	$0, -148(%rbp)
	sete	-148(%rbp)
	cmpl	$0, -148(%rbp)
	je	.Lif_end.47
	movl	$4, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.47:
	movsd	-16(%rbp), %xmm0
	call	double_isnan@PLT
	movl	%eax, -152(%rbp)
	cmpl	$0, -152(%rbp)
	movl	$0, -156(%rbp)
	sete	-156(%rbp)
	cmpl	$0, -156(%rbp)
	je	.Lif_end.50
	movl	$5, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.50:
	movl	$4, %r10d
	cvtsi2sdl	%r10d, %xmm15
	movsd	%xmm15, -168(%rbp)
	movsd	-168(%rbp), %xmm14
	movsd	%xmm14, -176(%rbp)
	movsd	-176(%rbp), %xmm15
	mulsd	-16(%rbp), %xmm15
	movsd	%xmm15, -176(%rbp)
	movsd	-176(%rbp), %xmm0
	call	double_isnan@PLT
	movl	%eax, -180(%rbp)
	cmpl	$0, -180(%rbp)
	movl	$0, -184(%rbp)
	sete	-184(%rbp)
	cmpl	$0, -184(%rbp)
	je	.Lif_end.53
	movl	$6, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.53:
	movsd	.Ldbl.67(%rip), %xmm14
	movsd	%xmm14, -192(%rbp)
	movsd	-192(%rbp), %xmm15
	divsd	-16(%rbp), %xmm15
	movsd	%xmm15, -192(%rbp)
	movsd	-192(%rbp), %xmm0
	call	double_isnan@PLT
	movl	%eax, -196(%rbp)
	cmpl	$0, -196(%rbp)
	movl	$0, -200(%rbp)
	sete	-200(%rbp)
	cmpl	$0, -200(%rbp)
	je	.Lif_end.58
	movl	$7, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.58:
	movsd	-16(%rbp), %xmm14
	movsd	%xmm14, -208(%rbp)
	movsd	-208(%rbp), %xmm15
	xorpd	.Ldbl.66(%rip), %xmm15
	movsd	%xmm15, -208(%rbp)
	movsd	-208(%rbp), %xmm0
	call	double_isnan@PLT
	movl	%eax, -212(%rbp)
	cmpl	$0, -212(%rbp)
	movl	$0, -216(%rbp)
	sete	-216(%rbp)
	cmpl	$0, -216(%rbp)
	je	.Lif_end.62
	movl	$8, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.62:
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
