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
- [Part II](#part-ii)
  - [Chapter 11: Long Integers](#chapter-11-long-integers)
  - [Chapter 12: Unsigned Integers](#chapter-12-unsigned-integers)

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

| Assembly top-level construct | Output                                                                                                                                      |
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

| Assembly top-level construct | Output                                                                                                                                                                                                                         |
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
<strong>binary_operator = Add | Subtract | Multiply | Divide | Remainder</strong>
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

| Assembly top-level construct | Output                                                                                                                                                                                                                         |
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
binary_operator = Add | Subtract | Multiply | Divide | Remainder <strong>| Equal | Not Equal</strong>
				<strong>| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual</strong>
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

| Assembly top-level construct | Output                                                                                                                                                                                                                         |
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

| Assembly top-level construct | Output                                                                                                                                                                                                                         |
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
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
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

| Assembly top-level construct      | Output                                                                                                                                                                                                                         |
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
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
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

| Assembly top-level construct                                          | Output                                                                                                                                                                                                                                                                                                                 |
| --------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(**top_levels**)                                               | **Printout each top-level construct.** <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                      |
| Function(name, global, instructions)                                  | &nbsp;&nbsp;&nbsp;&nbsp;**\<global-directive>**<br>&nbsp;&nbsp;&nbsp;&nbsp;**.text**<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| **StaticVariable(name, global, init) (Initialized to zero)**          | **&nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;.zero 4**                                                                                                                                       |
| **StaticVariable(name, global, init) (Initialized to nonzero value)** | **&nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;.long \<init>**                                                                                                                                |
| **Global directive**                                                  | **if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.**                                                                                                                                                                                                                                      |
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

---

# Part II

---

# Chapter 11: Long Integers

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

- _Long_ is a signed integer type.
- _Long_ is similar to _int_, the only difference is their range of values.
- We'll add explicit cast operation, which converts a value to a different type.

What we will do:

- Track the types of constants and variables.
- Attach type information to AST nodes.
- Identify implicit casts and make them explicit.
- Determine the operand sizes for assembly instructions.

## Long Integers in Assembly

Don't let the terms get you. The type _long_ in C, nowadays, is different from the long type in assembly.

| C    | x86/x64             |
| ---- | ------------------- |
| int  | longword/doubleword |
| long | quadword            |

## Type conversion

C standard says: "If the value can be represented by the new type, it is unchanged."
Because _long_ is larger than _int_, it is safe to convert _int_ to _long_ (movsx).

However, convert from _long_ to _int_ is a different matter as the value might be larger than what _int_ can store. And the C standard lets us decide what to do with the case.
We'll adopt the solution from GCC: to convert to type width N, the value is reduced modulo 2^N. That means we add or subtract the value by mutliple of 2^32 to bring _long_ value into the range of _int_.

To simply put, we'll drop the upper 4 bytes of the _long_ value to an _int_.

A long integer of value -3

```
11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111101
```

is represented like this in int type:

```
									11111111 11111111 11111111 11111101
```

The above example is simple as -3 can be represented in both _long_ and _int_, so the value after conversion doesn't change.
But what if we face the case where the _long_ value does not fit in _int_?

A long 2,147,483,648

```
00000000 00000000 00000000 00000000 10000000 00000000 00000000 00000000
```

is truncated to int of value –2,147,483,648:

```
									10000000 00000000 00000000 00000000
```

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordLong | Long |
| Long integer constants | [0-9]+[lL]\b |

## The Parser

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, <strong>type var_type,</strong> storage_class?)
function_declaration = (identifier name, identifier* params, block? body,<strong>type fun_type,</strong> storage_class?)
<strong>type = Int | Long | FunType(type* params, type ret)</strong>
storage_class = Static | Extern
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
exp = Constant(<strong>const</strong>) 
	| Var(identifier) 
	<strong>| Cast(type target_type, exp)</strong>
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
<strong>const = ConstInt(int) | ConstLong(int)</strong></pre></code>

We'll extend the type structure in the symbol table in Chapter 9 instead of defining another data structure.
If the implementation language has signed 64-bit and 32-bit integer types, use them. Otherwise, we should at least make sure the ConstLong node uses an integer type that can represent all long values.

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;param-list&gt; ::= "void" | <strong>{ &lt;type-specifier&gt; }+</strong> &lt;identifier&gt; { "," <strong>{ &lt;type-specifier&gt; }+</strong>  &lt;identifier&gt; }
<strong>&lt;type-specifier&gt; ::= "int" | "long"</strong> 
&lt;specifier&gt; ::= <strong>&lt;type-specifier&gt;</strong> | "static" | "extern"
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
&lt;factor&gt; ::= <strong>&lt;const&gt;</strong> | &lt;identifier&gt; 
	<strong>| "(" { &lt;type-specifier&gt; }+ ")" &lt;factor&gt;</strong> 
	| &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
<strong>&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt;</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? An int token ?&lt;/pre&gt;&lt;/code&gt;
<strong>&lt;long&gt; ::= ? An int or long token ?</strong></pre></code>

### Parser

```
parse_type(specifier_list):
	if specifier_list == ["int"]:
		return Int
	if (specifier_list == ["int", "long"]
		or specifier_list == ["long", "int"]
		or specifier_list == ["long"]):
		return Long
	fail("Invalid type specifier")
```

```
parse_type_and_storage_class(specifier_list):
	types = []
	storage_classes = []
	for specifier in specifier_list:
		if specifier is "int" or "long":
			types.append(specifier)
		else:
			storage_classes.append(specifier)

	type = parse_type(types)

	if length(storage_classes) > 1:
		fail("Invalid storage class")
	if length(storage_classes) == 1:
		storage_class = parse_storage_class(storage_classes[0])
	else:
		storage_class = null

	return (type, storage_class)
```

The tricky part is how to parse constant tokens.

```
parse_constant(token):
	v = integer value of token
	if v > 2^63 - 1:
		fail("Constant is too large to represent as an int or long")

	if token is an int token and v <= 2^31 - 1:
		return ConstInt(v)

	return ConstLong(v)
```

An _int_ is 32 bits, so it can hold the value in range -2^31 to 2^31 - 1. And a _long_ can hold -2^63 to 2^63 - 1.
We don't need to check against mininum value, as these tokens cannot represent negative numbers. The negative sign is a separate token we have as Unary.

## Semantic Analysis

### Identifier Resolution

As usual, whenever we have a new expression that has subexpressions, we extend the identifier resolution pass to traverse it.

### Type Checker

In type checker, we'll annotate every expression in the AST with the result type, which we use to generate temporary variables in TACKY to hold immediate results; the type lets us know how many bytes on the stack the temporary variables need.
Also, we'll identify any implicit type conversions and make them explicit using the new _Cast_ expression.

#### Adding type information to the AST

It depends on your implementation language.
For object-oriented languages, we can have a common base class for every _exp_, and add a type field to the base class.

```
class BaseExp {
	--snip--
	type expType;
}
```

For algebraic data type languages, we define mutually recursive _exp_ and _typed_exp_ nodes.

```
typed_exp = TypedExp(type, exp)
exp = Constant(const)
	| Cast(type target_type, typed_exp)
	| Unary(unary_operator, typed_exp)
	--snip--
```

The pseudocode introduces _set_type(e, t)_ returns a copy of expression, and _get_type(e)_ returns the type annotation from expression. The implementation is up to you.

```
exp = Constant(const, type)
	| Var(identifier, type)
	| Cast(type target_type, exp,type)
	| Unary(unary_operator, exp, type)
	| Binary(binary_operator, exp, exp, tye)
	| Assignment(exp, exp, type)
	| Conditional(exp condition, exp, exp, type)
	| FunctionCall(identifier, exp* args, type)
```

#### Type Checking Expressions

In Part I, the TypeChecker only validates the program, not modifying it. That now changes as we need to annotate each expression with resulting type.

```
typecheck_exp(e, symbols):
	match e with:
	| Var(v) ->
		v_type = symbols.get(v).type
		if v_type is a function type:
			fail("Function name used as variable")

		return set_type(e, v_type)
	| Constant(c) ->
		match c with
		| ConstInt(i) -> return set_type(e, Int)
		| ConstLong(l) -> return set_type(e, Long)
	| Cast(t, inner) ->
		typed_inner = typecheck_exp(inner, symbols)
		cast_exp = Cast(t, typed_inner)
		return set_type(cast_exp, t)
	| Unary(op, inner) ->
		typed_inner = typecheck_exp(inner)
		unary_exp = Unary(op, typed_inner)
		match op with
		| Not 	-> return set_type(unary_exp, Int)
		| _ 	-> return set_type(unary_exp, get_type(typed_inner))
	| Binary(op, e2, e2) ->
		typed_e1 = typecheck_exp(e1, symbols)
		typed_e2 = typecheck_exp(e2, symbols)
		if op is And or Or:
			binary_exp = Binary(op, typed_e1, typed_e2)
			return set_type(binary_exp, Int)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		common_type = get_common_type(t1, t2)
		converted_e1 = convert_to(typed_e1, common_type)
		converted_e2 = convert_to(typed_e1, common_type)
		binary_exp = Binary(opk, converted_e1, converted_e2)
		if Op is Add, Subtract, Multiply, Divide or Remainder:
			return set_type(binary_exp, common_type)
		else:
			return set_type(binary_exp, Int) // relational operators for comparisons.
	| Assignment(left, right) ->
		typed_left = typecheck_exp(left, symbols)
		typed_right = typecheck_exp(right, symbols)
		left_type = get_type(typed_left)
		converted_right = convert_to(typed_right, left_type)
		assign_exp = Assignment(typed_left, converted_right)
		return set_type(assign_exp, left_type)
	| Conditional(control, then_result, else_result):
		typecheck_exp(control, symbols)
		typed_then = typecheck_exp(then_result, symbols)
		typed_else = typecheck_exp(else_result, symbols)

		then_type = get_type(typed_then)
		else_type = get_type(typed_else)
		common_type = get_common_type(then_type, else_type)
		converted_then = convert_to(typed_then, common_type)
		converted_else = convert_to(typed_else, common_type)

		conditional_exp = Conditional(control, converted_then, converted_else)
		return set_type(conditional_exp, common_type)
	| FunctionCall(f, args) ->
		f_type = symbols.get(f).type

		match f_type with
		| FunType(param_types, ret_type) ->
			if length(param_types) != length(args):
				fail("Function called with wrong number of arguments")
			converted_args = []
			for (arg, param_type) in zip(args, param_types):
				typed_arg = typecheck_exp(arg, symbol)
				converted_args.append(convert_to(typed_arg, param_type))
			call_exp = FunctionCall(f, converted_args)
			return set_type(call_exp, ret_type)
		| _ -> fail("Variable used as function name")
```

```
get_common_type(type1, type2):
	if type1 == type2:
		return type1
	else:
		return Long

// A helper function that makes implicit conversions explicit
// If an expression already has the result type, return it unchanged
// Otherwise, wrap the expression in a Cast node.
convert_to(e, t):
	if get_type(e) == t:
		return e
	cast_exp = Cast(t, e)
	return set_type(cast_exp, t)
```

- A variable expression will be validated as before, plus the annotation of the type based on the type declared in symbol table.
- A constant has its own corresponding type; ConstInt has type Int, ConstLong has type Long
- A cast expression has the type of whatever type we cast it to.
- Expressions that evaluate to 1 or 0 (true or false), including comparisons and logical operators like !, all have type Int.
- Arithmetic and bitwise expressions have the same type as their operands. However, binary expressions are more complicated than the rest as their 2 operands may have different types. We'll adopt the _usual arithmetic conversions_, which implicitly convert both operands to the same type, called its _common type_.
- A conditional expression is similar to binary arithmetic expressions. We type check, find the common type and annotate to both branches. We do typecheck the controlling condition, but there's no need to convert it.

#### Type Checking return Statements

When a function returns a value, it's implicitly converted to the function's return type. We'll the conversion to be explicit. We need a way to keep track which function is enclosing the current return statement. We can simply pass down the function name, and look up the symbol table from there; or pass the whole return type.

#### Type Checking Declarations and Updating the Symbol Table

Here are some steps to typecheck function and variable declarations and store neccesary information in the symbol table:

1. We record the correct type for each entry in the symbol table (they are not only Int anymore).
2. Whenever we check for conflicting declarations, we validate the current and previous declarations have the same type. (expanded from checking conflicting between variable and function declarations)
3. When we type check an automatic variable (with initializer), we convert the initializer to the type of the variable. (remember that variable declaration with initialzier is treated as assignment?)
4. We change the representation of static initializers in the symbol table. (previously, Initial(int) can only hold _Int_, now we need another layer to hold the actual value in Initial)

Previously, we had:

```
initial_value = Tentative | Initial(int) | NoInitializer
```

Now, we expand the representation to:

```
initial_value = Tentative | Initial(static_init) | NoInitializer
static_init = IntInit(int) | LongInit(int)
```

_static_init_ is identical to the const AST node definition we just got through for now, but will grealy diverge in later chapters.
Similar to AST's ConstInt and ConstLong, make sure we use the implementation type that can hold both numbers (we do the checking of range on our own).

Type conversions need to be converted at compile time.

```
static int i = 100L
```

In symbol table, this will be stored as:

```
IntInit(100)
```

If the value is too large to be hold in _Int_, we use the same technique as with AST's ConstInt and ConstLong; that is subtracting 2<sup>32</sup> from the value.

Some tips to handle static initializers:
**1. Make your constant type conversion code reusable**. We'll need this in part III.
**2. Don't call typecheck_exp on static initializer**. The function _typecheck_exp_ transform expressions, which complicates our processing. Instead, convert each static initializer directly to _static_init_.

## TACKY Generation

- Now that the symbol table has the new representation of static variable initial values, called _static_init_, we also inject the _static_init_ the StaticVariable construct in TACKY AST.
- Update the Constant(int) to be Constant(const), and reuse the _const_ construct from the AST.
- Conversions from _Long_ to _Int_ may require _Truncate_, and _SignExtend_ from _Int_ to _Long_.

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, <strong>type t, static_init init</strong>)
instruction = Return(val) 
	<strong>| SignExtend(val src, val dst)
	| Truncate(val src, val dst)</strong>
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| Jump(identifier target)
	| JumpIfZero(val condition, identifier target)
	| JumpIfNotZero(val condition, identifier target)
	| Label(identifier)
	| FunCall(identifier fun_name, val* args, val dst)
val = Constant(<strong>const</strong>) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
</pre></code>

### Generating TACKY

- When converting a symbol to TACKY, we look up its _type_ and _attrs_ as StaticAttr. If if the StaticAttr has a Tentative definition, we convert it to ConstInt(0) or ConstLong(0) based on its _type_.
- _Constant_ nodes are the same between AST and TACKY. The converting is simple enough to not mention.
- _And_ and _Or_ binary operators in AST resolve to the type _Int_, so in TACKY, we'll emit them as ConstInt(1) and ConstInt(0).

Now we update the emit_tacky function to support new expression - \_Cast\*

```
emit_tacky(e, instructions, symbols):
	match e with:
	| --snip--
	| Cast(t, inner) ->
		result = emit_tacky(inner, instructions, symbols)
		if t == get_type(inner):
			return result
		dst_name = make_temporary()
		symbols.add(dst_name, t, attrs=LocalAttr)
		dst = Var(dst_name)
		if t == Long:
			instructions.append(SignExtend(result, dst))
		else:
			instructions.append(Truncate(result, dst))
		return dst
```

#### Tracking the Types of Temporary Variables

Previously, several temporary variables in our TackyGen stage do not exist in the symbol table, but still works because our Assembly Generation assume each of them has the type _Int_.
Now, we need to keep track of the variable during this TackyGen stage as well.
Every temporary variable holds the result of an expression. To know the type of the temporary variable, we simply check the type of the expression.
The following is an example:

```
emit_tacky(e, instructions, symbol):
	match e with:
	| --snip--
	| Binary(op, e1, e2):
		v1 = emit_tacky(e1, instructions, symbols)
		v2 = emit_tacky(e2, instructions, symbols)
		dst_name = make_temporary()
		symbols.add(dst_name, get_type(e), attrs=LocalAttr)
		dst = Var(dst_name)
		tacky_op = convert_binop(op)
		instructions.append(Binary(tacky_op, v1, v2, dst))
		return dst
	| --snip--
```

The main change is to add _dst_name_ to the symbol table.

This modification is required in several places in our existing code, let's refactor to have a helper function:

```
make_tacky_variable(var_type, symbols):
	var_name = make_temporary()
	symbols.add(var_name, var_type, attrs=LocalAttr)
	return Var(var_name)
```

#### Generating Extra Return Instructions

Our extra return instruction is mainly for the function _main_. For other arbitrary functions, the return value is undefined, so it doesn't matter if our return of ConstInt(0) is wrong.

## Assembly Generation

- We tag most instructions with the type of operands to choose the correct suffix for ecah instruction.
- Cdq also needs a type as 32-bit version of Cdq extends eax into edx:eax, while rax is extended into rdx:rax in 64-bit.
- Three instructions that take an operand but no need for type:

  - SetCC: the operand is also 1 byte-size
  - Push: the operand is always a quadwords
  - Movsx:

- Remove AllocateStack and DeallocateStack as we now support adding/subtracting quadwords.

### Assembly

<pre><code>program = Program(top_level*)
<strong>assembly_type = LongWord | Quadword</strong>
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, <strong>int alignment, static_init init</strong>)
instruction = Mov(<strong>assembly_type,</strong> operand src, operand dst)
		<strong>| Movsx(operand src, operand dst)</strong>
		| Unary(unary_operator, <strong>assembly_type,</strong> operand)
		| Binary(binary_operator, <strong>assembly_type,</strong> operand, operand)
		| Cmp(<strong>assembly_type,</strong>, operand, operand)
		| Idiv(<strong>assembly_type,</strong> operand)
		| Cdq(<strong>assembly_type</strong>)
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
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 <strong>| SP</strong></pre></code>

