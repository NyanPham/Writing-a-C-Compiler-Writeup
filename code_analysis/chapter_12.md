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
| KeywordSigned | signed |
| KeywordUnsigned | unsigned |
| ConstUInt | [0-9]+[uU]\b |
| ConstULong | [0-9]+\([uU][lL]\|[lL][uU]\)\b |

In lexer, when producing tokens for ConstUInt, we cut off the trailing character (u or U), and for ConstULong, we cut off 2 characters in the end (lL and uU)

# Types

```
type Int {}
type Long {}
type UInt {}
type ULong {}
type FunType {
    param_types: t[],
    ret_type: t
}

t = Int | Long | UInt | ULong | FunType
```

# Const

```
type t = ConstInt(int32) | ConstLong(int64) | ConstUInt(int32) | ConstULong(int64)

int_zero = ConstInt(0)
long_zero = ConstLong(0)
```

A new helper function to quickly get the Types from Const

```
type_of_const(const):
    match const
        case ConstInt: return Types.Int
        case ConstLong: return Types.Long
        case ConstUInt: return Types.UInt
        case ConstULong: return Types.ULong
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
```

# AST

```
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
type = Int | Long | UInt | ULong | FunType(type* params, type ret)
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
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int)
```

# Parser

We have two new functions to easily check if a token is to specify types or storage class duration.

```
is_type_specifier(token):
    if token is "int" or "long" or "signed" or "unsigned":
        return true
    else:
        return false
```

```
is_specifier(token):
    if token is "static" or "extern":
        return true
    else:
        return is_type_specifier(token)
```

```
parse_type_specifier_list(tokens):
    type_specifiers = []
    next_token = peek(tokens)

    while is_type_specifier(next_token):
        spec = take(tokens)
        type_specifiers.append(spec)
        next_token = peek(tokens)

    return type_specifiers
```

```
parse_specifier_list(tokens):
    specifiers = []
    next_token = peek(tokens)

    while is_specifier(next_token):
        spec = take(tokens)
        specifiers.append(spec)
        next_token = peek(tokens)

    return specifiers
```

```
parse_type(specifier_list):
    if (
        specifier_list.empty
        OR length(Set(specifier_list)) != length(specifier_list)
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
parse_type_and_storage_class(specifier_list):
    types = []
    storage_classes = []

    for tok in specifier_list:
        if is_type_specifier(tok):
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
    MAX_UINT32 =
    MAX_UINT64 =

    tok = take_token(tokens)

    if (tok is signed and tok.value > MAX_INT64) OR (tok is unsigned and tok.value > MAX_UINT64):
        fail("Constant is too large to fit in an int or long with given signedness")

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
	if next_token is Token.ConstInt or Token.ConstLong or Token.ConstUInt or Token.ConstULong:
		return parse_constant(tokens)
	if  next_token is an identifier:
        --snip--
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

```
parse_factor(tokens):
	next_tokens = npeek(2, tokens) // We peek 2 tokens
	// Unary expression
	if next_token[0] is Tilde, Hyphen, Bang, DoublePlus, DoubleHyphen:
		op = parse_unop(tokens)
		inner_exp = parse_factor(tokens)
		return Unary(op, inner_exp)
    else if next_token[0] is "(" and is_type_specifier(next_token[1]):
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
parse_block_item(tokens):

    if (is_specifier(peek(tokens)))
        return parse_declaration(tokens)
    else:
        return parse_statement(tokens)
```

```
parse_for_init(tokens):
    if is_specifier(peek(tokens)): // Note that we choose to parse them static and extern here, and let the semantic analysis to catch this invalid usage
        return AST.InitDecl(parse_variable_declaration(tokens))
    else:
        return AST.InitExp(parse_optional_exp(";", tokens))
```

# Initializers

```
type static_init =
    | IntInit(int32 v)
    | LongInit(int64 v)
    | UIntInit(uint32 v)
    | ULongInit(uint64 v)

zero(Types.t type):
    if type is Int:
        return IntInit(0)
    if type is Long:
        return LongInit(0)
    if type is UInt:
        return UIntInit(0)
    if type is ULong:
        return ULongInit(0)
    if type is FunType:
        fail("Internal error: zero doens't make sense for function type")

