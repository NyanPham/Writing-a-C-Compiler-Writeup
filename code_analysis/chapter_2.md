# Compiler Driver
We add one more stage in the compiler driver, so let's add one more command line option: `compiler.exe program.c --tacky`

# Source code
In structure of the code, we create a separate folder for backend. While we do not dedicate another folder for the frontend,
any source files outside the backend folder are considered of frontend.

The backend folder includes: CodeGen, ReplacePseudo and InstructionFixup

## Token
BOOK_NOTES references are sufficient for this file in this chapter.
- `-` -> hyphen
- `--` -> double hyphen
- `~` -> tilde

## Lexer
BOOK_NOTES references are sufficient for this file in this chapter.
Treat them the same as other tokens such as semicolon, parentheses.

We need to implement the `peek` and `npeek` functions in the lexer. 
This is relatively simple, in both functions:
- Before tokenize the input to get the next token, save the current position.
- Before return the token or list of tokens, restore the position.

## AST
BOOK_NOTES references are sufficient for this file in this chapter.

## Parser
The core change is in the `parseExp` function, and the BOOK_NOTES captures this very well. 

```
parse_unop():
	token = take_token()
	match token's type:
		case TokenType.Tilde:
			return AST.Complement
		case TokenType.Hyphen:
			return AST.Negate
		default: 
			return raise_error("a unary operator", token's type)
```

## TACKY
BOOK_NOTES references are sufficient for this file.
We can follow the same strategy as Assembly constructs.

## TACKYGEN
The main function is `emit_tacky_for_exp` (emit_tacky), whose pseudocode is already provided in the BOOK_NOTES.
There are 2 ways to actually how to emit instructions and return the `dst`.
- [ ] Pass instructions list into recursive call and append in it, return `dst` only. Or 
- [x] In each call, return list of instructions to evaluate expression a and resulting TACKY


```
convert_op(AST.Unop op):
	match op:
		case AST.Complement:
			return Tacky.Complement
		case AST.Negate:
			return Tacky.Negate

emit_tacky_for_statement(AST.Statement stmt): 
	instructions, v = emit_tacky_for_exp(stmt.val) // Right now we only support return statement, which has val as an expression
	instructions.push(Tacky.Return(v))
	return instructions

emit_tacky_for_function(AST.Function fn_def):
	instructions, _ = emit_tacky_for_statement(fn_def.body)
	return Function(fn_def.name, instructions)

gen(AST.Program prog):
	return Tacky.Program(emit_tacky_for_function(prog.fn_def))
```

## Assembly
In chapter 1, we only have one type of Register, which is then emitted into `ax` register. 
Now, we need to add support for another register, r10.

In chapter 1:
```
enum NodeType = Program, Function, Imm, Register, Mov, Ret

type Operand = Imm | Register
--snip--
```

Let's simplify Register into Reg, and create enum for different registers. 
```
enum NodeType = Program, Function, Imm, Reg, Mov, Ret

type Reg = AX | R10
type Operand = Imm | Reg | Pseudo | Stack
```

Add more constructs like Unary and AllocateStack as in BOOK_NOTES.

## CodeGen

```
convert_val(Tacky.Val val):
	match val.type:
		case Tacky.Constant:
			return Assembly.Imm(val.value)
		case Tacky.Var:
			return Assembly.Pseudo(val.name)

convert_op(Tacky.Operator op):
	match op:
		case Tacky.Complement: 
			return Assembly.Not
		case Tacky.Negate:
			return Assembly.Neg


convert_instruction(Tacky.Instruction inst):
	match inst.type:
		case Tacky.Return:
			asm_val = convert_val(inst.val)
			return [
				Assembly.Mov(asm_val, Reg(AX)),
				Assembly.Ret
			]
		case Tacky.Unary:
			asm_op = convert_op(inst.op)
			asm_src = convert_val(inst.src)
			asm_dst = convert_val(inst.dst)
			
			return [
				Assembly.Mov(asm_src, asm_dst),
				Assembly.Unary(asm_op, asm_dst)
			]

convert_function(Tacky.Function fn_def):
	instructions = []
	
	for inst in fn_def.body:
		instructions.push(convert_instruction(inst))
	
	return Assembly.Function(fn_def.name, instructions)

gen(Tacky.Program prog):
	return Assembly.Program(convert_function(prog.fn_def))
```

## ReplacePseudo

We'll need a map to keep track of what stack slots we've assigned