### Converting TACKY to Assembly

```
convert_function_call(FunCall(fun_name, args, dst)):
	--snip--
	// pass args on stack
	for tacky_arg in reverse(stack_args):
		assembly_arg = convert_val(tacky_arg)
		if assembly_arg is a Reg or Imm operand or has type Quadword:
			emit(Push(assembly_arg))
		else:
			emit(Mov(LongWord, assembly_arg, Reg(AX)))
			emit(Push(Reg(AX)))
	--snip--
```

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs)                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Function(name, global, params, instructions) | Function(name, global, [Mov(**\<param1 type>,** Reg(DI), param1), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<param2 type>,** Reg(SI), param2), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<param7 type>,** Stack(16), param7), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<param8 type>,** Stack(24), param8), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, **t,** init)    | StaticVariable(name, global, **\<alignment of t,** init)                                                                                                                                                                                                                                                                                                                                                                                                                          |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                            | Assembly instructions                                                                                                                     |
| -------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val)                                  | Mov(**\<val type>,** val, Reg(AX))<br>Ret                                                                                                 |
| Unary(Not, src, dst)                         | Cmp(**\<src type>,** Imm(0), src)<br>Mov(**\<dst type>,** Imm(0), dst)<br>SetCC(E, dst)                                                   |
| Unary(unary_operator, src, dst)              | Mov(**\<src type>,** src, dst)<br>Unary(unary_operator, **\<src type>,** dst)                                                             |
| Binary(Divide, src1, src2, dst)              | Mov(**\<src1 type>,** src1, Reg(AX))<br>Cdq(**\<src1 type>,**)<br>Idiv(**\<src1 type>,** src2)<br>Mov(**\<src1 type>,** Reg(AX), dst)     |
| Binary(Remainder, src1, src2, dst)           | Mov(**\<src1 type>,** src1, Reg(AX))<br>Cdq(**\<src1 type>,**)<br>Idiv(**\<src1 type>,** src2)<br>Mov(**\<src1 type>,** Reg(DX), dst)     |
| Binary(arithmetic_operator, src1, src2, dst) | Mov(**\<src1 type>,** src1, dst)<br>Binary(arithmetic_operator, **\<src1 type>,** src2, dst)                                              |
| Binary(relational_operator, src1, src2, dst) | Cmp(**\<src1 type>,** src1, src2)<br>Mov(**\<dst type>,** Imm(0), dst)<br>SetCC(relational_operator, dst)                                 |
| Jump(target)                                 | Jmp(target)                                                                                                                               |
| JumpIfZero(condition, target)                | Cmp(**\<condition type>,** Imm(0), condition)<br>SetCC(E, target)                                                                         |
| JumpIfNotZero(condition, target)             | Cmp(**\<condition type>,** Imm(0), condition)<br>SetCC(NE, target)                                                                        |
| Copy(src, dst)                               | Mov(**\<src type>,** src, dst)                                                                                                            |
| Label(identifier)                            | Label(identifier)                                                                                                                         |
| FunCall(fun_name, args, dst)                 | \<fix stack alignment><br>\<set up arguments><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(**\<dst type>,** Reg(AX), dst) |
| **SignExtend(src, dst)**                     | **Movsx(src, dst)**                                                                                                                       |
| **Truncate(src, dst)**                       | **Mov(Longword, src, dst)**                                                                                                               |

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

