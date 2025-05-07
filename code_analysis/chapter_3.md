# Table of Contents
- [Token](#token)
- [Lexer](#lexer)
- [AST](#ast)
- [Parser](#parser)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly](#assembly)
- [CodeGen](#codegen)
- [ReplacePseudo](#replacepseudo)
- [Instruction Fixup](#instruction-fixup)
- [Emit](#emit)
- [Output](#output)
- [Extra Credit: Bitwise Operators](#extra-credit-bitwise-operators)
  - [Token & Lexer](#token-and-lexer)
  - [AST](#ast-1)
  - [Parser](#parser-1)
  - [Tacky](#tacky-1)
  - [TackyGen](#tackygen-1)
  - [Assembly](#assembly-1)
  - [CodeGen](#codegen-1)
  - [Replace Pseudo](#replace-pseudo)
  - [Instruction Fixup](#instruction-fixup-1)
  - [Emit](#emit-1)
  - [Output](#output-1)

---

# Token
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# Lexer
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# AST
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# Parser
```
get_precedence(TokenType.Binop op):
	match op:
		case Star:
		case Slash
		case Percent:
			return 50
		case Plus:
		case Hyphen:
			return 45
		default:
			return None

parse_binop(tokens):
	token = take_tokens(tokens)
	match token type:
		case Plus:
			return AST.Add
		case Hyphen:
			return AST.Subtract
		case Star:
			return AST.Multiply
		case Slash:
			return AST.Divide
		case Percent:
			return AST.Mod
		default:
			raise_error("expected a binary operator but got other!")
```

We change the function name parse_exp into parse_factor, and recreate the parse_exp to support precedence climbing. Please reference to BOOK_NOTES.


# TACKY
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# TACKYGEN
```
convert_binop(Ast.Binop op):
	match op:
		case Add:
			return Tacky.Add 
		case Subtract:
			return Tacky.Subtract
		case Multiply:
			return Tacky.Multiply
		case Divide:
			return Tacky.Divide
		case Mod:
			return Tacky.Mod
```

# Assembly
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# CodeGen
Please referecence to the BOOK_NOTES for updates.
Change the name convert_op into convert_unop, as we also have a function to convert_binop.


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
			fail("Internal error: shouldn't handle division like other binary operators")
```

```
convert_instruction(Tacky.Instruction inst):
	match inst.type:
		--snip--
		case Binary:
			asm_src1 = convert_val(inst.src1)
			asm_src2 = convert_val(inst.src2)
			asm_dst = convert_val(inst.dst)
			
			match inst.op:
				(* Division/modulo *)
				case  Divide case Mod:
					result_reg = op == Divide ? AX : DX
					
					return [
						Mov(asm_src1, Reg(AX)),
						Cdq,
						Idiv(asm_src2),
						Mov(Reg(result_reg), asm_dst)
					]
				 (* Addition/subtraction/mutliplication*)
				default:
					asm_op = convert_binop(inst.op)
					
					return [
						Mov(asm_src1, asm_dst),
						Binary(asm_op, asm_src2, asm_dst)
					]
					
		
```

# ReplacePseudo
```
replace_pseudos_in_instruction(Assembly.Instruction inst, state):
	match inst.type:
		--snip--
		case Binary:
			state1, new_src = replace_operand(inst.src, state)
			state2, new_dst = replace_operand(inst.dst, state1)
			new_binary = Binary(inst.op, new_src, new_dst)
			
			return (state2, new_binary)
		case Idiv:
			state1, new_operand = replace_operand(inst.operand, state)
			return (state1, new_operand)
		case Ret:
		case Cdq:
			return (state, inst)
		case AllocateStack:
		--snip--
```

# Instruction Fixup
```
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	case Mov:
		(* Mov can't move a value from one memory address to another *)
		if both `src` and `dst` are stack operands:
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
		if inst.op is Add Or Sub and both operands are stacks:
			return [
				Mov(inst.src, Reg(R10)),
				Binary(inst.op, Reg(R10), inst.dst)
			]
		 (* Destination of Mult can't be in memory *)
		else if inst.op is Mult and dst is Stack:
			return [
				Mov(inst.dst, Reg(R11)),
				Binary(inst.op, inst.src, Reg(R11)),
				Mov(Reg(R11), inst.dst)
			]
		else:
			return [ inst ]
	default:
		return [ inst ]
```

# Emit
Update show_operand to support DX and R11 registers.
Create function show_binary_operator to convert Add, Sub and Mult
Also, in emit_instruction, add cases for Binary, Idiv and Cdq.

# Output
From C:
```C
int main(void) {
    return 1 * 2 - 3 * -(4 + 5) / 7;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	24, %rsp
	movl	$1, -4(%rbp)
	movl	-4(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -4(%rbp)
	movl	$4, -8(%rbp)
	addl	$5, -8(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	negl	-12(%rbp)
	movl	$3, -16(%rbp)
	movl	-16(%rbp), %r11d
	imull	-12(%rbp), %r11d
	movl	%r11d, -16(%rbp)
	movl	-16(%rbp), %eax
	cdq
	movl	$7, %r10d
	idivl	%r10d
	movl	%eax, -20(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-20(%rbp), %r10d
	subl	%r10d, -24(%rbp)
	movl	-24(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```

# Extra Credit: Bitwise Operators

## Token and Lexer:

| Token | Regular expression |
| ------- | -------------------- |
| Ampersand | & |
| Caret | ^ |
| Pipe | \| |
| DoubleLeftBracket | << |
| DoubleRightBracket | >> |

## AST:
Add to the enum of binary operators to have:
- BitwiseAnd
- BitwiseOr
- BitwiseXor
- BitShiftLeft
- BitShiftRight

## Parser
Update the get_precedence function to support this table

| Operator | Precedence |
| -------- | ---------- | 
| \* | 50 |
| / | 50 |
| % | 50 |
| + | 45 |
| - | 45 |
| << | 40 |
| >> | 40 |
| & | 25 |
| ^ | 20 |
| \| | 15 |

Update parse_binop to also turn the tokens into corresponding AST binary operators.

## Tacky

Add to the enum of binary operators to have:
- BitwiseAnd
- BitwiseOr
- BitwiseXor
- BitShiftLeft
- BitShiftRight

## TackyGen
Update convert_binop to support new binary operators. The names are similar so it's not much of a trouble.

## Assembly
Add to the enum of binary opertors:
- And
- Or 
- Xor
- Sal
- Sar

## CodeGen
Update convert_binop to support new binary operators.

```
convert_instruction(Tacky.Instruction inst):
	match inst.type:
		--snip--
		case Binary:
			asm_src1 = convert_val(inst.src1)
			asm_src2 = convert_val(inst.src2)
			asm_dst = convert_val(inst.dst)
			
			match inst.op:
				(* Division/modulo *)
				case  Divide case Mod:
					--snip--
				case BiftShiftLeft:
				case BiftShiftRight:
					asm_op = convert_binop(inst.op)
					
					match type of asm_src2:
						case Imm:
							return [
								Mov(asm_src1, asm_dst),
								Binary(asm_op, asm_src2, asm_dst)
							]
						default:
							return [
								Mov(asm_src1, asm_dst),
								Mov(asm_src2, Reg(CX)),
								Binary(asm_op, Reg(CX), asm_dst)
							]
				 (* Addition/subtraction/mutliplication*)
				default:
					--snip--
```

## Replace Pseudo: 
No changes

## Instruction Fixup:
Assembly instructions of And, Or and Xor cannot operate on operands with both being memory addresses.
So extends the check from Add/Sub to include And/Or/Xor.

## Emit
Update show_operand to show rcx register.
Also, show_binary_operator should also match:
- And -> andl
- Or -> orl
- Xor -> xorl
- Sal -> sall
- Sar -> sarl

And in emit_instruction, for `sal` and `sar` where the source is `CX` register, we need to handle them separately to print out `%cl`. 

## Output
From C:
```C
int main(void) {
    return 80 >> 2 | 1 ^ 5 & 7 << 1;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	20, %rsp
	movl	$80, -4(%rbp)
	sarl	$2, -4(%rbp)
	movl	$7, -8(%rbp)
	sall	$1, -8(%rbp)
	movl	$5, -12(%rbp)
	movl	-8(%rbp), %r10d
	andl	%r10d, -12(%rbp)
	movl	$1, -16(%rbp)
	movl	-12(%rbp), %r10d
	xorl	%r10d, -16(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-16(%rbp), %r10d
	orl 	%r10d, -20(%rbp)
	movl	-20(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