is_zero(static_init):
    if static_init is IntInit(i):
        return i == 0 (in 32-bit)
    else if static_init is LongInit(l):
        return l == 0 (in 64-bit)
    else if static_init is UIntInit(u):
        return u == 0 (in 32-bit)
    else if static_init is ULongInit(ul):
        return u == 0 (in 64-bit)
```

# Identifier Resolution

_No changes_

# ValidateLabels

_No changes_

# LoopLabeling

_No changes_

# CollectSwitchCases

_No changes_

# TypeUtils

A simple util file for types. If you already defined the Type system in a different file, not in AST, define the following functions in that file instead.

```
get_type(AST.Expression exp):
    return exp.type
```

```
set_type(AST.Expression exp, Types.t type):
    return exp.type = type
```

```
get_size(Types.t type):
    switch type:
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
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
            fail("Internal error: signedness doesn't make sense for function types")
```

# Symbols

_No Changes_

# TypeCheck

```
// get_size is from TypeUtils
get_common_type(type1, type2):
	if type1 == type2:
		return type1
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
typecheck_var(string v):
    v_type = symbols.get(v).t
    e = AST.Var(v)

    if v_type is FunType:
        fail("Tried to use function name as variable")
    else:
        return set_type(e, v_type)
```

```
typecheck_const(Const c):
    e = Constant(c)

    return set_type(e, type_of_const(c))
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
            new_arg = convert_to(typecheck_exp(arg), param_type)
            converted_args.append(new_arg)

        call_exp = FunctionCall(f, converted_args)
        return set_type(call_exp, ret_type)
    else:
        fail("Tried to use variable as function name")
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
        else:
            fail("Internal error: invalid const type")

        return Symbols.Initial(init_val)
    else:
        fail("Non-constant initializer on static variable")
```

# TACKY

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

We have only one new instruction in TACKY: ZeroExtend.

# TACKYGEN

We only use mk_const with relatively small number, specifically 1, in prefix/postfix increment or decrement.
So it's safe to assume a constant of int32 value and use the const_convert helper function.

```
mk_const(Types.t t, int i):
    as_int = Const.ConstInt(to_int32(i))
    return ConstConvert.const_convert(t, as_int)
```

Now, we have totally 4 types of integers, so there many more combinations of source and destination.
Let's update the get_cast_instruction and emit_cast_expression.

```
emit_cast_expression(cast):
    eval_inner, result = emit_tacky_for_exp(cast.exp)
    src_type = get_type(cast.exp)
    if src_type == cast.target_type:
        return (eval_inner, result)
    else:
        dst_name = create_tmp(cast.target_type)
        dst = Var(dst_name)
        cast_inst = get_cast_instruction(result, dst, src_type, cast.target_type)

        return (
            [...eval_inner, cast_inst],
            dst
        )
```

```
get_cast_instruction(src, dst, src_t, dst_t):
    // Note: assumes src and dst have different types
    if TypeUtils.get_size(dst_t) == TypeUtils.get_size(src_t):
        return Copy(src, dst)
    else if TypeUtils.get_size(dst_t) < TypeUtils.get_size(src_t):
        return Truncate(src, dst)
    else if TypeUtils.is_signed(src_t):
        return SignExtend(src, dst)
    else:
        return ZeroExtend(src, dst)
```

The get_cast_instruction is also used in the emit_compound_expression.

```
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
        cast_lhs_to_tmp = get_cast_instruction(dst, tmp, lhs.type, result_t)
        binary_inst = Tacky.Binary(tacky_op, src1=tmp, src2=rhs, dst=tmp)
        cast_tmp_to_lhs = get_cast_instruction(tmp, dst, result_t, lhs.type)

        insts.append(
            cast_lhs_to_tmp,
            binary_inst,
            cast_tmp_to_lhs,
        )

	return (insts, dst)
```

# Assembly

We add MovZeroExtend and Div instructions.
Note that we also add two more binary operators that are not mentioned in BOOK_NOTES: Shl and Shr.

```
program = Program(top_level*)
asm_type = Longword | Quadword
top_level =
    | Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int alignment, static_init)