| TACKY operand                | Assembly operand   |
| ---------------------------- | ------------------ |
| Constant(**ConstInt(int)**)  | Imm(int)           |
| **Constant(ConstLong(int))** | **Imm(int)**       |
| Var(identifier)              | Pseudo(identifier) |

#### Converting Types to Assembly

| Source type | Assembly type | Alignment |
| ----------- | ------------- | --------- |
| **Int**     | **Longword**  | **4**     |
| **Long**    | **Quadword**  | **8**     |

#### Tracking Assembly Types in the Backend Symbol Table

Note that in TACKY instruction constructs, we have no type information.
For example:

```
Unary(unary_operator, val src, val dst)
```

But in Assembly, it does need type to determine which suffix to use in the instruction:

```
Unary(unary_operator, assembly_type, operand)
```

To get the _assembly_type_, we infer the TACKY's operand type from the symbol table.
We will need the same information in later passes as well, so it's best to convert the symbol table to a another form, called it Backend Symbol Table.
Beside the assembly types (already processed from their source types), there are other properties we'll store for later.

Each symbol is converted to _asm_symtab_entry_:

```
asm_symtab_entry = ObjectEntry(assembly_type, bool is_static)
	| FunEntry(bool defined)
```

- ObjEntry is to represent variables, and constants (later chapters)
- FunEntry needs no assembly_type because subroutines in assembly have no types.
- If the implementation of FunAttr in symbol table has _stack_frame_size_ field, add the same field of the FunEntry.

At the end of the CodeGen, we iterate over the frontend symbol table and convert each entry to an entry in the backend symbol table.
For Replace Pseudoregisters, Fix-up Instructions, and Emit stage, we replace any place we refer to the frontend symbol table with the backend symbol table.

### Replacing Pseudoregisters

#### Replacing Longword and Quadword Pseudoregisters

- Extend to replace pseudoregisters in the new _movsx_ instruction.
- Look up the assembly_type in the backend symbol table when assigning a stack space for a pseudoregister: 8 bytes for Quadword and 4 bytes for Longword.
- Make sure the address of each Quadword pseudoregister is 8-byte aligned on the stack. We'll round down the misalignment to the next multiple of 8.

### Fixing Up Instructions

- Specify operand sizes for all the rewrite-rules. The operand size is the same as the original instruction being fixed-up.
- Rewrite _movsx_ instruction if its destination is a memory address, or its source is an immediate value.

The rewrite-rule for _movsx_ is a bit complex, as it might envolve both R10 and R11 registers.

If its both operands are invalid

```
Movsx(Imm(10), Stack(-16))
```

it is rewritten to:

```
Mov(Longword, Imm(10), Reg(R10))
Movsx(Reg(R10), Reg(R11))
Mov(Quadword, Reg(R11), Stack(-16))
```

For _addq_, _imulq_, _subq_, _cmpq_ and _pushq_, immediate values that are outside of the range of _int_ (> 32-bit integer), the values must be copied into register (R10 in our case) before we can use it.
_movq_ can help us with that; it can move immediate values that are outside the _int_ range, but cannot move it directly to a memory.

```
Mov(Quadword, Imm(4294967295), Stack(-16))
```

is fixed up into:

```
Mov(Quadword, Imm(4294967295), Reg(R10))
Mov(Quadword, Reg(R10), Stack(-16))
```

To answer why the _addq_, _imulq_, _subq_, _cmpq_ and _pushq_ instructions cannot deal with integers larger than 32-bit, it's because they all sign extend the immediate operands from 32 to 64 bits. If we have unsigned values that are larger than 32-bit, the sign extending operation will change the value.

We remove AllocateStack so we replace it with:

```
Binary(Sub, Quadword, Imm(bytes), Reg(SP))
```

Lastly, our _Truncate_ TACKY instruction is converted to a 4-byte movl instruction; that is we `movl` an 8-byte operand to a 4-byte destination.

```
Mov(Longword, Imm(4294967299), Reg(R10))
```

The assemblers will help truncate such values for us, some issue warnings, some don't. To make things predictable, we'll truncate the operand value ourselves.

```
Mov(Longword, Imm(3), Reg(R10))
```

## Code Emission

- Add appropriate suffix to instructions
- Emit alignment and initial value for static variables
- Handle new _movsx_ instruction

For most instructions, suffix _l_ for 4-byte operands and _q_ for 8-byte operands.
_Cdq_ is an exception: it's emitted as _cdq_ for 4-byte version and _cdo_ for 8-byte.
_Movsx_ instruction needs suffixes for both operands. Currently, we only sign extend from _int_ to _long_, so we will hardcode it as _lq_ suffix: movslq.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                     | Output                                                                                                                                                                                                                                                                                                           |
| -------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                              | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                             | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, **alignment,** init) (Initialized to zero)          | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;**<init>**                                                                                                                                  |
| StaticVariable(name, global, **alignment,** init) (Initialized to nonzero value) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;**<init>**                                                                                                                                 |
| Global directive                                                                 | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                              | For Linux only: .align **<alignment>**<br>For macOS and Linux: .balign **<alignment>**                                                                                                                                                                                                                           |

#### Formatting Static Initializers

| Static Initializer | Output         |
| ------------------ | -------------- |
| **IntInit(0)**     | **.zero 4**    |
| **IntInit(i)**     | **.long \<i>** |
| **LongInit(0)**    | **.zero 8**    |
| **LongInit(i)**    | **.quad \<i>** |

#### Formatting Assembly Instructions

| Assembly instruction                     | Output                                                                                 |
| ---------------------------------------- | -------------------------------------------------------------------------------------- |
| Mov(**t,** src, dst)                     | mov **\<t>**&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                     |
| **Movsx(src, dst)**                      | **movslq &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                                      |
| Ret                                      | ret                                                                                    |
| Unary(unary_operator, **t,** operand)    | \<unary_operator>**\<t>**&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                            |
| Binary(binary_operator, **t,** src, dst) | \<binary_operator>**\<t>**&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                       |
| Idiv(**t,** operand)                     | idiv **\<t>**&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                        |
| **Cdq(Longword)**                        | cdq                                                                                    |
| **Cdq(Quadword)**                        | **cdo**                                                                                |
| AllocateStack(int)                       | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Cmp(**t,** operand, operand)             | cmp **\<t>**&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                             |
| Jmp(label)                               | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)                  | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)                | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                             | .L\<label>:                                                                            |
| DeallocateStack(int)                     | addq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Push(operand)                            | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                              | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | **neg**          |
| Not               | **not**          |
| Add               | **add**          |
| Sub               | **sub**          |
| Mult              | **imul**         |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |

#### Instruction Suffixes for Assembly Types

| Assembly Type | Instruction suffix |
| ------------- | ------------------ |
| **Longword**  | **l**              |
| **Quadword**  | **q**              |

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
| **Reg(SP)**      | **%rsp**    |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Summary

We set foot on the type system now.
Though our update with Long integers is not a flashy update, it has laid a groundwork for us in later chapters when we need to support more complex types.
The two types we have now, _int_ and _long_, are both signed. We'll implement unsigned version of them in the next chapter.

### Reference Implementation Analysis

[Chapter 11 Code Analysis](./code_analysis/chapter_11.md)

---

# Chapter 12: Unsigned Integers

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

We already have 2 types: Int and Long.
In this chapter, we'll implement their unsigned counterparts: Unsigned Int, Unsigned Long.

## Type Conversions, Again

We'll need to take two aspects into consideration while doing the conversions:

1. How the value is changed
2. How the binary representation is changed

Let's break down our conversions into 4 cases:

### Converting Between Signed and Unsigned Types of the Same Size

- Cases:
  - Int vs Unsigned Int
  - Long vs Unsigned Long

Their binary representations won't change, but how we interpret them is; that is whether to use the two's complement or not.

If a signed integer is positive, its upper bit is 0. So interpreting it as an unsigned number won't change its value.
Also, if an unsigned integer is within the maximum value that the signed counterpart can hold, interpreting it with 2's complement still won't change its value.
Issues only occur when an integer has its upper bit set.

Converting a negative signed integer to the unsigned one, we add its value by a multiple of 2<sup>N</sup> (N is the number of bit of the type).
Conversely, converting an unsigned integer with a leading 1 bit requires us to subtract 2<sup>N</sup> from the value.
To keep things short, "the value is reduced modulo 2^N to be within range of the type."

### Converting Unsigned Int to a Larger Type

- Cases:
  - Unsigned Int -> Long
  - Unsigned Int -> Unsigned Long

As both Long and Unsigned Long can represent any Unsigned Int, we simply _zero extend_ the integer.

### Converting Signed Int to a Larger Type

- Cases:
  - Int -> Long
  - Int -> Unsigned Long

We've already handled Int to Long in Chapter 11. We'll do the same to convert from _Int_ to _Unsigned Long_.
If an _Int_ is positive, both _Long_ and _Unsigned Long_ can hold the value.
If an _Int_ is negative, sign extends work for _Long_, but for _Unsigned Long_, we add 2<sup>64</sup> to the value.

### Converting from Larger to Smaller Types

- Cases:
  - Long -> Int
  - Long -> Unsigned Int
  - Unsigned Long -> Int
  - Unsigned Long -> Unsigned Int

We always do the Truncate for these conversions by reducing the value modulo 2<sup>32</sup> until the value is in range of the new type.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordSigned | signed |
| KeywordUnsigned | unsigned |
| Unsigned integer constants | [0-9]+[uU]\b |
| Unsigned long integer constants | [0-9]+([lL][uU]|[uU][lL])\b |

## The Parser

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
type = Int | Long | <strong>UInt | ULong |</strong> FunType(type* params, type ret)
storage_class = Static | Extern
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
exp = Constant(const) 
	| Var(identifier) 
	| Cast(type target_type, exp)
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) <strong>| ConstUInt(int) | ConstULong(int)</strong></pre></code>

