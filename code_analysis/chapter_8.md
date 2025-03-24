# Table of Contents
- [Token, Lexer, AST](#token-lexer-ast)
- [Parser](#parser)
- [Variable Resolution](#varresolution)
- [Loop Labeling](#looplabeling)
- [TACKY](#tacky)
- [TackyGen](#tackygen)
- [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit)
- [Output](#output)

---

# Token, Lexer, AST
*Please consult the BOOK_NOTES for guidance on Token, Lexer, and AST, as they provide sufficient details.*

# Parser
```
parse_optional_exp(delim, tokens)
    if peek_token(tokens) is delim:
        take_token(tokens)
        return null
    else:
        e = parse_exp(0, tokens)
        expect(delim, tokens)
        return e
```

```
parse_for_init(tokens):
    if peek_tokens(tokens) is "int":
        return AST.InitDecl(parse_declaration(tokens))
    else:
        return AST.InitExp(parse_optional_exp(";", tokens))
```

We have *parse_optional_exp*, so it's easier now to parse Null and Expression Statement using it. Let's rewrite the stars:

```
parse_statement(tokens):
	token1, token2 = npeek(2, tokens)
	match token1 type:
		case "return":
			--snip--
		case "if":
            --snip--
		case "goto":
            --snip--
		case Identifier:
			--snip--
		case "break":
            take_token(tokens)
            expect(";", tokens)

            return Break(id="")
        case "continue":
            take_token(tokens)
            expect(";", tokens)

            return Continue(id="")
        case "while":   
            return parse_while_loop(tokens)
        case "do":
            return parse_do_loop(tokens)
        case "for":
            return parse_for_loop(tokens)
        default:
            // For Expression statement and null statement
            opt_exp = parse_optional_exp(";", tokens)
            if opt_exp is null:
                return Null
            else:
                Expression(opt_exp)
```

```
parse_do_loop(tokens):
    expect("do", tokens)
    body = parse_statement(tokens)
    expect("while", tokens)
    expect("(", tokens)
    condition = parse_exp(0, tokens)
    expect(")", tokens)
    expect(";", tokens)
    return DoWhile(body, condition, id="")

parse_while_loop(tokens):
    expect("while", tokens)
    expect("(", tokens)
    condition = parse_exp(0, tokens)
    expect(")", tokens)
    body = parse_statement(tokens)
    return While(condition, body, id="")

parse_for_loop(tokens):
    expect("for",tokens)
    expect("(",tokens)
    init = parse_for_init(tokens)
    condition = parse_optional_exp(";", tokens)
    post = parse_optional_exp(")", tokens)
    body = parse_statement(tokens)
    return For(init, condition, post, body)
```

# VarResolution
```
resolve_optional_exp(opt_exp, var_map):
    if opt_exp is not null:
        resolve_exp(opt_exp, var_map)
```

*resolve_for_init* is already provided in the BOOK_NOTES.

A more comprehensive *resolve_statement*.
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
		case LabeledStatement(lbl, stmt):
			return LabeledStatement(lbl, resolve_statement(stmt, var_map))
		case Goto(lbl):
			return statement
		case While(condition, body, id):
            return While(
                resolve_exp(condition, var_map),
                resolve_statement(body, var_map)
                id
            )
        case DoWhile(body, condition, id):
            return DoWhile(
                resolve_statement(body, var_map),
                resolve_exp(codition, var_map),
                id
            )
        case For(init, condition, post, body, id):
            new_var_map = copy_variable_map(var_map)

            return For(
                resolve_for_init(init, new_var_map),
                resolve_optional_exp(condition, new_var_map),
                resolve_optional_exp(post, new_var_map),
                resolve_statement(body, new_var_map)
            )
        case Compound:
            --snip--
        case Null:
        case Break:
        case Continue:
            return statement
```

# Validate Labels

```
collect_labels_from_statement(Set<string> defined, Set<string> used, statement):
	match statement type:
		case Goto:
			--snip--
		case LabeledStatement:
			--snip--
		case If:
			--snip--
		case Compound:
			--snip--
        case While:
            collect_labels_from_statement(defined, used, statement.body)
        case DoWhile:
            collect_labels_from_statement(defined, used, statement.body)
        case For:
            collect_labels_from_statement(defined, used, statement.body)
		default:
            // For Return, Null, Expression, Break, Continue
			return
```

# LoopLabeling

```
label_statement(stmt, curr_label):
    match stmt type:
        case Break:
            if curr_label is null:
                fail("Break outside of loop")
            return Break(id=curr_label)
        case Continue:
            if curr_label is null:
                fail("Continue outside of loop")
            return Continue(id=curr_label)
        case While(condition, body, id=""):
            new_id = UniqueIds.make_label("while")
            return While(
                condition,
                label_statement(body, new_id),
                id=new_id                 
            )
        case DoWhile(body, condition, id=""):
            new_id = UniqueIds.make_label("do_while")
            return DoWhile(
                label_statement(body, new_id)
                condition,
                id=new_id 
            )
        case For(init, condition, post, body, id="")
            new_id = UniqueIds.make_label("for")
            return For(
                init,
                condition,
                post, 
                label_statement(body, new_id),
                id=new_id
            )
        case Compound(block):
            return Compound(label_block(block, curr_label))
        case If(condition, then_clause, else_clause):
            return If(
                condition,
                label_statement(then_clause, curr_label),
                else_clause ? label_statement(else_clause, curr_label) : null
            )
        case LabeledStatement(lbl, stmt):
            return LabeledStatement(
                lbl,
                label_statement(stmt, curr_label)
            )
        case Null:
        case Return:
        case Expression:
        case Goto:
            return stmt
```

```
label_block_item(block_item, curr_label):
    if block_item is a statement:
        return label_statement(block_item, curr_label)
    else:
        return block_item as declaration
```

```
label_block(block, curr_label):
    labeled_blocks = []

    for block_item in block:
        labeled_block = label_block_item(block_item, curr_label)
        labeled_blocks.append(labeled_block)

    return labeled_blocks
```

```
label_function_def(AST.FunctionDef funDef):
    return AST.FunctionDef(
        funDef.name,
        label_block(funDef.body, curr_label)
    )
```

```
label_loops(AST.Program prog):
    return label_function_def(prog.funDef)
```

# TACKY
*No changes for our TACKY AST!*

# TACKYGEN
We need a uniform way to create break and continue labels both for the statements that jumpt to them, or the labels themselves.

```
break_label(id):
    return "break.{id}"

continue_label(id):
    return "continue.{id}"
```

We added new statements, so let's update our *emit_tacky_for_statement* function:
```
emit_tacky_for_statement(statement):
	match statement type:
		case Return:
			--snip--
		case Expression:
			--snip--
		case If:
			--snip--
        case LabeledStatement:
            --snip--
        case Goto:
            --snip--
		case Compound:
			--snip--
        case Break:
            return [
                Jump(break_label(statement.id))
            ]
        case Continue:
            return [
                Jump(continue_label(statement.id))
            ]
        case DoWhile:
            return emit_tacky_for_do_loop(statement)
        case While:
            return emit_tacky_for_while_loop(statement)
        case For:
            return emit_tacky_for_for_loop(statement)
		case Null:
			return []
```

```
emit_tacky_for_do_loop(do_loop):
    start_label = UniqueIds.make_label("do_loop_start")
    cont_label = continue_label(do_loop.id)
    br_label = break_label(do_loop.id)
    eval_cond, c = emit_tacky_for_exp(do_loop.condition)
    body_insts = emit_tacky_for_statement(do_loop.body)

    insts = [
        Label(start_label),
        ...body_insts,
        Label(cont_label), 
        ...eval_cond,
        JumpIfNotZero(c, start_label),
        Label(br_label)
    ]

    return insts
```

```
emit_tacky_for_while_loop(while_loop):
    cont_label = continue_label(while_loop.id)
    br_label = break_label(while_loop.id)
    eval_cond, c = emit_tacky_for_exp(while_loop.condition)
    body_insts = emit_tacky_for_statement(while_loop.body)

    insts = [
        Label(cont_label),
        ...eval_cond,
        JumpIfZero(c, br_label),
        ...body_insts
        Jump(cont_label),
        Label(br_label)
    ]

    return insts
```

```
emit_tacky_for_for_loop(for_loop):
    start_label = UniqueIds.make_label("for_start")
    cont_label = continue_label(for_loop.id)
    br_label = break_label(for_loop.id)
    for_init_insts = []

    if for_loop.init is declaration:
        for_init_insts = emit_declaration(for_loop.init)
    else: // for_loop.init is an opt_exp
        if for_loop.init is not null:
            for_init_insts = emit_tacky_for_exp(for_loop.init)
        else:
            for_init_insts = []

    test_condition = []

    if for_loop.condition is not null:
        inner_insts, v = emit_tacky_for_exp(for_loop.condition)

        test_condition = [
            ...inner_insts,
            JumpIfZero(v, br_label)
        ]

    post_insts = []

    if for_loop.post is not null:
        inner_insts, v = emit_tacky_for_exp(for_loop.post)

        post_insts = inner_insts
    
    body_insts = emit_tacky_for_statement(for_loop.body)

    insts = [
        ...for_init_insts,
        Label(start_label),
        ...test_condition,
        ...body_insts,
        Label(cont_label),
        ...post_insts,
        Jump(start_label),
        Label(br_label)
    ]
```

Refactor our case to emit for declaration:
```
emit_tacky_for_block_item(block_item):
	match block_item type:
		case Statement:
			return emit_tacky_for_statement(block_item)
		case Declaration:
			return emit_declaration(block_item)

emit_declaration(declaration):
    if declaration has init:
        // Treat declaration with initializer as assignment
        eval_assignment, v = emit_tacky_for_exp(AST.Assignment(AST.Var(declaration.name), declaration.init))
        return eval_assignment
    else:
        // Do not generate any instructions
        return []
```

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
*We don't make any modifications, as no constructs in the TACKY AST were altered.*

# Output
From C:
```C
int main(void) {
    int a = -2147483647;

    while (a < 5)
    {
        if (a == -10) {
            a = a + 1;
            continue;
        }
        
        a = a + 2;
    }

    do {
        a = a * 2;
        if (a == 6)  break;
    } while (a < 11);

    for (a = 3; a % 5 != 0; a = a + 1)
        a = a + 1;

    return a % 5 || a > 0;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	68, %rsp
	movl	$2147483647, -4(%rbp)		
	negl	-4(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -8(%rbp)			; a = -2147483647
.Lcontinue.while.1:
	movl	$5, %r11d				;  5
	cmpl	-8(%rbp), %r11d			; a
	movl	$0, -12(%rbp)
	setl	-12(%rbp)
	cmpl	$0, -12(%rbp)
	je	.Lbreak.while.1				; !(a < 5) -> break
	movl	$10, -16(%rbp)			; 10
	negl	-16(%rbp)				; -10
	movl	-8(%rbp), %r10d			; a
	cmpl	%r10d, -16(%rbp)		
	movl	$0, -20(%rbp)
	sete	-20(%rbp)
	cmpl	$0, -20(%rbp)
	je	.Lif_end.6
	movl	-8(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	addl	$1, -24(%rbp)
	movl	-24(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	jmp	.Lcontinue.while.1
.Lif_end.6:
	movl	-8(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	addl	$2, -28(%rbp)
	movl	-28(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	jmp	.Lcontinue.while.1
.Lbreak.while.1:
.Ldo_loop_start.11:
	movl	-8(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	movl	-32(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -32(%rbp)
	movl	-32(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	movl	$6, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -36(%rbp)
	sete	-36(%rbp)
	cmpl	$0, -36(%rbp)
	je	.Lif_end.13
	jmp	.Lbreak.do_while.2
.Lif_end.13:
.Lcontinue.do_while.2:
	movl	$11, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -40(%rbp)
	setl	-40(%rbp)
	cmpl	$0, -40(%rbp)
	jne	.Ldo_loop_start.11
.Lbreak.do_while.2:
	movl	$3, -8(%rbp)
.Lfor_start.16:
	movl	-8(%rbp), %eax
	cdq
	movl	$5, %r10d
	idivl	%r10d
	movl	%edx, -44(%rbp)
	movl	$0, %r11d
	cmpl	-44(%rbp), %r11d
	movl	$0, -48(%rbp)
	setne	-48(%rbp)
	cmpl	$0, -48(%rbp)
	je	.Lbreak.for.3
	movl	-8(%rbp), %r10d
	movl	%r10d, -52(%rbp)
	addl	$1, -52(%rbp)
	movl	-52(%rbp), %r10d
	movl	%r10d, -8(%rbp)
.Lcontinue.for.3:
	movl	-8(%rbp), %r10d
	movl	%r10d, -56(%rbp)
	addl	$1, -56(%rbp)
	movl	-56(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	jmp	.Lfor_start.16
.Lbreak.for.3:
	movl	-8(%rbp), %eax
	cdq
	movl	$5, %r10d
	idivl	%r10d
	movl	%edx, -60(%rbp)
	cmpl	$0, -60(%rbp)
	je	.Lor_true.23
	movl	$0, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -64(%rbp)
	setg	-64(%rbp)
	cmpl	$0, -64(%rbp)
	je	.Lor_true.23
	movl	$0, -68(%rbp)
	jmp	.Lor_end.24
.Lor_true.23:
	movl	$1, -68(%rbp)
.Lor_end.24:
	movl	-68(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