instruction =
    | Mov(asm_type, operand src, operand dst)
    | Movsx(operand src, operand dst)
    | MovZeroExtend(operand src, operand dst)
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
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult | And | Or | Xor | Sal | Sar | Shr | Shl
operand = Imm(int64) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP
```

# AssemblySymbols

_No changes_

# CodeGen

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
		case Tacky.Var:
			return Assembly.Pseudo(val.name)
```

```
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong:
        return asm_type.Quadword
    else:
        fail("Internal error: converting function type to assembly")
```

We add a new helper function to get the type of TACKY Var, so it's easier to get asm_type.

```
// returns Types.t
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
asm_type(Tacky.Val operand):
    return convert_type(tacky_type(operand))
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
        case BitwiseAnd:
            return Assembly.And
        case BitwiseOr:
            return Assembly.Or
        case BitwiseXor:
            return Assembly.Xor
		case Divide:
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

A new function to convert bitshift operations, as the opcodes are different for signed and unsigned operands.

```
convert_shift_op(Tacky.BinOp op, bool signed):
    /*
        Note: Sal/Shl are actually the same operations;
        we use different mnemonics for symmetry with Sar/Shr, which are distinct
    */
    if op is Tacky.BitShiftLeft:
        if signed:
            return Assembly.Sal
        else:
            return Assembly.Shl
    else if op is Tacky.BitShiftRight:
        if signed:
            return Assembly.Sar
        else:
            return Assembly.Shr
    else:
        fail("Internal error: not a bitwise shift operation")
```

```
convert_cond_code(Tacky.Binop op, bool signed):
	match op:
		case Equal: return Assembly.CondCode.E
		case NotEqual: return Assembly.CondCode.NE
		case GreaterThan:
            if signed:
                return Assembly.CondCode.G
            else:
                return Assembly.CondCode.A
		case GreaterOrEqual:
            if signed:
                return Assembly.CondCode.GE
            else:
                return Assembly.CondCode.AE
		case LessThan:
            if signed:
                return Assembly.CondCode.L
            else:
                return Assembly.CondCode.B
		case LessOrEqual:
            if signed:
                return Assembly.CondCode.LE
            else:
                return Assembly.CondCode.BE
		default:
			fail("Internal error: not a condition code!")
```

We updated the convert_cond_code to base on the signedness of operands to retrieve the correct condcode in Assembly. In convert_instruction, we update the following:

- Binary instruction: we need to get the signedness for condcode
- Modulo/Division: we need signedness to emit Idiv or Div
- BitShiftLeft/BitShiftRight: use convert_shift_op instead of convert_binop
- ZeroExtend: add a new case for the new instruction

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
                    signed = TypeUtils.is_signed(tacky_type(inst.src1))
					cond_code = convert_cond_code(inst.op, signed)

					return [
						Cmp(src_t, asm_src2, asm_src1),
						Mov(dst_t, zero(), asm_dst),
						SetCC(cond_code, asm_dst),
					]

                (* Division/modulo *)
                case Divide:
                case Mod:
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
        case ZeroExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ MovZeroExtend(asm_src, asm_dst) ]
```

# ReplacePseudo

We extend the pass to replace operands in MovZeroExtend and Div instructions.

```
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

We extend the pass to rewrite MovZeroExtend and Div instructions.

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

# Emit

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
        case Shl: return "shl"
        case Shr: return "shr"
```

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
```

Consequentally, we add the check for Special Logic in Binary for Shl, Shr besides Sal and Sar, extend for cases of Div and MovZeroExtend (error at this point)

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
            else:
                return "\t{show_binary_operator(op)}{suffix(t)} {show_operand(t, src)}, {show_operand(t, dst)}\n"

        case Cmp(t, src, dst):
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

        case MovZeroExtend:
            fail("Internal error: MovZeroExtend should have been removed in instruction rewrite pass")
```

```
emit_zero_init(Initializers.static_init init):
    if init is IntInit:
    if init is UIntInit:
        return "\t.zero 4\n"
    else if init is LongInit:
    else if init is ULongInit:
        return "\t.zero 8\n"
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
```

# Output

From C:

```C
static unsigned long global_var;

int switch_on_uint(unsigned int ui) {
    switch (ui) {
        case 5u:
            return 0;
        // this will be converted to an unsigned int, preserving its value
        case 4294967286l:
            return 1;
        // 2^35 + 10, will be converted to 10
        case 34359738378ul:
            return 2;
        default:
            return 3;
    }
}

int main(void) {
    unsigned u = 2147483647u;
    int res = (u + 2u == 2147483649u);

    static unsigned long shiftcount = 5;
    res >>= shiftcount;

    if (switch_on_uint(5) != 0)
        return res * 12;
    if (switch_on_uint(4294967286) != 1)
        return res + 2ul;
    if (switch_on_uint(10) != 2)
        return res - 3ul;

    return global_var;
}
```

To x64 Assembly on Linux:

```asm
	.bss
	.align 8
global_var:
	.zero 8

	.data
	.align 8
shiftcount.3:
	.quad 5

	.global switch_on_uint
switch_on_uint:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	cmpl	$5, -4(%rbp)
	movl	$0, -8(%rbp)
	sete	-8(%rbp)
	cmpl	$0, -8(%rbp)
	jne	.Lcase.5
	cmpl	$10, -4(%rbp)
	movl	$0, -8(%rbp)
	sete	-8(%rbp)
	cmpl	$0, -8(%rbp)
	jne	.Lcase.7
	cmpl	$4294967286, -4(%rbp)
	movl	$0, -8(%rbp)
	sete	-8(%rbp)
	cmpl	$0, -8(%rbp)
	jne	.Lcase.6
	jmp	.Ldefault.8
	jmp	.Lbreak.switch.4
.Lcase.5:
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lcase.6:
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lcase.7:
	movl	$2, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Ldefault.8:
	movl	$3, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lbreak.switch.4:
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$112, %rsp
	movl	$2147483647, -4(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	addl	$2, -8(%rbp)
	movl	$2147483649, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -12(%rbp)
	sete	-12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -16(%rbp)
	movl	-16(%rbp), %r10d
	movl	%r10d, -16(%rbp)
	movl	shiftcount.3(%rip), %ecx
	sarl	%cl, -16(%rbp)
	movl	$5, -20(%rbp)
	movl	-20(%rbp), %edi
	call	switch_on_uint
	movl	%eax, -24(%rbp)
	movl	$0, %r11d
	cmpl	-24(%rbp), %r11d
	movl	$0, -28(%rbp)
	setne	-28(%rbp)
	cmpl	$0, -28(%rbp)
	je	.Lif_end.12
	movl	-16(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	movl	-32(%rbp), %r11d
	imull	$12, %r11d
	movl	%r11d, -32(%rbp)
	movl	-32(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.12:
	movl	$4294967286, -36(%rbp)
	movl	-36(%rbp), %edi
	call	switch_on_uint
	movl	%eax, -40(%rbp)
	movl	$1, %r11d
	cmpl	-40(%rbp), %r11d
	movl	$0, -44(%rbp)
	setne	-44(%rbp)
	cmpl	$0, -44(%rbp)
	je	.Lif_end.17
	movslq 	-16(%rbp), %r11
	movq	%r11, -56(%rbp)
	movq	-56(%rbp), %r10
	movq	%r10, -64(%rbp)
	addq	$2, -64(%rbp)
	movl	-64(%rbp), %r10d
	movl	%r10d, -68(%rbp)
	movl	-68(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.17:
	movl	$10, -72(%rbp)
	movl	-72(%rbp), %edi
	call	switch_on_uint
	movl	%eax, -76(%rbp)
	movl	$2, %r11d
	cmpl	-76(%rbp), %r11d
	movl	$0, -80(%rbp)
	setne	-80(%rbp)
	cmpl	$0, -80(%rbp)
	je	.Lif_end.24
	movslq 	-16(%rbp), %r11
	movq	%r11, -88(%rbp)
	movq	-88(%rbp), %r10
	movq	%r10, -96(%rbp)
	subq	$3, -96(%rbp)
	movl	-96(%rbp), %r10d
	movl	%r10d, -100(%rbp)
	movl	-100(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.24:
	movl	global_var(%rip), %r10d
	movl	%r10d, -104(%rbp)
	movl	-104(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