We'll extend the type structure in the symbol table in Chapter 9 instead of defining another data structure.
If the implementation language has signed 64-bit and 32-bit integer types, use them. Otherwise, we should at least make sure the ConstLong node uses an integer type that can represent all long values.

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;param-list&gt; ::= "void" | { &lt;type-specifier&gt; }+ &lt;identifier&gt; { "," { &lt;type-specifier&gt; }+  &lt;identifier&gt; }
&lt;type-specifier&gt; ::= "int" | "long" <strong>| "unsigned" | "signed"</strong>
&lt;specifier&gt; ::= &lt;type-specifier&gt; | "static" | "extern"
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
&lt;factor&gt; ::= &lt;const&gt; | &lt;identifier&gt; 
	| "(" { &lt;type-specifier&gt; }+ ")" &lt;factor&gt; 
	| &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; <strong>| &lt;uint&gt; | &lt;ulong&gt;</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? An int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;long&gt; ::= ? An int or long token ?
<strong>&lt;uint&gt; ::= ? An unsigned int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?</strong>
</pre></code>

### Parser

Parsing type now is complicated as the order is not important.

```
parse_type(specifier_list):
	if (specifier_list is empty
		or specifier_list contains the same specifier twice
		or specifier_list contains both "signed" and "unsigned"):
		fail("Invalid type specifier")
	if specifier_list contains "unsigned" and "long":
		return ULong
	if specifier_list contains "unsigned":
		return UInt
	if specifier_list contains "long":
		return Long
	return Int
```

I won't provide pseudocode for ConstUInt and ConstULong. We parse an unsigned integer constant token as ConstUInt if its value is within _unsigned int_. Otherwise, it's ConstULong.

## Semantic Analysis

Pheww! We don't need to change the Loop Labeling and Idenfier Resolution passes. We only deal with _unsigned_ types during TypeChecking.

### Type Checker

The C standard defines how we should do the usual arithmetic conversions:

- [x] If both operands have the same type, then no further conversion is needed.
- [x] Otherwise, if both operands have signed integer types or both have unsigned integer types, the operand with the type of the lesser integer conversion rank is converted to the type of the operand with greater rank.
- [x] Otherwise, if the operand that has unsigned integer type has rank greater or equal to the rank of the type of the other operand, then the operand with signed integer type is converted to the type of the operand with unsigned integer type.
- [x] Otherwise, if the type of the operand with signed integer type can represent all of the values of the type of the operand with unsigned integer type, then the operand with unsigned integer type is converted to the type of the operand with signed integer type.
- [ ] Otherwise, both operands are converted to the unsigned integer type corresponding to the type of the operand with signed integer type.

The fifth and final rule doesn't apply to us, as in our case, _int_ and _long_ don't have the same size.
The rules lead us to three rules for finding the common type:

```
get_common_type(type1, type2):
	if type1 == type2:
		return type1
	if size(type1) == size(type2):
		if type1 is signed:
			return type2
		else:
			return type1
	if size(type1) > size(type2):
		return type1
	else:
		return type2
```

We add two new kinds of static initializers.

```
static_init = IntInit(int) | LongInit(int) | UIntInit(int) | ULongInit(int)
```

Make sure we do the convert each initializer to the type of the variable. For example:

```
static unsigned int u = 4294967299L;
```

is converted into

```
UIntInit(3)
```

because 4294967299 is outside of the range of unsigned int, so we subtracting 2<sup>32</sup> from it to make it within range. Also,

```
static int i = 4294967246u;
```

is also out of range for signed int, so it is converted into:

```
IntInit(-50)
```

## TACKY Generation

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, static_init init)
instruction = Return(val) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	<strong>| ZeroExtend(val src, val dst)</strong>
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
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
</pre></code>

### Generating TACKY

```
emit_tacky(e, instructions, symbols):
	match e with
	| --snip--
	| Cast(t, inner) ->
		result = emit_tacky(inner, instructions, symbols)
		inner_type = get_type(inner)
		if t == inner_type :
			return result
		dst = make_tacky_variable(t, symbols)
		if size(t) == size(inner_type):
			instructions.append(Copy(result, dst))
		else if size(t) < size(inner_type):
			instructions.append(Truncate(result, dst))
		else if inner_type is signed:
			instructions.append(SignExtend(result, dst))
		else:
			instructions.append(ZeroExtend(result, dst))
		return dst
```

## Assembly Generation

### Assembly

<pre><code>program = Program(top_level*)
assembly_type = LongWord | Quadword
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, static_init init)
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(operand src, operand dst)
		<strong>| MovZeroExtend(operand src, operand dst)</strong>
		| Unary(unary_operator, assembly_type, operand)
		| Binary(binary_operator, assembly_type, operand, operand)
		| Cmp(assembly_type,, operand, operand)
		| Idiv(assembly_type, operand)
		<strong>| Div(assembly_type, operand)</strong>
		| Cdq(assembly_type)
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
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE <strong>| A | AE | B | BE</strong>
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP</pre></code>

Note that MovZeroExtend is a placeholder for now. In the Fix-Up pass, we replace it with either one or two mov instructions depending on the destination is a memory and a register.
In PART II, the destination is always in memory, but it will be different in PART III.

### Converting TACKY to Assembly

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| -------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs)                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Function(name, global, params, instructions) | Function(name, global, [Mov(\<param1 type>, Reg(DI), param1), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<param2 type>, Reg(SI), param2), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<param7 type>, Stack(16), param7), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<param8 type>, Stack(24), param8), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, init)        | StaticVariable(name, global, \<alignment of t, init)                                                                                                                                                                                                                                                                                                                                                                                                              |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                                 | Assembly instructions                                                                                                                    |
| ------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val)                                       | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                    |
| Unary(Not, src, dst)                              | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                          |
| Unary(unary_operator, src, dst)                   | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                    |
| Binary(Divide, src1, src2, dst) (Signed)          | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                     |
| **Binary(Divide, src1, src2, dst) (Unsigned)**    | **Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)** |
| Binary(Remainder, src1, src2, dst) (Signed)       | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                     |
| **Binary(Remainder, src1, src2, dst) (Unsigned)** | **Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)** |
| Binary(arithmetic_operator, src1, src2, dst)      | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                     |
| Binary(relational_operator, src1, src2, dst)      | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                        |
| Jump(target)                                      | Jmp(target)                                                                                                                              |
| JumpIfZero(condition, target)                     | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                            |
| JumpIfNotZero(condition, target)                  | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                           |
| Copy(src, dst)                                    | Mov(\<src type>, src, dst)                                                                                                               |
| Label(identifier)                                 | Label(identifier)                                                                                                                        |
| FunCall(fun_name, args, dst)                      | \<fix stack alignment><br>\<set up arguments><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, Reg(AX), dst)    |
| SignExtend(src, dst)                              | Movsx(src, dst)                                                                                                                          |
| Truncate(src, dst)                                | Mov(Longword, src, dst)                                                                                                                  |
| **ZeroExtend(src, dst)**                          | **MovZeroExtend(src, dst)**                                                                                                              |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------------- | ----------------- |
| Complement     | Not               |
| Negate         | Neg               |
| Add            | Add               |
| Subtract       | Sub               |
| Multiply       | Mult              |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code (signed) | **Assembly condition code (unsigned)** |
| ---------------- | -------------------------------- | -------------------------------------- |
| Equal            | E                                | **E**                                  |
| NotEqual         | NE                               | **NE**                                 |
| LessThan         | L                                | **B**                                  |
| LessOrEqual      | LE                               | **BE**                                 |
| GreaterThan      | G                                | **A**                                  |
| GreaterOrEqual   | GE                               | **AE**                                 |

#### Converting TACKY Operands to Assembly

| TACKY operand                 | Assembly operand   |
| ----------------------------- | ------------------ |
| Constant(ConstInt(int))       | Imm(int)           |
| **Constant(ConstUInt(int))**  | **Imm(int)**       |
| Constant(ConstLong(int))      | Imm(int)           |
| **Constant(ConstULong(int))** | **Imm(int)**       |
| Var(identifier)               | Pseudo(identifier) |

#### Converting Types to Assembly

| Source type | Assembly type | Alignment |
| ----------- | ------------- | --------- |
| Int         | Longword      | 4         |
| **UInt**    | **Longword**  | **4**     |
| Long        | Quadword      | 8         |
| **ULong**   | **Quadword**  | **8**     |

### Replacing Pseudoregisters

We add two more Assembly instructions that have operands: MovZeroExtend and Div.
Extend the pass to replace pseudos in them.

### Fixing Up Instructions

We'll rewrite _Div_ exactly the same as _Idiv_. For _MovZeroExtend_, we look at the destination.
If the destination is a register, we issue a single movl instruction. For example

```
MovZeroExtend(Stack(-16), Reg(AX))
```

is rewritten to this

```
Mov(Longword, Stack(-16), Reg(AX))
```

Otherwise, the destination is a memory, so we _movl_ the source to R11, then move from R11 to the destination. For example

```
MovZeroExtend(Imm(100), Stack(-16))
```

is rewritten to

```
Mov(Longword, Imm(100), Reg(R11))
Mov(Quadword, Reg(R11), Stack(-16))
```

## Code Emission

- Extend to emit _div_ instruction and new condition code.
- Static initializers, UIntInit and ULongInit, are emitted exactly the same as their signed counterparts.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                 | Output                                                                                                                                                                                                                                                                                                           |
| ---------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                          | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                         | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, init) (Initialized to zero)          | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                      |
| StaticVariable(name, global, alignment, init) (Initialized to nonzero value) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                     |
| Global directive                                                             | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                          | For Linux only: .align <alignment><br>For macOS and Linux: .balign <alignment>                                                                                                                                                                                                                                   |

#### Formatting Static Initializers

| Static Initializer | Output         |
| ------------------ | -------------- |
| IntInit(0)         | .zero 4        |
| IntInit(i)         | .long \<i>     |
| LongInit(0)        | .zero 8        |
| LongInit(i)        | .quad \<i>     |
| **UIntInit(0)**    | **.zero 4**    |
| **UIntInit(i)**    | **.long \<i>** |
| **ULongInit(0)**   | **.zero 8**    |
| **ULongInit(i)**   | **.quad \<i>** |

#### Formatting Assembly Instructions

| Assembly instruction                 | Output                                                                                 |
| ------------------------------------ | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                     | mov \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                         |
| Movsx(src, dst)                      | movslq &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| Ret                                  | ret                                                                                    |
| Unary(unary_operator, t, operand)    | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst) | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| Idiv(t, operand)                     | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| **Div(t, operand)**                  | **div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>**                                         |
| Cdq(Longword)                        | cdq                                                                                    |
| Cdq(Quadword)                        | cdo                                                                                    |
| AllocateStack(int)                   | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Cmp(t, operand, operand)             | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| Jmp(label)                           | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)              | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)            | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                         | .L\<label>:                                                                            |
| DeallocateStack(int)                 | addq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Push(operand)                        | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                          | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |
| **B**          | **b**              |
| **BE**         | **be**             |
| **A**          | **a**              |
| **AE**         | **ae**             |

