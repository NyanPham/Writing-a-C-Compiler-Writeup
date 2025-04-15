# Table of Contents

- [Part I](#part-i)
  - [Chapter 1: A Minimal Compiler](#chapter-1-a-minimal-compiler)
  - [Chapter 2: Unary Operators](#chapter-2-unary-operators)
  - [Chapter 3: Binary Operators](#chapter-3-binary-operators)
  - [Chapter 4: Logical and Relational Operators](#chapter-4-logical-and-relational-operators)
  - [Chapter 5: Local Variables](#chapter-5-local-variables)
  - [Chapter 6: If Statements and Conditional Expressions](#chapter-6-if-statements-and-conditional-expressions)
  - [Chapter 7: Compound Statements](#chapter-7-compound-statements)
  - [Chapter 8: Loops](#chapter-8-loops)
  - [Chapter 9: Functions](#chapter-9-functions)
  - [Chapter 10: File Scope Variable Declarations and Storage Class Specifiers](#chapter-10-file-scope-variable-declarations-and-storage-class-specifiers)

---

# Part I

---

# Chapter 1: A Minimal Compiler

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Assembly Generation**
   - Input: AST
   - Output: Assembly code
4. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

## The Lexer

| Token             | Regular expression |
| ----------------- | ------------------ |
| Identifier        | [a-zA-Z_]\w\*\b    |
| Constant          | [0-9]+\b           |
| _int_ keyword     | int\b              |
| _void_ keyword    | void\b             |
| _return_ keyword  | return\b           |
| Open parenthesis  | \(                 |
| Close parenthesis | \)                 |
| Open brace        | {                  |
| Close brace       | }                  |
| Semicolon         | ;                  |

```Lexer
while input isn't empty:
	if input starts with whitespace:
		trim whitespace from start of input
	else:
		find longest match at start of input for any regex in table above
		if no match is found, raise an error
		convert matching substring into a token
		remove matching substring from start of input
```

**Notes**:

- Treat keywords like other identifiers. First find the end fo the token. Then, if it is an identifier, lookup the list of preserved keywords.

## The Parser

_Zephyr Abstract Syntax Description Language (ASDL)_ is used to represent AST and _extended Backus Naur form (EBNF)_ to represent formal grammar.

### AST

```AST
program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int)
```

### EBNF

```EBNF
<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int>
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?
```

### Parser

```Parser
parse_statement(tokens):
	expect("return", tokens)
	return_val = parse_exp(tokens)
	expect(";", tokens)
	return Return(return_val)

expect(expected, tokens):
	actual = take_tokens(tokens)
	if actual != expected:
		fail("Syntax error")
```

## Assembly Generation

### Assembly

```
program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst) | Ret
operand = Imm(int val) | Register
```

#### Converting AST Nodes to Assembly

| AST Node                     | Assembly construct           |
| ---------------------------- | ---------------------------- |
| Program(function_definition) | Program(function_definition) |
| Function(name, body)         | Function(name, instructions) |
| Return(exp)                  | Mov(exp, Register) <br> Ret  |
| Constant(int)                | Imm(int)                     |

## Code Emission

On macOS, we add \_ in front of the function name. For example, _main_ becomes _\_main_. Don't do this on Linux.

On Linux, add this at the end of the file:

```
	.section .note.GNU-stack,"",@progbits
```

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput                                                                                                                                       |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(function_definition) | Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_ |
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;\<instructions>                                           |

#### Formatting Assembly Instructions

| Assembly instruction | Output                                      |
| -------------------- | ------------------------------------------- |
| Mov(src, dst)        | movl &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret                  | ret                                         |

#### Formatting Assembly Operands

| Assembly operand | Output  |
| ---------------- | ------- |
| Register         | %eax    |
| Imm(int)         | $\<int> |

## Summary

We have 4 stages for compiler are the foundation for the rest of the journey.
In the next chapter, we'll support unary operators, add a new IR to the compiler.

## Additional Resources

**Linkers:**

- [Beginner’s Guide to Linkers](https://www.lurklurk.org/linkers/linkers.html)
- [Ian Lance Taylor’s 20-part essay](https://www.airs.com/blog/archives/38)
- [Ian Lance Taylor’s 20-part table of contents](https://lwn.net/Articles/276782/)
- [Position Independent Code (PIC) in Shared Libraries](https://eli.thegreenplace.net/2011/11/03/position-independent-code-pic-in-shared-libraries)
- [Position Independent Code (PIC) in Shared Libraries on x64](https://eli.thegreenplace.net/2011/11/11/position-independent-code-pic-in-shared-libraries-on-x64)

**AST definitions:**

- [Abstract Syntax Tree Implementation Idioms](https://hillside.net/plop/plop2003/Papers/Jones-ImplementingASTs.pdf)
- [The Zephyr Abstract Syntax Description Language](https://www.cs.princeton.edu/~appel/papers/asdl97.pdf)

**Executable stacks:**

- [Executable Stack](https://www.airs.com/blog/archives/518)

### Reference Implementation Analysis

[Chapter 1 Code Analysis](./code_analysis/chapter_1.md)

---

# Chapter 2: Unary Operators

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **TACKY Generation**
   - Input: AST
   - Output: TAC IR (Tacky)
4. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
5. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

## The Lexer

New tokens to support
| Token | Regular expression |
| ------- | -------------------- |
| Negation | - |
| Complement | ~ |
| Decrement | -- |

We will not yet implement the decrement operator _--_, but we need to let the lexer regonize it so it won't mistake -- for -(-

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int) <strong>| Unary(unary_operator, exp)</strong>
<strong>unary_operator = Complement | Negate</strong></pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" &lt;statement&gt; "}"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";"
&lt;exp&gt; ::= &lt;int&gt; <strong>| &lt;unop&gt; &lt;exp&gt; | "(" &lt;exp&gt; ")"</strong>
<strong>&lt;unop&gt; ::= "-" | "~"</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

```Parser
parse_exp(tokens):
	next_token = peek(tokens)
	if next_token is an int:
		--snip--
	else if next_token is "~" or "-":
		operator = parse_unop(tokens)
		inner_exp = parse_exp(tokens)
		return Unary(operator, inner_exp)
	else if next_token == "(":
		take_token(tokens)
		inner_exp = parse_exp(tokens)
		expect(")", tokens)
		return inner_exp
	else:
		fail("Malformed expression")
```

## TACKY Generation

In C, expressions can be nested, and Assembly instructions cannot.
For expression like `-(~2)`, first we need to apply the complement operation before the negation operation.

TAC stands for Three-Address Code, because most instructions use at most three values: two sources and one destination.

TAC is useful in:

- Major structural transformation
- Optimization.

### TACKY

```
program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) | Unary(unary_operator, val src, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate
```

TACKY instructions get operand of type Val, which is either a Var or a Constant.
However, for dst must be a temporary Var, not a Constant.

### Generating TACKY

```
emit_tacky(e, instructions):
	match e with
	| Constant(c) ->
		return Constant(c)
	| Unary(op, inner) ->
		src = emit_tacky(inner, instructions)
		dst_name = make_temporary()
		dst = Var(dst_name)
		tacky_op = convert_unop(op)
		instructions.append(Unary(tacky_op, src, dst))
		return dst
```

**_Notes:_**

- The first Constant is of AST, and the second is of TACKY.
- The first Unary is of AST, and the second is of TACKY.

### Generating Names for Temporary Variables

We keep a counter, and increment it whenever we use it to create a unique name.
Example: tmp.0
This won't conflict with user-defined identifier

### TACKY representation of Unary Expressions

| AST                                                                                                                        | TACKY                                                                                                                                                          |
| -------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(Constant(3))                                                                                                        | Return (Constant(3))                                                                                                                                           |
| Return(Unary(Complement, Constant(2)))                                                                                     | Unary(Complement, Constant(2), Var("tmp.0")) <br>Return(Var("tmp.0"))                                                                                          |
| Return(Unary(Negate,<br>&nbsp;&nbsp;&nbsp;&nbsp;Unary(Complement,<br>&nbsp;&nbsp;&nbsp;&nbsp;Unary(Negate, Constant(8))))) | Unary(Negate, Constant(8), Var("tmp.0"))<br>Unary(Complement, Var("tmp.0"), Var("tmp.1"))<br>Unary(Negate, Var("tmp.1"), Var("tmp.2"))<br>Return(Var("tmp.2")) |

## Assembly Generation

There are a few ways to handle prolouge and epilogue:

- [ ] Add push, pop, sub instructions
- [x] Use high-level constructs for the whole prolouge and epilogue
- [x] Leave out the function prolouge and epilouge and add them during code emission

In TACKY, we introduced temporary variables, which is likely to appear a lot in the program.
How can we represent those variables in Assembly while we only have a limited set of registers?<br>
We'll add one more type of operand in Assembly: Pseudoregister (Pseudo).<br>
As its name, pseudoregisters are to help us not worry about the resources of registers we have, and focus on how
instructions are converted from TAC IR to Assembly.
In Part III, we'll implement a way to allocate as many available registers as possible; for now
each pseudoregister (from temporary variable in TAC) is assigned with a stack memory.<br>
And how do we represent stack memory? We also add one more type of operand: Stack

### Assembly

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
		<strong>| Unary(unary_operator, operand dst)</strong>
		<strong>| AllocateStack(int)</strong>
		| Ret
<strong>unary_operator = Neg | Not</strong>
operand = Imm(int) <strong>| Reg(reg) | Pseudo(identifier) | Stack(int)</strong>
<strong>reg = AX | R10</strong></pre></code>

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct    | Assembly top-level construct |
| ---------------------------- | ---------------------------- |
| Program(function_definition) | Program(function_definition) |
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction               | Assembly instructions                       |
| ------------------------------- | ------------------------------------------- |
| Return(val)                     | Mov(val, Reg(AX))<br>Ret                    |
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst) |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters

- For each pseudoregister, we subtract 4 bytes (we only support int datatype now) from the stack.
- We'll need a map from identifiers to stack offsets.
- And this compiler pass should also return the total stack offset so we can allocate enough stack space for local variables.

### Fixing Up Instructions

- We'll insert AllocateStack instruction at the beginning of the instructions list in the function_definition.
- Fix Mov instructions which have both src and dst as Stack operands.
  So this instruction

```
movl	-4(%rbp), -8(%rbp)
```

is fixed up into:

```
movl	-4(%rbp), %r10d
movl 	%r10d, -8(%rbp)
```

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput                                                                                                                                                                                                                          |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Program(function_definition) | Printout the function definition <br> On Linux, add at the end of file <br> nbsp;&&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                    |
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

#### Formatting Assembly Instructions

| Assembly instruction           | Output                                              |
| ------------------------------ | --------------------------------------------------- |
| Mov(src, dst)                  | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>          |
| Ret                            | ret                                                 |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand> |
| AllocateStack(int)             | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp           |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |

#### Formatting Assembly Operands

| Assembly operand | Output      |
| ---------------- | ----------- |
| Reg(AX)          | %eax        |
| Reg(R10)         | %r10d       |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Summary

We added one more stage in the compiler: TACKY, our name for TAC IR.

## Additional Resources

**Two's Complement:**

- [“Two’s Complement”](https://www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html)
- [Chapter 2 of The Elements of Computing Systems](https://www.nand2tetris.org/course)

### Reference Implementation Analysis

[Chapter 2 Code Analysis](./code_analysis/chapter_2.md)

---

# Chapter 3: Binary Operators

### Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **TACKY Generation**
   - Input: AST
   - Output: TAC IR (Tacky)
4. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
5. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

In this chapter, we'll add support for 5 binary operators:

- Addition
- Subtraction
- Multiplication
- Divison
- Remainder

Also, we'll explore some reasons to use **Precedence Climbing** method in **Recursive Descent Parser**.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Plus | + |
| Star | \* |
| Slash | / |
| Percent | / |

We already added HYPHEN for _-_ operator in the previous chapter.
The lexing stage doens't need to know the role of the token in grammar.

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int) 
	| Unary(unary_operator, exp)
	<strong>| Binary(binary_operator, exp, exp)</strong>
unary_operator = Complement | Negate
<strong>binary_operator = Add | Subtract | Multiply | Divide | Remainder</strong></pre></code>

Designing grammar for binary operators in Recursive Descent Parser is tricky.
Let's say we could naively add the grammar of binary operators in expression like the following:

```
<exp> ::= <int> | <unop> <exp> | <exp> <binop> <exp>
```

This wouldn't work. Why?

- Parsing expression `1 + 2 * 3` would become ambigious as we do not specify we should parse them as `(1 + 2) * 3` or `1 + (2 * 3)`
- It's a left-recursive rule, and in Recursive Descent Parser, we'll be in an unbounded recursion.

There are 2 ways to solve this:

- Refactor our grammar to use pure Recursive Descent Parser.
- Mix Precedence Climbing into Recurisve Descent Parser.

Let's find out which one to use in our project!

### Refactoring the Grammar

```
<exp> ::= <term> {("+"|"-") <term>}
<term> ::= <factor> {("*"|"/"|"%") <factor}
<factor> ::= <int> | <unop> <factor> | "(" <exp> ")"
```

The precedence of operators are represented in the level of non-terminals.
An expression involves only + and -, so they have the lowest precedence.
A term only involves \*, / and %, and they have higher precedence level than expression.

For our problem about, without any parantehesis, `1 + 2 * 3`, it's parsed as following:

```
Step 1. <term> + <term>
Step 2. <factor> + (<factor> <factor>)
Step 3. <int> + (<int> * <int>)
```

This works, but what if we have more operators with more levels of precedence? We'll then need to design several non-terminals for each level.
And modifying grammar each time means we also have to modify our parsing functions. That's a lot of work to do.
How about **Precedence Climbing**?

### Precedence Climbing

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" &lt;statement&gt; "}"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";"
<strong>&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt;</strong>
<strong>&lt;factor&gt; ::= &lt;int&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"</strong>
&lt;unop&gt; ::= "-" | "~"
<strong>&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%"</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

Unary operators and inner expression in parentheses have higher precedence level than binary operators, so they are in deeper non-terminal.
The previous parse_exp function will now become parse_factor, with almost no changes, except for we call parse_factor for inner_exp in Unary as
unary expressions don't accept rules like `-1 + 2`.

```
parse_factor(tokens):
	next_token = peek(tokens)
	if next_token is an int:
		--snip--
	else if next_token is "~" or "-":
		operator = parse_unop(tokens)
		inner_exp = parse_factor(tokens)
		return Unary(operator, inner_exp)
	else if next_token is "(":
		take_token(tokens)
		inner_exp = parse_exp(tokens)
		expect(")", tokens)
		return inner_exp
	else:
		fail("Malformed factor")
```

If we call parse_exp for inner_exp of unary, we'll parse `-1 + 2` as `-(1 + 2)`. Wrong! So calling parse_factor makes sure that
`-1 + 2` is parsed as `(-1) + 2`. Factor does have rules for "(" <exp> ")", so if the input is `-(1 + 2)`, it's still correct.

For the binary operators themselves, we assign precedence value to each of them and perform the following parsing:

```
parse_exp(tokens, min_prec):
	left = parse_factor(tokens)
	next_token = peek(tokens)

	while next_token is a binary operator and precedence(next_token) >= min_prec:
		operator = parse_binop(tokens)
		right = parse_exp(tokens, precedence(next_token) + 1)
		left = Binary(operator, left, right)
		next_token = peek(tokens)

	return left
```

### Precedence Values of Binary Operators

| Operator | Precedence |
| -------- | ---------- |
| \*       | 50         |
| /        | 50         |
| %        | 50         |
| +        | 45         |
| -        | 45         |

The values don't matter as long as higher precedence operators have higher values.

## TACKY Generation

### TACKY

<pre><code>
program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) 
	| Unary(unary_operator, val src, val dst)
	<strong>| Binary(binary_operator, val src1, val src2, val dst)</strong>
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate
<strong>binary_operator = Add | Subtract | Mulitply | Divide | Remainder</strong>
</pre></code>

### Generating TACKY

```
emit_tacky(e, instructions):
	match e with
	--snip--
	| Binary(op, e1, e2):
		v1 = emit_tacky(e1, instructions)
		v2 = emit_tacky(e2, instructions)
		dst_name = make_temporary()
		dst = Var(dst_name)
		tacky_op = convert_binop(op)
		instructions.append(Binary(tacky_op, v1, v2, dst))
		return dst
```

**_Notes:_**
In C standard, subexpressions of e1 and e2 are usually unsequenced. They can be evaluated in any order.

## Assembly Generation

We need new instructions to represent binary operators in Assembly.
For add, subtract, multiply, they have the same form:

```
op	src, dst
```

However, divide and remainder don't follow the same form as we're dealing with dividend, divisor, quotient and remainder.
We need to sign extend the _src_ into rdx:rax before performing the division using `cdq` instruction.
So to compute 9 / 2, we do:

```
movl	$2, -4(%rbp)
movl	$9, %eax
cdq
idivl 	-4(%rbp)
```

The quotient is stored in eax and the remainder is stored in edx.

### Assembly

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
		| Unary(unary_operator, operand dst)
		<strong>| Binary(binary_operator, operand, operand)</strong>
		<strong>| Idiv(operand)</strong>
		<strong>| Cdq</strong>
		| AllocateStack(int)
		| Ret
unary_operator = Neg | Not
<strong>binary_operator = Add | Sub | Mult</strong>
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
reg = AX | <strong>DX</strong> | R10 | <strong>R11</strong></pre></code>

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct    | Assembly top-level construct |
| ---------------------------- | ---------------------------- |
| Program(function_definition) | Program(function_definition) |
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                            | Assembly instructions                                            |
| -------------------------------------------- | ---------------------------------------------------------------- |
| Return(val)                                  | Mov(val, Reg(AX))<br>Ret                                         |
| Unary(unary_operator, src, dst)              | Mov(src, dst)<br>Unary(unary_operator, dst)                      |
| **Binary(Divide, src1, src2, dst)**          | **Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)** |
| **Binary(Remainder, src1, src2, dst)**       | **Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)** |
| **Binary(binary_operator, src1, src2, dst)** | **Mov(src1, dst)<br>Binary(binary_operator, src2, dst)**         |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| **Add**        | **Add**           |
| **Subtract**   | **Sub**           |
| **Multiply**   | **Mult**          |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters

We added two instruction that have operands: _Binary_ and _Idiv_.
Treat them like Mov, Unary.

### Fixing Up Instructions

The _idiv_ can't take a constant operand, so we copy the constant into our scratch register R10.

So this instruction

```
idivl	$3
```

is fixed up into:

```
movl 	$3, %r10d
idivl	%r10d
```

The _add_ and _sub_ instructions cannot memory addresses as both operands, similar to the _mov_ instruction.
From this:

```
addl	-4(%rbp), -8(%rbp)
```

is fixedup into:

```
movl 	-4(%rbp), %r10d
addl	%r10d, -8(%rbp)
```

The _imul_ instruction cannot use memory address as its destination. We'll dedicate R11 to fix up destination, also for later chapters.
From this:

```
imull 	$3, -4(%rbp)
```

to this:

```
movl 	-4(%rbp), %r11d
imull 	$3, %r11d
movl 	%r11d, -4(%rbp)
```

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput                                                                                                                                                                                                                          |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Program(function_definition) | Printout the function definition <br> On Linux, add at the end of file <br> nbsp;&&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                    |
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

#### Formatting Assembly Instructions

| Assembly instruction                  | Output                                                       |
| ------------------------------------- | ------------------------------------------------------------ |
| Mov(src, dst)                         | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                   |
| Ret                                   | ret                                                          |
| Unary(unary_operator, operand)        | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>          |
| **Binary(binary_operator, src, dst)** | **\<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>** |
| **Idiv(operand)**                     | **idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>**                  |
| **Cdq**                               | **cdq**                                                      |
| AllocateStack(int)                    | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                    |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |
| **Add**           | **addl**         |
| **Sub**           | **subl**         |
| **Mult**          | **imull**        |

#### Formatting Assembly Operands

| Assembly operand | Output      |
| ---------------- | ----------- |
| Reg(AX)          | %eax        |
| **Reg(DX)**      | **%eax**    |
| Reg(R10)         | %r10d       |
| **Reg(R11)**     | **%r11d**   |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Extra Credit: Bitwise Operators

Bitwise operators are also binary operators. Add support for:

- AND (&)
- OR (|)
- XOR (^)
- Left Shift (<<)
- Right Shift (>>)

## Summary

We've learned to use precedence climbing method to support 5 binary operators.
In the next chapter, we'll implement more unary and binary operators upon the foundation we've just created.

## Additional Resources

**Two's Complement:**

- [Parsing Expressions by Precedence Climbing](https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing)
- [Some Problems of Recursive Descent Parsers](https://eli.thegreenplace.net/2009/03/14/some-problems-of-recursive-descent-parsers)
- [Pratt Parsing and Precedence Climbing Are the Same Algorithm](https://www.oilshell.org/blog/2016/11/01.html)
- [Precedence Climbing Is Widely Used](https://www.oilshell.org/blog/2017/03/30.html)

### Reference Implementation Analysis

[Chapter 3 Code Analysis](./code_analysis/chapter_3.md)

---

# Chapter 4: Logical and Relational Operators

### Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **TACKY Generation**
   - Input: AST
   - Output: TAC IR (Tacky)
4. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
5. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

We'll add three logical operators:

- Not (!)
- And (&&)
- Or (\|\|)

and relational operators: ==, !=, < , > , <=, >=

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Bang | ! |
| LogicalAnd | && |
| LogicalOr | \|\| |
| DoubleEqual | == |
| NotEqual | != |
| LessThan | < |
| GreaterThan | > |
| LessOrEqual | <= |
| GreaterOrEqual | >= |

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int) 
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
unary_operator = Complement | Negate <strong>| Not </strong>
binary_operator = Add | Subtract | Multiply | Divide | Remainder <strong>| And | Or</strong>
				<strong>| Equal | NotEqual | LessThan | LessOrEqual</strong>
				<strong>| GreaterThan | GreaterOrEqual</strong></pre></code>

### Parser

First, update parse_factor to handle the new ! operator, which should be parsed the similarly to Negate and Complement.
Then, update parse_exp to handle new binary operators.

### Precedence Values of Binary Operators

| Operator | Precedence |
| -------- | ---------- |
| \*       | 50         |
| /        | 50         |
| %        | 50         |
| +        | 45         |
| -        | 45         |
| <        | 35         |
| <=       | 35         |
| >        | 35         |
| >=       | 35         |
| ==       | 30         |
| !=       | 30         |
| &&       | 10         |
| \|\|     | 5          |

This table is spaced enough for other binary operators implemented in Extra Credit section in chapter 3.
Extend both parse_unop and parse_binop.

## TACKY Generation

Relational operators are converted to TACKY the same way as implemented binary operators.
However, for && and \|\|, we cannot do this as our TACKY evaluates both expressions in binary, while && and \|\| sometimes allow to skip the 2nd expression.
That's why we're going to add _unconditional jump_ and _conditional jump_ to support the two _short-circuit_ operators.

### TACKY

<pre><code>
program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) 
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	<strong>|Copy(val src, val dst)</strong>
	<strong>|Jump(identifier target)</strong>
	<strong>|JumpIfZero(val condition, identifier target)</strong>
	<strong>|JumpIfNotZero(val condition, identifier target)</strong>
	<strong>|Label(identifier)</strong>
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate <strong>| Not</strong>
binary_operator = Add | Subtract | Mulitply | Divide | Remainder <strong>| Equal | Not Equal</strong>
				<strong>| LessThan | LessOrEaual | GreaterThan | GreaterOrEqual</strong>
</pre></code>

- **Jump** instruction works in goto in C.
- **JumpIfZero** evaluates the condition, if condition is 0, then jump to the target, skip to the next instruction otherwise.
- **JumpIfNotZero** does the opposite to the _JumpIfZero_
- **Copy** to move 1, or 0 based on the results of && and ||, into a destination

### Generating TACKY

**Convert Short-Circuiting Operators to TACKY**

For &&

```
<instructions for e1>
v1 = <result of e1>
JumpIfZero(v1, false_label)
<instructions for e2>
v2 = <result of e2>
JumpIfZero(v2, false_label)
result = 1
Jump(end)
Label(false_label)
result = 0
Label(end)
```

For ||, it's the same, except we change the JumpIfZero(value, false_label) into JumpIfNotZero(value, true_label)

**Generating Labels**

Unlike temporary variables, labels will appear in the final assembly program, so the names generated for labels must be valid.
Label names accept letters, digits, periods and underscores.

It's ok if our autogenerated labels conflict with user-defined names, we'll mangle them during code emission stage.

## Assembly Generation

### Assembly

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
		| Unary(unary_operator, operand dst)
		| Binary(binary_operator, operand, operand)
		<strong>| Cmp(operand, operand)</strong>
		| Idiv(operand)
		| Cdq
		<strong>| Jmp(identifier)</strong>
		<strong>| JmpCC(cond_code, identifier)</strong>
		<strong>| SetCC(cond_code, operand)</strong>
		<strong>| Label(identifier)</strong>
		| AllocateStack(int)
		| Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
<strong>cond_code = E | NE | G | GE | L | LE</strong>
reg = AX | DX | R10 | R11</pre></code>

All conditional jumps have the same form, and differ only at the condition code. So we represent them using the same construct, but with different cond_code member.
We do the same for Set instructions.

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct    | Assembly top-level construct |
| ---------------------------- | ---------------------------- |
| Program(function_definition) | Program(function_definition) |
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                                | Assembly instructions                                                      |
| ------------------------------------------------ | -------------------------------------------------------------------------- |
| Return(val)                                      | Mov(val, Reg(AX))<br>Ret                                                   |
| Unary(unary_operator, src, dst)                  | Mov(src, dst)<br>Unary(unary_operator, dst)                                |
| **Unary(Not, src, dst)**                         | **Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst)**                  |
| Binary(Divide, src1, src2, dst)                  | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)               |
| Binary(Remainder, src1, src2, dst)               | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)               |
| Binary(binary_operator, src1, src2, dst)         | Mov(src1, dst)<br>Binary(binary_operator, src2, dst)                       |
| **Binary(relational_operator, src1, src2, dst)** | **Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst)** |
| **Jump(target)**                                 | **Jmp(target)**                                                            |
| **JumpIfZero(condition, target)**                | **Cmp(Imm(0), condition)<br>SetCC(E, target)**                             |
| **JumpIfNotZero(condition, target)**             | **Cmp(Imm(0), condition)<br>SetCC(NE, target)**                            |
| **Copy(src, dst)**                               | **Mov(src, dst)**                                                          |
| **Label(identifier)**                            | **Label(identifier)**                                                      |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| Add            | Add               |
| Subtract       | Sub               |
| Multiply       | Mult              |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison   | Assembly condition code |
| ------------------ | ----------------------- |
| **Equal**          | **E**                   |
| **NotEqual**       | **NE**                  |
| **LessThan**       | **L**                   |
| **LessOrEqual**    | **LE**                  |
| **GreaterThan**    | **G**                   |
| **GreaterOrEqual** | **GE**                  |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters

We added two instruction that have operands: _Cmp_ and _SetCC_.
Update this pass to replace any pseudoregisters used in these.

### Fixing Up Instructions

The _cmp_ instructions, like _mov_, _add_ and _sub_, cannot have memory addresses for both operands.
So the instruction

```
cmpl	-4(%rbp), -8(%rbp)
```

is fixed up into

```
movl 	-4(%bp), %r10d
cmpl	%r10d, -8(%rbp)
```

Also, _cmp_, similarly to _sub_, cannot have destination as constant.
The instruction:

```
cmpl	%eax, $5
```

is fixed up into

```
movl 	%5, %r11d
cmpl	%eax, %r11d
```

From the convention of the previous chapter, we use R10 to fix the first operand (source) and R11 to fix the second operand (destination)

## Code Emission

As mentioned in the TACKY stage, we'll mangle the conflicts between labels and user-defined identifiers by adding local prefix to our labels.
The prefix is .L on linux and L on MacOS.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput                                                                                                                                                                                                                          |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Program(function_definition) | Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                    |
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

#### Formatting Assembly Instructions

| Assembly instruction              | Output                                                   |
| --------------------------------- | -------------------------------------------------------- |
| Mov(src, dst)                     | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>               |
| Ret                               | ret                                                      |
| Unary(unary_operator, operand)    | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>      |
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Idiv(operand)                     | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                  |
| Cdq                               | cdq                                                      |
| AllocateStack(int)                | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                |
| **Cmp(operand, operand)**         | **cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>**   |
| **Jmp(label)**                    | **jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>**                |
| **JmpCC(cond_code, label)**       | **j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>**      |
| **SetCC(cond_code, operand)**     | **set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>**    |
| **Label(label)**                  | **.L\<label>:**                                          |

**Notes**:

- _jmp_ and _.L\<label>_ don't have size suffixes as they don't take operands.
- _jmpcc_ and _setcc_ do need suffixes to indicate the size of the condition they test.

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |
| Add               | addl             |
| Sub               | subl             |
| Mult              | imull            |

**Instruction Suffixes for Condition Codes**

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| **E**          | **e**              |
| **NE**         | **ne**             |
| **L**          | **l**              |
| **LE**         | **le**             |
| **G**          | **g**              |
| **GE**         | **ge**             |

#### Formatting Assembly Operands

| Assembly operand    | Output      |
| ------------------- | ----------- |
| Reg(AX) 4-byte      | %eax        |
| **Reg(AX) 1-byte**  | **%al**     |
| Reg(DX) 4-byte      | %eax        |
| **Reg(DX) 1-byte**  | **%dl**     |
| Reg(R10) 4-byte     | %r10d       |
| **Reg(R10) 1-byte** | **%r10b**   |
| Reg(R11) 4-byte     | %r11d       |
| **Reg(R11) 1-byte** | **%r11b**   |
| Stack(int)          | <int>(%rbp) |
| Imm(int)            | $\<int>     |

## Summary

Relational and short-circuiting operators are important to let programs branch and make decisions.
The implementation of this step is a foundation for our _if_ statement and loops later chapters.

## Additional Resources

- [A Guide to Undefined Behavior in C and C++, Part 1](https://blog.regehr.org/archives/213)
- [With Undefined Behavior, Anything Is Possible](https://raphlinus.github.io/programming/rust/2018/08/17/undefined-behavior.html)

### Reference Implementation Analysis

[Chapter 4 Code Analysis](./code_analysis/chapter_4.md)

---

# Chapter 5: Local Variables

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
     1. Variable resolution
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

In this chapter, we'll add assignment, support local variables in C. Thus, we introduce a new stage in our compiler: Semantic analysis.

A variable declaration consists of the variable's type, name and optional expression (intializer to specify initial value).<br>
Variable assignment is an expression that has side-effect (updating the variable value), and we, most of the time, do not care its resulting value.
Variable is also an expression type.<br>
An expression that can appear on the left side of an assignment is called _lvalue_.<br> For now, our only _lvalue_ is variable.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| EqualSign | = |

Variable names are just identifiers, and our lexer has already recognized them.

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, <strong>block_item*</strong> body)
<strong>block_item = S(statement) | D(declaration)</strong>
<strong>declaration = Declaration(identifier name, exp? init)</strong>
statement = Return(exp) <strong>| Expression(exp) | Null </strong>
exp = Constant(int) 
	<strong>| Var(identifier)</strong> 
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
	<strong>| Assignment(exp, exp)</strong> 
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual</pre></code>

- Null statement is added in this stage here just because of its simplicity. It has nothing to do with local variables.
- Declarations are different from statements, so we need a separate node for declarations.
- Statements can finally be compiled and executed when the program runs, while declarations are simply to tell the compiler that some identifiers exist and ready to be used.

We've defined our function to have the body of a single statement. That's changed now. A function body can now contain a list of **block items**. A **block item** can be either a declaration or a statement.

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" <strong>{ &lt;block-item&gt; }</strong> "}"
<strong>&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;</strong>
<strong>&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"</strong>
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" <strong>| &lt;exp&gt; ";" | ";"</strong>
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt;
&lt;factor&gt; ::= &lt;int&gt; <strong>| &lt;identifier&gt;</strong> | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "<" | "<=" | ">" | ">=" <strong>| "="</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

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

To parse*block_item, peek the first token. If it's the \_int* keyword, it's a declaration, a statement otherwise.

Also, our "=" operator is right associative, different from any previous binary operators. Let's update our parse_exp:

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

### Precedence Values of Binary Operators

| Operator | Precedence |
| -------- | ---------- |
| \*       | 50         |
| /        | 50         |
| %        | 50         |
| +        | 45         |
| -        | 45         |
| <        | 35         |
| <=       | 35         |
| >        | 35         |
| >=       | 35         |
| ==       | 30         |
| !=       | 30         |
| &&       | 10         |
| \|\|     | 5          |
| **=**    | **1**      |

**Note**:

- Our Parser allows any expression can be on the left handside of an assignment.
- Our Semantic Analysis stage will check if the expression is actually a valid _lvalue_ expression.
- We check for the validity of _lvalue_ not in Parser but in Semantic Analysis because we will support more complex lvalues later.

## Semantic Analysis

### Variable Resolution

```
resolve_declaration(Declaration(name, init), variable_map):
	if name is in variable_map:
		fail("Duplicate variable declaration!")
	unique_name = make_temporary()
	variable_map.add(name, unique_name)
	if init is not null:
		init = resolve_exp(init, variable_map)
	return Declaration(unique_name, init)
```

```
resolve_statement(statement, variable_map):
	match statement with
	| Return(e) -> return Return(resolve_exp(e, variable_map))
	| Expression(e) -> return Expression(resolve_exp(e, variable_map))
	| Null -> return Null
```

```
resolve_exp(exp, variable_map):
	match e with
	| Assignment(left, right) ->
		if left is not a Var node:
			fail("Invalid lvalue")
		return Assignment(resolve_exp(left, variable_map), resolve_exp(right, variable_map))
	| Var(v) ->
		if v is in variable_map:
			return Var(variable_map.get(v))
		else:
			fail("Undeclared variable!")
	| --snip--
```

In resolve_exp, for other kinds of expression that are not Assignment nor Var, we simply process any subexpression with resolve_exp.
Ultimately, this pass should return a transformed AST that uses autogenerated instead of user-defined identifiers.

### Update compiler driver

Make sure you add a new stage for compiler driver. Let's use **--validate** flag.

## TACKY Generation

### TACKY

We don't modify anything in TACKY as it already supports Var and Copy to assign values to Var.

### Generating TACKY

We'll need to extend the Tacky Generation to support two new expression nodes in AST: Var and Assignment.

```
emit_tacky(e, instructions):
	match e with
	| --snip--
	| Var(v) -> Var(v)
	| Assignment(Var(v), rhs) ->
		result = emit_tacky(rhs, instructions)
		instructions.append(Copy(result, Var(v)))
		return Var(v)
```

If a declaration in AST has initializer, we treat it as an assignment. Otherwise, we omit it in the TACKY program.
For expression statement, we call emit_tacky for the inner expression of the statement. This will return a new temporary variable holding the result of the expression, but we ignore the variable.

Emit an extra Return(Constant(0)) to the end of every function. If the function already has a return statement, it won't reach the guard one.

## Assembly Generation

We do not change our TACKY AST, so Assembly stage stays intact. You can still reference the whole converting below.

### Assembly

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
        | Unary(unary_operator, operand dst)
        | Binary(binary_operator, operand, operand)
        | Cmp(operand, operand)
        | Idiv(operand)
        | Cdq
        | Jmp(identifier)
        | JmpCC(cond_code, identifier)
        | SetCC(cond_code, operand)
        | Label(identifier)
        | AllocateStack(int)
        | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
cond_code = E | NE | G | GE | L | LE
reg = AX | DX | R10 | R11</pre></code>

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct    | Assembly top-level construct |
| ---------------------------- | ---------------------------- |
| Program(function_definition) | Program(function_definition) |
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                            | Assembly instructions                                                  |
| -------------------------------------------- | ---------------------------------------------------------------------- |
| Return(val)                                  | Mov(val, Reg(AX))<br>Ret                                               |
| Unary(unary_operator, src, dst)              | Mov(src, dst)<br>Unary(unary_operator, dst)                            |
| Unary(Not, src, dst)                         | Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst)                  |
| Binary(Divide, src1, src2, dst)              | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)           |
| Binary(Remainder, src1, src2, dst)           | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)           |
| Binary(binary_operator, src1, src2, dst)     | Mov(src1, dst)<br>Binary(binary_operator, src2, dst)                   |
| Binary(relational_operator, src1, src2, dst) | Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst) |
| Jump(target)                                 | Jmp(target)                                                            |
| JumpIfZero(condition, target)                | Cmp(Imm(0), condition)<br>SetCC(E, target)                             |
| JumpIfNotZero(condition, target)             | Cmp(Imm(0), condition)<br>SetCC(NE, target)                            |
| Copy(src, dst)                               | Mov(src, dst)                                                          |
| Label(identifier)                            | Label(identifier)                                                      |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| Add            | Add               |
| Subtract       | Sub               |
| Multiply       | Mult              |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code |
| ---------------- | ----------------------- |
| Equal            | E                       |
| NotEqual         | NE                      |
| LessThan         | L                       |
| LessOrEqual      | LE                      |
| GreaterThan      | G                       |
| GreaterOrEqual   | GE                      |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

## Code Emission

Remains unchanged as there are no modifications in the Assembly stage.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput                                                                                                                                                                                                                          |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Program(function_definition) | Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                    |
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

#### Formatting Assembly Instructions

| Assembly instruction              | Output                                                   |
| --------------------------------- | -------------------------------------------------------- |
| Mov(src, dst)                     | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>               |
| Ret                               | ret                                                      |
| Unary(unary_operator, operand)    | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>      |
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Idiv(operand)                     | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                  |
| Cdq                               | cdq                                                      |
| AllocateStack(int)                | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                |
| Cmp(operand, operand)             | cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>       |
| Jmp(label)                        | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                    |
| JmpCC(cond_code, label)           | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>          |
| SetCC(cond_code, operand)         | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>        |
| Label(label)                      | .L\<label>:                                              |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |
| Add               | addl             |
| Sub               | subl             |
| Mult              | imull            |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |

#### Formatting Assembly Operands

| Assembly operand | Output      |
| ---------------- | ----------- |
| Reg(AX) 4-byte   | %eax        |
| Reg(AX) 1-byte   | %al         |
| Reg(DX) 4-byte   | %eax        |
| Reg(DX) 1-byte   | %dl         |
| Reg(R10) 4-byte  | %r10d       |
| Reg(R10) 1-byte  | %r10b       |
| Reg(R11) 4-byte  | %r11d       |
| Reg(R11) 1-byte  | %r11b       |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Extra Credit: Compound Assignment, Increment, and Decrement

Add support for compound assignments:
+=, -=, \*=, /=, %=

We've also implemented Extra Credit in chapter 3, so we also support:
&=, |=, ^=, <<=, >>=

Increment and decrement also can be implemented:
++, --

Note that ++ and -- can be either prefix or postfix their operands.

## Summary

This chapter is a milestone as we added new kind of statement and first construct that has side effect.
Also the Semantic Analysis stage is implemented to make sure the program makes sense.

### Reference Implementation Analysis

[Chapter 5 Code Analysis](./code_analysis/chapter_5.md)

---

# Chapter 6: If Statements and Conditional Expressions

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
     1. Variable resolution
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

Control flow lets a program decide what statements to execute at runtime based on the current state of the program.
We'll support **If Statements** and **Conditional Expressions** in this chapter.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordIf | if |
| KeywordElse | else |
| QuestionMark | ? |
| Colon | \: |

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, block_item* body)
block_item = S(statement) | D(declaration)
declaration = Declaration(identifier name, exp? init)
statement = Return(exp) 
	| Expression(exp) 
	<strong>| If(exp condition, statement then, statement? else)</strong>
	| Null 
exp = Constant(int) 
	| Var(identifier) 
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	<strong>| Conditional(exp condition, exp, exp)</strong>
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual</pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" 
	| &lt;exp&gt; ";" 
	<strong>| "if" "(" &lt;exp&gt; ")" &lt;statement&gt; ["else" &lt;statement&gt;]</strong>
	| ";"
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; <strong>| &lt;exp&gt "?" &lt;exp&gt ":" &lt;exp&gt</strong>
&lt;factor&gt; ::= &lt;int&gt; | &lt;identifier&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

Firstly, let's tackle the **If** construct.
We've not implemented compound statements yet, so we can only compile the following example:

```
if (a == 3)
	return a;
else
	b = 8;
```

AST definition of the **If** construct doesn't include _else if_ because an if statement can have at most one else clause. An else if clause is an else with another if statement.

So this example:

```
if (a):
	return 0;
else if (b):
	return 1;
else:
	return 2;
```

is similar to this:

```
if (a):
	return 0;
else:
	if (b):
		return 1;
	else:
		return 2;
```

Interestingly, the grammar to parse the if-else is also ambiguous.

```
"if" "(" <exp> ")" <statement> ["else" <statement>]
```

If the statement right after the if(condition) is another if statement, then the else statement, if exists, belongs to which if?
The grammar itself doesn't tell this. Luckily, this is a famous quirk that has it own name: _dangling else ambiguity_.

The dangling else ambiguity cause problems for parser generators, not our handwritten recursive descent parser.
When we parse the if statement, if we see an else statement right after it, attach it the the closest if.

Now, how about **condition expression**?
The expression has its form: `condition ? exp : exp`
We treat it as a binary expression, whose operator is the whole _? exp :_.
Such operator has higher precedence than assignments, but lower than anything else.
Condition expressions are right associative.

### Parser

```
parse_exp(tokens, min_prec):
	left = parse_factor(tokens)
	next_token = peek(tokens)

	while next_token is a binary operator and precedence(next_token) >= min_prec:
		if next_token is "=":
			take_token(tokens) // remove "=" from list of tokens
			right = parse_exp(tokens, precedence(next_token))
			left = Assignment(left, right)
		else if next_token is "?":
			middle = parse_condition_middle(tokens)
			right = parse_exp(tokens, precedence(next_token))
			left = Conditional(left, middle, right)
		else:
			operator = parse_binop(tokens)
			right = parse_exp(tokens, precedence(next_token) + 1)
			left = Binary(operator, left, right)
		next_token = peek(tokens)
	return left
```

### Precedence Values of Binary Operators

| Operator | Precedence |
| -------- | ---------- |
| \*       | 50         |
| /        | 50         |
| %        | 50         |
| +        | 45         |
| -        | 45         |
| <        | 35         |
| <=       | 35         |
| >        | 35         |
| >=       | 35         |
| ==       | 30         |
| !=       | 30         |
| &&       | 10         |
| \|\|     | 5          |
| **?**    | **3**      |
| =        | 1          |

## Semantic Analysis

### Variable Resolution

Extend the _resolve_statement_ and _resolve_exp_ to handle new constructs.

## TACKY Generation

### TACKY

We leverage the constructs used to support && and || operator in chapter 4 for if statements and conditional expressions.
So we do not make any changes to the TACKY for now.

### Generating TACKY

To **convert If Statements to TACKY**, we do the following:

```
<instructions for condition>
c = <result of condition>
JumpIfZero(c, end)
<instructions for statement>
Label(end)
```

With an else, add a few more instructions:

```
<instructions for condition>
c = <result of condition>
JumpIfZero(c, else_label)
<instructions for statement1>
Jump(end)
Label(else_label)
<instructions for statement2>
Label(end)
```

**Converting Conditional Expressions to TACKY** is very similar, except for that we need to copy which result to the destination.

```
<instructions for condition>
c = <result of condition>
JumpIfZero(c, e2_label)
<instructions to calculate e1>
v1 = <result of e1>
result = v1
Jump(end)
Label(e2_label)
<instructions to calculate e2>
v2 = <result of e2>
result = v2
Label(end)
```

## Assembly Generation

We do not change our TACKY AST, so Assembly stage stays intact.

## Extra Credit: Labeled Statements and Goto

If we can support _If Statements_ and _Conditional Expressions_, we can move forward to add support for **Goto** and **Labeled Statements**.
Goto is like Jump in Assembly, whereas Labeled Statements specify the target for Goto.
A new pass in Semantic Analysis is needed to check for the errors like using the same label for two labeled statements.

## Summary

Congratulations! We've just implemented the first control structures in our compiler. However, we are limited by having a single statement in each clause.
We'll extend to use compound statement, which is also a single statement but contains several inner statements in the next chapter.

### Reference Implementation Analysis

[Chapter 6 Code Analysis](./code_analysis/chapter_6.md)

---

# Chapter 7: Compound Statements

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
     1. Variable resolution
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

We will only change the **Parser**, **Variable Resolution** and a bit of **TackyGen** to support **compound statements**.
We won't touch _Lexer_ or the **CodeGen** stages at all.

Compound statements are to:

- Group statements and declarations into a single unit.
- Delineate the different _scopes_ within a function.

## The Parser

### AST

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

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" <strong>&lt;block&gt;</strong>
<strong>&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"</strong>
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" 
	| &lt;exp&gt; ";" 
	| "if" "(" &lt;exp&gt; ")" &lt;statement&gt; ["else" &lt;statement&gt;]
	<strong>| &lt;block&gt;</strong>
	| ";"
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt "?" &lt;exp&gt ":" &lt;exp&gt
&lt;factor&gt; ::= &lt;int&gt; | &lt;identifier&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

When we parse a \<statement>, a "{" token tells us that we've hit a compound statement.

## Semantic Analysis

### Variable Resolution

```
resolve_declaration(Declaration(name, init), variable_map):
	if name is in variable_map and variable_map.get(name).from_current_block:
		fail("Duplicate variable declaration!")
	unique_name = make_temporary()
	variable_map.add(name, MapEntry(new_name=unique_name, from_current_block=true))
	if init is not null:
		init = resolve_exp(init, variable_map)
	return Declaration(unique_name, init)
```

```
resolve_statement(statement, variable_map):
	match statement with
	| Return(e) -> return Return(resolve_exp(e, variable_map))
	| Expression(e) -> return Expression(resolve_exp(e, variable_map))
	| Compound(block) ->
		new_variable_map = copy_variable_map(variable_map)
		return Compound(resolve_block(block, variable_map)
	| Null -> return Null
```

Finally, the function _copy_variable_map_ simply copies the whole variable map, but set from_current_block to false to each entry.

## TACKY Generation

### TACKY

No modifications needed!

### Generating TACKY

Straightforwardly, to emit TACKY for a compound statement, we emit TACKY for each block item in it. This should use the same function with the function body.

## Assembly Generation

We do not change our TACKY AST, so Assembly stage stays intact.

## Summary

We've supported a new kind of statements and extended our Variable Resolution pass to be a little more sophisticated. It's important for us to implement loops in the next chapter.

### Reference Implementation Analysis

[Chapter 7 Code Analysis](./code_analysis/chapter_7.md)

---

# Chapter 8: Loops

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
     1. Variable resolution
     2. Loop Labeling
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
     1. Converting TACKY to Assembly
     2. Replacing pseudoregisters
     3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

In this chapter, we'll add support for:

- While loop
- DoWhile loop
- ForLoop
- Break
- Continue

We need to update our lexer, parser to support all 5 new statements and add a new semantic analysis pass called _loop labeling_.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordDo | do |
| KeywordWhile | while |
| KeywordFor | for |
| KeywordBreak | break |
| KeywordContinue | continue |

## The Parser

### AST

<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, block body)
block_item = S(statement) | D(declaration)
block = Block(block_item*)
declaration = Declaration(identifier name, exp? init)
<strong>for_init = InitDecl(declaration) | InitExp(exp?)</strong>
statement = Return(exp) 
	| Expression(exp) 
	| If(exp condition, statement then, statement? else)
	| Compound(block)
	<strong>| Break
	| Continue
	| While(exp condition, statement body)
	| DoWhile(statement body, exp condition)
	| For(for_init init, exp? condition, exp? post, statement body)</strong>
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

### EBNF

<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" &lt;block&gt;
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
<strong>&lt;for-init&gt; ::= &lt;declaration&gt; | [ &lt;exp&gt; ] ";"</strong>
&lt;statement&gt; ::= "return" &lt;exp&gt; ";" 
	| &lt;exp&gt; ";" 
	| "if" "(" &lt;exp&gt; ")" &lt;statement&gt; ["else" &lt;statement&gt;]
	| &lt;block&gt;
	<strong>| "break" ";"
	| "continue" ";"
	| "while" "(" &lt;exp&gt; ")" &lt;statement&gt;
	| "do" &lt;statement&gt; "while" "(" &lt;exp&gt; ")" ";"
	| "for" "(" &lt;for-init&gt; [ &lt;exp&gt; ] ";" [ &lt;exp&gt; ] ")" &lt;statement&gt;</strong>
	| ";"
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
&lt;factor&gt; ::= &lt;int&gt; | &lt;identifier&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

We can write helper function to parse optional expressions, specifically two optional expressions in a _for_ loop header.

## Semantic Analysis

### Variable Resolution

For _while_ and _do_ loops, we can simply recursively resolve*statement of their substatements and subexpressions.
For \_break* and _continue_, we don't do anything to them as they have no substatements nor subexpressions.

The _for_ loop is more complicated as its header introduces a new variable scope.

```
resolve_statement(statement, variable_map):
	match statement with
	| --snip-- // While, do, break and continue are simple to implement here as well.
	| For(init, condition, post, body) ->
		new_variable_map = copy_variable_map(variable_map)
		init = resolve_for_init(init, new_variable_map)
		condition = resolve_optional_exp(condition, new_variable_map)
		post = resolve_optional_exp(post, new_variable_map)
		body = resolve_statement(body, new_variable_map)
		return For(init, condition, post, body)
```

```
resolve_for_init(init, variable_map):
	match init with
	| InitExp(e) -> return InitExp(resolve_optional_exp(e, variable_map))
	| InitDecl(d) -> return InitDecl(resolve_declaration(d, variable_map))
```

_resolve_optional_exp_ is omitted, as it's simple as: it calls _resolve_exp_ if the expression is present, does nothing otherwise.

### Loop Labeling

We traverse the AST tree again. This time, whenever we encounter a loop statement, we generate a unique ID and annotate to the ID to it.
We then traverse deeply into its statements to see if it uses any break or continue, if yes we also annotate the ID to the break/continue.

```
label_statement(statement, current_label)
	match statement with:
	| Break ->
		if current_label is null:
			fail("break statement outside of loop")
		return annotate(break, current_label)
	| Continue ->
		if current_label is null:
			fail("continue statement outside of loop")
		return annotate(continue, current_label)
	| While(condition, body) ->
		new_label = make_label()
		labeled_body = label_statement(body, new_label)
		labeled_statement = While(condition, labeled_body)
		return annotate(labeled_statement, new_label)
	| --snip--
	// For DoWhile and For loop are processed  the same way as While.
```

## TACKY Generation

### TACKY

No modifications needed!

### Generating TACKY

Both _break_ and _continue_ are just jump instructions in TACKY (and Assembly). Question is where to jump to!
Generally, the label to break is always the last instructions of the total loop, outside the scope of the loop iterations; the label to continue is put right after the last instructions of the loop body.

**DoWhile**

```
Label(start)
<instructions for body>
Label(continue_label)
<instructions for condition>
v = <result of condition>
JumpIfNotZero(v, start)
Label(break_label)
```

**While**

```
Label(continue_label)
<instructions for condition>
v = <result of condition>
JumpIfZero(v, break_label)
<instructions for body>
Jump(continue_label)
Label(break_label)
```

**For**

```
<instructions for init>
Label(start)
<instructions for condition>
v = <result of condition>
JumpIfZero(v, break_label)
<instructions for body>
Label(continue_label)
<instructions for post>
Jump(Start)
Label(break_label)
```

## Assembly Generation

We do not change our TACKY AST, so Assembly stage stays intact.

## Extra Credit: Switch Statements

To support switch case statements, we're required to significantly change the Loop Labeling pass.
_break_ can be used to break out of a switch, while _continue_ is only for loops.
Let's create another pass in the Semantic Analysis stage to collect all the cases in a switch statements.

## Summary

We added three loop statements, break and continue statements. We also had a new Semantic Analysis pass to associate each _break_ and _continue_ to their enclosing loops.

### Reference Implementation Analysis

[Chapter 8 Code Analysis](./code_analysis/chapter_8.md)

---

# Chapter 9: Functions

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
   1. Variable resolution
   2. Type Checking
   3. Loop Labeling
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
   1. Converting TACKY to Assembly
   2. Replacing pseudoregisters
   3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

- In this chapter, we'll dive in _Functions_, chunks of code that can be defined in one place and called in another.
- Functions are complex, to the point that an instruction is dedicated to call them.
- Functions are implemented as _subroutines_ in the Assembly level.

We'll expand the Semantic Analysis stage by adding another pass: **Type Checking**

## Compiler Driver

Firstly, update our Compiler Driver to recognize the -c flag.
When we see this flag, we compile our C program to assembly as usual, then run the following command to convert it to object file:

```
gcc -c ASSEMBLY_FILE -o OUTPUT_FILE
```

We also need to update the Compiler to be able to accept multiple input source files.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| COMMA | , |

## The Parser

### AST

<pre><code>program = Program(<strong>function_declaration*</strong>)
<strong>declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init)
function_declaration = (identifier name, identifier* params, block? body)</strong>
block_item = S(statement) | D(declaration)
block = Block(block_item*)
for_init = InitDecl(<strong>variable_declaration</strong>) | InitExp(exp?)
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
exp = Constant(int) 
	| Var(identifier) 
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	<strong>| FunctionCall(identifier, exp* args)</strong>
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual</pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= <strong>{ &lt;function-declaration&gt; }</strong>
<strong>&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= "int" &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= "int" &lt;identifier&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;param-list&gt; ::= "void" | "int" &lt;identifier&gt; { "," "int" &lt;identifier&gt; }</strong>
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;for-init&gt; ::= <strong>&lt;variable-declaration&gt;</strong> | [ &lt;exp&gt; ] ";"
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
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
&lt;factor&gt; ::= &lt;int&gt; | &lt;identifier&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	<strong>| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }</strong>
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

### Parser

Function calls have higher precedence than any unary or binary expressions, so we'll parse them in factor.
If a \<factor> starts with an identifier, look ahead to check if the next token is "(". If it is, it's a function call.

## Semantic Analysis

Now that we support functions, and function names are also identifiers, we need to extend our Variable Resolution pass to check the function names as well. The name Variable Resolution is changed to **Identifier Resolution**.

**Note**:
We have some more errors to check for:

- Every function declaration of one type must have the same number of parameters (and types in the future).
- Variables aren't used as functions and vice versa.
- Functions are called with correct number of arguments (types and orders in the future).

These errors have nothing to do with _scope_, which is the primary job of Identifer Resolution. We need another pass for these which is called **Type Checking**.

### Identifier Resolution

Change _variable_map_ to _identifier_map_, and _from_current_block_ to _from_current_scope_.

```
resolve_exp(e, identifier_map):
	match e with:
	| --snip--
	| FunctionCall(fun_name, args) ->
		if fun_name is in identifier_map:
			new_fun_name = identifier_map.get(fun_name).new_name
			new_args = []
			for arg in args:
				new_args.append(resolve_exp(arg, identifier_map))

			return FunctionCall(new_fun_name, new_args)
		else:
			fail("Undeclared function!")
```

```
resolve_function_declaration(decl, identifier_map):
	if decl.name is in identifier_map:
		prev_entry = identifier_map.get(decl.name)
		if prev_entry.from_current_scope and (not prev_entry.has_linkage)
			fail("Duplicate declaration")

		identifier_map.add(decl.name, MapEntry(
			new_name=decl.name, from_current_scope=True, has_linkage=True
		))

		inner_map = copy_identifier_map(identifier_map)
		new_params = []
		for param in decl.params:
			new_params.append(resolve_param(param, inner_map))

		new_body = null
		if decl.body is not null:
			new_body = resolve_block(decl.body, inner_map)

		return (decl.name, new_params, new_body)
```

_resolve_param_ is omitted as the logic is the same as resolving variable declarations.

For local variable declaration, we resolve exactly the same as previous chapters. Make sure these local variables have no linkage.
For local function declaration (inside function body), check if it has a body. If it does, throw an error, otherwise call _resolve_function_declaration_.

### Type Checker

Variables have types like _int_, _long_, _double_.
A function's type depends on its _return type_ and the _types of its parameters_.

Firstly, we need a way to represent types in the compiler.

```
type = Int | FunType(int param_count)
```

For now, we only support int for variable. And as int is the only types for variables as well as parameters, the only information we need to keep track at the moment is how many parameters a function has.

In Part I of the book, this pass doens't transform AST, we just simply add entries to the symbol tables and check for errors.

```
typecheck_variable_declaration(decl, symbols):
	symbols.add(decl.name, Int)
	if decl.init is not null:
		typecheck_exp(decl.init, symbols)
```

```
typecheck_function_declaration(decl, symbols):
	fun_type = FunType(length(decl.params))
	has_body = decl.body is not null
	already_defined = False

	if decl.name is in symbols:
		old_decl = symbols.get(decl.name)
		if old_decl.type != fun_type:
			fail("Incompatiable function declaration")
		already_defined = old_decl.defined
		if already_defined and has_body:
			fail("Function is defined more than once")

	symbols.add(decl.name, fun_type, defined=(already_defined or has_body)) // Overwrite the existing entry if any, but ok as we already make sure the function types are the same above.

	if has_body:
		for param in decl.params:
			symbols.add(param, Int)
		typecheck_block(decl.body)
```

Note that symbols table includes all declarations we've type checked so far, regardless of the scope.
Why no scope? Functions can be declared several times, in the top-level or locally. In the future, variables can be global as well.

```
typecheck_exp(e, symbols):
	match e with:
	| FunctionCall(f, args) ->
		f_type = symbols.get(f).type
		if f_type == Int:
			fail("Variable used as function name")
		if f_type.param_count != length(args):
			fail("Function called with the wrong number of arguments")

		for arg in args:
			typecheck_exp(arg)

	| Var(v):
		if symbols.get(v).type != Int:
			fail("Function name used as variable")
	| --snip--
```

## TACKY Generation

### TACKY

<pre><code>
program = Program(<strong>function_definition*</strong>)
function_definition = Function(identifier, <strong>identifier* params</strong>, instruction* body)
instruction = Return(val) 
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| Jump(identifier target)
	| JumpIfZero(val condition, identifier target)
	| JumpIfNotZero(val condition, identifier target)
	| Label(identifier)
	<strong>| FunCall(identifier fun_name, val* args, val dst)</strong>
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
</pre></code>

The changes correspond closely to the changes to the AST. However, we'll discard function declarations without body the same as variables without initializers.

### Generating TACKY

The _function_declaration_ with body is converted to _function_definition_ in TACKY.
To convert a function call, we generate instructions to evaluate each argument, create a list of the resulting values.

```
<instructions for e1>
v1 = <result of e1>
<instructions for e2>
v2 = <result of e2>
--snip--
result = FunCall(fun, [v1, v2, ...])
```

Don't forget to add a Return(0) instruction in the end of every function body.

## Assembly Generation

### Assembly

<pre><code>program = Program(<strong>function_definition*</strong>)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
		| Unary(unary_operator, operand dst)
		| Binary(binary_operator, operand, operand)
		| Cmp(operand, operand)
		| Idiv(operand)
		| Cdq
		| Jmp(identifier)
		| JmpCC(cond_code, identifier)
		| SetCC(cond_code, operand)
		| Label(identifier)
		| AllocateStack(int)
		<strong>| DeallocateStack(int)
		| Push(operand)
		| Call(identifier)</strong> 
		| Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
cond_code = E | NE | G | GE | L | LE
reg = AX <strong>| CX</strong> | DX <strong>| DI</strong> <strong>| SI</strong> <strong>| R8</strong> <strong>| R9</strong> | R10 | R11</pre></code>

### Converting TACKY to Assembly

At the start of each function body, we copy each parameter from registers or memory addresses into slots in the current stack's frame.
We do this so we don't need to worry about saving caller-saved registers and restore them afterward. So now some registers are dedicated to certain roles and we're not afraid of clobbering them. For example:

- _rax_ to return value
- _rdx_ to store the remainder of _idiv_ instruction
- _r10d_ and _r11d_ to fix up instructions.

However, this is inefficient. We'll fix this in Part III.

To **Implement FunCall**, we can do as following:

```
convert_function_call(FunCall(fun_name, args, dst)):
	arg_registers = [DI, SI, DX, CX, R8, R9]

	// adjust stack alignment
	register_args, stack_args = first 6 args, remaining args
	if lengh(stack_args) is odd:
		stack_padding = 8
	else:
		stack_padding = 0

	if stack_padding != 0:
		emit(AllocateStack(stack_padding))

	// pass args in registers
	reg_index = 0
	for tacky_arg in register_args:
		r = arg_registers[reg_index]
		assembly_arg = convert_val(tacky_arg)
		emit(Mov(assembly_arg, Reg(r)))
		reg_index += 1

	// pass args on stack
	for tacky_arg  in stack_args:
		assembly_arg = convert_val(tacky_arg)
		if assembly_arg is a Reg or Imm operand:
			emit(Push(assembly_arg))
		else:
			emit(Mov(assembly_arg, Reg(AX)))
			emit(Push(Reg(AX)))

	// emit call instruction
	emit(Call(fun_name))

	// adjust stack pointer
	bytes_to_remove = 8 * length(stack_args) + stack_padding

	if bytes_to_remove != 0:
		emit(DeallocateStack(bytes_to_remove))

	// retrieve return value
	assembly_dst = convert_val(dst)
	emit(Mov(Reg(AX), assembly_dst))
```

When we push a register or an Immediate value on the stack, they are automatically 8-byte values. In Emission stage, we'll use corresponding alias of registers.
However, if we want to push a value from a memory, like -4(%rbp), onto the stack, we'll push 4 bytes of our operand and another 4 bytes of whatever it is. Bad thing is when the another 4 bytes are not readable memory, this will cause segmentation fault error. So it's always safe to first, put the 4-byte value from memory into our register (automatically clearning the another 4 bytes), and push the register to the stack.

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                  |
| ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(function_definitions)            | Program(**function_definitions**)                                                                                                                                                                                                                                                                                                                                                             |
| Function(name, **params**, instructions) | Function(name, **[Mov(Reg(DI), param1), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Reg(SI), param2), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Stack(16), param7), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Stack(24), param8), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +**<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                            | Assembly instructions                                                                                                        |
| -------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Return(val)                                  | Mov(val, Reg(AX))<br>Ret                                                                                                     |
| Unary(unary_operator, src, dst)              | Mov(src, dst)<br>Unary(unary_operator, dst)                                                                                  |
| Unary(Not, src, dst)                         | Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst)                                                                        |
| Binary(Divide, src1, src2, dst)              | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)                                                                 |
| Binary(Remainder, src1, src2, dst)           | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)                                                                 |
| Binary(binary_operator, src1, src2, dst)     | Mov(src1, dst)<br>Binary(binary_operator, src2, dst)                                                                         |
| Binary(relational_operator, src1, src2, dst) | Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst)                                                       |
| Jump(target)                                 | Jmp(target)                                                                                                                  |
| JumpIfZero(condition, target)                | Cmp(Imm(0), condition)<br>SetCC(E, target)                                                                                   |
| JumpIfNotZero(condition, target)             | Cmp(Imm(0), condition)<br>SetCC(NE, target)                                                                                  |
| Copy(src, dst)                               | Mov(src, dst)                                                                                                                |
| Label(identifier)                            | Label(identifier)                                                                                                            |
| **FunCall(fun_name, args, dst)**             | **\<fix stack alignment><br>\<set up arguments><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(Reg(AX), dst)** |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| Add            | Add               |
| Subtract       | Sub               |
| Multiply       | Mult              |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code |
| ---------------- | ----------------------- |
| Equal            | E                       |
| NotEqual         | NE                      |
| LessThan         | L                       |
| LessOrEqual      | LE                      |
| GreaterThan      | G                       |
| GreaterOrEqual   | GE                      |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters

- Extend to replace pseudoregisters in the new _Push_ instruction. We don't directly push registers now, but we will in Part II.
- Return stack size for each function. We can record them in symbol table, or annotate each function with its stack size in Assembly AST.

### Fixing Up Instructions

We have no new construct that takes operands, but we need to update the AllocateStack to round the stack size to the next multiple of 16.

## Code Emission

On MacOS, function names are prefixed with underscores. For example: _main_ -> _\_main_
On Linux, include @PLT after the function names. For example: _foo_ -> _foo@PLT_

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct      | Ouput                                                                                                                                                                                                                          |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Program(**function_definitions**) | **Printout each function definition** <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                               |
| Function(name, instructions)      | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

#### Formatting Assembly Instructions

| Assembly instruction              | Output                                                                                     |
| --------------------------------- | ------------------------------------------------------------------------------------------ |
| Mov(src, dst)                     | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                                 |
| Ret                               | ret                                                                                        |
| Unary(unary_operator, operand)    | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                        |
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                   |
| Idiv(operand)                     | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                    |
| Cdq                               | cdq                                                                                        |
| AllocateStack(int)                | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                                  |
| Cmp(operand, operand)             | cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                         |
| Jmp(label)                        | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                      |
| JmpCC(cond_code, label)           | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                            |
| SetCC(cond_code, operand)         | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                          |
| Label(label)                      | .L\<label>:                                                                                |
| **DeallocateStack(int)**          | **addq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp**                                              |
| **Push(operand)**                 | **pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>**                                                |
| **Call(label)**                   | **call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT** |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |
| Add               | addl             |
| Sub               | subl             |
| Mult              | imull            |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |

#### Formatting Assembly Operands

| Assembly operand    | Output      |
| ------------------- | ----------- |
| Reg(AX) **8-byte**  | **%rax**    |
| Reg(AX) 4-byte      | %eax        |
| Reg(AX) 1-byte      | %al         |
| Reg(DX) **8-byte**  | **%rdx**    |
| Reg(DX) 4-byte      | %edx        |
| Reg(DX) 1-byte      | %dl         |
| **Reg(CX) 8-byte**  | **%rcx**    |
| **Reg(CX) 4-byte**  | **%ecx**    |
| **Reg(CX) 1-byte**  | **%cl**     |
| **Reg(DI) 8-byte**  | **%rdi**    |
| **Reg(DI) 4-byte**  | **%edi**    |
| **Reg(DI) 1-byte**  | **%dil**    |
| **Reg(SI) 8-byte**  | **%rsi**    |
| **Reg(SI) 4-byte**  | **%esi**    |
| **Reg(SI) 1-byte**  | **%sil**    |
| **Reg(R8) 8-byte**  | **%r8**     |
| **Reg(R8) 4-byte**  | **%r8d**    |
| **Reg(R8) 1-byte**  | **%r8b**    |
| **Reg(R9) 8-byte**  | **%r9**     |
| **Reg(R9) 4-byte**  | **%r9d**    |
| **Reg(R9) 1-byte**  | **%r9b**    |
| **Reg(R10) 8-byte** | **%r10**    |
| Reg(R10) 4-byte     | %r10d       |
| Reg(R10) 1-byte     | %r10b       |
| **Reg(R11) 8-byte** | **%r11**    |
| Reg(R11) 4-byte     | %r11d       |
| Reg(R11) 1-byte     | %r11b       |
| Stack(int)          | <int>(%rbp) |
| Imm(int)            | $\<int>     |

