# Table of Contents
- [Token, Lexer](#token-lexer)
- [AST](#ast)
- [Parser](#parser)
- [Variable Resolution](#varresolution)
- [TACKY](#tacky)
- [TackyGen](#tackygen)
- [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit)
- [Output](#output)
- [Legacy Extra Credit Integration](#legacy-extra-credit-integration)
  - [ValidateLabels](#validatelabels)
  - [Output](#output-1)

---

# Token, Lexer
*We only extend our grammar; no new tokens need to be recognized.*

# AST
<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, <strong>block body</strong>)
block_item = S(statement) | D(declaration)
<strong>block = Block(block_item*)</strong>
declaration = Declaration(identifier name, exp? init)
statement = Return(exp) 
	| Expression(exp) 
	| If(exp condition, statement then, statement? else)
	<strong>| Compound(block)</strong>
	| Null 
exp = Constant(int) 
	| Var(identifier) 
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual</pre></code>

# Parser

```
parse_statement(tokens):
	next_token = peek(tokens)
	match next_token type:
		case "return":
			--snip--
		case ";":
			--snip--
		case "if":
			--snip--
		case "{":
			return AST.Compound(parse_block(tokens))
		default:
			-snip--
```

```
parse_block(tokens):
	expect("{", tokens)
	block = []
		
	while peekToken(tokens) is not "}":
		next_block_item = parse_block_item(tokens)
		block.append(next_block_item)
	
	expect("}", tokens)
	
	return block
```

```
parse_function_definition(tokens):
	// parse everything up through the close paren.
	--snip--
	function_body = parse_block(tokens)
	return Function(name, function_body)
```

# VarResolution

```
type VarEntry {
	string unique_name,
	bool from_current_block,
}

copy_variable_map(var_map):
	new_map = clone(var_map)
	
	for var_entry in new_map:
		var_entry.from_current_block = false
	
	return new_map
```

We updated resolve_exp for the Var case to reflect the changes in our map entry structure.
```
resolve_exp(exp, var_map):
	match e with
		case Assignment(left, right):
			if left is not a Var node:
				fail("Invalid lvalue")
			return Assignment(resolve_exp(left, var_map), resolve_exp(right, var_map))
		case Var(v):
			if v is in var_map:
				return Var(var_map.get(v)).unique_name
			else:
				fail("Undeclared variable!")
		case Unary(op, e):
			return Unary(op, resolve_exp(e, var_map))
		case Binary(op, e1, e2):
			return Binary(op, resolve_exp(e1, var_map), resolve_exp(e2, var_map))
		case Constant:
			return exp
		default:
			fail("Internal error: unknown expression")
```

```
resolve_declaration(Declaration(name, init), var_map):
	if name is in var_map and var_map.get(name).from_current_block:
		fail("Duplicate variable declaration!")
	unique_name = make_temporary()
	var_map.add(name, MapEntry(new_name=unique_name, from_current_block=true))
	if init is not null:
		init = resolve_exp(init, var_map)
	return Declaration(unique_name, init)
```

```
resolve_statement(statement, var_map):
	match statement with
	| Return(e) -> return Return(resolve_exp(e, var_map))
	| Expression(e) -> return Expression(resolve_exp(e, var_map))
	| Compound(block) -> 
		new_var_map = copy_variable_map(var_map)
		return Compound(resolve_block(block, var_map)
	| Null -> return Null
```

```
resolve_block(block, var_map):
	resolved_block = []
	
	for block_item in block:
		resolved_item = resolve_block_item(block_item, var_map)
		resolved_block.append(resolved_item)
		
	return resolved_block
```

```
resolve_function_def(Function fun):
	var_map = {} // Empty map
	resolved_body = resolve_block(fun.block, var_map)
	
	return Function(fun.name, resolved_body)
```

# TACKY
*No changes for our TACKY AST!*

# TACKYGEN

Add a new case for Compound
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
		case Compound:
			insts = []
			
			for block_item in statement.block:
				emit_insts = emit_tacky_for_block_item(block_item)
				insts.append(...emit_insts)
			
			return insts
		case Null:
			return []
```

```
emit_tacky_for_function(AST.Function fun_def):
	insts = []
	
	for block_item in fun_def.block:
		emit_insts = emit_tacky_for_block_item(block_item)
		insts.append(...emit_insts)
	
	extra_return = Tacky.Return(Constant(0))
	insts.append(extra_return)
	
	return Tacky.Function(fun_def.name, insts)
```

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
*We don't make any modifications, as no constructs in the TACKY AST were altered.*

# Output
From C:
```C
int main(void) {
    int a = 0;
    if (a) {
        int b = 2;
        return b;
    } else {
        int c = 3;
        int a = c + 2;
        if (a < c) {
            return !a;
        } else {
            return 5;
        }
    }
    return a;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	28, %rsp
	movl	$0, -4(%rbp)
	cmpl	$0, -4(%rbp)
	je	.Lif_else.4
	movl	$2, -8(%rbp)
	movl	-8(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	jmp	.Lif_end.5
.Lif_else.4:
	movl	$3, -12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -16(%rbp)
	addl	$2, -16(%rbp)
	movl	-16(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-20(%rbp), %r10d
	cmpl	%r10d, -12(%rbp)
	movl	$0, -24(%rbp)
	setl	-24(%rbp)
	cmpl	$0, -24(%rbp)
	je	.Lif_else.7
	cmpl	$0, -20(%rbp)
	movl	$0, -28(%rbp)
	sete	-28(%rbp)
	movl	-28(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	jmp	.Lif_end.8
.Lif_else.7:
	movl	$5, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.8:
.Lif_end.5:
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

# Legacy Extra Credit Integration
The following changes are to make our code compatible with **Extra Credit** sections of previous chapters.

## ValidateLabels
```
collect_labels_from_statement(Set<string> defined, Set<string> used, statement):
	match statement type:
		case Goto:
			used.add(statement.lbl)
		case LabeledStatement:
			if defined already has statement.lbl:
				fail("Duplicate label: " + statement.lbl)
			defined.add(statement.lbl)
			collect_labels_from_statement(defined, used, statement.statement)
		case If:
			collect_labels_from_statement(defined, used,  statement.thenClause)
			if statement has elseClause:
				collect_labels_from_statement(statement.elseClause)
		case Compound:
			for block_item in statement.block:
				collect_labels_from_block_item(defined, used, block_item)
		default:
			return
```

## Output
From C:
```C
int main(void) {
    int a = 5;
    if (a > 4) {
        a -= 4;
        int a = 5;
        
        if (a >>= 4) {
            goto end;
            a -= 4;
        }
    }
end:
    return a;
}
```

To assembly:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	12, %rsp
	movl	$5, -4(%rbp)
	movl	$4, %r11d
	cmpl	-4(%rbp), %r11d
	movl	$0, -8(%rbp)
	setg	-8(%rbp)
	cmpl	$0, -8(%rbp)
	je	.Lif_end.2
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	subl	$4, -4(%rbp)
	movl	$5, -12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	sarl	$4, -12(%rbp)
	cmpl	$0, -12(%rbp)
	je	.Lif_end.4
	jmp	.Lend
	movl	-12(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	subl	$4, -12(%rbp)
.Lif_end.4:
.Lif_end.2:
.Lend:
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