#### Instruction Suffixes for Assembly Types

| Assembly Type | Instruction suffix |
| ------------- | ------------------ |
| Longword      | l                  |
| Quadword      | q                  |

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
| Reg(SP)          | %rsp        |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Summary

Thanks to the groundwork we established in chapter 11, supporting unsigned integer types requires much less effort.
Our next target is Floating-point numbers, which are processed differently from integers in the hardware levels as they have their own dedicated registers.

### Reference Implementation Analysis

[Chapter 12 Code Analysis](./code_analysis/chapter_12.md)

---

# Chapter 13: Floating-point numbers

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

We already have 4 integer types, now we'll support a floating-point binary representation type called _double_.
Many aspects of floating-point numbers are implementation-defined, based on the C standards. So we'll need to work on the two tasks:

- Figure out the behavior based on IEEE 754.
- New set of Assembly instructions and registers are required to operate on floating-point numbers.

## The IEEE 754 Double-Precision format

```
			  64-bit IEEE 754 Double Precision Layout

   Bit Index:      63        62                      52         51              0
				   +---------+-----------------------+---------+----------------+
				   |   S     |       Exponent        |         |    Fraction    |
				   | (1 bit) |      (11 bits)        |         |   (52 bits)    |
				   +---------+-----------------------+---------+----------------+
```

The bianry representation of a floating-point number has 3 fields:

- S: sign bit
- E: Exponent
- F: Fraction, sometimes also called _Significant_ or _Mantissa_

The binary fraction has an implicit integer part: 1.
The 52 bit of the significand only encodes the the fractional part, representing negative powers of 2: 1/2, 1/4, 1/8, so on.

For example, the representation

```
1000000000000000000000000000000000000000000000000000
```

is the binary fraction 1.1, which is 1.5 in decimal notation.

The Sign bit is 1, indicating the floating-number to be negative, positive otherwise.

The Exponent has value between -1,022 and 1,023. It uses _biased encoding_: get the binary value of it, subtract 1,023 to get the exponent. For example:

```
00000000010
```

is means 2 in decimal, so the exponent is 2 - 1,023 = -1,021.
All 11 bits of the Exponent being set to 0 or 1 indicates special values as follow

### Zero and negative zero

```
if S == 0 AND E == 0 AND F == 0:
	X = 0.0
else if S == 1 AND E == 0 AND F == 0:
	X = -0.0
```

0.0 and -0.0 equal when compared, mainly to follows the usual rules for determining the sign of arithmetic results: -1.0 _ 0.0 == 1.0 _ -0.0 == -0.0

### Subnormal numbers

```
if E == 0s:
	F has significand of 0 instead of 1
	E has value of -1,022
```

The significand part has a value between 1 and 2. We say that floating-point numbers are _normalized_.
The smallest number that it can represent is 1x2<sup>-1,022</sup>.
But what if we want to represent number closer to 0?
We'll use _subnormal_ numbers. It's enabled when the **Exponent is all 0s**, indicating an exponent of still -1,022, but the significand now is 0 instead of 1.
Subnormal numbers are slower to work, so many implementations have allowed users to disable them and round any results to zero.

### Infinity

```
if E == 1s AND F == 0s:
	if S = 0:
		X = Positive Infinity
	else:
		X = Negative Infinity
```

The largest magnitude a floating-number can represent is ~2<sup>1,023</sup>.
Anything larger than that is rounded to Infinity, represented by **Exponent is all 1s** and **Fraction is all 0s**.
Positive or Negative Infinity is detemrined by the sign bit.
For example: -1.0 / 0.0 = Negative Infinity

### NaN

```
if E == 1s AND F != 0s:
	X = NaN
```

Not-A-Number is a floating-point number whose **Exponent is all 1s** and **Fraction is nonzero**.

We'll support the first 3 values, and leaving Quiet NaN for Extra Credit.

## Rounding Behavior

### Rounding Modes

There are several modes:

- Rounding to nearest
- Rounding toward zero
- Rounding toward positive infinity
- Rounding toward negative infinity

All of the four modes above are supported modern processors, and we'll support 2 of them in our compiler: _round-to-nearest_ and _ties-to-even_.

Round-to-nearest rounds the value to the nearest representable double.
Ties-to-even rounds the value to one whose least significand bit is 0, of the two representable value.

### Rounding Constants

Most of the time, double constants cannot be represented exactly in binary floating point.
For example, a constant is 0.1, and in binary floating point, there is no adding-up of any power of 2 in the fractions to get 0.1.
In decimal notation, the value is rounded to the nearest representable value: `0.1000000000000000055511151231257827021181583404541015625`

However, in binary floating point of 53-bit, the 0.1 is shown as: `1.100110011001100110011001100110011001100110011001101 * 2^-4`

### Rounding Type Conversions

An integer may be converted to a double because of the spacing between double's representable values.
Let's take a decimal format three-digit realm as an example.
992 -> 9.92 x 10<sup>2</sup>
993 -> 9.93 x 10<sup>2</sup>
...
1000 -> 1.00 x 10<sup>3</sup>

Now, for numbers larger than 1000

1001 -> ??? 1.01 x 10<sup>3</sup> is 1010.
There's a gap of 10; and the gap increase as the magnitude grows.
We have the same problem when converting _Long_ and _Unsigned Long_ to double.

_Long_ and _Unsigned Long_ have 64-bit precision, while double has only 53-bit precision in its fraction.

### Rounding Arithmetic Operations

We may also need to round results of operations like addition, subtraction, multiplication, also because of the gap between representable values.
For example in the decimal format three-digit realm again:
993 + 45 = 1,038; the value is more than 3 digits, so we'll round it to 1.04 x 10<sup>3</sup>.
Division is also a pain, just like 1/3 cannot be represented in any decimal digits, but rounding to 0.33333.

This rounding error is handled by the hardware, so we're good with this.

## The Lexer

Support floating-point numbers requires to change the way to recognize integers.

Old tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Signed integer constant | ([0-9]+)[^\w.] |
| Unsigned integer constant | ([0-9]+[uU])[^\w.] |
| Unsigned long integer constant| ([0-9]+([lL][uU]|[uU][lL]))[^\w.] |
| Signed integer constant | ([0-9]+)[^\w.] |

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordDouble | double |
| Floating-point constants | (([0-9]_\.[0-9]+|[0-9]+\.?)[Ee][+-]?[0-9]+|[0-9]_\.[0-9]+|[0-9]+\.)[^\w.] |

The changes for integer will match `100;` for example. Make sure you process it to remove the trailing `;`.

## The Parser

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
type = Int | Long | UInt | ULong <strong>| Double</strong> | FunType(type* params, type ret)
storage_class = Static | Extern
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
exp = Constant(const) 
	| Var(identifier) 
	| Cast(type target_type, exp)
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) <strong>| ConstDouble(double)</strong></pre></code>

We'll extend the type structure in the symbol table in Chapter 9 instead of defining another data structure.
If the implementation language has signed 64-bit and 32-bit integer types, use them. Otherwise, we should at least make sure the ConstLong node uses an integer type that can represent all long values.

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;identifier&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;param-list&gt; ::= "void" | { &lt;type-specifier&gt; }+ &lt;identifier&gt; { "," { &lt;type-specifier&gt; }+  &lt;identifier&gt; }
&lt;type-specifier&gt; ::= "int" | "long" | "unsigned" | "signed" <strong>| "double"</strong>
&lt;specifier&gt; ::= &lt;type-specifier&gt; | "static" | "extern"
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
&lt;factor&gt; ::= &lt;const&gt; | &lt;identifier&gt; 
	| "(" { &lt;type-specifier&gt; }+ ")" &lt;factor&gt; 
	| &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;unop&gt; ::= "-" | "~" 
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; | &lt;uint&gt; | &lt;ulong&gt; | &lt;double&gt;
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? An int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;long&gt; ::= ? An int or long token ?
&lt;uint&gt; ::= ? An unsigned int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?
<strong>&lt;double&gt; ::= ? A floating-point constant token ?</strong>
</pre></code>

### Parser

There are many combination of integer type specifiers, but for double, we have only one.

```
parse_type(specifier_list):
	if specifier_list == ["double"]:
		return Double
	if specifier_list contains "double":
		fail("Can't combine 'double with other type specifiers")
	--snip--
```

**Notes**:

- The implementation language should handle the rounding from decimal constants to binary floating point.
- Floating-point constants can't go out of range as they support positive/negative infinity; its range includes all real numbers.

## Semantic Analysis

### Type Checker

- Annotate the double constant with type Double
- Update how to get the common real type of two values

```
get_common_type(type1, type2):
	if type1 == type2:
		return type1
	if  type1 == Double or type2 == Double:
		return Double
	--snip--
```

The bitwise complemnt `~` and remainer `%` accept only integer operands.

Here is the pseudocode for the update for `~` operator. We do the same for `%`.

```
typecheck_exp(e, symbols):
	match e with:
	| --snip--
	| Unary(complement, inner) ->
		typed_inner = typecheck_exp(inner, symbols)
		if get_type(typed_inner) == Double:
			fail("Can't take the bitwise complement of a double")
		unary_exp = Unary(complement, typed_inner)
		return set_type(unary_exp, get_type(typed_inner))
```

We add a new kind of initializer for static variables.

```
static_init = IntInit(int) | LongInit(int) | UIntInit(int) | ULongInit(int) | DoubleInit(double)
```

Here, if we convert from double to an integer, we truncate toward zero. For example: 2.7 -> 2.
Otherwise, if we convert from an integer to a double, we preverse the value if it can be represented exactly; round to the nearest representable value otherwise.

## TACKY Generation

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, static_init init)
instruction = Return(val) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	| ZeroExtend(val src, val dst)
	<strong>| DoubleToInt(val src, val dst)
	| DoubleToUInt(val src, val dst)
	| IntToDouble(val src, val dst)
	| UIntToDouble(val src, val dst)</strong>
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
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
</pre></code>

We have no distinctions in the size of integer operands when converting to or from double.

### Generating TACKY

To update this pass, emit the appropriate cast instruction when we encounter a cast from or to _double_.

## Assembly Generation

SSE instructions an't use general-purpose registers, and non-SSE intructions can't use XMM registers.
Both SSE and non-SSE instructions can refer to values in memory.

SSE instructions can't use immediate operands. If we need to use a contants in an SSE instruction, we'll define that constant in read-only memory.
For example, here is a program with a subroutine to compute 1.0 + 1.0