## Summary

Yeehaw! We just implemented Function calls, the most powerful and complicated features we've faced.
In the next chapter, we expand on the idea of identifier linkage to implement file scope variables and storage-class specifiers.

### Reference Implementation Analysis

[Chapter 9 Code Analysis](./code_analysis/chapter_9.md)

---

# Chapter 10: File Scope Variable Declarations and Storage Class Specifiers

## Stages of a Compiler

1. **Lexer**
   - Input: Source code (program.c)
   - Output: Token list
2. **Parser**
   - Input: Token list
   - Output: Abstract Syntax Tree (AST)
3. **Semantic Analysis**
   - Input: AST
   - Output: Transformed AST
   - Passes:
   1. Variable resolution
   2. Type Checking
   3. Loop Labeling
4. **TACKY Generation**
   - Input: Transformed AST
   - Output: TAC IR (Tacky)
5. **Assembly Generation**
   - Input: Tacky
   - Output: Assembly code
   - Passes:
   1. Converting TACKY to Assembly
   2. Replacing pseudoregisters
   3. Instruction fix-up
6. **Code Emission**
   - Input: Assembly code
   - Output: Final assembly file

- To wrap up our first part, we'll support variable declarations at file scope, and introduce the storage class specifiers: Extern and Static.
- The storage class specifiers control the linkage and storage duration of the declared objects.
- We'll update semantic analysis stage to determine the linkage and storage duration of every declaration.
- We'll also add some new assembly directives to represent the linkage.

