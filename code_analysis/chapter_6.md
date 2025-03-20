# Table of Contents
- [Token, Lexer, AST](#token-lexer-ast)
- [Parser](#parser)
- [Variable Resolution](#varresolution)
- [TACKY](#tacky)
- [TackyGen](#tackygen)
- [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit)
- [Output](#output)
---

# Token, Lexer, AST
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# Parser
Update our precedence or "?" and parse_exp. Both are provided in BOOK_NOTES.

We add one new function to parse conditional expressions.
```
parse_conditional_middle(tokens):
	expect(TokenType.QuestionMark, tokens)
	e = parse_exp(0, tokens)
	expect(TokenType.Colon, tokens)
	
	return e
```

Update our **parse_statement** for **If Statements**.
```
parse_statement(tokens):
	next_token = peek(tokens)
	match next_token type:
		case "return":
			--snip--
		case ";":
			-snip--
		case "if":
			take_token(tokens)
			expect(TokenType.OpenParen, tokens)
			condition = parse_exp(0, tokens)
			expect(TokenType.CloseParent, tokens)
			then_clause = parse_statement(tokens)
			else_clause = none
			
			if peek_token(tokens) is "else":
				take_token(tokens)
				else_clause = parse_statement(tokens)
			
			return AST.If(condition, then_clause, else_clause)
			
		default:
			-snip--
```

# VarResolution

```
resolve_exp(exp, var_map):
	match e with
		case Assignment(left, right):
			--snip--
		case Var(v):
			--snip--
		case Unary(op, e):
			--snip--
		case Binary(op, e1, e2):
			--snip--
		case Conditional(condition, then_result, else_result):
			return Condition(resolve_exp(condition, var_map), resolve_exp(then_result, var_map), resolve_exp(else_result, var_map))
		case Constant:
			--snip--
		default:
			--snip--
```

```
resolve_statement(statement, var_map):
	match statement with
		case Return(e) -> return Return(resolve_exp(e, variable_map))
		case Expression(e) -> return Expression(resolve_exp(e, variable_map))
		case If(condition, then_clause, else_clause):
			return If(
				resolve_exp(condition, var_map),
				resolve_statement(then_clause, var_map),
				else_clause != None ? resolve_statement(else_clause, var_map) : None
			)
		case Null -> return Null
```

# TACKY
No changes for our TACKY AST!

# TACKYGEN
```
emit_tacky_for_exp(exp):
	match exp type:
		case AST.Constant: return TACKY.Constant
		case AST.Var: return TACKY.Var
		case Unary: return emit_unary_expression(exp)
		case Binary: 
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_tacky_for_exp(exp)
			else:
				return emit_binary_expression(exp)
		case Assignment:
			if exp.left is not AST.Var:
				fail("Internal Error: bad lvalue!") // We also checked in the Semantic Analysis that Left is a Var
		
			rhs_instructions, rhs_result = emit_tacky_for_exp(exp.right)
			insts = [
				...rhs_instructions,
				Tacky.Copy(rhs_result, Tacky.Var(exp.left.name)) 
			]
			
			return insts
		case Optional:
			return emit_conditional_expression(exp)
```

```
emit_conditional_expression(conditional_exp):
	eval_cond, v = emit_tacky_for_exp(conditional_exp.condition)
	eval_v1, v1 = emit_tacky_for_exp(conditional_exp.e1)
	eval_v2, v2 = emit_tacky_for_exp(conditional_exp.e2)
	
	e2_label = UniqueIds.make_label("conditional_else")
	end_label = UniqueIds.make_label("conditional_end")
	dst_name = UniqueIds.make_temporary()
	dst = Tacky.Var(dst_name)
	insts = [
		...eval_cond,
		JumpIfZero(v, e2_label),
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
emit_tacky_for_statement(statement):
	match statement type:
		case Return:
			eval_exp, v = emit_tacky_for_exp(statement.value)
			eval_exp.append(Tacky.Return(v))
			
			return eval_exp
		case Expression:
			eval_exp, v = emit_tacky_for_exp(statement.exp) // Discard the evaluated v destination, we only care the side effect
			return eval_exp
		case If:
			return emit_tacky_for_if_statement(statement)
		case Null:
			return []
```

```
emit_tacky_for_if_statement(statement):
	if statement has no else_clause:
		end_label = UniqueIds.make_label("if_end")
		eval_condition, c = emit_tacky_for_exp(statement.condition)
		
		return [
			...eval_condition,
			JumpIfZero(c, end_label),
			...emit_tacky_for_statement(statement.then_clause),
			Label(end_label)
		]
	else:
		else_label = UniqueIds.make_label("else")
		end_label = UniqueIds.make_label("if_end")
		eval_condition, c = emit_tacky_for_exp(statement.condition)
		
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

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
We don't make any modifications, as no constructs in the TACKY AST were altered.

# Output
From C:
```C
int main(void) {
    int a = 0;
    if (!a)
        if (3 / 4)
            a = 3;
        else
            a = a ? 4 : 5;

    return a;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	16, %rsp
	movl	$0, -4(%rbp)
	cmpl	$0, -4(%rbp)
	movl	$0, -8(%rbp)
	sete	-8(%rbp)
	cmpl	$0, -8(%rbp)
	je	.Lif_end.1
	movl	$3, %eax
	cdq
	movl	$4, %r10d
	idivl	%r10d
	movl	%eax, -12(%rbp)
	cmpl	$0, -12(%rbp)
	je	.Lif_else.3
	movl	$3, -4(%rbp)
	jmp	.Lif_end.4
.Lif_else.3:
	cmpl	$0, -4(%rbp)
	je	.Lconditional_else.6
	movl	$4, -16(%rbp)
	jmp	.Lconditional_end.7
.Lconditional_else.6:
	movl	$5, -16(%rbp)
.Lconditional_end.7:
	movl	-16(%rbp), %r10d
	movl	%r10d, -4(%rbp)
.Lif_end.4:
.Lif_end.1:
	movl	-4(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```