```
	.section .rodata
	.align 8
.L_one:
	.double 1.0
	.text
one_plus_one:
	movsd .L_one(%rip), %xmm0
	addsd .L_one(%rip), %xmm0
	--snip--
```

#### Calling conventions

A function's first 8 floating-point arguments are passed in registers XMM0 through XMM7.
Any remaining floating point-arguments are pushed onto the stack in reverse order.
Floating-point return values are passed in XMM0 instead of RAX.

```
int pass_paramenters(
	double d1, double d2, int i1,
	double d3, double d4, double d5,
	double d6, unsigned int i2, long i3,
	double d7, double d8, unsigned long i4,
	double d9, int i5, double d10,
	int i6, int i7, double d11,
	int i8, int i9);
```

```
General-purpose				Floating point					Stack contents
registers					registers
+-------+------+			+--------+--------+				+-----------+ 		+-----------+
|  RDI  |  i1  | 			|  XMM0  |   d1   | 			|  d9       |   <-- |    RSP	|
|  RSI  |  i2  |			|  XMM1  |   d2   |				|  d10      |		+-----------+
|  RDX  |  i3  |			|  XMM2  |   d3   |				|  i7       |
|  RCX  |  i4  | 			|  XMM3  |   d4   | 			|  d11      |
|  R8   |  i5  | 			|  XMM4  |   d5   | 			|  i8       |
|  R9   |  i6  |			|  XMM5  |   d6   |				|  i9       |
+-------+------+			|  XMM6  |   d7   |				+-----------+
							|  XMM7  |   d8   |				|   Caller  |
							+--------+--------+				|   stack   |
															|   frame   |
															+-----------+
```

#### Arithmetic with SSE instructions

We have addsd, subsd, mulsd and divsd.
All of these require an XMM register or memory address as a source and an XMM register as a destination.
There's no negation for floating-point numbers. We use `xorpd` (16 bytes aligned only) with `-0.0` constant to flip the sign bit.

To zero an XMM register, we also use `xorpd` it with itself. This trick works for general-purpose registers as well, but we didn't apply as we prioritize clarity and simplicity.
But for XMM registers, XORing them is the simplest way so we don't have to make a `0.0` constant in the read-only memory.

#### Comparing floating-point numbers

We use `comisd` instruction.

```
Executing comisd a, b
if a < b:
	CF = 1
else:
	CF = 0

SF = 0
OF = 0
```

Floating-point numbers are always signed, and the comparisons result in only the Carry flag, we use condition code: A, AE, B and BE.

#### Converting between Floating-point and Integer types

The SSE instruction set includes conversions to and from signed integer types, so implementing `IntToDouble` and `DoubleToInt` is easy.
Implementing `UIntToDouble` and `DoubleToUInt` takes more effort.

##### Converting a double to a Signed Integer

We use `cvttsd2si` instruction for `DoubleToInt`. We don't care if the value of double is outside the range of the target integer type: Int (suffix l) and Long (suffix q).

##### Converting a double to an Unsigned Integer

To convert a double to an unsigned int, we'll convert it to signed long, and truncate the value. This is because any value that is in range of unsigned int is also in range of signed long.

```
	cvttsd2si %xmm0, %rax
	movl %eax, -4(%rbp)
```

Converting from double to unsigned long is trickier.
First, we check if the value is in range of signed long.
If yes, we use the cvttsd2siq as usual.
If not, we subtract the value of LONG_MAX + 1 from the double to make it in range of signed long, convert it to signed long using cvttsd2siq, then add the LONG_MAX + 1 after the conversion.
We use LONG_MAX + 1 (2<supt>63</sup>), not LONG_MAX because LONG_MAX + 1 is the representable value of double so we minimize any unecessary rounding.

```
	.section .rodata
	.align 8
.L_upper_bound:
	.double 9223372036854775808.0
	.text
	--snip--
	comisd 	.L_upper_bound(%rip), %xmm0
	jae .L_out_of_range
	cvttsd2siq	%xmm0, %rax
	jmp .L_end
.L_out_of_range:
	movsd	%xmm0, %xmm1
	subsd 	.L_upper_bound(%rip), %xmm1
	cvttsd2siq	%xmm1, %rax
	movq	$9223372036854775808, %rdx
	addq	%rdx, %rax
.L_end:
```

##### Converting a Signed Integer to a Double

The `cvtsi2sd` instruction converts a signed int (suffix l) or long (suffix q) to a double.
If the result is not representable, the CPU helps round it for us.

##### Converting an Unsigned Integer to a Double

To convert an unsigned int to a double, we zero extend it to a long, and convert it to a double with cvtsi2sdq.
For example, we want to convert 4294967290 to a double:

```
	movl $4294967290, %rax
	cvtsi2sdq %rax, %xmm0
```

For the conversion from unsigned long to a double, we'll first check whether the value is in the range of signed long. If it is, we use `cvtsi2sdq` directly. Otherwise, we halve the source value, convert it with `cvtsi2sdq`, and double back the result.

```
	cmpq 	$0, -8(%rbp)
	jl 	.L_out_of_range
	cvtsi2dsq 	-8(rbp), %xmm0
	jmp .L_end
.L_out_of_range:
	movq 	-8(%rbp), %rax
	movq 	%rax, %rdx
	shrq 	%rdx
	andq 	$1, %rax
	orq 	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
.L_end:
```

Note that before halving the source value, we check if it's odd or even, and add 1 if needed to the halfed value before converting to make sure we don't commit the double rounding error.

### Assembly

<pre><code>program = Program(top_level*)
assembly_type = LongWord | Quadword <strong>| Double</strong>
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, static_init init)
	<strong>| StaticConstant(identifier name, int alignment, static_init init)</strong>
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(operand src, operand dst)
		| MovZeroExtend(operand src, operand dst)
		<strong>| Cvttsd2si(assembly_type dst_type, operand src, operand dst)
		| Cvtsi2sd(assembly_type src_type, operand src, operand dst)</strong>
		| Unary(unary_operator, assembly_type, operand)
		| Binary(binary_operator, assembly_type, operand, operand)
		| Cmp(assembly_type,, operand, operand)
		| Idiv(assembly_type, operand)
		| Div(assembly_type, operand)
		| Cdq(assembly_type)
		| Jmp(identifier)
		| JmpCC(cond_code, identifier)
		| SetCC(cond_code, operand)
		| Label(identifier)
		| AllocateStack(int)
		| DeallocateStack(int)
		| Push(operand)
		| Call(identifier) 
		| Ret
unary_operator = Neg | Not <strong>| Shr</strong>
binary_operator = Add | Sub | Mult <strong>| DivDouble | And | Or | Xor</strong>
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int) | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP 
	<strong>| XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15</strong></pre></code>

**Note**:

- _Cmp_ and _Binary_ instructions are reused for _comsid_ and _addsd_, _subsd_, and so on.
- _DivDouble_ is added, as _Div_ and _Idiv_ don't follow other arithmetic instructions' pattern.
- _Xor_ binary operator is added to negate the floating-point values, as well as bitwise _And_ and _Or_ to convert unsigned long to double. (If you haven't worked on the previous extra credit)
- _Shr_ unary operator is necessary for type conversion too. (If you haven't worked on the previous extra credit)

#### Floating-point constants

When we encounter an integer constant in TACKY, we convert it to Imm in Assembly.
For Double constants, it's not possible. We have to create _StaticConstant_ with a unique identifier, and refer to it with _Data_ operand, similar to _StaticVariable_.
So, this TACKY instruction

```
Copy(Constant(ConstDouble(1.0)), Var("x"))
```

is converted to this

```
StaticConstant(const_label, 8, DoubleInit(1.0))
--snip--
Mov(Double, Data(const_label), Pseudo("x"))
```

Ensure to keep track of every _StaticConstant_ you define throughout the entire assembly generation pass.
Then, add these constants to the top-level constructs.

**Notes**:

- Avoid duplicate constants: don't generate every StaticConstant everytime, check if one already exists first with the same value and alignment.
- Using local labels for top-level constants: don't add the local label prefix now, but in the code emission pass.

To distinguish static constants with static variables in the Assembly Symbol Table, we simply add a new boolean _is_constant_ field.

```
asm_symtab_entry = ObjEntry(assembly_type, bool is_static, bool is_constant)
	| FunEntry(bool defined)
```

During the code emission pass, we'll base on the _is_constant_ to determine to add the local label prefix to their names.
If a variable is a constant, is_static must be true as well.

#### Unary instructions, Binary Instructions, and Conditional Instructions

Floating-point addition, subtraction, multiplciation have similar patterns to their integer equivalents.
Double division also does the same, though the integer division does not.

The instruction

```
Division(Double, Var("src1"), Var("src2"), Var("dst"))
```

is converted to

```
Mov(Double, Pseudo("src1"), Pseudo("dst"))
Binary(DivDouble, Double, Pseudo("src2"), Pseudo("dst"))
```

Floating-point negation is more complicated than the integer counterpart.
We need to XOR it with -0.0 to flip the sign bit.
For example, the instruction

```
Unary(Negate, Var("src"), Var("dst"))
```

is converted to

```
StaticConstant(const, 16, DoubleInit(-0.0))
--snip--
Mov(Double, Pseudo("src"), Pseudo("dst"))
Binary(Xor, Double, Data(const), Pseudo("dst"))
```

We need to align the -0.0 16-byte as we'll emit the `xorpd` instruction.
We don't care the alignment of the _dst_ because _xorpd_'s destination is always a register. We'll take care of it during the instruction-fixup pass.

Finally, for relational operators, we treat floating-point comparisions the same as unsigned integer comparisons because _comisd_ only sets CF and ZF, the SF and OF are always 0.
An example:

```
Binary(LessThan, Var("x"), Var("y"), Var("dst"))
```

is converted to

```
Cmp(Double, Pseudo("y"), Pseudo("x"))
Mov(Longword, Imm(0), Pseudo("dst"))
SetCC(B, Pseudo("dst"))
```

This approach is also applied to the three TACKY instructions that compare a value to zero: JumpIfZero, JumpIfNotZero, and Not

For example, the instruction

```
JumpIfZero(var("x"), "label")
```

is converted to

```
Binary(Xor, Double, Reg(XMM0), Reg(XMM0))
Cmp(Double, Pseudo("x"), Reg(XMM0))
JmpCC(E, "label")
```

It doesn't have to be XMM0 here, but make sure you don't use scratch registers for the rewrite pass to avoid conflicting uses.

#### Type Conversion

We already covered this part in the beginning. There are some notes:

- Avoid the callee-saved registers: RBX, R12, R13, R14 and R15
- Avoid the registers used for rewrite pass: R10, R11, XMM14 and XMM15.
- Emit the instruction _MovZeroExtend_ when converting _unsigned int_ to _double_.

For example, if `x` is an _unsigned int_, then the instrution

```
UIntToDouble(Var("x"), Var("y"))
```

is converted to Assembly:

```
MovZeroExtend(Var("x"), Reg(AX))
Cvtsi2sd(Quadword, Reg(AX), Pseudo("y"))
```

#### Function Calls

There are two places we need to determine which register, or stack, is needed for an argument: In function body, and in function call.
We'll write a helper function _classify_parameters_ to handle the bookkeeping we need in both places.
The function split the arguments list into 3:

- Arguments to be passed in general-purpose registers
- Arguments to be passed in XMM registers
- Arguments to be passed on the stack

```
classify_parameters(values):
	int_reg_args = []
	double_reg_args = []
	stack_args = []

	for v in values:
		operand = convert_val(v)
		t = assembly_type_of(v)
		typed_operand = (t, operand)
		if t == Double:
			if length(double_reg_args) < 8:
				double_reg_args.append(operand)
			else:
				stack_args.append(typed_operand)
		else:
			if length(int_reg_args) < 6:
				int_reg_args.append(typed_operand)
			else:
				stack_args.append(typed_operand)

	return (int_reg_args, double_reg_args, stack_args )
```

Note that when we're building the stack_args list, we preserve the order they appear.

In a function body, we copy every parameter from their initial locations into pseudoregisters.

```
set_up_parameters(parameters):

	// classify them
	int_reg_params, double_reg_params, stack_params = classify_parameters(parameters)

	// copy parameters from general-purpose registers
	int_regs = [ DI, SI, DX, CX, R8, R9 ]
	reg_index = 0
	for (param_type, param) in int_reg_params:
		r = int_regs[reg_index]
		emit(Mov(param_type, Reg(r), param))
		reg_index += 1

	// copy parameters from XMM registers
	double_regs = [ XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7 ]
	reg_index = 0
	for param in double_reg_params:
		r = double_regs[reg_index]
		emit(Mov(Double, Reg(r), param))
		reg_index += 1

	// Copy parameters from stack
	offset = 16
	for (param_type, param) in stack_params:
		emit(Mov(param_type, Stack(offset), param))
		offset += 8
```

We also use _classify_parameters_ in TACKY FunCall.

```
convert_function_call(FunCall(fun_name, args, dst)):
	int_registers = [ DI, SI, DX, CX, R8, R9 ]
	double_registers = [ XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7 ]

	// classify arguments
	int_args, double_args, stack_args = classify_parameters(args)

	// adjust stack alignment
	if length(stack_args) is odd:
		stack_padding = 8
	else:
		stack_padding = 0

	if stack_padding != 0:
		emit(Binary(Sub, Quadword, Imm(stack_padding), Reg(SP)))

	// pass args in registers
	reg_index = 0
	for (assembly_type, assembly_arg) in int_args:
		r = int_registers[reg_index]
		emit(Mov(assembly_type, assembly_arg, Reg(r)))
		reg_index += 1

	reg_index = 0
	for assembly_arg in double_args:
		r = double_registers[reg_index]
		emit(Mov(Double, assembly_arg, Reg(r)))
		reg_index += 1

	// pass args on stack
	for (assembly_type, assembly_arg) in reverse(stack_args):
		if (assembly_arg is a Reg or Imm operand
			or assembly_type == Quadword
			or assembly_type == Double):
			emit(Push(assembly_arg))
		else:
			emit(Mov(assembly_type, assembly_arg, Reg(AX)))
			emit(Push(Reg(AX)))

	// emit call instructions
	emit(Call(fun_name))

	// adjust stack pointer
	bytes_to_remove = 8 * length(stack_args) + stack_padding
	if bytes_to_remove != 0:
		emit(Binary(Add, Quadword, Imm(bytes_to_remove), Reg(SP)))

	// Retrieve return value
	assembly_dst = convert_val(dst)
	return_type = assembly_type_of(dst)
	if return_type == Double:
		emit(Mov(Double, Reg(XMM0), assembly_dst))
	else:
		emit(Mov(return_type, Reg(AX), assembly_dst))
```

#### Return Instructions

```
Return(Var("x"))
```

We first look up the type of x in the backend symbol table. If it's an integer, we copy the value to AX. If it's a double, we copy the value to XMM0 for the return.

```
Mov(Double, Pseudo("x"), Reg(XMM0))
Ret
```

### Converting TACKY to Assembly

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| -------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs **+ \<all StaticConstant constructs for floating-point constants>**)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| Function(name, global, params, instructions) | Function(name, global, [<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<first int param type>**, Reg(DI), **\<first int param>**), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<second int param type>**, Reg(SI), **\<second int param>**), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four integer parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;**Mov(\<first double param type>, Reg(XMM0), \<first double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second double param type>, Reg(XMM1), \<second double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next six double parameters from registers>**<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<first stack param type>**, Stack(16), **\<first stack param>**), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(**\<second stack param type>**, Stack(24), **\<second stack param>**), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, init)        | StaticVariable(name, global, \<alignment of t, init)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                             | Assembly instructions                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| --------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val) (Integer)                         | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| **Return(val) (Double)**                      | **Mov(\<Double>, val, Reg(XMM0))<br>Ret**                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Unary(Not, src, dst) (Integer)                | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **Unary(Not, src, dst) (Double)**             | **Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, src, Reg(\<X>))<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)**                                                                                                                                                                                                                                                                                                                                                                     |
| **Unary(Negate, src, dst) (Double negation)** | **Mov(Double, src, dst)<br>Binary(Xor, Double, Data(\<negative-zero>), dst)<br>And add a top-level constant:<br>StaticConstant(\<negative-zero>, 16, DoubleInit(-0.0))**                                                                                                                                                                                                                                                                                                                           |
| Unary(unary_operator, src, dst)               | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Binary(Divide, src1, src2, dst) (Signed)      | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                                               |
| Binary(Divide, src1, src2, dst) (Unsigned)    | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                               |
| Binary(Remainder, src1, src2, dst) (Signed)   | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                                               |
| Binary(Remainder, src1, src2, dst) (Unsigned) | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                               |
| Binary(arithmetic_operator, src1, src2, dst)  | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                                                                                                                                                                                                                                                                                                                                                                               |
| Binary(relational_operator, src1, src2, dst)  | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                                                                                                                                                                                                                                                                                                                                                                                  |
| Jump(target)                                  | Jmp(target)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| JumpIfZero(condition, target) (Integer)       | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| **JumpIfZero(condition, target) (Double)**    | **Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(E, target)**                                                                                                                                                                                                                                                                                                                                                                                             |
| JumpIfNotZero(condition, target)              | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| **JumpIfNotZero(condition, target) (Double)** | **Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(NE, target)**                                                                                                                                                                                                                                                                                                                                                                                            |
| Copy(src, dst)                                | Mov(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| Label(identifier)                             | Label(identifier)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| FunCall(fun_name, args, dst)                  | \<fix stack alignment><br>**\<move arguments to general-purpose registers>**<br>**\<move arguments to XMM registers>**<br>**\<push arguments onto the stack>**<br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, **\<dst register>**, dst)                                                                                                                                                                                                                                 |
| SignExtend(src, dst)                          | Movsx(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Truncate(src, dst)                            | Mov(Longword, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| ZeroExtend(src, dst)                          | MovZeroExtend(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| **IntToDouble(src, dst)**                     | **Cvtsi2sd(<src type>, src, dst)**                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| **DoubleToInt(src, dst)**                     | **Cvttsd2si(<dst type>, src, dst)**                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| **UIntToDouble(src, dst) (unsigned int)**     | **MovZeroExtend(src, Reg(\<R>))<br>Cvtsi2s(Quadword, Reg(\<R>), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                             |
| **UIntToDouble(src, dst) (unsigned long)**    | **Cmp(Quadword, Imm(0), src)<br>JmpCC(L, \<label1>)<br>Cvtsi2sd(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Quadword, src, Reg(\<R1>))<br>Mov(Quadword, Reg(\<R1>), Reg(\<R2>))<br>Unary(Shr, Quadword, Reg(\<R2>))<br>Binary(And, Quadword, Imm(1), Reg(\<R1>))<br>Binary(Or, Quadword, Reg(\<R1>), Reg(\<R2>))<br>Cvtsi2sd(Quadword, Reg(\<R2>), dst)<br>Binary(Add, Double, dst, dst)<br>Label(\<label2>)**                                                                |
| **DoubleToUInt(src, dst) (unsigned int)**     | **Cvttsd2si(Quadword, src, Reg(\<R>))<br>Mov(Longword, Reg(\<R>), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                           |
| **DoubleToUInt(src, dst) (unsigned long)**    | **Cmp(Double, Data(\<upper-bound>), src)<br>JmpCC(AE, \<label1>)<br>Cvttsd2si(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Double, src, Reg(\<X>))<br>Binary(Sub, Double, Data(\<upper-bound>),Reg(\<X>))<br>Cvttsd2si(Quadword, Reg(\<X>), dst)<br>Mov(Quadword, Imm(9223372036854775808), Reg(\<R>))<br>Binary(Add, Quadword, Reg(\<R>), dst)<br>Label(\<label2>)<br>And add a top-level constant:<br>StaticConstant(\<upper-bound>, 8, DoubleInit(9223372036854775808.0))** |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator               | Assembly operator |
| ---------------------------- | ----------------- |
| Complement                   | Not               |
| Negate                       | Neg               |
| Add                          | Add               |
| Subtract                     | Sub               |
| Multiply                     | Mult              |
| **Divide (double division)** | **DivDouble**     |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code (signed) | Assembly condition code (unsigned **or double**) |
| ---------------- | -------------------------------- | ------------------------------------------------ |
| Equal            | E                                | E                                                |
| NotEqual         | NE                               | NE                                               |
| LessThan         | L                                | B                                                |
| LessOrEqual      | LE                               | BE                                               |
| GreaterThan      | G                                | A                                                |
| GreaterOrEqual   | GE                               | AE                                               |

#### Converting TACKY Operands to Assembly

| TACKY operand                     | Assembly operand                                                                                     |
| --------------------------------- | ---------------------------------------------------------------------------------------------------- |
| Constant(ConstInt(int))           | Imm(int)                                                                                             |
| Constant(ConstUInt(int))          | Imm(int)                                                                                             |
| Constant(ConstLong(int))          | Imm(int)                                                                                             |
| Constant(ConstULong(int))         | Imm(int)                                                                                             |
| **Constant(ConstDouble(double))** | **Data(\<ident>)<br>And add top-level constant:<br>StaticConstant(\<ident>, 8, DoubleInit(Double))** |
| Var(identifier)                   | Pseudo(identifier)                                                                                   |

#### Converting Types to Assembly

| Source type | Assembly type | Alignment |
| ----------- | ------------- | --------- |
| Int         | Longword      | 4         |
| UInt        | Longword      | 4         |
| Long        | Quadword      | 8         |
| ULong       | Quadword      | 8         |
| **Double**  | **Double**    | **8**     |

### Replacing Pseudoregisters

Extend the pass to support new instructions, allocate 8 bytes on the stack for each double pseudoregister and make sure they are 8-byte aligned.
If the backend symbol table indicates that a double has static storage duration, you should replace any references to it with _Data_ operands, similar to Quadword pseudoregisters.

### Fixing Up Instructions

We'll dedicate XMM14 to rewrite source operands, and XMM15 for destination operands, the same as R10 and R11 of general-purpose registers.

The destination of _cvttsd2si_ must be a register.

```
Cvttsd2si(Quadword, Stack(-8), Stack(-16))
```

is rewritten to

```
Cvttsd2si(Quadword, Stack(-8), Reg(R11))
Mov(Quadword, Reg(R11), Stack(-16))
```

The instruction _cvtsi2sd_ has two constraints:

- The source CAN'T be a constant
- The destination must be a register

```
Cvtsi2sd(Longword, Imm(10), Stack(-8))
```

is fixed up into

```
Mov(Longword, Imm(10), Reg(R10))
Cvtsi2sd(Longword, Reg(R10), Reg(XMM15))
Mov(Double, Reg(XMM15), Stack(-8))
```

The _comisd_ instruction requires its destination to be a register.

```
Cmp(Double, Stack(-8), Stack(-16))
```

is rewritten into

```
Mov(Double, Stack(-16), Reg(XMM15))
Cmp(Double, Stack(-8), Reg(XMM15))
```

The _addsd_, _subsd_, _mulsd_ and _divsd_ or _xorpd_ instructions also need register destinations.
Further more, the _xorpd_ needs a register or a 16-byte-aligned memory address as the source operand. (We alreay satisfy this when emitting the instruction)

The _movsd_ has the same constraints as _mov_; that is both its source and destination can't be memory address at the same time. So do _and_ and _or_ instructions; they also can't take immediate source operands outside the range of int.
The _push_ instruction cannot push XMM registers. Right now, we only push immediate values or memory operands, so we don't have this error right now, but we will in Chapter III.

## Code Emission

- Use decimal floating-point constants in Assembly: 20.0, instead of `4626322717216342016` or `0x2.8p+3`
- StaticConstant needs to have Local Label Prefix
- Double constants are stored in Read-Only section
- Static variables of type double are not stored in BSS section, or initialized with `.zero` directive because their representation in binary are never all zeros.

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                                   | Output                                                                                                                                                                                                                                                                                                           |
| ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                                            | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                                           | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, init) (Initialized to zero)                            | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                      |
| StaticVariable(name, global, alignment, init) (Initialized to nonzero value **OR any double**) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                     |
| **StaticConstant(name, alignment, init) (Linux)**                                              | **&nbsp;&nbsp;&nbsp;&nbsp;.section .rodata<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>**                                                                                                                                                                     |
| **StaticConstant(name, alignment, init) (MacOS 8-byte aligned constants)**                     | **&nbsp;&nbsp;&nbsp;&nbsp;.literal8<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 8<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>**                                                                                                                                                                                         |
| **StaticConstant(name, alignment, init) (MacOS 16-byte aligned constants)**                    | **&nbsp;&nbsp;&nbsp;&nbsp;.literal16<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 16<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init><br>&nbsp;&nbsp;&nbsp;&nbsp;.quad 0**                                                                                                                                                    |
| Global directive                                                                               | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                                            | For Linux only: .align <alignment><br>For macOS and Linux: .balign <alignment>                                                                                                                                                                                                                                   |

#### Formatting Static Initializers

| Static Initializer | Output                                                   |
| ------------------ | -------------------------------------------------------- |
| IntInit(0)         | .zero 4                                                  |
| IntInit(i)         | .long \<i>                                               |
| LongInit(0)        | .zero 8                                                  |
| LongInit(i)        | .quad \<i>                                               |
| UIntInit(0)        | .zero 4                                                  |
| UIntInit(i)        | .long \<i>                                               |
| ULongInit(0)       | .zero 8                                                  |
| ULongInit(i)       | .quad \<i>                                               |
| **DoubleInit(d)**  | **.double \<d><br>OR<br>.quad \<d-interpreted-as-long>** |

#### Formatting Assembly Instructions

| Assembly instruction                 | Output                                                                                 |
| ------------------------------------ | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                     | mov \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                         |
| Movsx(src, dst)                      | movslq &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| **Cvtsi2sd(t, src, dst)**            | **cvtsi2sd\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                                |
| **Cvttsd2si(t, src, dst)**           | **cvttsd2si\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                               |
| Ret                                  | ret                                                                                    |
| Unary(unary_operator, t, operand)    | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst) | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| **Binary(Xor, Double, src, dst)**    | **xorpd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                                        |
| **Binary(Mult, Double, src, dst)**   | **mulsd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                                        |
| Idiv(t, operand)                     | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| Div(t, operand)                      | div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                             |
| Cdq(Longword)                        | cdq                                                                                    |
| Cdq(Quadword)                        | cdo                                                                                    |
| AllocateStack(int)                   | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Cmp(t, operand, operand)             | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| **Cmp(Double, operand, operand)**    | **comisd&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>**                               |
| Jmp(label)                           | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)              | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)            | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                         | .L\<label>:                                                                            |
| DeallocateStack(int)                 | addq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp                                              |
| Push(operand)                        | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                          | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |
| **Shr**           | **shr**          |
| **DivDouble**     | **div**          |
| **And**           | **and**          |
| **Or**            | **or**           |

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| -------------- | ------------------ |
| E              | e                  |
| NE             | ne                 |
| L              | l                  |
| LE             | le                 |
| G              | g                  |
| GE             | ge                 |
| B              | b                  |
| BE             | be                 |
| A              | a                  |
| AE             | ae                 |

#### Instruction Suffixes for Assembly Types

| Assembly Type | Instruction suffix |
| ------------- | ------------------ |
| Longword      | l                  |
| Quadword      | q                  |
| **Double**    | **sd**             |

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
| Reg(SP)          | %rsp        |
| **Reg(XMM0)**    | **%xmm0**   |
| **Reg(XMM1)**    | **%xmm1**   |
| **Reg(XMM2)**    | **%xmm2**   |
| **Reg(XMM3)**    | **%xmm3**   |
| **Reg(XMM4)**    | **%xmm4**   |
| **Reg(XMM5)**    | **%xmm5**   |
| **Reg(XMM6)**    | **%xmm6**   |
| **Reg(XMM7)**    | **%xmm7**   |
| **Reg(XMM8)**    | **%xmm8**   |
| **Reg(XMM9)**    | **%xmm9**   |
| **Reg(XMM10)**   | **%xmm10**  |
| **Reg(XMM11)**   | **%xmm11**  |
| **Reg(XMM12)**   | **%xmm12**  |
| **Reg(XMM13)**   | **%xmm13**  |
| **Reg(XMM14)**   | **%xmm14**  |
| **Reg(XMM15)**   | **%xmm15**  |
| Stack(int)       | <int>(%rbp) |
| Imm(int)         | $\<int>     |

## Extra Credit: NaN

As promised, quiet NaN is here for you. SSE instructions can help do the arithmetic operations without any extra effort from your side.
Type conversions from and to NaN is undefined, so we don't need to handle this.
We only care the comparisons with NaN. When comparing x to y, knowing that x is NaN, then all comparisons `x > y`, `x < y`, `x == y` are False.
Even `NaN == NaN` is False.

We call that an unordered result, which sets ZF, CF and PF to be 1.
You can utilize the `jp` instruction, which jumps when Parity Flag is 1, to handle NaN in floating-point comparisons.

## Summary

We've supported floating-point numbers, the first non-integer type we handle.
The floating-point numbers are meant to be imprecise, due to the limitation in hardware. But in reality, that's acceptable.
In the next chapter, we will continue our journey to explore `pointers`.

## Additional Resources

**IEEE 754**

- [The IEEE 754 standard](https://ieeexplore.ieee.org/document/8766229)
- [Double-Precision Floating-Point Format](https://en.wikipedia.org/wiki/Double-precision_floating-point_format)
- [What Every Computer Scientist Should Know About Floating-Point Arithmetic](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html)
- [The Floating-Point Guide](https://floating-point-gui.de/)

- To learn more about support for IEEE 754 standard in GCC and Clang, see the following resources:
  - [Semantics of Floating Point Math in GCC](https://gcc.gnu.org/wiki/FloatingPointMath)
  - [Controlling Floating-Point Behavior](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior)

**Reference for "Rounding Behavior" on page 299**

- [The Spacing of Binary Floating-Point Numbers](https://www.exploringbinary.com/the-spacing-of-binary-floating-point-numbers/)

**References for "Floating-Point Operations in Assembly" on page 310**

- [System V x64 ABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
- [Intel 64 Software Developer’s Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Sometimes Floating Point Math Is Perfect](https://randomascii.wordpress.com/2017/06/19/sometimes-floating-point-math-is-perfect/)
- [Pascal Cuoq has written an excellent answer to a Stack Overflow question about the assembly-level conversion from unsigned long to double](https://stackoverflow.com/a/26799227)
- [GCC Avoids Double Rounding Errors with Round-to-Odd](https://www.exploringbinary.com/gcc-avoids-double-rounding-errors-with-round-to-odd/)

**References for "Code Emission" on page 338**

- [Hexadecimal Floating-Point Constants](https://www.exploringbinary.com/hexadecimal-floating-point-constants/)
- [Number of Digits Required for Round-Trip Conversions](https://www.exploringbinary.com/number-of-digits-required-for-round-trip-conversions/)

**Floating-point visualization tools**

- [The Decimal to Floating-Point Converter](https://www.exploringbinary.com/floating-point-converter/)
- [Float Exposed](https://float.exposed/)

### Reference Implementation Analysis

[Chapter 13 Code Analysis](./code_analysis/chapter_13.md)