## All About Declarations

- To support storage class specifiers, we'll keep track some properties of a declaration:

  - Scope
  - Linkage
  - Is a definition?
  - Type, which we will handle in more details in Part II

- Some terms to get started:
  - File/Source file: a preprocessed source file, a "translation unit"
  - Static variable: a variable that has its lifetime from the program starts until it exits. The word static represents the storage duration, not the keyword _static_ specifier
  - Automatic variable: a variable that has its life time from the point it's declared to the end of its scope.
  - External variable: a variable that has internal or external linkage. The word external represents the linkage, not the keyword _extern_ specifier

### Scope

Functions and variables have the same rules of scoping. A variable or function must be declared before they can be used. We already know this, so nothing much to explain here.

### Linkage

Linkage represents if an object is visible to the current file, or can be found from another file via Linker:

- Functions always have linkage (external by default).
- File scope variables always have linkage (external by default).
- Local variables have no linkage.

A declaration's linkage depends on two things:

- Storage-class specifier.
- Declared at block scope or file scope.

Functions that are declared without any storage-specifier are handled as if they have _extern_.

Let's talk about _static_ specifier first:

- At file scope, _static_ specifier indicates a function/variable has internal linkage.
- At block scope, _static_ controls storage duration, not linkage.
- It's illegal to declare static functions at block scope because functions have no storage duration.

