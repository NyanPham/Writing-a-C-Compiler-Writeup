# Table of Contents
- [Compiler Driver](#compiler-driver)
- [Token, Lexer, AST](#token-lexer-ast)
- [Parser](#parser)
- [Resolve](#resolve)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit)
- [UniqueIds](#uniqueids)
- [Output](#output)
- [Extra Credit: Compound Assignment, Increment, and Decrement](#extra-credit-compound-assignment-increment-and-decrement)
  - [Token & Lexer](#token-and-lexer)
  - [AST](#ast-1)
  - [Parser](#parser-1)
  - [Tacky](#tacky-1)
  - [TackyGen](#tackygen-1)
  - [Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit](#assembly-codegen-replacepseudo-instruction-fixup-emit-1)
  - [Output](#output-1)

---

# Compiler Driver
We add another stage to our compiler: Semantic Analysis, so let's add one more command line option: `compiler.exe program.c --validate`

---

# Source code structure
We create one more folder named semantic_analysis that contains passes in the stage. 
For now, the only pass we have in this stage is VarResolution. We'll add more passes in later chapters.

# Token, Lexer, AST
The BOOK_NOTES guide is sufficient to update this file for the chapter.

# Parser
Update the *get_precedence* for new "=" operator.
"=" operator is right associative, so it needs to be parse differently from other binary operators. We treat it as a special case, so parse_binop should not parse "=".

```
parse_factor(tokens):
	next_token = peek(tokens)
	if next_token is an int:
		--snip--
	if  next_token is an identifier:
		return AST.Var(parse_id(tokens))
	else if next_token is "~" or "-" or "!":
		--snip--
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

*parse_exp* is already provided in BOOK_NOTES.
```
parse_exp(tokens, min_prec):
	left = parse_factor(tokens)
	next_token = peek(tokens)
	
	while next_token is a binary operator and precedence(next_token) >= min_prec:
		if next_token is "=":
			take_token(tokens) // remove "=" from list of tokens
			right = parse_exp(tokens, precedence(next_token))
			left = Assignment(left, right)
		else:
			operator = parse_binop(tokens)
			right = parse_exp(tokens, precedence(next_token) + 1)
			left = Binary(operator, left, right)
		next_token = peek(tokens)
	return left
```

```
parse_declaration(tokens):
	expect("int", tokens)
	var_name = parse_id(tokens)
		
	token = take_token(tokens)	
	init;
		
	if token is ";":
		init = null
	else if  token is "=":
		init = parse_exp(tokens, 0)
		expect(";", tokens)
	else:
		raise_error("An initializer or semicolon", token type)
	
	return Declaration(var_name, init)
```

We update our parse_statement:
```
parse_statement(tokens):
	next_token = peek(tokens)
	match next_token type:
		case "return":
			take_token(tokens) // consume the return keyword
			exp = parse_exp(tokens, 0)
			expect(";", tokens)
			return AST.Return(exp)
		case ";":
			take_tokens(tokens)
			return AST.Null()
		default:
			// A statement expression
			exp = parse_exp(tokens)
			expect(";", tokens)
			return AST.Expression(exp)	
```

```
parse_block_item(tokens):
	if (peek(tokens)) type is "int" keyword:
		return parse_declaration(tokens)
	else:
		return parse_statement(tokens)
```

parse_function_definition is provided in the BOOK_NOTES. I put here for an easier access:
```
parse_function_definition(tokens):
	// parse everything up through the open brace as before
	--snip--
	function_body = []
	while peek(tokens) != "}":
		next_block_item = parse_block_item(tokens)
		function_body.append(next_block_item)
	take_token(tokens)
	return Function(name, function_body)
```

# VarResolution
A more complete resolve_exp for clarity of how to implement it:

```
resolve_exp(exp, var_map):
	match e with
		case Assignment(left, right):
			if left is not a Var node:
				fail("Invalid lvalue")
			return Assignment(resolve_exp(left, var_map), resolve_exp(right, var_map))
		case Var(v):
			if v is in var_map:
				return Var(var_map.get(v))
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

**resolve_statement** and **resolve_declaration** in BOOK_NOTES are complete.

And lastly, *resolve_block_item*, *resolve_function_def*, *resolve*
```
resolve_block_item(block_item, var_map):
	if block_item is a declaration:
		return resolve_declaration(block_item, var_map)
	else:
		return resolve_statement(block_item, var_map)
```

```
resolve_function_def(Function fun):
	var_map = {} // Empty map
	resolved_body = []
	
	for block_item in fun.body:
		new_map, resolved_item = resolve_block_item(block_item, var_map)
		resolved_body.append(resolved_item)
		var_map = new_map
	
	return Function(fun.name, resolved_body)
```

```
resolve(Program prog):
	return Program(resolve_function_def(prog.fun_def))
```

# TACKY
We don't modify anything in TACKY as it already supports Var and Copy to assign values to Var.
We'll convert assignment to copy instruction. For example:

**AST**
```
Assignment(a, b)
```

We already check that a must be a Var node to be an lvalue. For b, we emit instruction for it, and move the result to a.

```
dst = emit_tacky_for_exp(b)
copy(dst, Var(a))
```

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
				return emit_or_expression(exp
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
```

Update our *emit_tacky_for_statement*:
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
		case Null:
			return []
```

```
emit_tacky_for_block_item(block_item):
	match block_item type:
		case Statement:
			return emit_tacky_for_statement(block_item)
		case Declaration:
			if block_item as declaration has init:
				// Treat declaration with initializer as assignment
				eval_assignment, v = emit_tacky_for_exp(AST.Assignment(AST.Var(declaration.name), declaration.init))
				return eval_assignment
			else:
				// Do not generate any instructions
				return []
```

```
emit_tacky_for_function(AST.Function fun_def):
	insts = []
	
	for block_item in fun_def.body:
		emit_insts = emit_tacky_for_block_item(block_item)
		insts.append(...emit_insts)
	
	extra_return = Tacky.Return(Constant(0)) 	// To make sure our main function ends with a return statement if the source code doesn't include one.
	insts.append(extra_return)
	
	return Tacky.Function(fun_def.name, insts)
```

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
After the TACKY stage, we don't make any modification as we haven't changed any constructs in TACKY AST.

# UniqueIds
In resolve_declaration, we need to autogenerate names for local variables as well. 
We can reuse our make_temporary function, or ideally, create another function called make_named_temporary, which calls make_label.

# Output
From C:
```C
int main(void) {
    int a = 2147483646;
    int b = 0;
    int c = a / 6 + !b;
    return c * 2 == a - 1431655762;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	36, %rsp
	movl	$2147483646, -4(%rbp)
	movl	$0, -8(%rbp)
	movl	-4(%rbp), %eax
	cdq
	movl	$6, %r10d
	idivl	%r10d
	movl	%eax, -12(%rbp)
	cmpl	$0, -8(%rbp)
	movl	$0, -16(%rbp)
	sete	-16(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-16(%rbp), %r10d
	addl	%r10d, -20(%rbp)
	movl	-20(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-24(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	-28(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -28(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	subl	$1431655762, -32(%rbp)
	movl	-28(%rbp), %r10d
	cmpl	%r10d, -32(%rbp)
	movl	$0, -36(%rbp)
	sete	-36(%rbp)
	movl	-36(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```


# Extra Credit: Compound Assignment, Increment, and Decrement
We will implement compound assignment, increment and decrement operators: +=, -=, *=, /=, %=, ++, --

### Legacy Extra Credit Integration
We've implemented Extra Credit in chapter 3, so we also need to support: &=, |=, ^=, <<=, >>=

## Token and Lexer:
DoubleHyphen token has already existed.

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| DoublePlus | ++ |
| PlusEqual | += |
| HyphenEqual | -= |
| StarEqual | \*= |
| SlashEqual | /= |
| PercentEqual | % = |
| AmpersandEqual | &= |
| PipeEqual | \|= |
| CaretEqual | ^= |
| DoubleLeftBracketEqual | <<= |
| DoubleRightBracketEqual | >>= |

## AST:
<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, block_item*body)
block_item = S(statement) | D(declaration)
declaration = Declaration(identifier name, exp? init)
statement = Return(exp) | Expression(exp) | Null 
exp = Constant(int) 
	| Var(identifier)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
	| Assignment(exp, exp)
	<strong>| CompoundAssignment(binary_operator, exp, exp)
	| PostfixIncr(exp)
	| PostfixDecr(exp)</strong>
unary_operator = Complement | Negate | Not <strong>| Incr | Decr </strong>
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual</pre></code>


## Parser
We make changes to our grammar

### EBNF
A factor now can be either unary, or postfix unary.
We create one layer primary-exp in place of factor now so we can move factor higher in the hierachy.

```
<primary-exp> ::= "int" | <identifier> | "( <exp> ")
```

One layer above our primary-exp is <postfix-exp> that contains our expression, and optionally ++ or --
```
<postfix-exp> ::= <primary-exp> { "++" | "--" }
```

Now, the factor:
```
<factor> ::= <unop> <factor> | <postfix-exp>
```

Expression is unchanged:
```
<exp> ::= <factor> | <exp> <binop> <exp>
```

**Updated EBNF**
<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" | &lt;exp&gt; ";" | ";"
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt;
<strong>&lt;factor&gt; ::= &lt;unop&gt; &lt;factor&gt; | &lt;postfix-exp&gt;
&lt;postfix-exp&gt; ::= &lt;primary-exp&gt; { "++" | "--" } 
&lt;primary-exp&gt; ::= &lt;int&gt; | &lt;identifier&gt; | "(" &lt;exp&gt; ")"</strong>
&lt;unop&gt; ::= "-" | "~" <strong>| "++" | "--" </strong>
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "<" | "<=" | ">" | ">=" 
				| "=" <strong>| "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

Precedence of all compound assignement operators are 1, similar to the equal sign.

### Precedence Values of Binary Operators

| Operator | Precedence |
| -------- | ---------- | 
| \* | 50 |
| / | 50 |
| % | 50 |
| + | 45 |
| - | 45 |
| < | 35 |
| <= | 35 |
| > | 35 |
| >= | 35 |
| == | 30 |
| != | 30 |
| && | 10 |
| \|\| | 5 |
| = | 1 |
| += | 1 |
| -= | 1 |
| \*= | 1 |
| /= | 1 |
| %= | 1 |
| &= | 1 |
| \|= | 1 |
| ^= | 1 |
| <<= | 1 |
| >>= | 1 |

We parse compound operator in expression similar to "=" for assignment.
```
get_compound_operator(token):
	match token type:
		case EqualSign: return None
		case PlusEqual: return AST.Binop.Add
		case HyphenEqual: return AST.Binop.Subtract
		case StarEqual: return AST.Binop.Multiply
		case SlashEqual: return AST.Binop.Divide
		case PercentEqual: return AST.Binop.Remainder
		case AmpersandEqual: return AST.Binop.BitwiseAnd 
		case PipeEqual: return AST.Binop.BitwiseOr
		case CaretEqual: return AST.Binop.BitwiseXor
		case DoubleLeftBracketEqual: return AST.Binop.BitShiftLeft
		case DoubleRightBracketEqual: return AST.Binop.BitShiftRight
		default:
			fail("Internal Error: not an assignment operator")
```

```
is_assignment(token)
	match token type:
		case EqualSign: 
		case PlusEqual:
		case HyphenEqual: 
		case StarEqual:
		case SlashEqual:
		case PercentEqual:
		case AmpersandEqual:
		case PipeEqual: 
		case CaretEqual:
		case DoubleLeftBracketEqual:
		case DoubleRightBracketEqual:
			return true
		default:
			return false
```

```
parse_unop(tokens):
	next_token(tokens)
	match next_token type:
		--snip--
		case DoublePlus: 
			return Incr
		case DoubleHyphen:
			return Decr
		default:
			--snip--
```

```
parse_primary_exp(tokens):
	next_token = peek(tokens)
	if next_token is an int:
		--snip--
	if  next_token is an identifier:
		--snip--
	else if next_token is "(":
		--snip--
	else:
		fail("Malformed factor")
```

```
parse_postfix_helper(primary, tokens):
	next_token = peek(tokens)
	if next_token is "--":
		decr_exp = PostfixDecr(primary)
		return parse_postfix_helper(decr_exp, tokens)
	else if next_token is "++":
		incr_exp = PostfixDecr(primary)
		return parse_postfix_helper(incr_exp, tokens)
	else:
		return primary
```

```
parse_postfix_exp(tokens):
	primary = parse_primary_exp(tokens)
	return parse_postfix_helper(primary, tokens)
```

```
parse_factor(tokens):
	next_token = peek(tokens)
	// Unary expression
	if next_token is Tilde, Hyphen, Bang, DoublePlus, DoubleHyphen:
		op = parse_unop(tokens)
		inner_exp = parse_factor(tokens)
		return Unary(op, inner_exp)
	else:
		return parse_postfix_exp(tokens)
```

```
parse_exp(tokens, min_prec):
	left = parse_factor(tokens)
	next_token = peek(tokens)
	
	while next_token is a binary operator and precedence(next_token) >= min_prec:
		if is_assignment(next_token):
			take_token(tokens) // remove "=" from list of tokens
			right = parse_exp(tokens, precedence(next_token))
			op = get_compound_operator(next_token)
			if op is null:
				left = Assignment(left, right)
			else:
				left = CompoundAssignment(op, left, right)
		else:
			operator = parse_binop(tokens)
			right = parse_exp(tokens, precedence(next_token) + 1)
			left = Binary(operator, left, right)
		next_token = peek(tokens)
	return left
```

## VarResolution
We add a case to resolve compound assignments, postfix increments and postfix decrements.

```
resolve_exp(exp, var_map):
	match e with
		case Assignment(left, right):
			if left is not a Var node:
				fail("Invalid lvalue")
			return Assignment(resolve_exp(left, var_map), resolve_exp(right, var_map))
		case CompoundAssignment(op, left, right):
			if left is not a Var node:
				fail("Invalid lvalue")
			return CompoundAssignment(op, resolve_exp(left, var_map), resolve_exp(right, var_map))
		case PostfixIncr(e):
			if e is not a Var node:
				fail("Operand of postfix expression must be an lvalue!")
			return PostfixIncr(resolve_exp(e, var_map))
		case PostfixDecr(e):
			if e is not a Var node:
				fail("Operand of postfix expression must be an lvalue!")
			retrun PostfixDecr(resolve_exp(e, var_map))
		case Var(v):
			if v is in var_map:
				return Var(var_map.get(v))
			else:
				fail("Undeclared variable!")
		case Unary(op, e):
			// Validate prefix operands
			if op is Inr or Decr, and e is not a Var node:
				fail("Operand of ++\-- must be an lvalue!")
			return Unary(op, resolve_exp(e, var_map))
		case Binary(op, e1, e2):
			return Binary(op, resolve_exp(e1, var_map), resolve_exp(e2, var_map))
		case Constant:
			return exp
		default:
			fail("Internal error: unknown expression")
```

## Tacky
No modifications

## TackyGen
```
convert_unop(op):
	match op:
		--snip--
		case Incr:
		case Decr:
			fail("Internal error: Shouldn't handle increment/decrement operator here!")
```

For postfix ++/--, we create destination, copy the value from current Var into it before increment the value of Var by one.

```
emit_postfix(op, var):
	dst_name = UniqueIds.make_temporary()
	dst = Var(dst_name)
	
	op = convert_binop(op)
	
	insts = [
		Copy(Tacky.Var(var.name), dst),
		Binary(op, Var(var.name), Constant(1), Var(var.name))
	]
	
	return (insts, dst)
```

For prefix increment in this form:
```
++exp
```
We treat it similar to this:
```
exp += 1
```
Check 
So let's update our emit_tacky_for_exp to handle unary of increment and decrement separately.
```
emit_tacky_for_exp(exp):
	match exp type:
		case AST.Constant: return TACKY.Constant
		case AST.Var: return TACKY.Var
		case Unary: 
			if exp.op is increment:
				if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
				return emit_compound_assignment(AST.Add, exp.exp.name, AST.Constant(1))
			else if exp.op is decrement:
				if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
				return emit_compound_assignment(AST.Subtract, exp.exp.name, AST.Constant(1))
			else:
				return emit_unary_expression(exp)
		case Binary: 
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_or_expression(exp
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
		case CompoundAssignment:
			if exp.left is not AST.Var:
				fail("Internal Error: bad lvalue!")
			return emit_compound_assignment(exp.op, exp.left, exp.right)
		case PostfixIncr:
			if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
			return emit_postfix(AST.Add, exp.exp.name)
		case PostfixDecr:
			if exp.exp is not AST.Var:
					fail("Internal Error: bad lvalue!")
			return emit_postfix(AST.Subtract, exp.exp.name)
```

```
emit_compound_assignment(op, var_name, rhs):
	eval_rhs, rhs = emit_tacky_for_exp(rhs)	
	dst = Tacky.Var(var_name)
	tacky_op = convert_binop(op)
	
	insts = [
		...eval_rsh,
		Tacky.Binary(tacky_op, dst, rhs, dst)
	]
	
	return (insts, dst)
```

# Assembly, CodeGen, ReplacePseudo, Instruction Fixup, Emit
After the TACKY stage, we don't make any modification as we haven't changed any constructs in TACKY AST.


# Output
From C:
```C
int main(void) {
	int a = 5;
	int b = 2;
	int c = 1;

	b += a++;
	b--;
	c *= a;
	b += c;
	a >>= b;
	c %= a;
	a += b-- * c++;

	return a + b + c;
}
```

To x64 Assembly on Linux:
```asm
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	40, %rsp
	movl	$5, -4(%rbp)
	movl	$2, -8(%rbp)
	movl	$1, -12(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -16(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	addl	$1, -4(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	movl	-16(%rbp), %r10d
	addl	%r10d, -8(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	subl	$1, -8(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %r11d
	imull	-4(%rbp), %r11d
	movl	%r11d, -12(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	movl	-12(%rbp), %r10d
	addl	%r10d, -8(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	movl	-8(%rbp), %ecx
	sarl	%cl, -4(%rbp)
	movl	-12(%rbp), %eax
	cdq
	idivl	-4(%rbp)
	movl	%edx, -12(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-8(%rbp), %r10d
	movl	%r10d, -8(%rbp)
	subl	$1, -8(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -28(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, -12(%rbp)
	addl	$1, -12(%rbp)
	movl	-24(%rbp), %r10d
	movl	%r10d, -32(%rbp)
	movl	-32(%rbp), %r11d
	imull	-28(%rbp), %r11d
	movl	%r11d, -32(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -4(%rbp)
	movl	-32(%rbp), %r10d
	addl	%r10d, -4(%rbp)
	movl	-4(%rbp), %r10d
	movl	%r10d, -36(%rbp)
	movl	-8(%rbp), %r10d
	addl	%r10d, -36(%rbp)
	movl	-36(%rbp), %r10d
	movl	%r10d, -40(%rbp)
	movl	-12(%rbp), %r10d
	addl	%r10d, -40(%rbp)
	movl	-40(%rbp), %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```