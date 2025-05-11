# Table of Contents

- [Token, Lexer, AST](#token-lexer-ast)
- [Parser](#parser)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly](#assembly)
- [CodeGen](#codegen)
- [ReplacePseudo](#replacepseudo)
- [Instruction Fixup](#instruction-fixup)
- [Emit](#emit)
- [Output](#output)
- [Legacy Extra Credit Integration](#legacy-extra-credit-integration)
  - [Emit](#emit-1)
  - [Output](#output-1)

---

# Token, Lexer, AST

The BOOK_NOTES guide is sufficient to update this file for the chapter.

# Parser

The BOOK_NOTES guide is sufficient to update this file for the chapter.
We will extend our get_precendence, parse_unop, parse_binop, parse_factor and parse_exp.

# TACKY

The BOOK_NOTES guide is sufficient to update this file for the chapter.

# TACKYGEN

We've had a separate helper class as UniqueIds generator. Update the class to have make_label function

```
make_label(label):
	return "{label}.{counter++}"
```

Extend convert_unop and convert_binop

Two short-circuiting operators && and || are handled differently.
Relational operators should be in the same function with other binary operators such as +, -.

```
emit_tacky_for_exp(exp):
	match exp type:
		case AST.Constant: return TACKY.Constant
		case Unary: return emit_unary_expression(exp)
		case Binary:
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_or_expression(exp)
			else:
				return emit_binary_expression(exp)
```

```
emit_and_expression(and_exp):
	eval_1, v1 = emit_tacky_for_exp(and_exp.exp1)
	eval_2, v2 = emit_tacky_for_exp(and_exp.exp2)
	false_label = UniqueIds.make_label('and_false')
	end_label = UniqueIds.make_label('and_end')
	dst_name = Uniqueids.make_temporary()
	dst = Var(dst_name)

	insts = [
		...eval_1,
		JumpIfZero(v1, false_label),
		...eval_2,
		JumpIfZero(v2, false_label),
		Copy(Constant(1), dst),
		Jump(end_label)
		Label(false_label),
		Copy(Contant(0), dst),
		Label(end_label)
	]

	return insts
```

```
emit_or_expression():
	eval_1, v1 = emit_tacky_for_exp(and_exp.exp1)
	eval_2, v2 = emit_tacky_for_exp(and_exp.exp2)
	true_label = UniqueIds.make_label('or_true')
	end_label = UniqueIds.make_label('or_end')
	dst_name = Uniqueids.make_temporary()
	dst = Var(dst_name)

	insts = [
		...eval_1,
		JumpIfNotZero(v1, true_label),
		...eval_2,
		JumpIfNotZero(v2, true_label),
		Copy(Constant(0), dst),
		Jump(end_label)
		Label(true_label),
		Copy(Contant(1), dst),
		Label(end_label)
	]

	return insts
```

# Assembly

The BOOK_NOTES guide is sufficient to update this file for the chapter.

# CodeGen

Though Not is a unary operator during the Parse and Tacky stages, but in Assembly, Not operator in high-level languages is like comparing the value with 0.
We do need to handle Not differently in this stage.

```
convert_unop(op):
	match op:
		--snip--
		case Not:
			fail("Internal error: Cannot convert TACKY Not directly!")
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
		case Mod:
		case Equal:
		case NotEqual:
		case GreaterThan:
		case GreaterOrEqual:
		case LessThan:
		case LessOrEqual:
			fail("Internal error: Not a binary assembly instruction!")
```

```
convert_cond_code(Tacky.Binop op):
	match op:
		case Equal: return Assembly.CondCode.E
		case NotEqual: return Assembly.CondCode.NE
		case GreaterThan: return Assembly.CondCode.G
		case GreaterOrEqual: return Assembly.CondCode.GE
		case LessThan: return Assembly.CondCode.L
		case LessOrEqual: return Assembly.CondCode.LE
		default:
			fail("Internal error: not a condition code!")
```

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
```

# ReplacePseudo

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
		case Ret:
		case Cdq:
		case Label:
		case JmpCC:
		case Jmp:
			return (state, inst)
		case AllocateStack:
		--snip--
```

# Instruction Fixup

```
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	--snip--
	case Cmp:
		(* Both operands of cmp can't be in memory *)
		if both src and dst of cmp are memory addresses:
			return [
				Mov(inst.src, Reg(R10)),
				Cmp(Reg(10), inst.dst)
			]

		else if dst of cmp is an immediate:
			return [
				Mov(Imm(inst.dst.value), Reg(R11)),
				Cmp(inst.src, Reg(R11)),
			]
	default:
		--snip--
```

# Emit

We need a function to emit 1-byte variants of registers for the operand of SetCC.

```
show_byte_operand(operand):
	match operand:
		case Reg(AX): return '%al'
		case Reg(DX): return '%dl'
		case Reg(R10): return '%r10b'
		case Reg(R11): return '%r11b'
		default:
			return show_operand(operand)
```

```
show_local_label(string name):
	if on OS_X:
		return "L" + name
	else:
		return ".L" + name

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
			return "\tcmpl, {show_operand(inst.src)}, {show_operand(inst.dst)}\n"
		Idiv:
			--snip--
		Cdq:
			--snip--
		Jmp:
			return "\tjmp {show_local_label(inst.target)}\n"
		JmpCC:
			return "\tj{show_cond_code(inst.cond_code)} {show_local_label(inst.target)}\n"
		SetCC:
			return "\tset{show_cond_code(inst.cond_code)} {show_byte_operand(inst.operand)}\n"
		Label:
			return "{show_local_label(inst.name)}:\n"
		--snip--
```

# Output

From C:

```C
int main(void) {
    return (0 == 0 && 3 == 2 + 1 > 1) + 1;
}
```

To assembly:

```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	24, %rsp
	movl	$0, %r11d
	cmpl	$0, %r11d
	movl	$0, -4(%rbp)
	sete	-4(%rbp)
	cmpl	$0, -4(%rbp)
	je	.Land_false.4
	movl	$2, -8(%rbp)
	addl	$1, -8(%rbp)
	movl	$1, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -12(%rbp)
	setg	-12(%rbp)
	cmpl	$3, -12(%rbp)
	movl	$0, -16(%rbp)
	sete	-16(%rbp)
	cmpl	$0, -16(%rbp)
	je	.Land_false.4
	movl	$1, -20(%rbp)
	jmp	.Land_end.5
.Land_false.4:
	movl	$0, -20(%rbp)
.Land_end.5:
	movl	-20(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	addl	$1, -24(%rbp)
	movl	-24(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits

```

# Legacy Extra Credit Integration

The following changes are to make our code compatible with **Extra Credit** sections of previous chapters.

## Emit

In Extra Credit in Chapter 3, we treat %cl in bitshift emission as a special case.
But now, as we already support 1-byte register in chapter 4, we can simply use the convert_byte_operand to emit %cl for Reg(CX).
So we don't need to check if it's CX register along with Sal | Sar operator anymore.

## Output

From C:

```C
int main(void) {
    return 20 & 7 >> 4 <= 3 ^ 7 < 5 | 7 != 5;
}
```

To assembly:

```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	28, %rsp
	movl	$7, -4(%rbp)
	sarl	$4, -4(%rbp)
	movl	$3, %r11d
	cmpl	-4(%rbp), %r11d
	movl	$0, -8(%rbp)
	setle	-8(%rbp)
	movl	$20, -12(%rbp)
	movl	-8(%rbp), %r10d
	andl	%r10d, -12(%rbp)
	movl	$5, %r11d
	cmpl	$7, %r11d
	movl	$0, -16(%rbp)
	setl	-16(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-16(%rbp), %r10d
	xorl	%r10d, -20(%rbp)
	movl	$5, %r11d
	cmpl	$7, %r11d
	movl	$0, -24(%rbp)
	setne	-24(%rbp)
	movl	-20(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	-24(%rbp), %r10d
	orl 	%r10d, -28(%rbp)
	movl	-28(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