The _extern_ specifier is more complicated:

- The current declaration with _extern_ will have the same linkage as the previous visible declaration.
- If none is visible, the declaration will have external linkage.

### Storage Duration

Storage duration is a property of variables. Functions don't have storage duration.
To determine the storage duration of a variable:

- All file scope variables have static storage duration
- Local variables that are declared with _static_ or _extern_ also have static storage duration
- All other local variables have automatic storage duration

For automatic variables, the scope and lifetime are so closely linked that the distinction is almost irrelevant.
A static variable's lifetime depends on its scope: - If the static variable is declared at file scope, its scope extends from the start to the end of the program. - Otherwise, the static variable is declared at block scope, though it has a lifetime from the start of the end of the program, its scope only extends from the point and to the end of the scope where's it's declared/brought into.

Static variables are initialized before program runs, so their initializers must be constants.

### Definitions vs. Declarations

- [x] Every variable declaration with an initializer is a definition. (We already know this)
- [ ] Every variable declaration without linkage is a definition: local variables are allocated on stack, and not necessarily initialized; local static variables are allocated in the **data** or **bss** sections, and are always initialized (with initializers or to zero)

_Note_:

- _extern_ variable declarations at block scope cannot have initializers. Why? The use of the _extern_ for a local variable declaration means that the variable is defined somewhere in the same file.
- A variable declaration with internal or external linkage, no _extern_ specifier and no initializer is a _tentative_ definition. Though it's illegal to define a variable more than once, it's totally fine to have multiple tentative definition for a variable.