```
type replacement_state = {
	current_offset: int; // Last used stack slot
	offset_map: Map<name, int>;
}

replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
			if state.offset_map.has(operand.name):
				return (state, state.offset_map.get(operand.name))
			else:
				new_offset = state.current_offset - 4
				new_state = {
					current_offset: new_offset,
					offset_map: state.offset_map.add(operand.name, new_offset)
				}
				
				return (new_state, Assembly.Stack(new_offset))
		default:
			return (state, operand)

replace_pseudos_in_instruction(Assembly.Instruction inst, state):
	match inst.type:
		case Mov:
			state1, new_src = replace_operand(inst.src, state)
			state2, new_dst = replace_operand(inst.dst, state1)
			new_mov = Mov(new_src, new_dst)
			
			return (state_2, new_mov)
		case Unary:
			state1, new_dst = replace_operand(inst.dst, state)
			new_unary = Unary(inst.op, new_dst)
			
			return (state_1, new_unary)
		case Ret:	
			return (state, inst)
		case AllocateStack:
			fail("Internal error: AllocateStack shouldn't be present at this point")

init_state = { current_offset: 0, offset_map = empty_map }

replace_pseudos_in_function(Assembly.Function fn):	
	curr_state = init_state
	fixed_instructions = []
	
	for inst in fn.instrustions:
		new_state, new_inst = replace_pseudos_in_instruction(inst, curr_state)
		curr_state = new_state
		fixed_instructions.push(new_inst)
	
	new_fn = Assembly.Function(fn.name, fixed_instructions)
	
	return new_fn, curr_state.current_offset 

replace_pseudos(Assembly.Program prog):
	fixed_fn_def, last_stack_slot = replace_pseudos_in_function(prog.fn_def)
	
	return Assembly.Program(fixed_fn_def), last_stack_slot
```

## Instruction Fixup

```
fixup_instruction(Assembly.Instruction inst):
	match inst.type:
	case Mov:
		if both `src` and `dst` are stack operands:
			return [
				Mov(src, Reg(R10)),
				Mov(Reg(R10), dst)
			]
		else:
			return [ inst ] 
	default:
		return [ other ]


fixup_function(Assembly.Function fn_def, int last_stack_slot):
	alloc_stack = AllocateStack(-last_stack_slot)
	
	return Assembly.Function(fn_def.name, alloc_stack...fn_def.instructions)


fixup_program(Assembly.Program prog, int last_stack_slot):
	return Assembly.Program(fixup_function(prog.fn_def, last_stack_slot))
```

## Emit

### Update show_operand
Previous
```
show_operand(Assembly.Operand operand):
	match operand.type:
		case Register:
			return '%eax'
		case Imm:
			return '$' + operand.val
```
As we added register R10, let's update the function

Current:
```
show_operand(Assembly.Operand operand):
	match operand.type:
		case Reg(AX):
			return "%eax"
		case Reg(R10):
			return "%r10d"
		case Imm:
			return "${operand.val}"
		case Stack:
			return "{operand.offset}(%rbp)"
		case Pseudo:
			return "{operand.name}" // For debugging only
```

### Show operator
We added unary construct, so we need a separate converter function for it
```
show_unary_operator(Assembly.UnaryOperator op):
	match op:
		case Assembly.Neg:
			return "negl"
		case Assembly.Not:
			return "notl"
```

### Update emit_instruction
Previous
```
emit_instruction(Assembly.Instruction inst):
	match inst.type:
		case Mov:
			return '\tmovl {show_operand(inst.src)}, {show_operand(inst.dst)}\n'
		case Ret:
			return '\tret\n'
```

Updated
```
emit_instruction(Assembly.Instruction inst):
	match inst.type:
		case Mov:
			return '\tmovl {show_operand(inst.src)}, {show_operand(inst.dst)}\n'
		case Unary:
			return '\t{show_unary_operator(inst.op)} {show_operand(inst.dst)}\n'
		case AllocateStack:
			return '\tsubq	${inst.bytes}, %rsp\n'
		case Ret:
			return """
				movq 	%rbp, %rsp
				popq	%rbp
				ret
			"""
```

### Update emit_function
Previous:
```
emit_function(Assembly.Function func):
	label = show_label(func.name)
	return """
	.globl {label}
{label}:
	{emit_instruction(inst) for inst in func.instructions}
"""
```

Updated:
```
emit_function(Assembly.Function func):
	label = show_label(func.name)
	
	return """
	.globl {label}
{label}:
	pushq	%rbp
	movq	%rsp, %rbp
	
	{emit_instruction(inst) for inst in func.instructions}
"""
```

## Output
From C:
```C
int main(void) {
    return ~(-2);
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	8, %rsp
	movl	$2, -4(%rbp)
	negl	-4(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	notl	-8(%rbp)
	movl	-8(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```

