# Table of Contents
- [Token, Lexer, AST](#token-lexer-ast)
- [Parser](#parser)
- [Variable Resolution](#varresolution)
- [Loop Labeling](#loop-labeling)
- [TACKY](#tacky)
- [TackyGen](#tackygen)
- [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit)
- [Output](#output)
- [Extra Credit: Switch Statements](#extra-credit-switch-statements)
  - [Token & Lexer](#token-lexer)
  - [AST](#ast)
  - [Parser](#parser-1)
  - [VarResolution](#varresolution)
  - [ValidateLabels](#validatelabels)
  - [LoopLabeling](#looplabeling)
  - [CollectSwitchCases](#collectswitchcases)
  - [TackyGen](#tackygen-1)
  - [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit-1)
  - [Output](#output-1)

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

# Loop Labeling

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

# Extra Credit: Switch Statements

## Token, Lexer:

New tokens to support
| Token | Regular expression |
| ------- | -------------------- |
| KeywordSwitch | switch |
| KeywordCase | case |
| KeywordDefault | default |

## AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, block body)
block_item = S(statement) | D(declaration)
block = Block(block_item*)
declaration = Declaration(identifier name, exp? init)
for_init = InitDecl(declaration) | InitExp(exp?)
statement = Return(exp) 
	| Expression(exp) 
	| If(exp condition, statement then, statement? else)
    | Compound(block)
	| Break
	| Continue
	| While(exp condition, statement body)
	| DoWhile(statement body, exp condition)
	| For(for_init init, exp? condition, exp? post, statement body)
	| Null 
	| LabeledStatement(identifier lbl, statement)
	| Goto(identifier lbl) 
    <strong>| Switch(exp control, statement body, string* cases,  identifier id)
    | Case(exp, statement body, identifier id)  // exp must be a constant, validate during semantic analysis 
    | Default(statement body, identifier id)</strong>
exp = Constant(int) 
	| Var(identifier)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
	| Assignment(exp, exp)
	| CompoundAssignment(binary_operator, exp, exp)
	| PostfixIncr(exp)
	| PostfixDecr(exp)
	| Conditional(exp condition, exp then, exp else) 
unary_operator = Complement | Negate | Not | Incr | Decr 
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual</pre></code>

## EBNF

**Updated EBNF**
<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" &lt;block&gt;
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;for-init&gt; ::= &lt;declaration&gt; | [ &lt;exp&gt; ] ";"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" 
	| &lt;exp&gt; ";" 
	| "if" "(" &lt;exp&gt; ")" &lt;statement&gt; ["else" &lt;statement&gt;]
	| &lt;block&gt;
	| "break" ";"
	| "continue" ";"
	| "while" "(" &lt;exp&gt; ")" &lt;statement&gt;
	| "do" &lt;statement&gt; "while" "(" &lt;exp&gt; ")" ";"
	| "for" "(" &lt;for-init&gt; [ &lt;exp&gt; ] ";" [ &lt;exp&gt; ] ")" &lt;statement&gt;
	| ";"
	|  &lt;label&gt; ":"  &lt;statement&gt;
	| "goto"  &lt;label&gt; ";"
	<strong>| "switch" "(" &lt;exp&gt; ")" &lt;statement&gt;
	| "case" &lt;exp&gt; ":" &lt;statement&gt;
	| "default" ":" &lt;statement&gt;</strong>
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
&lt;factor&gt; ::= &lt;unop&gt; &lt;factor&gt; | &lt;postfix-exp&gt;
&lt;postfix-exp&gt; ::= &lt;primary-exp&gt; { "++" | "--" } 
&lt;primary-exp&gt; ::= &lt;int&gt; | &lt;identifier&gt; | "(" &lt;exp&gt; ")"
&lt;unop&gt; ::= "-" | "~" | "++" | "--" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" 
				| "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "&lt;&lt;=" | "&gt;&gt;="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>


## Parser

```
parse_statement(tokens):
    --snip--
    case "switch":
        return parse_switch_statement(tokens)
    case "case":
        take_token(tokens)
        case_val = parse_exp(0, tokens)
        expect(":", tokens)
        stmt = parse_statement(tokens)
        return Case(case_val, stmt, id="")
    case "default":
        take_token(tokens)
        expect(":", tokens)
        stmt = parse_statement(tokens)
        return Default(stmt, id="")
```

```
parse_switch_statement(tokens):
    expect("switch", tokens)
    expect("(", tokens)
    control = parse_exp(0, tokens)
    expect(")", tokens)
    body = parse_statement(tokens)
    return Switch(control, body, id="", cases=[])
```

## VarResolution

```
resolve_statement(statement, var_map):
	match statement with
    --snip--
	case Switch(control, body, id, cases):
        return Switch(
            control=resolve_exp(control, var_map),
            body=resolve_statement(body, var_map),
            id,
            cases
        )
    case Case(v, body, id):
        return Case(
            v,
            body=resolve_statement(body, var_map),
            id
        )
    case Default(body, id):
        return Default(
            resolve_statement(body, var_map),
            id
        )
```

## ValidateLabels

```
collect_labels_from_statement(Set<string> defined, Set<string> used, statement):
	match statement type:
		--snip--
        case Switch:
            collect_labels_from_statement(defined, used, statement.body)
        case Case:
            collect_labels_from_statement(defined, used, statement.body)
        case Default:
            collect_labels_from_statement(defined, used, statement.body)
		default:
			return
```

## LoopLabeling

Break and continue now can be pointing to different enclosing statement. Break can be either associated to either Switch or Loop, while Continue is only for Loop. We separate them in the LabelLoop pass.

```
label_statement(stmt, curr_break_id, curr_continue_id):
    match stmt type:
        case Break:
            if curr_break_id is null:
                fail("Break outside of loop or switch")
            return Break(id=curr_break_id)
        case Continue:
            if curr_continue_id is null:
                fail("Continue outside of loop")
            return Continue(id=curr_continue_id)
        case While:
            new_id = UniqueIds.make_label("while")
            return While(
                condition,
                label_statement(body, new_id, new_id),
                id=new_id
            )
        case DoWhile(body, condition, id=""):
            new_id = UniqueIds.make_label("do_while")
            return DoWhile(
                label_statement(body, new_id, new_id)
                condition,
                id=new_id
            )
        case For(init, condition, post, body, id="")
            new_id = UniqueIds.make_label("for")
            return For(
                init,
                condition,
                post,
                label_statement(body, new_id, new_id),
                id=new_id
            )
        case Compound(block):
            return Compound(label_block(block, curr_break_id, curr_continue_id))
        case If(condition, then_clause, else_clause):
            return If(
                condition,
                label_statement(then_clause, curr_break_id, curr_continue_id),
                else_clause ? label_statement(else_clause, curr_break_id, curr_continue_id) : null
            )
        case LabeledStatement(lbl, stmt):
            return LabeledStatement(
                lbl,
                label_statement(stmt, curr_break_id, curr_continue_id)
            )
        case Default(stmt, id):
            return Default(
                label_statement(stmt, curr_break_id, curr_continue_id),
                id
            )
        case Case(v, stmt, id):
            return Case(
                v,
                label_statement(stmt, curr_break_id, curr_continue_id),
                id
            )
        case Switch(control, body, cases, id):
            new_break_id = UniqueIds.makeLabel("switch")
            labeled_body = label_statement(body, new_break_id, curr_continue_id)

            return Switch(
                control,
                labeled_body,
                cases,
                id=new_break_id
            )
        case Null:
        case Return:
        case Expression:
        case Goto:
            return stmt
```

```
label_block_item(block_item, curr_break_id, curr_continue_id):
    if block_item is a statement:
        return label_statement(block_item, curr_break_id, curr_continue_id)
    else:
        return block_item as declaration
```

```
label_block(block, curr_break_id, curr_continue_id):
    labeled_blocks = []

    for block_item in block:
        labeled_block = label_block_item(block_item, curr_break_id, curr_continue_id)
        labeled_blocks.append(labeled_block)

    return labeled_blocks
```

```
label_function_def(AST.FunctionDef funDef):
    return AST.FunctionDef(
        funDef.name,
        label_block(funDef.body, null, null)
    )
```

## CollectSwitchCases

We have a new pass in to collect all the cases in the switch so in TACKY, we know which Case statement we jump to.

```
analyze_case_or_default(key, opt_case_map, lbl, inner_stmt):
    // First, make sure we're in a switch statement
    if opt_case_map is null:
        fail("Found case statement outside of switch")

    case_map = opt_case_map

    // Check for duplicates
    if case_map already has key:
        if key is not null:
            fail("Duplicate case in switch statement: {key}")
        else:
            fail("Duplicate default in switch statement")

    // Generate new ID - lbl should be "case" or "default"
    case_id = UniqueIds.make_label(lbl)
    updated_map = case_map.set(key, case_id)

    // Analyze inner statement
    final_map, new_inner_statement = analyze_statement(updated_map, inner_statement)

    return final_map, new_inner_statement, case_id
```

```
analyze_statement(stmt, opt_case_map):
    switch stmt type:
        case Default(stmt, id):
            new_map, new_stmt, default_id = analyze_case_or_default(null, opt_case_map, "default", stmt)
            return (new_map, Default(new_stmt, default_id))
        case Case(v, stmt, id):
            // Get integer value of this case
            if v is not a Constant(c):
                fail("Non-constant label in case statement")

            key = v.c
            new_map, new_stmt, case_id = analyze_case_or_default(key, opt_case_map, "case", stmt)

            return (new_map, Case(v, new_stmt, case_id))
        case Switch(control, body, cases, id):
            // Use fresh map when traversing body
            new_case_map, new_body = analyze_statement(body, {})
            // annotate switch with new case map; dont' pass new case map to caller
            return (opt_case_map, Switch(control, new_body, cases=new_case_map, id))

        // Just pass case_map through to substatements
        case If(condition, then_clause, else_clause):
            case_map1, new_then_clause = analyze_statement(then_clause, opt_case_map)
            
            case_map2 = case_map1
            new_else_clause = null

            if else_clause is not null:
                case_map2, new_else_clause = analyze_statement(else_clause, case_map1)
    
            return (case_map2, If(condition, new_then_clause, new_else_clause))
        case Compound(block):
            new_case_map, new_block = analyze_block(opt_case_map, block)
            return (new_case_map, Compound(new_block))
        case While(condition, body, id):
            new_map, new_body = analyze_statement(opt_case_map, body)
            return (new_map, While(condition, new_body, id))
        case DoWhile(body, condition, id):
            new_map, new_body = analyze_statement(opt_case_map, body)
            return (new_map, DoWhile(new_body, condition, id))
        case For(init, condition, post, body, id):
            new_map, new_body = analyze_statement(opt_case_map, body)
            return (new_map, For(init, condition, post, new_body, id))
        case LabeledStatement(lbl, stmt):
            new_case_map, new_stmt = analyze_statement(opt_case_map, stmt)
            return (new_case_map, LabeledStatement(lbl, new_stmt))
        // These don't include sub-statements
        case Return:
        case Null:
        case Expression:
        case Break:
        case Continue:
        case Goto:
            return stmt
```

```
analyze_block_item(opt_case_map, blk_item):
    if blk_item is a statement:
        new_opt_case_map, new_stmt = analyze_statement(opt_case_map, blk_item)

        return (new_opt_case_map, new_stmt)
    else:
        return (opt_case_map, blk_item) // don't need to change or traverse declaration

analyze_block(opt_case_map, blk):
    new_opt_case_map = opt_case_map
    new_blk = []

    for item in blk:
        updated_case_map, new_item = analyze_block_item(new_opt_case_map, item)
        new_opt_case_map = updated_case_map
        new_blk.append(new_item)
    
    return (new_opt_case_map, new_blk)

analyze_function_def(FunctionDef fun_def):
    _, blk = analyze_block(null, fun_def.body)
    return FunctionDef(fun_def.name, blk)

analyze_switches(Program fun_def):
    return Program(analyze_function_def(fun_def))
```

## TackyGen
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
            --snip--
        case Continue:
            --snip--
        case DoWhile:
            --snip--
        case While:
            --snip--
        case For:
            --snip--
        case Case:
            return [
                Label(statement.id),
                ...emit_tacky_for_statement(statement.statement)
            ]
        case Default:
            return [
                Label(statement.id),
                ...emit_tacky_for_statement(statement.statement)
            ]
        case Switch:
            return emit_tacky_for_switch(statement)
		case Null:
			return []
```

```
emit_tacky_for_switch(switch_stmt):
    br_label = break_label(switch_stmt.id)
    eval_control, c = emit_tacky_for_exp(switch_stmt.control)
    cmp_result = Tacky.Var(UniqueId.make_temporary())

    jump_to_cases = []
    for case in switch_stmt.cases:
        if case.key is not null:    // It's a case statement
            insts = [
                Binary(Equal, src1=Constant(case.key), src2=c, dst=cmp_result),
                JumpIfNotZero(cmp_result, case.id)
            ]

            jump_to_cases.append(...insts)
        else: // It's a default statement, handle later
            // do nothing

    default_tacky = []
    if switch_stmt.cases has key that is null:
        default_id = switch_stmt.cases.find(null).id
        default_tacky = [
            Jump(default_id)
        ]
    
    body_tacky = emit_tacky_for_statement(switch_stmt.body)

    insts = [
        ...eval_control,
        ...jump_to_cases, 
        ...default_tacky,
        Jump(br_label),
        ...body_tacky,
        Label(br_label),
    ]
```

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
After the TACKY stage, we don't make any modification as we haven't changed any constructs in TACKY AST.

# Output
From C:
```C
int main(void)
{
    int acc = 0;
    int ctr = 0;
    for (int i = 0; i < 10; i = i + 1)
    {
        switch (i)
        {
        case 0:
            acc = 2;
            break;
        case 1:
            acc = acc * 3;
            break;
        case 2:
            acc = acc * 4;
            break;
        case 3:
            continue;
        default:
            acc = acc + 1;
        }
        ctr = ctr + 1;
    }

    return ctr == 10 && acc == 31;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	52, %rsp
	movl	$0, -4(%rbp)
	movl	$0, -8(%rbp)
	movl	$0, -12(%rbp)
.Lfor_start.10:
	movl	$10, %r11d
	cmpl	-12(%rbp), %r11d
	movl	$0, -16(%rbp)
	setl	-16(%rbp)
	cmpl	$0, -16(%rbp)
	je	.Lbreak.for.3
	cmpl	$0, -12(%rbp)
	movl	$0, -20(%rbp)
	sete	-20(%rbp)
	cmpl	$0, -20(%rbp)
	jne	.Lcase.5
	cmpl	$1, -12(%rbp)
	movl	$0, -20(%rbp)
	sete	-20(%rbp)
	cmpl	$0, -20(%rbp)
	jne	.Lcase.6
	cmpl	$2, -12(%rbp)
	movl	$0, -20(%rbp)
	sete	-20(%rbp)
	cmpl	$0, -20(%rbp)
	jne	.Lcase.7
	cmpl	$3, -12(%rbp)
	movl	$0, -20(%rbp)
	sete	-20(%rbp)
	cmpl	$0, -20(%rbp)
	jne	.Lcase.8
	jmp	.Ldefault.9
	jmp	.Lbreak.switch.4
.Lcase.5:
	movl	$2, -4(%rbp)
	jmp	.Lbreak.switch.4
.Lcase.6:
	movl	-4(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-24(%rbp), %r11d
	imull	$3, %r11d
	movl	%r11d, -24(%rbp)
	movl	-24(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	jmp	.Lbreak.switch.4
.Lcase.7:
	movl	-4(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	-28(%rbp), %r11d
	imull	$4, %r11d
	movl	%r11d, -28(%rbp)
	movl	-28(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	jmp	.Lbreak.switch.4
.Lcase.8:
	jmp	.Lcontinue.for.3
.Ldefault.9:
	movl	-4(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	addl	$1, -32(%rbp)
	movl	-32(%rbp), %r10d
	movl	%r10d, -4(%rbp)
.Lbreak.switch.4:
	movl	-8(%rbp), %r10d
	movl	%r10d, -36(%rbp)
	addl	$1, -36(%rbp)
	movl	-36(%rbp), %r10d
	movl	%r10d, -8(%rbp)
.Lcontinue.for.3:
	movl	-12(%rbp), %r10d
	movl	%r10d, -40(%rbp)
	addl	$1, -40(%rbp)
	movl	-40(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	jmp	.Lfor_start.10
.Lbreak.for.3:
	movl	$10, %r11d
	cmpl	-8(%rbp), %r11d
	movl	$0, -44(%rbp)
	sete	-44(%rbp)
	cmpl	$0, -44(%rbp)
	je	.Land_false.20
	movl	$31, %r11d
	cmpl	-4(%rbp), %r11d
	movl	$0, -48(%rbp)
	sete	-48(%rbp)
	cmpl	$0, -48(%rbp)
	je	.Land_false.20
	movl	$1, -52(%rbp)
	jmp	.Land_end.21
.Land_false.20:
	movl	$0, -52(%rbp)
.Land_end.21:
	movl	-52(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```