### Summarize

Here's how an identifier's linkage, storage duration and status as a definition are determined.

- The leftmost columns (Scope, Specifier) refer to a declaration's syntax during the parsing.
- The other columns are properties we need to determine during the semantic analysis.

#### Properties of Variable Declarations

| Scope       | Specifier | Linkage                                                  | Storage duration | Definition with initializer | Definition without initializer  |
| ----------- | --------- | -------------------------------------------------------- | ---------------- | --------------------------- | ------------------------------- |
| File scope  | None      | External                                                 | Static           | Yes                         | Tentative                       |
| File scope  | Static    | Internal                                                 | Static           | Yes                         | Tentative                       |
| File scope  | Extern    | _Matches prior visible declaration; external by default_ | Static           | Yes                         | No                              |
| Block scope | None      | None                                                     | Automatic        | Yes                         | Yes (defined but uninitialized) |
| Block scope | Static    | None                                                     | Static           | Yes                         | Yes (initialized to zero)       |
| Block scope | Extern    | _Matches prior visible declaration; external by default_ | Static           | Invalid                     | No                              |

#### Properties of Function Declarations

| Scope       | Specifier      | Linkage                                                  | Definition with body | Definition without body |
| ----------- | -------------- | -------------------------------------------------------- | -------------------- | ----------------------- |
| File scope  | None or extern | _Matches prior visible declaration; external by default_ | Yes                  | No                      |
| File scope  | Static         | Internal                                                 | Yes                  | No                      |
| Block scope | None or extern | _Matches prior visible declaration; external by default_ | Invalid              | No                      |
| Block scope | Static         | Invalid                                                  | Invalid              | Invalid                 |

### Error Cases

We'll need to detect more errors. Some of them are familiar from previous chapters, with slight detail changes; some are brand new. So let's prepare:

#### Conflicting Declarations

- Two declarations of an identifier in the same scope, and one of them has no linkage.
- Two declarations of an identifier refer to different types.

#### Multiple definitions

- External variables are defined multiple times. Note that our compiler cannot detect the error, but the linker will.

#### No Definitions

- We use a variable or a function that is declared but never defined, we'll have error at link time. Again, our compiler won't detect it, but the linker will.

#### Invalid Initializers

- Initializers for static variables must be constants.
- Extern declarations at block scope cannot have any initializer.

#### Restrictions on Storage-Class Specifiers

- _extern_ and _static_ specifiers cannot be used on function parameters nor variable declaration in for-loop headers.
- Local function declarations cannot have _static_. The use of static at local scope controls storage duration, but functions have no storage duration.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Static | static |
| Extern | extern |

## The Parser

### AST

<pre><code>program = Program(<strong>declaration*</strong>)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, <strong>storage_class?</strong>)
function_declaration = (identifier name, identifier* params, block? body, <strong>storage_class?</strong>)
<strong>storage_class = Static | Extern</strong>
block_item = S(statement) | D(declaration)
block = Block(block_item*)
for_init = InitDecl(variable_declaration) | InitExp(exp?)
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
exp = Constant(int) 
	| Var(identifier) 
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual</pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= { <strong>&lt;declaration&gt;</strong> }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= <strong>{ &lt;specifier&gt; }+</strong> &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= <strong>{ &lt;specifier&gt; }+</strong> &lt;identifier&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;param-list&gt; ::= "void" | "int" &lt;identifier&gt; { "," "int" &lt;identifier&gt; }
<strong>&lt;specifier&gt; ::= "int" | "static" | "extern"</strong>
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;for-init&gt; ::= &lt;variable-declaration&gt; | [ &lt;exp&gt; ] ";"
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
&lt;exp&gt; ::= &lt;factor&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
&lt;factor&gt; ::= &lt;int&gt; | &lt;identifier&gt; | &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

**Note**:

- {\<specifier>}+ represents a non-empty list of specifiers.
- \<param-list> rule hasn't changed because each parameter is defined with "int" keyword, we don't accept a list of specifier that might have "extern" or "static".

### Parser

#### Parsing Type and Storage-Class Specifiers

The parser consumes a list of specifiers at the start of a declaration, converts them into exactly one type and at most one storage-class specifier.

```
parse_type_and_storage_class(specifier_list):
	types = []
	storage_classes = []
	for specifier in specifier_list:
		if specifier is "int":
			types.append(specifier)
		else:
			storage_classes.append(specifier)

	if length(types) != 1:
		fail("Invalid type specifier")
	if length(storage_classes) > 1:
		fail("Invalid storage class")

	type = Int

	if length(storage_classes) == 1:
		storage_class = parse_storage_class(storage_classes[0])
	else:
		storage_class = null

	return (type, storage_class)
```

#### Distinguishing Between Function and Variable Declarations

The logic of parsing function and variable declarations are similar, even in the future when we support more types.
So it's ideal to have a single function to parse both and return a declaration AST node.

In the for loop header, we can also use the same function, then check if the returned declaration is FunDecl or not? If it is, throw an error.

## Semantic Analysis

In the identifier resolution pass, we check for duplicate declarations in the same scope.
In the type checking pass, we add storage class and linkage information to the symbol table as we'll need them in later stages. We also deal with more error checking.

### Identifier Resolution

We resolve variables at file scopes here.
In the identifier map, we track whether each identifier has linkage. We don't need to distinguish between internal and external linkage until the type checking pass.

```
resolve_file_scope_variable_declaration(decl, identifier_map):
	identifier_map.add(decl.name, MapEntry(
		new_name=decl.name,
		from_current_scope=True,
		has_linkage=True
	))

	return decl
```

As we see, for file scope declarations, they has_linkage is always true. File scope variable declarations have initializers which must be constants, so we don't need to call resolve_exp. But we'll check if it's really a constant in the type checking pass.

```
resolve_local_variable_declaration(decl, identifier_map):
	if decl.name is in identifier_map:
		prev_entry = identifier_map.get(decl.name)
		if prev_entry.from_current_scope:
			if not(prev_entry.has_linkage and decl.storage_class == Extern):
				fail("Duplicate local declaration")

	if decl.storage_class == Extern:
		identifier_map.add(decl.name, MapEntry(
			new_name=decl.name,
			from_current_scope=True,
			has_linkage=True
		))
		return decl
	else:
		unique_name = UniqueIds.make_temporary()
		identifier_map.add(decl.name, MapEntry(
			new_name=unique_name,
			from_current_scope=True,
			has_linkage=False
		))

		--snip--
```

At block scope, if a variable has extern keyword, we record it has linkage in the map and retain its name.

We don't need to change how to process function declaration in this pass, except one change: Block scope functions cannot have "static" specifier.

### Type Checker

We'll track Static functions and variables in this pass.

- Record each variable's storage duration
- Record the initial values of variables with static storage duration
- Record whether functions and variables with static storage duration are globally visible (to other files)

#### Identifier Attributes in the Symbol Table

```
identifier_attrs = FunAttr(bool defined, bool global)
				| StaticAttr(initial_value init, bool global)
				| LocalAttr

initial_value = Tentative | Initial(int) | NoInitializer
```

#### Function Declarations

```
typecheck_function_declaration(decl, symbols):
	fun_type = FunType(length(decl.params))
	has_body = decl.body is not null
	already_defined = False
	global = decl.storage_class != Static

	if decl.name is in symbols:
		old_decl = symbols.get(decl.name)
		if old_decl.type != fun_type:
			fail("Incompatible function declarations")
		already_defined = old_decl.attrs.defined
		if already_defined and has_body:
			fail("Function is defined more than once")

		if old_decl.attrs.global and decl.storage_class == Static:
			fail("Static function declaration follows non-static")
		global = old_decl.attrs.global

	attrs = FunAttr(
		defined=(already_defined or has_body),
		global=global
	)

	symbols.add(decl.name, fun_type, attrs)
	--snip--
```

#### File Scope Variable Declarations

We determine the initial value and its global visibility. Both of these depend on the current declaration and previous declarations of the same variable.

```
typecheck_file_scope_variable_declaration(decl, symbols):
	if decl.init is constant integer i:
		initial_value = Initial(i)
	else if decl.init is null:
		if decl.storage_class == Extern:
			initial_value = NoInitializer
		else:
			initial_value = Tentative
	else:
		fail("Non-constant initializer")

	global = (decl.storage_class != Static)

	if decl.name is in symbols:
		old_decl = symbols.get(decl.name)
		if old_decl.type != Int:
			fail("Function redeclared as variable")
		if decl.storage_class == Extern:
			global = old_decl.attrs.global
		else if old_decl.attrs.global != global:
			fail("Conflicting variable linkage")

		if old_decl.attrs.init is a constant:
			if initial_value is a constant:
				fail("Conflicting file scope variable definitions")
			else:
				initial_value = old_decl.attrs.init
		else if initial_value is not a constant and old_decl.attrs.init == Tentative:
			initial_value = Tentative

		attrs = StaticAttr(init=initial_value, global=global)
		symbols.add(decl.name, Int, attrs)
```

#### Block Scope Variable Declarations

```
typecheck_local_variable_declaration(decl, symbols):
	if decl.storage_class == Extern:
		if decl.init is not null:
			fail("Initializer on local extern variable declaration")
		if decl.name is in symbols:
			old_decl = symbols.get(decl.name)
			if old_decl.type != Int:
				fail("Function redeclared as variable")
		else:
			symbols.add(decl.name, Int, attrs=StaticAttr(
				init=NoInitializer,
				global=True
			))

	else if decl.storage_class == Static:
		if decl.init is constant integer i:
			initial_value = Initial(i)
		else if decl.init is null:
			initial_value = Initial(0)
		else:
			fail("Non-constant initializer on local static variable")

		symbols.add(decl.name, Int, attrs=StaticAttr(
			init=initial_value,
			global=False
		))
	else:
		symbols.add(decl.name, Int, attrs=LocalAttr)
		if decl.init is not null:
			typecheck_exp(decl.init, symbols)
```

**Note**:

- A local extern declaration will never change the initial value or linkage we've already recorded, so we do nothing if it's already declared. Otherwise, we add it the symbol table and mark it as globally visible and not initialized.
- In for loop header, we validate that the declaration in it doesn't have any storage-class specifier.

## TACKY Generation

### TACKY

<pre><code>
program = Program(<strong>top_level*</strong>)
<strong>top_level</strong> = Function(identifier, <strong>bool global</strong>, identifier* params, instruction* body)
		<strong>| StaticVariable(identifier, bool global, int init)</strong>
instruction = Return(val) 
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| Jump(identifier target)
	| JumpIfZero(val condition, identifier target)
	| JumpIfNotZero(val condition, identifier target)
	| Label(identifier)
	| FunCall(identifier fun_name, val* args, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
</pre></code>

### Generating TACKY

Our final TACKY program will have both function definitions converted from the original AST, and variable definitions generated from the symbol table.

```
convert_symbols_to_tacky(symbols):
	tacky_defs = []
	for (name, entry) in symbols:
		match entry.attrs with
		| StaticAttr(init, global) ->
			match init with
			| Initial(i) -> tacky_defs.append(StaticVariable(name, global, i))
			| Tentative -> tacky_defs.append(StaticVariable(name, global, 0))
			| NoInitializer -> continue
		| -> continue
	return tacky_defs
```

**Note**: Right now, it doesn't matter whether we process the AST or the symbol table first. Since chapter 16, it will be important that we process the AST first and the symbol table second. The reason is that we will also update the symbol table while converting the AST to TACKY first then.

## Assembly Generation

### Assembly

<pre><code>program = Program(<strong>top_level*</strong>)
<strong>top_level</strong> = Function(identifier name, <strong>bool global,</strong> instruction* instructions)
	<strong>| StaticVariable(identifier name, bool global, int init)</strong>
instruction = Mov(operand src, operand dst)
		| Unary(unary_operator, operand dst)
		| Binary(binary_operator, operand, operand)
		| Cmp(operand, operand)
		| Idiv(operand)
		| Cdq
		| Jmp(identifier)
		| JmpCC(cond_code, identifier)
		| SetCC(cond_code, operand)
		| Label(identifier)
		| AllocateStack(int)
		| DeallocateStack(int)
		| Push(operand)
		| Call(identifier) 
		| Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int) <strong>| Data(identifier)</strong>
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11</pre></code>

### Converting TACKY to Assembly

Beside program, function and static variable constructs, we won't change any other conversion.
We'll convert every TACKY Var operand to Pseudo operand, regardless of whether it has static or automatic duration.

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                        | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                          |
| ------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Program(top_level_defs)**                      | **Program(top_level_defs)**                                                                                                                                                                                                                                                                                                                                                                           |
| Function(name, **global**, params, instructions) | Function(name, **global**, [Mov(Reg(DI), param1), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Reg(SI), param2), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Stack(16), param7), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(Stack(24), param8), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| **StaticVariable(name, global, init)**           | **StaticVariable(name, global, init)**                                                                                                                                                                                                                                                                                                                                                                |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                            | Assembly instructions                                                                                                    |
| -------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| Return(val)                                  | Mov(val, Reg(AX))<br>Ret                                                                                                 |
| Unary(Not, src, dst)                         | Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst)                                                                    |
| Unary(unary_operator, src, dst)              | Mov(src, dst)<br>Unary(unary_operator, dst)                                                                              |
| Binary(Divide, src1, src2, dst)              | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)                                                             |
| Binary(Remainder, src1, src2, dst)           | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)                                                             |
| Binary(arithmetic_operator, src1, src2, dst) | Mov(src1, dst)<br>Binary(arithmetic_operator, src2, dst)                                                                 |
| Binary(relational_operator, src1, src2, dst) | Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst)                                                   |
| Jump(target)                                 | Jmp(target)                                                                                                              |
| JumpIfZero(condition, target)                | Cmp(Imm(0), condition)<br>SetCC(E, target)                                                                               |
| JumpIfNotZero(condition, target)             | Cmp(Imm(0), condition)<br>SetCC(NE, target)                                                                              |
| Copy(src, dst)                               | Mov(src, dst)                                                                                                            |
| Label(identifier)                            | Label(identifier)                                                                                                        |
| FunCall(fun_name, args, dst)                 | \<fix stack alignment><br>\<set up arguments><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(Reg(AX), dst) |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| Add            | Add               |
| Subtract       | Sub               |
| Multiply       | Mult              |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code |
| ---------------- | ----------------------- |
| Equal            | E                       |
| NotEqual         | NE                      |
| LessThan         | L                       |
| LessOrEqual      | LE                      |
| GreaterThan      | G                       |
| GreaterOrEqual   | GE                      |

#### Converting TACKY Operands to Assembly

| TACKY operand   | Assembly operand   |
| --------------- | ------------------ |
| Constant(int)   | Imm(int)           |
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters

Now, not every variable is defined on the stack anymore. We check from the symbol table to know whether the variable should be in the stack or in Data/BSS section.
How to handle this:

- If the pseudo(name) has the name NOT in the assigned map:
  - Look it up in the symbol table:
  - Found with static duration: we map it to a Data operand by the same name
  - Otherwise: assign it a new slot on the stack as usual

Static variables don't live on the stack, so they don't count toward the total stack size.

### Fixing Up Instructions

Data operands are memory addresses too! Several rules we've written do not allow us to have both operands as memory.

The instruction

```
Mov(Data("x"), Stack(-4))
```

is fixed up into:

```
Mov(Data("x"), Reg(R10))
Mov(Reg(R10), Stack(-4))
```

## Code Emission

- Emit _.globl_ directive for static variable that has global attribute as true.
- Emit _.text_ directive at the start of each function definition.
- Emit Data operand from _Data("foo")_ to _foo(%rip)_ in Linux and _\_foo(%rip)_ in MacOS.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                          | Ouput                                                                                                                                                                                                                                                                                                                  |
| --------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(**top_levels**)                                               | **Printout each top-level construct.** <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                      |
| Function(name, global, instructions)                                  | &nbsp;&nbsp;&nbsp;&nbsp;**\<global-directive>**<br>&nbsp;&nbsp;&nbsp;&nbsp;**.text**<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| **StaticVariable(name, global, init) (Initialized to zero)**          | **&nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;.zero 4**                                                                                                                                       |
| **StaticVariable(name, global, init) (Initialized to nonzero value)** | **&nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;.long \<init>**                                                                                                                                |
| **Gobal directive**                                                   | **if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.**                                                                                                                                                                                                                                      |
| **Alignment directive**                                               | **For Linux only: .align 4<br>For macOS and Linux: .balign 4**                                                                                                                                                                                                                                                         |

#### Formatting Assembly Instructions

| Assembly instruction              | Output                                                                                 |
| --------------------------------- | -------------------------------------------------------------------------------------- |
| Mov(src, dst)                     | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                             |
| Ret                               | ret                                                                                    |
| Unary(unary_operator, operand)    | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                    |
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                               |
| Idiv(operand)                     | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Cdq                               | cdq                                                                                    |
| AllocateStack(int)                | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Cmp(operand, operand)             | cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                     |
| Jmp(label)                        | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)           | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)         | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                      | .L\<label>:                                                                            |
| DeallocateStack(int)              | addq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Push(operand)                     | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                       | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | negl             |
| Not               | notl             |
| Add               | addl             |
| Sub               | subl             |
| Mult              | imull            |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |

#### Formatting Assembly Operands

| Assembly operand | Output      |
| ---------------- | ----------- |
| Reg(AX) 8-byte   | %rax        |
| Reg(AX) 4-byte   | %eax        |
| Reg(AX) 1-byte   | %al         |
| Reg(DX) 8-byte   | %rdx        |
| Reg(DX) 4-byte   | %edx        |
| Reg(DX) 1-byte   | %dl         |
| Reg(CX) 8-byte   | %rcx        |
| Reg(CX) 4-byte   | %ecx        |
| Reg(CX) 1-byte   | %cl         |
| Reg(DI) 8-byte   | %rdi        |
| Reg(DI) 4-byte   | %edi        |
| Reg(DI) 1-byte   | %dil        |
| Reg(SI) 8-byte   | %rsi        |
| Reg(SI) 4-byte   | %esi        |
| Reg(SI) 1-byte   | %sil        |
| Reg(R8) 8-byte   | %r8         |
| Reg(R8) 4-byte   | %r8d        |
| Reg(R8) 1-byte   | %r8b        |
| Reg(R9) 8-byte   | %r9         |
| Reg(R9) 4-byte   | %r9d        |
| Reg(R9) 1-byte   | %r9b        |
| Reg(R10) 8-byte  | %r10        |
| Reg(R10) 4-byte  | %r10d       |
| Reg(R10) 1-byte  | %r10b       |
| Reg(R11) 8-byte  | %r11        |
| Reg(R11) 4-byte  | %r11d       |
| Reg(R11) 1-byte  | %r11b       |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Summary

Congratulations! This chapter marks the end checkpoint of our journey of Part I.
We've implemented all the basic mechanics of C, from local and file scope varibles to control-flow statements to
function calls.

In Part II, we'll implement more types: signed, unsigned integers, floating-point numbers, pointers, arrays, and structures.
Or we can skip to Part III where we will optimize the compiler.

### Reference Implementation Analysis

[Chapter 10 Code Analysis](./code_analysis/chapter_10_.md)
