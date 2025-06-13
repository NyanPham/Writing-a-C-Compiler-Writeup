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
  - [Chapter 13: Floating-Point Numbers](#chapter-13-floating-point-numbers)
  - [Chapter 14: Pointers](#chapter-14-pointers)
  - [Chapter 15: Arrays and Pointer Arithmetic](#chapter-15-arrays-and-pointer-arithmetic)
  - [Chapter 16: Characters and Strings](#chapter-16-characters-and-strings)
  - [Chapter 17: Supporting Dynamic Memory Allocation](#chapter-17-supporting-dynamic-memory-allocation)

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

---

# Chapter 14: Pointers

## Stages of a Compiler

1. **Lexer**
   - Input: Source code
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

In this chapter, we'll implement _pointer types_, which represents memory addresses.
We also have two new operators to work with pointers:

- Address operator &
- Dereference operator \*

To support pointers, we need to parse more complex type specifiers and handle new kinds of type errors.
In TACKY, we'll have some new constructs to read to and write from memory.

## Objects and Values

To put things simply:

- Value: a sequence of bits with a type.
- Object: a location in memory that contains a value.

So now, we can have a more precise definition of _lvalue_ since chapter 5: An _lvalue_ is an expression that potentially designates an object.
Note that an lvalue, despite its name, is not a value but an expression.

Evaluating a non-lvalue produces a value: 2, 3.5, etc
Evaluating an lvalue identifies the designated object.

In expression like `x + 1` (case 1), we're using its current value, but we don't in assignment like `x = 3` (case 2).
In case 1, _x_ is treated as the _actual_ value with a type, while in case 2, it's like a container that we can store a value of type int in.
In case 1, we call it _lvalue conversion_; that is we're not identifying the designated object, but producing its value.

For case 2 where _x_ is not lvalue converted, there are 2 places:

- Assignment = (including compound assignment)
- Address operator &

So, _x_ is an lvalue in the following cases:

```
x = 3;
y = &x;
```

and is a non-lvalue in:

```
foo(x);
x == y;
a = x;
```

## Operations on Pointers

```C
int main(void) {
	int x = 0;
	int *ptr = &x;
	*ptr = 4;
	return *ptr;
}
```

`int x = 0;`: x is an lvalue, to store the value 0
`int *ptr = &x;`: ptr is declared to store the int * (pointer to int), &x returns an address of x where an integer 0 is stored. So ptr now contains the address of x.
`*ptr = 4;`: ptr is evaluated to get the address of x, the preceding * means that we try to access the object x, so we treat *ptr as x. Thus, this is an expression that store 4 into x.
`return \*ptr;`: this also access the object x through dereferencing, but we're not assigning or applying the & operator, so we perform lvalue conversion to get its current value for return: 4.

## Null Pointers and Type Conversions

A pointer can hold 0, called a null pointer: `int *null = 0;`. 0 is implicitly converted to null pointer constant, a pointer type.

The following cases are invalid:

```C
// x is of type int, not a pointer.
int x = 0;
int *ptr = x;
```

```C
int *ptr1 = 3; // 3 is of type int.
int *ptr2 = 0x7ffeee67b938; // this is an integer, not a pointer either
```

Implicit conversions between pointers is not allowed:

```C
double *d = 0;
long *l = d;
```

However, explicit cast from a pointer type to another is OK:

```C
double negative_zero = -0.0;
double *d = &negative_zero;
unsigned long *l = (unsigned long *) d;
```

d and l both store the same memory address, intepreted as two different pointer types.
However, we we dereference the l, we get undefined behavior.
In other words, explicit cast from one pointer type to another is always legal, but using the result may not.

Explicit casts between pointer types and integers are allowed, but are not between double types and pointer types.

## Pointer Comparisons

- We can use == and != to compare pointers.
- Two non-null pointers are equal if the point to the same object.
- A non-null pointer is unequal to a null pointer.
- Two null pointers are equal.
- Pointers can be compared in any other constructs that compares to expression of zero: !, &&, ||, controlling expressions in if-statement, loop-statements.
- Null pointer is considered 0, 1 otherwise.

Note: We will support other relational operators like <, >, <=, >= in the next chapter for arrays.

## & Operations on Dereferenced Pointers

```C
int *ptr = &var;
int *ptr2 = &*ptr;
```

The &_ptr ends up to ptr, so this is not very useful.
In practice, when we see &_ as in this case, we'll omit them.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Ampersand | & |

We already support the \* token for multiplication. If the Extra credit in Chapter 3 is implemented, this & token is already recognized.

## The Parser

A declaration consists of 3 parts:

- a list of speicifier -> basic type
- a declarator -> can nest several declarator, but the innermost must be an identifier
- optional initializer or a function body.

Function and array declarators, which we treat as postfix expressions, have higher precedence than pointer declarator.
So

```C
int *arr[3];
```

is meant to declare:

```C
int *(arr[3]);
```

which is "an array of 3 pointers to int".

To support the casts to pointer types, we need new constructs to represent declartor, without the identifier.
We call it _abstract declartor_.
For example: `(int ***) exp;`

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
type = Int | Long | UInt | ULong | Double 
	| FunType(type* params, type ret)
	<strong>| Pointer(type referenced)</strong>
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
	<strong>| Dereference(exp)
	| AddrOf(exp)</strong>
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)</pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ <strong>&lt;declarator&gt;</strong> [ "=" &lt;exp&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ <strong>&lt;declarator&gt;</strong> "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
<strong>&lt;declarator&gt; ::= "*" &lt;declarator&gt; | &lt;direct-declarator&gt;
&lt;direct-declarator&gt; ::= &lt;simple-declarator&gt; [ &lt;param-list&gt; ]
&lt;param-list&gt; ::= "(" "void" ")" | "(" &lt;param&gt; { "," &lt;param&gt; } ")"
&lt;param&gt; ::= { &lt;type-specifier&gt; }+ &lt;declarator&gt;
&lt;simple-declarator&gt; ::= &lt;identifier&gt; | "(" &lt;declarator&gt; ")"</strong>
&lt;type-specifier&gt; ::= "int" | "long" | "unsigned" | "signed" | "double"
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
	| "(" { &lt;type-specifier&gt; }+ ")" <strong>[ &lt;abstract-declarator&gt; ]</strong> &lt;factor&gt; 
	| &lt;unop&gt; &lt;factor&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
<strong>&lt;abstract-declarator&gt; ::= "*" [ &lt;abstract-declarator&gt; ]
	| &lt;direct-abstract-declarator&gt;
&lt;direct-abstract-declarator&gt; ::= "(" &lt;abstract-declarator&gt; ")"</strong>
&lt;unop&gt; ::= "-" | "~" | "!" <strong>| "*" | "&"</strong>
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; | &lt;uint&gt; | &lt;ulong&gt; | &lt;double&gt;
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? An int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;long&gt; ::= ? An int or long token ?
&lt;uint&gt; ::= ? An unsigned int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?
&lt;double&gt; ::= ? A floating-point constant token ?
</pre></code>

### Parser

The grammar to parse derive types are from outside in, while the AST defines them as inside out.
To parse a declaration, we can't derive its type as we go. Instead, we'll first parse each declaration to a one-off representation that more closely mirrors the grammar.

We create an intermediate layer to mirror the grammar:

```
declarator = Ident(identifier)
	| PointerDeclarator(declarator)
	| FunDeclarator(param_info* params, declarator)

param_info = Param(type, declarator)
```

Then, we traverse the constructs and derive the information: declaration's type, identifier, and paramaters if any.

```
process_declarator(declarator, base_type):
	match declarator with
	| Ident(name) -> return (name, base_type, [])
	| PointerDeclarator(d) ->
		derived_type = Pointer(base_type)
		return process_declarator(d, derived_type)
	| FunDeclarator(params, d) ->
		match d with
		| Ident(name) ->
			param_names = []
			param_types = []
			for Param(p_base_type, p_declarator) in params:
				param_name, param_t, _ = process_declarator(p_declarator, p_base_type)
				if param_t is a function type:
					fail("Function pointers in parameters aren't supported")
				param_names.append(param_name)
				param_types.append(param_t)

			derived_type = FunType(param_types, base_type)
			return (name, derived_type, param_names)
		| _ -> fail("Can't apply additional type derivations to a function type")
```

To parse an entire declaration, we make changes on parse_declaration:

```
parse_declaration(tokens):
	specifiers = parse_specifier_list(tokens)
	base_type, storage_class = parse_type_and_storage_class(specifiers) // We do use the type now, dealing with function and variable declaration.
	declarator = parse_declarator(tokens)
	name, decl_type, params = process_declarator(declarator, base_type)
	if decl_type is a function type:
		<construct function_declaration>
	else:
		<construct variable_declaration>
```

Abstract declarators are handle similarly. Their intermediate constructs are:

```
abstract_declarator = AbstractPointer(abstract_declarator)
	| AbstractBase
```

The function process_abstract_declarator is omitted as it's similar to process_declarator, but simpler.

## Semantic Analysis

There are some new errors we need to handle:

1. Applying an operator to a type it doesn't support: multiply/divide pointers, dereference arithmetic values.
2. Operating on values of two incompatible types: implicit cast from or to pointers is not allowed, so we can't compare a pointer to a double for example.
3. Not using an lvalue where one is required: assignment, AddrOf.

The third error is currently handled during the Identifier Resolution pass. We'll move it to the Type Checker.

### Identifier Resolution

Remove the checking for _lvalue_.
Extend to traverse the new _Dereference_ and _AddrOf_ expressions.

### Type Checker

#### Dereference and AddrOf Expressions

A _Dereference_ expression must take an operand of pointer type.
An _AddrOff_ expression must take an _lvalue_ expression, which is Var or Dereference.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Dereference(inner) ->
		typed_inner = typecheck_exp(inner, symbols)
		match get_type(typed_inner) with
		| Pointer(referenced_t) ->
			deref_exp = Dereference(typed_inner)
			return set_type(deref_exp, referenced_t)
		| _ -> fail("Cannot dereference non-pointer")
	| AddrOf(inner) ->
		if inner is an lvalue:
			typed_inner = typecheck_exp(inner, symbols)
			referenced_t = get_type(typed_inner)
			addr_exp = AddrOf(typed_inner)
			return set_type(addr_exp, Pointer(referenced_t))
		else:
			fail("Can't take the address of a non-lvalue!")
```

#### Comparisons and Conditional Expressions

Remember that pointer types are not allowed to be implictly converted.
So while checking a binary expression of operator of Equal or Not Equal, if only one of the operand is pointer type, it fails.
Null pointer constants are the only exceptions, as we allow integer 0 to be come a null pointer.

```C
double *d = get_pointer();
return d == 0;
```

When we type check d == 0, we implicitly convert the 0 to the type `double *`.
We define a helper function to identify null pointer constants.

```
is_null_pointer_constant(e):
	match e with
	| Constant(c) ->
		match c with
		| ConstInt(0) -> return True
		| ConstUInt(0) -> return True
		| ConstLong(0) -> return True
		| ConstULong(0) -> return True
		| _ -> return False
	| _ -> return False
```

Another helper function is necessary to get the common pointer types:

```
get_common_pointer_type(e1, e2):
	e1_t = get_type(e1)
	e2_t = get_type(e2)
	if e1_t == e2_t:
		return e1_t
	else if is_null_pointer_constant(e1):
		return e2_t
	else if is_null_pointer_constant(e2):
		return e1_t
	else:
		fail("Expressions have incompatible types")
```

We have everything needed to typecheck Binary of Equal operator (NotEqual is the same)

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Binary(Equal, e1, e2) ->
		typed_e1 = typecheck_exp(e1, symbols)
		typed_e2 = typecheck_exp(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 or t2 is a pointer type:
			common_type = get_common_pointer_type(typed_e1, typed_e2)
		else:
			common_type = get_common_type(t1, t2)
		converted_e1 = convert_to(typed_e1, common_type)
		converted_e2 = convert_to(typed_e2, common_type)
		equality_exp = Binary(Equal, converted_e1, converted_e2)
		return set_type(equality_exp, Int)
```

_Conditional_ expressions are treated with the similar logic.
In `<cond> ? <clause1> : <clause2>`, we check if either _clause1_ and _clause2_ is a pointer.
If one is, we use get*common_pointer_type. Otherwise, we call get_common_type.
The \_cond* can be either a pointer or an arithmetic value, since we compare it to zero either way.

#### Assignment and Conversions as if by Assignment

To handle assignment expressions, we first validate the left-hand side is an lvalue.
Then we convert the right-hand side to the type of the left, or fail if the conversion is illegal.
This pattern is called "as if by assignment", and turns up in a few places, not just in assignment expressions.
It's best to create a helper function for this conversion

```
convert_by_assignment(e, target_type):
	if get_type(e) == target_type:
		return e
	if get_type(e) is arithmetic and target_type is arithmetic:
		return convert_to(e, target_type)
	if is_null_pointer_constant(e) and target_type is a pointer type:
		return convert_to(e, target_type)
	else:
		fail("Cannot convert type for assignment")
```

So where do we use this beside assignment?
Well, we have FunctionCall where we need to convert each argument to the type of the corresponding paramaeter.
We also have variable initializers, which is treated similar to assignment.
Lastly, return statement also needs to convert the expression being returned to the return type of the enclosing function.

#### Other Expressions

- **Cast**: Any explicit cast between double and pointer is illegal. Otherwise, handle the cast to/from pointers normally.
- **Unary**: _Negate_ and _Complement_ on pointers are weird, as negating or bitwise-complemnting a memory address doesn't make sense. _Not_ is ok, as we can compare pointer to 0.
- **Binary**:
  - _And_ and _Or_, similarly to _Not_, don't convert operands to a common type, so they can freely operate between pointer and arithmetic types.
  - _Multiply_, _Divide_ and _Remainder_ don't accept pointers.
  - _Add_ and _Subtract_ work on pointers.
  - Other pointer comparisons are valid, but we won't implement them in this chapter: GreaterThan, LessThan, GreaterOrEqual, and LessOrEqual

#### Tracking Static Pointer Initializers in the Symbol Table

Static variables of pointer type, can be initialized to null pointers.

```C
static int *ptr = 0;
```

We need a way to represent a null pointer as a static_init in the symbol table.
A pointer is an unsigned 64-bit integer, so we'll reuse the ULongInit(0) construct.

## TACKY Generation

We'll add three new instructions for TACKY.

- _GetAddress_: corresponding to AST's _AddrOf_.
- _Load_: corresponding to AST's _Dereference_ without lvalue conversion.
- _Store_: corresponding to AST's _Dereference_ with lvalue conversion.

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, static_init init)
instruction = Return(val) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	| ZeroExtend(val src, val dst)
	| DoubleToInt(val src, val dst)
	| DoubleToUInt(val src, val dst)
	| IntToDouble(val src, val dst)
	| UIntToDouble(val src, val dst)
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	<strong>| GetAddress(val src, val dst)
	| Load(val src_ptr, val dst)
	| Store(val src, val dst_ptr)</strong>
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

#### Pointer Operations in TACKY

We use _Load_ instruction to dereference a pointer and then lvalue convert the result.

For `*<exp>`:

```
<instructions for exp>
ptr = <result of exp>
result = Load(ptr)
```

We use _Store_ to assign to a dereferenced pointer instead of lvalue converting it.

For `*<left> = <right>`:

```
<instruction for left>
ptr = <result of left>
<instruction for right>
result = <result of right>
Store(result, ptr)
```

We use _GetAddress_ for _AddrOf_ expression.

For `&var`:

```
result = GetAddress(var)
```

Whenever we see an expression of form `&*<exp>`, we translate only the inner `<exp>` to TACKY.

#### A Strategy for TACKY Conversion

Our _emit_tacky_ will no longer return operand, but a new intermediate construct called `exp_result`,
which represent an expression result that hasn't been lvalue converted.

The new function _emit_tacky_and_convert_ will call _emit_tacky_, and lvalue convert the result if it's an lvalue, then return a TACKY operand.
For expressions that shouldn't be lvalue converted (e.g. left-hand side of assignment), we call the _emit_tacky_ directly.

```
exp_result = PlainOperand(val) | DereferencedPointer(val)
```

A _DereferencedPointer_ takes a single argument: a TACKY operand of pointer type.
A _PlainOperand_ is an ordinary constant or var.

Now, let's update our functions.

```
// All recursive calls of emit_tacky are replaced with emit_tacky_and_convert,
// except the left-hand side of an assignment.
// Wrap all returned TACKY operands in PlainOperand
emit_tacky(e, instructions, symbols):
	match e with
	| --snip--
	| Unary(op, inner) ->
		src = emit_tacky_and_convert(inner, instructions, symbols)
		dst = make_tacky_variable(get_type(e), symbols)
		tacky_op = convert_unop(op)
		instructions.append(Unary(tacky_op, src, dst))
		return PlainOperand(dst)
	| --snip-- // other expressions are changed the same way
	| Assignment(left, right) ->
		lval = emit_tacky(left, instructions, symbols)
		rval = emit_tacky_and_convert(right, instructions, symbols)
		match lval with
		| PlainOperand(obj) ->
			instructions.append(Copy(rval, obj))
			return lval
		| DereferencedPointer(ptr) ->
			instructions.append(Store(rval, ptr))
			return PlainOperand(rval)
	| --snip--
	| Dereference(inner) ->
		result = emit_tacky_and_convert(inner, instructions, symbols)
		return DereferencedPointer(result)
	| AddrOf(inner) ->
		v = emit_tacky_and_convert(inner, instructions, symbols)
		match v with
		| PlainOperand(obj) ->
			dst = make_tacky_variable(get_type(e), symbols)
			instructions.append(GetAddress(obj, dst))
			return PlainOperand(dst)
		| DereferencedPointer(ptr) ->
			return PlainOperand(ptr)
```

```
emit_tacky_and_convert(e, instructions, symbols):
	result = emit_tacky(e, instructions, symbols)
	match result with
	| PlainOperand(val) -> return val
	| DereferencedPointer(ptr) ->
		dst = make_tacky_variable(get_type(e), symbols)
		instructions.append(Load(ptr, dst))
		return dst
```

A full expression is an expression that is not a part of another expression, such as the controlling expressions in loops, and if statements.
Full expressions always undergo lvalue conversion. So make sure you use emit_tacky_and_convert on them, even though some time the results are not used (unless you want optimization to save unnecessary Load instructions).

## Assembly Generation

### Assembly

<pre><code>program = Program(top_level*)
assembly_type = LongWord | Quadword | Double
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, static_init init)
	| StaticConstant(identifier name, int alignment, static_init init)
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(operand src, operand dst)
		| MovZeroExtend(operand src, operand dst)
		<strong>| Lea(operand src, operand dst)</strong>
		| Cvttsd2si(assembly_type dst_type, operand src, operand dst)
		| Cvtsi2sd(assembly_type src_type, operand src, operand dst)
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
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | <strong>Memory(reg, int)</strong> | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP <strong>| BP</strong>
	| XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15</pre></code>

### Converting TACKY to Assembly

We treat pointer types exactly like unsigned long.
We convert pointer types to the Quadword assembly type,
compare pointers with `cmp` instruction,
pass return values of pointer type in RAX
and pass parameters of pointer type in the same general-purpose regisgers as integer parameters.

Also, we change everywhere we used `Stack(<i>)` into `Memory(BP, <i>)`.

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| -------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs + \<all StaticConstant constructs for floating-point constants>)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Function(name, global, params, instructions) | Function(name, global, [<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first int param type>, Reg(DI), \<first int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second int param type>, Reg(SI), \<second int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four integer parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first double param type>, Reg(XMM0), \<first double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second double param type>, Reg(XMM1), \<second double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next six double parameters from registers><br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first stack param type>, **Memory(BP, 16)**, \<first stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second stack param type>, **Memory(BP, 24)**, \<second stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, init)        | StaticVariable(name, global, \<alignment of t, init)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                             | Assembly instructions                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| --------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val) (Integer)                         | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Return(val) (Double)                          | Mov(\<Double>, val, Reg(XMM0))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Unary(Not, src, dst) (Integer)                | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                                                                |
| Unary(Not, src, dst) (Double)                 | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, src, Reg(\<X>))<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                     |
| Unary(Negate, src, dst) (Double negation)     | Mov(Double, src, dst)<br>Binary(Xor, Double, Data(\<negative-zero>), dst)<br>And add a top-level constant:<br>StaticConstant(\<negative-zero>, 16, DoubleInit(-0.0))                                                                                                                                                                                                                                                                                                                           |
| Unary(unary_operator, src, dst)               | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Binary(Divide, src1, src2, dst) (Signed)      | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Divide, src1, src2, dst) (Unsigned)    | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Signed)   | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Unsigned) | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(arithmetic_operator, src1, src2, dst)  | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(relational_operator, src1, src2, dst)  | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                                                                                                                                                                                                                                                                                                                                                                              |
| Jump(target)                                  | Jmp(target)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| JumpIfZero(condition, target) (Integer)       | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| JumpIfZero(condition, target) (Double)        | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                             |
| JumpIfNotZero(condition, target)              | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| JumpIfNotZero(condition, target) (Double)     | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                            |
| Copy(src, dst)                                | Mov(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Label(identifier)                             | Label(identifier)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| FunCall(fun_name, args, dst)                  | \<fix stack alignment><br>\<move arguments to general-purpose registers><br>\<move arguments to XMM registers><br>\<push arguments onto the stack><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, \<dst register>, dst)                                                                                                                                                                                                                                             |
| SignExtend(src, dst)                          | Movsx(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Truncate(src, dst)                            | Mov(Longword, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ZeroExtend(src, dst)                          | MovZeroExtend(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| IntToDouble(src, dst)                         | Cvtsi2sd(<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| DoubleToInt(src, dst)                         | Cvttsd2si(<dst type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| UIntToDouble(src, dst) (unsigned int)         | MovZeroExtend(src, Reg(\<R>))<br>Cvtsi2s(Quadword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                             |
| UIntToDouble(src, dst) (unsigned long)        | Cmp(Quadword, Imm(0), src)<br>JmpCC(L, \<label1>)<br>Cvtsi2sd(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Quadword, src, Reg(\<R1>))<br>Mov(Quadword, Reg(\<R1>), Reg(\<R2>))<br>Unary(Shr, Quadword, Reg(\<R2>))<br>Binary(And, Quadword, Imm(1), Reg(\<R1>))<br>Binary(Or, Quadword, Reg(\<R1>), Reg(\<R2>))<br>Cvtsi2sd(Quadword, Reg(\<R2>), dst)<br>Binary(Add, Double, dst, dst)<br>Label(\<label2>)                                                                |
| DoubleToUInt(src, dst) (unsigned int)         | Cvttsd2si(Quadword, src, Reg(\<R>))<br>Mov(Longword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                           |
| DoubleToUInt(src, dst) (unsigned long)        | Cmp(Double, Data(\<upper-bound>), src)<br>JmpCC(AE, \<label1>)<br>Cvttsd2si(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Double, src, Reg(\<X>))<br>Binary(Sub, Double, Data(\<upper-bound>),Reg(\<X>))<br>Cvttsd2si(Quadword, Reg(\<X>), dst)<br>Mov(Quadword, Imm(9223372036854775808), Reg(\<R>))<br>Binary(Add, Quadword, Reg(\<R>), dst)<br>Label(\<label2>)<br>And add a top-level constant:<br>StaticConstant(\<upper-bound>, 8, DoubleInit(9223372036854775808.0)) |
| **Load(ptr, dst)**                            | **Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<dst type>, Memory(\<R>, 0), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **Store(src, ptr)**                           | **Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<src type>, src, Memory(\<R>, 0))**                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **GetAddress(src, dst)**                      | **Lea(src, dst)**                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator           | Assembly operator |
| ------------------------ | ----------------- |
| Complement               | Not               |
| Negate                   | Neg               |
| Add                      | Add               |
| Subtract                 | Sub               |
| Multiply                 | Mult              |
| Divide (double division) | DivDouble         |

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

| TACKY operand                 | Assembly operand                                                                                 |
| ----------------------------- | ------------------------------------------------------------------------------------------------ |
| Constant(ConstInt(int))       | Imm(int)                                                                                         |
| Constant(ConstUInt(int))      | Imm(int)                                                                                         |
| Constant(ConstLong(int))      | Imm(int)                                                                                         |
| Constant(ConstULong(int))     | Imm(int)                                                                                         |
| Constant(ConstDouble(double)) | Data(\<ident>)<br>And add top-level constant:<br>StaticConstant(\<ident>, 8, DoubleInit(Double)) |
| Var(identifier)               | Pseudo(identifier)                                                                               |

#### Converting Types to Assembly

| Source type               | Assembly type | Alignment |
| ------------------------- | ------------- | --------- |
| Int                       | Longword      | 4         |
| UInt                      | Longword      | 4         |
| Long                      | Quadword      | 8         |
| ULong                     | Quadword      | 8         |
| Double                    | Double        | 8         |
| **Pointer(referenced_t)** | **Quadword**  | **8**     |

### Replacing Pseudoregisters

We'll use the new Memory operand instead of Stack.
Also, we extend this pass to replace the operand in the _Lea_ instruction.

### Fixing Up Instructions

The destination of _Lea_ must be a register.
We'll add some rewrite rules for _Push_.

This invalid instruction

```
pushq	%XMM0
```

is rewritten into

```
subq	$8, %rsp
movsd	%xmm0, (%rsp)
```

This new rule will take effect in Chapter III.

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                                   | Output                                                                                                                                                                                                                                                                                                           |
| ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                                            | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                                           | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, init) (Initialized to zero)                            | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                      |
| StaticVariable(name, global, alignment, init) (Initialized to nonzero value **OR any double**) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                     |
| StaticConstant(name, alignment, init) (Linux)                                                  | &nbsp;&nbsp;&nbsp;&nbsp;.section .rodata<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                                                         |
| StaticConstant(name, alignment, init) (MacOS 8-byte aligned constants)                         | &nbsp;&nbsp;&nbsp;&nbsp;.literal8<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 8<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                                                                             |
| StaticConstant(name, alignment, init) (MacOS 16-byte aligned constants)                        | &nbsp;&nbsp;&nbsp;&nbsp;.literal16<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 16<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init><br>&nbsp;&nbsp;&nbsp;&nbsp;.quad 0                                                                                                                                                        |
| Global directive                                                                               | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                                            | For Linux only: .align <alignment><br>For macOS and Linux: .balign <alignment>                                                                                                                                                                                                                                   |

#### Formatting Static Initializers

| Static Initializer | Output                                               |
| ------------------ | ---------------------------------------------------- |
| IntInit(0)         | .zero 4                                              |
| IntInit(i)         | .long \<i>                                           |
| LongInit(0)        | .zero 8                                              |
| LongInit(i)        | .quad \<i>                                           |
| UIntInit(0)        | .zero 4                                              |
| UIntInit(i)        | .long \<i>                                           |
| ULongInit(0)       | .zero 8                                              |
| ULongInit(i)       | .quad \<i>                                           |
| DoubleInit(d)      | .double \<d><br>OR<br>.quad \<d-interpreted-as-long> |

#### Formatting Assembly Instructions

| Assembly instruction                 | Output                                                                                 |
| ------------------------------------ | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                     | mov \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                         |
| Movsx(src, dst)                      | movslq &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| Cvtsi2sd(t, src, dst)                | cvtsi2sd\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                    |
| Cvttsd2si(t, src, dst)               | cvttsd2si\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                   |
| Ret                                  | ret                                                                                    |
| Unary(unary_operator, t, operand)    | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst) | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| Binary(Xor, Double, src, dst)        | xorpd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Binary(Mult, Double, src, dst)       | mulsd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Idiv(t, operand)                     | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| Div(t, operand)                      | div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                             |
| Cdq(Longword)                        | cdq                                                                                    |
| Cdq(Quadword)                        | cdo                                                                                    |
| Cmp(t, operand, operand)             | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| Cmp(Double, operand, operand)        | comisd&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                   |
| Jmp(label)                           | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)              | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)            | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                         | .L\<label>:                                                                            |
| Push(operand)                        | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                          | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |
| **Lea(src, dst)**                    | **leaq&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                                         |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |
| Shr               | shr              |
| DivDouble         | div              |
| And               | and              |
| Or                | or               |

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
| Double        | sd                 |

#### Formatting Assembly Operands

| Assembly operand     | Output          |
| -------------------- | --------------- |
| Reg(AX) 8-byte       | %rax            |
| Reg(AX) 4-byte       | %eax            |
| Reg(AX) 1-byte       | %al             |
| Reg(DX) 8-byte       | %rdx            |
| Reg(DX) 4-byte       | %edx            |
| Reg(DX) 1-byte       | %dl             |
| Reg(CX) 8-byte       | %rcx            |
| Reg(CX) 4-byte       | %ecx            |
| Reg(CX) 1-byte       | %cl             |
| Reg(DI) 8-byte       | %rdi            |
| Reg(DI) 4-byte       | %edi            |
| Reg(DI) 1-byte       | %dil            |
| Reg(SI) 8-byte       | %rsi            |
| Reg(SI) 4-byte       | %esi            |
| Reg(SI) 1-byte       | %sil            |
| Reg(R8) 8-byte       | %r8             |
| Reg(R8) 4-byte       | %r8d            |
| Reg(R8) 1-byte       | %r8b            |
| Reg(R9) 8-byte       | %r9             |
| Reg(R9) 4-byte       | %r9d            |
| Reg(R9) 1-byte       | %r9b            |
| Reg(R10) 8-byte      | %r10            |
| Reg(R10) 4-byte      | %r10d           |
| Reg(R10) 1-byte      | %r10b           |
| Reg(R11) 8-byte      | %r11            |
| Reg(R11) 4-byte      | %r11d           |
| Reg(R11) 1-byte      | %r11b           |
| Reg(SP)              | %rsp            |
| **Reg(BP)**          | **%rbp**        |
| Reg(XMM0)            | %xmm0           |
| Reg(XMM1)            | %xmm1           |
| Reg(XMM2)            | %xmm2           |
| Reg(XMM3)            | %xmm3           |
| Reg(XMM4)            | %xmm4           |
| Reg(XMM5)            | %xmm5           |
| Reg(XMM6)            | %xmm6           |
| Reg(XMM7)            | %xmm7           |
| Reg(XMM8)            | %xmm8           |
| Reg(XMM9)            | %xmm9           |
| Reg(XMM10)           | %xmm10          |
| Reg(XMM11)           | %xmm11          |
| Reg(XMM12)           | %xmm12          |
| Reg(XMM13)           | %xmm13          |
| Reg(XMM14)           | %xmm14          |
| Reg(XMM15)           | %xmm15          |
| **Memory(reg, int)** | **\<int>(reg)** |
| Imm(int)             | $\<int>         |

## Summary

With pointers, we've learned to parse and process complex types.
In the next chapter, we'll tackle arrays, our first non-scalar types.

### Reference Implementation Analysis

[Chapter 14 Code Analysis](./code_analysis/chapter_14.md)

---

# Chapter 15: Arrays and Pointer Arithmetic

## Stages of a Compiler

1. **Lexer**
   - Input: Source code
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

We will implement:

- Compound Initializers
- Subscript Operators
- Pointer Arithmetic

In this chapter, the type checker will handle the implicit conversions from arrays to pointers and annotate the AST with the type information you'll rely on to perform pointer arithmetic.

### Arrays and Pointer Arithmetic

We already knew 2 categories of object type:

- Arithmetic
- Non-arithmetic

Now, we will have another layer of distinction:

- Scalar type: represents a single value
  - Arithmetic types
  - Pointer
- Aggregate type: respresents a collection of values
  - Array
  - Structure (in Chapter 18)

### Array Declarations and Initializers

We interpret a declaration by starting with the basic type, then applying the type derivations from the outside in.

Some examples:

```C
int int_array[3]; // an array of 3 int
double *(ptr_array[5]); // an array of 5 pointers to double
long nested_array[3][2]; // array of 3 arrays of 2 long, this is a multidimensional array
```

We can initialize an array with _compound initializers_.

```C
int int_array[3] = {1, foo(), a*4};
```

IF an array is leaved uninitialized and it has

- Automatic storage duration, the array has undefined initial value.
- Static storage duration, the array is initialized to all zeros; this is similar to _scalar types_.

### Memory Layout of Arrays

```C
// An array of scalar types
int six_ints[6] = { 1, 2, 3, 4, 5, 6 };

// A 2d array
int three_arrays[3][2] = {{1, 2}, {3, 4}, {5, 6}}
```

Both of the definitions of the both arrays have the same layout in memory:

```
					+-----------------------------------------------+
Memory content:		|	1	|	2	|	3	|	4	|	5	|	6	|
					+-----------------------------------------------+
Memory address:		|  0x10	|  0x14	|  0x18	|  0x1c	| 0x20	| 0x24	|
					+-----------------------------------------------+
```

How we access the elements, however, is different on each array.

### Array-to-Pointer Decay

There are only 2 valid operations on objects of array type:

- Get the array size using _sizeof_ operator (Chapter 17)
- Get the address with the & operator

```C
int my_array[3] = {1, 2, 3};
int (*my_pointer)[3] = &my_array;
```

In order to make arrays more useful like getting, updating values in an array, we turn them into pointers.
With pointers, we can do all kinds of arithmetic operations.
This implicit conversion from arrays to pointers is called _array-to-pointer decay_.

```C
int my_array[3] = {1, 2, 3};
int *my_pointer = my_array;
return *my_pointer;
```

### Pointer Arithmetic to Access Array Elements

```C
int array[3] = { 1, 2, 3 };
int *ptr = array + 1;			// array is decayed to pointer, pointing to the first element in the array,
								// add 1, which is then multiplied by the size of int (4) to point to the second element: 2
								// store the pointer to ptr.
```

**Note**:
There is a special case: if x is an n-element array, x + n points one past the end of x.
It's useful to test whether you've reached the end of the array.

### Array Types in Function Declarations

Functions do not accept arguments of array type. According to the C standard, our compiler will adjust the function to take pointers instead.

### Things We Aren't Implementing

We only allow constants for the size of array declarations.

```C
int x = 3;
int arr1[3]; // OK
int arr2[x]; // this is rejected by our compiler
```

We require array dimensions in declarations as well as in definitions.
We won't implement _compound literals_

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| Open square | [ |
| Close square | ] |

## The Parser

We'll add array types, subscript expressions, and compound initializers to the AST.

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, <strong>initializer? init</strong>, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
<strong>initializer = SingleInit(exp) | CompoundInit(initializer*)</strong>
type = Int | Long | UInt | ULong | Double 
	| FunType(type* params, type ret)
	| Pointer(type referenced)
	<strong>| Array(type element, int size)</strong>
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
	| Dereference(exp)
	| AddrOf(exp)
	<strong>| Subscript(exp, exp)</strong>
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)</pre></code>

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; [ "=" <strong>&lt;initializer&gt;</strong> ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;declarator&gt; ::= "*" &lt;declarator&gt; | &lt;direct-declarator&gt;
&lt;direct-declarator&gt; ::= &lt;simple-declarator&gt; <strong>[ &lt;declarator-suffix&gt; ]</strong>
<strong>&lt;declarator-suffix&gt; ::= &lt;param-list&gt; | { "[" &lt;const&gt; "]" }+ </strong>
&lt;param-list&gt; ::= "(" "void" ")" | "(" &lt;param&gt; { "," &lt;param&gt; } ")"
&lt;param&gt; ::= { &lt;type-specifier&gt; }+ &lt;declarator&gt;
&lt;simple-declarator&gt; ::= &lt;identifier&gt; | "(" &lt;declarator&gt; ")"
&lt;type-specifier&gt; ::= "int" | "long" | "unsigned" | "signed" | "double"
&lt;specifier&gt; ::= &lt;type-specifier&gt; | "static" | "extern"
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
<strong>&lt;initializer&gt; ::= &lt;exp&gt; | "{" &lt;initializer&gt; { "," &lt;initializer&gt; } [ "," ] "}"</strong>
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
&lt;exp&gt; ::= <strong>&lt;unary-exp&gt;</strong> | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
<strong>&lt;unary-exp&gt; ::= &lt;unop&gt; &lt;unary-exp&gt;
	| "(" { &lt;type-specifier&gt; }+ ")" [ &lt;abstract-declarator&gt; ] &lt;unary-exp&gt;
	| &lt;postfix-exp&gt;</strong>
<strong>&lt;postfix-exp&gt; ::= &lt;primary-exp&gt; { "[" &lt;exp&gt; "]" }</strong>
<strong>&lt;primary-exp&gt; ::= &lt;const&gt; | &lt;identifier&gt; | "(" &lt;exp&gt; ")"
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"</strong>
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;abstract-declarator&gt; ::= "*" [ &lt;abstract-declarator&gt; ]
	| &lt;direct-abstract-declarator&gt;
&lt;direct-abstract-declarator&gt; ::= "(" &lt;abstract-declarator&gt; ")" <strong>{ "[" &lt;const&gt; "]" }
	| { "[" &lt;const&gt; "]" }+</strong>
&lt;unop&gt; ::= "-" | "~" | "!" | "*" | "&"
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; | &lt;uint&gt; | &lt;ulong&gt; | &lt;double&gt;
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? An int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;long&gt; ::= ? An int or long token ?
&lt;uint&gt; ::= ? An unsigned int token ?&lt;/pre&gt;&lt;/code&gt;
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?
&lt;double&gt; ::= ? A floating-point constant token ?
</pre></code>

### Parser

Let's walk through how to parse each of these additions to the AST.

We update the declarator to have ArrayDeclarator

```
declarator = Ident(identifier)
	| PointerDeclarator(declarator)
	| ArrayDeclarator(declarator, int size)
	| FunDeclarator(param_info* params, declarator)
```

When you parse a \<declarator-suffix>, reject floating point constants and accept constants of any integer type.
Array dimension must be greater than zero.

```
process_declarator(declarator, base_type):
	match declarator with
	| --snip--
	| ArrayDeclarator(inner, size) ->
		derived_type = Array(base_type, size)
		return process_declarator(inner, derived_type)
```

We take the same approach for abstract declarators

```
abstract_declarator = AbstractPointer(abstract_declarator)
	| AbstractArray(abstract_declarator, int size)
	| AbstractBase
```

You can update the process_abstract_declarator accordingly.

## Semantic Analysis

### Identifier Resolution

_No changes_

### Type Checker

The type checker will handle the heavy lifting in this chapter. It will:

- Add type information to subscript and pointer arithmetic expressions
- Validate the dimensions of compound initializers
- Detect type errors: casting an expression to an array type.
- Handle implicit conversions from array to pointer types.

We wrap an expression with a Cast expression to make the implicit type conversion explicit.
For Array type, we wrap the array with AddrOf expressions.

#### Converting Arrays to Pointers

Similar to our previous approach where we use emit_tacky_and_convert in TACKY to manage lvalue conversions,
we define a new typecheck_and_convert function to convert arrays to pointers.

```
typecheck_and_convert(e, symbols):
	typed_e = typecheck_exp(e, symbols)
	match get_type(typed_e)
	| Array(elem_t, size) ->
		addr_of = AddrOf(typed_e)
		return set_type(addr_of, Pointer(elem_t))
	| -> return typed_e
```

Note that the implicit conversions return produce a different result type on the AddrOf than explicit & operator on arrays.

```C
int arr[3];
&arr;			// has type int(*)[3]
arr;			// has type int*
```

We'll use this typecheck_and_convert in place of typecheck_exp to check both subexpressions and full expressions.
One exception is that we don't convert array to pointer if the expression being checked is the operand of an AddrOf, so we'll use typecheck_exp directly.

#### Validating Lvalues

We'll change some details:

- Subscript expressions are lvalues, besides Var and Dereference
- Reject assignment expressions that assign to arrays.

Once an array decays to a pointer, it's no longer an lvalue and can't be assigned to.

We'll process the left operand with typecheck_and_convert before we check whether it's a lvalue.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Assignment(left, right) ->
		typed_left = typecheck_and_convert(left, symbols)
		if typed_left is not an lvalue:
			fail("Tried to assign to non-lvalue")
		typed_right = typecheck_and_convert(right, symbols)
		--snip--
```

How does this work? If the left operand is an array, the typecheck_and_convert will wrap it in an AddrOf operation.
Since AddrOf is not an lvalue, the error is raised.

#### Type Checking Pointer Arithmetic

We'll extend the operations of addition, subtraction, and relational operators to work with pointers.
Any integer type is compatible with pointers in these operations.

```
	| Binary(Add, e1, e2) ->
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 and t2 are arithmetic:
			--snip--
		else if t1 is a pointer type and t2 is an integer type:
			converted_e2 = convert_to(typed_e2, Long)
			add_exp = Binary(Add, typed_e1, converted_e2)
			return set_type(add_exp, t1)
		else if t2 is a pointer type and t1 is an integer type:
			--snip--
		else:
			fail("Invalid operands for addition")
```

There are some notes for the conversions of the integer type to Long to support arithmetic with pointers:

- This simplifies later compiler passes as pointer indices will need to be 8 bytes wide to add to 8-byte memory addresses.
- This doesn't validate any C standard, as converting a valid array index to long won't change its value
- If an integer is too big to represent as a long, we can safely assume that it's not a valid array index because no hardware supports arrays with anywhere close to 2<sup>63</sup> elements.

Subtracting an integer from pointer works the same way: we convert integer operand to a long and annotate the result with the same type as the pointer operand.
But for subtraction, operand order matters. We can subtract an integer from a pointer, but can't subtract a pointer from an integer.

Subtracting two pointers require they both have the same type, and the result type is a signed integer type.

```
	| Binary(Subtract, e1, e2) ->
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 and t2 are arithmetic
			--snip--
		else if t1 is a pointer and t2 is an integer type:
			converted_e2 = convert_to(typed_e2, Long)
			sub_exp = Binary(Subtract, typed_e1m, converted_e2)
			return set_type(sub_exp, t1)
		else if  t1 is a pointer and t1 == t2:
			sub_exp = Binary(Subtract, typed_e1, typed_e2)
			return set_type(sub_exp, Long)
		else:
			fail("Invalid operands for subtraction")
```

Now for relational operators <, <=, >, >=.
Each of these accepts two pointers of the same type and returns an int.
None of these operators accept null pointer constants. Why? We use them to compare pointers to elements in the same array, but a null pointer doesn't point to an array element at all.
However, == and != accept null pointer constants: x == 0, x != 0.
These rules are simple so no pseudocode is provided.

#### Type Checking Subscript Expressions

One operand of an Subscript must be a pointer, and the other must be an integer type.
The result type is the referecenced type of the pointer operand.

```
	| Subscript(e1, e2) ->
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 is a pointer type and t2 is an integer type:
			ptr_type = t1
			typed_e2 = convert_to(typed_e2, Long)
		else if t1 is an integer type and t2 is a pointer type:
			ptr_type = t2
			typed_e1 = convert_to(typed_e1, Long)
		else:
			fail("Subscript must have integer and pointer operands")
		subscript_exp = Subscript(typed_e1, typed_e2)
		return set_type(subscript_exp, ptr_type.referenced)
```

#### Type Checking Cast Expressions

The rule is simple: casting an expression to an array type is invalid.

```C
(int[3]) foo; // Invalid casting to array type
```

#### Type Checking Function Declarations

We extend to check 2 things:

- Return type: if it's an array type, throw error
- Argument type: if an argument is an array type, decay it to pointer.

```
typecheck_function_declaration(decl, symbols):
	if decl.fun_type.ret is an array type:
		fail("A function cannot return an array")
	adjusted_params = []
	for t in decl.fun_type.params:
		match t with:
		| Array(elem_t, size) ->
			adjusted_type = Pointer(elem_t)
			adjusted_params.append(adjusted_type)
		| _ -> adjusted_params.append(t)
	decl.fun_type.params = adjusted_params
	--snip--
```

We add this logic before resolving the conflicts with previous declarations.
The symbol table and the AST node should use the adjusted params as well.

#### Type Checking Compound Initializers

```
typecheck_init(target_type, init, symbols):
	match target_type, init with
	| _, SingleInit(e) ->
		typechecked_exp = typecheck_and_convert(e, symbols)
		cast_exp = convert_by_assignment(typechecked_exp, target_type)
		return set_type(SingleInit(cast_exp), target_type)
	| Array(elem_t, size), CompoundInit(init_list) ->
		if length(init_list) > size:
			fail("wrong number of values in initializer")
		typechecked_list = []
		for init_elem in init_list:
			typechecked_elem = typecheck_init(elem_t, init_elem, symbols)
			typechecked_list.append(typechecked_elem)

		while length(typechecked_list) < size:
			typecheck_list.append(zero_initializer(elem_t))
		return set_type(CompondInit(typechecked_list), target_type)
	| _ -> fail("can't initialize a scalar object with a compound initializer")
```

The helper function zero_initializer should return a SingleInit with the value of 0 if a scalar type is given.
If given an array type, it should return a CompountInit whose scalar elements (which may be nested several layers deep) have the value 0.

For example:
Calling zero_initializer on the type UInt gives: `SingleInit(Constant(ConstInt(0)))`
Calling it on Array(Array(Int, 2), 2):

```
CompoundInit([
	CompoundInit([SingleInit(Constant(ConstInt(0))), SingleInit(Constant(ConstInt(0)))]),
	CompoundInit([SingleInit(Constant(ConstInt(0))), SingleInit(Constant(ConstInt(0)))])
])
```

#### Initializing Static Arrays

We need to update how we represent arrays in our symbol table.

Initial construct now holds a list of static_init, not a single static_init as previous chapters.

```
initial_value = Tentative | Initial(static_init* init_lists) | NoInitializer
```

Examples:

```C
static int a = 3
```

becomes

```
Initial([IntInit(3)])
```

```C
static int nested[3][2] = {{1,2}, {3,4}, {5,6}}
```

becomse

```
Initial([IntInit(1), IntInit(2), IntInit(3), IntInit(4), IntInit(5), IntInit(6)])
```

Next, static_init should also have abibility to represent zeroed-out objects.

```
static_init = InitInit(int) | LongInit(int) | UIntInit(int) | ULongInit(int)
	| DoubleInit(double)
	| ZeroInit(int bytes)
```

Examples:

```C
static int nested[3][2] = {{100}, {200, 300}};
```

becomes

```
Initial([
	IntInit(100),
	ZeroInit(4),
	IntInit(200),
	IntInit(300),
	ZeroInit(8)
])
```

Before converting a compound initializer to static_init list, we need to validate that the initializers for static arrays have the correct size and structure.

#### Initializng Scalar Variables with ZeroInit

ZeroInit can be used to initialize scalar variables to zero as well.

```C
static long x = 0;
```

is meant to be this initializer:

```
Initial([ZeroInit(8)])
```

## TACKY Generation

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, <strong>static_init* init_list</strong>)
instruction = Return(val) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	| ZeroExtend(val src, val dst)
	| DoubleToInt(val src, val dst)
	| DoubleToUInt(val src, val dst)
	| IntToDouble(val src, val dst)
	| UIntToDouble(val src, val dst)
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| GetAddress(val src, val dst)
	| Load(val src_ptr, val dst)
	| Store(val src, val dst_ptr)
	<strong>| Addptr(val src, val index, int scale, val dst)
	| CopyToOffset(val src, identifier dst, int offset)</strong>
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

#### Pointer Arithmetic

We will implement the expression `<ptr> + <int>` with an AddPtr instruction.

```
<instructions for ptr>
p = <result of ptr>
<instructions for int>
i = <result of int>
result = AddPtr(p, i, <size of referenced type of ptr>)
```

You need to make sure that the pointer is always the first operand, regardless the order of operands in AST.
And the size of pointer's referenced type needs calculating at compile time.

For expression `<ptr> - <int>` is identical, except the index is negated before included in Addptr.

```
<instructions for ptr>
p = <result of ptr>
<instructions for int>
i = <result of int>
j = Unary(Negate, i)
result = AddPtr(p, j, <size of referenced type of ptr>)
```

Subtracting one pointer from another is handled differently.
We use the Binary(Subtract, ...) instruction to compute the offsets between 2 pointers,
then divide the result by the number of bytes in one array element, we get the difference in array indices.
So for `<ptr1> - <ptr2>`

```
<instructions for ptr1>
p1 = <result of ptr1>
<instructions for ptr2>
p2 = <result of ptr2>
diff = Binary(Subtract, p1, p2)
result = Binary(Divide, diff, <size of referenced type of ptr1>)
```

Our type checker validates that both pointers must have the same type, so it doesn't matter which pointer we choose to compute the size of referenced type.

Comparison of pointers works the same.

#### Subscripting

The expression `<ptr>[<int>]` is equivalent to `*(<ptr> + <int>)`.
So we use the same AddPtr instruction, but then returns a DereferencedPointer(result) instead of PlainOperand(result).

#### Compound Initializers

We use CopyToOffset to copy each scalar expression to the appropriate location in memory.

```C
long arr[3] = {1l, 2l, 3l};
```

is converted to:

```
CopyToOffset(1l, "arr", 0)
CopyToOffset(2l, "arr", 8)
CopyToOffset(3l, "arr", 16)
```

And for multidimentional arrays:

```C
long nested[2][3] = {{1l, 2l, 3l}, {4l, 5l, 6l}};
```

is converted to:

```
CopyToOffset(1l, "nested", 0)
CopyToOffset(2l, "nested", 8)
CopyToOffset(3l, "nested", 16)
CopyToOffset(4l, "nested", 24)
CopyToOffset(5l, "nested", 32)
CopyToOffset(6l, "nested", 40)
```

Our type checker has annotated each compound initializer with a type, so calculating offsets is not a big deal.

#### Tentative Array Definitions

We've zero initialized tentative static variables, and now we do the same for arrays, utilizing the ZeroInit(n) construct.

## Assembly Generation

We don't introduce new instructions, but we have a new memory addressing mode, which is called _indexed addressing_.
Right now, we can specify a memory operand with a base address in a register and a constant offset, like `4(%rax)`.
With indexed addressing, we can store the base address in one register and an index in another, and the scale must be a constant: 1, 2, 4, or 8.

```
movl	$5, (%rax, %rbp, 4)
```

### Assembly

Here we add _Indexed_, and _PseudoMem_.
PseudoMem helps represent variables in assembly, which is similar to Pseudo operand.
The difference is that PseudoMem represents aggregate objects, which we'll always store in memory.
Pseudo, on the other hand, represent scalar objects that could potentially be stored in registers.

In assembly, we treat an array like an undifferentiated chunk of memory. We don't care its element count, or element type once they are put in memory.
However, we do care its alignment and the space it takes up.

<pre><code>program = Program(top_level*)
assembly_type = LongWord | Quadword | Double <strong>| ByteArray(int size, int alignment)</strong>
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, <strong>static_init* init_list</strong>)
	| StaticConstant(identifier name, int alignment, static_init init)
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(operand src, operand dst)
		| MovZeroExtend(operand src, operand dst)
		| Lea(operand src, operand dst)
		| Cvttsd2si(assembly_type dst_type, operand src, operand dst)
		| Cvtsi2sd(assembly_type src_type, operand src, operand dst)
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
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier) <strong>| PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)</strong>
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP
	| XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15</pre></code>

### Converting TACKY to Assembly

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                      | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| ---------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                        | Program(top_level_defs + \<all StaticConstant constructs for floating-point constants>)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Function(name, global, params, instructions)   | Function(name, global, [<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first int param type>, Reg(DI), \<first int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second int param type>, Reg(SI), \<second int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four integer parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first double param type>, Reg(XMM0), \<first double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second double param type>, Reg(XMM1), \<second double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next six double parameters from registers><br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first stack param type>, Memory(BP, 16), \<first stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second stack param type>, Memory(BP, 24), \<second stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, **init_list**) | StaticVariable(name, global, \<alignment of t, **init_list**)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                                                               | Assembly instructions                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| ------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val) (Integer)                                                           | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Return(val) (Double)                                                            | Mov(\<Double>, val, Reg(XMM0))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Unary(Not, src, dst) (Integer)                                                  | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                                                                |
| Unary(Not, src, dst) (Double)                                                   | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, src, Reg(\<X>))<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                     |
| Unary(Negate, src, dst) (Double negation)                                       | Mov(Double, src, dst)<br>Binary(Xor, Double, Data(\<negative-zero>), dst)<br>And add a top-level constant:<br>StaticConstant(\<negative-zero>, 16, DoubleInit(-0.0))                                                                                                                                                                                                                                                                                                                           |
| Unary(unary_operator, src, dst)                                                 | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Binary(Divide, src1, src2, dst) (Signed)                                        | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Divide, src1, src2, dst) (Unsigned)                                      | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Signed)                                     | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Unsigned)                                   | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(arithmetic_operator, src1, src2, dst)                                    | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(relational_operator, src1, src2, dst)                                    | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                                                                                                                                                                                                                                                                                                                                                                              |
| Jump(target)                                                                    | Jmp(target)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| JumpIfZero(condition, target) (Integer)                                         | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| JumpIfZero(condition, target) (Double)                                          | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                             |
| JumpIfNotZero(condition, target)                                                | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| JumpIfNotZero(condition, target) (Double)                                       | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                            |
| Copy(src, dst)                                                                  | Mov(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Label(identifier)                                                               | Label(identifier)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| FunCall(fun_name, args, dst)                                                    | \<fix stack alignment><br>\<move arguments to general-purpose registers><br>\<move arguments to XMM registers><br>\<push arguments onto the stack><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, \<dst register>, dst)                                                                                                                                                                                                                                             |
| SignExtend(src, dst)                                                            | Movsx(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Truncate(src, dst)                                                              | Mov(Longword, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ZeroExtend(src, dst)                                                            | MovZeroExtend(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| IntToDouble(src, dst)                                                           | Cvtsi2sd(<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| DoubleToInt(src, dst)                                                           | Cvttsd2si(<dst type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| UIntToDouble(src, dst) (unsigned int)                                           | MovZeroExtend(src, Reg(\<R>))<br>Cvtsi2s(Quadword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                             |
| UIntToDouble(src, dst) (unsigned long)                                          | Cmp(Quadword, Imm(0), src)<br>JmpCC(L, \<label1>)<br>Cvtsi2sd(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Quadword, src, Reg(\<R1>))<br>Mov(Quadword, Reg(\<R1>), Reg(\<R2>))<br>Unary(Shr, Quadword, Reg(\<R2>))<br>Binary(And, Quadword, Imm(1), Reg(\<R1>))<br>Binary(Or, Quadword, Reg(\<R1>), Reg(\<R2>))<br>Cvtsi2sd(Quadword, Reg(\<R2>), dst)<br>Binary(Add, Double, dst, dst)<br>Label(\<label2>)                                                                |
| DoubleToUInt(src, dst) (unsigned int)                                           | Cvttsd2si(Quadword, src, Reg(\<R>))<br>Mov(Longword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                           |
| DoubleToUInt(src, dst) (unsigned long)                                          | Cmp(Double, Data(\<upper-bound>), src)<br>JmpCC(AE, \<label1>)<br>Cvttsd2si(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Double, src, Reg(\<X>))<br>Binary(Sub, Double, Data(\<upper-bound>),Reg(\<X>))<br>Cvttsd2si(Quadword, Reg(\<X>), dst)<br>Mov(Quadword, Imm(9223372036854775808), Reg(\<R>))<br>Binary(Add, Quadword, Reg(\<R>), dst)<br>Label(\<label2>)<br>And add a top-level constant:<br>StaticConstant(\<upper-bound>, 8, DoubleInit(9223372036854775808.0)) |
| Load(ptr, dst)                                                                  | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<dst type>, Memory(\<R>, 0), dst)                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Store(src, ptr)                                                                 | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<src type>, src, Memory(\<R>, 0))                                                                                                                                                                                                                                                                                                                                                                                                                        |
| GetAddress(src, dst)                                                            | Lea(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| **AddPtr(ptr, index, scale, dst) (Constant index)**                             | **Mov(Quadword, ptr, Reg(\<R>))<br>Lea(Memory(\<R>, index \* scale), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **AddPtr(ptr, index, scale, dst) (Variable index and scale of 1, 2, 4, or 8 )** | **Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)**                                                                                                                                                                                                                                                                                                                                                                               |
| **AddPtr(ptr, index, scale, dst) (Variable index and other scale )**            | **Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Binary(Mult, Quadword, Imm(scale), Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)**                                                                                                                                                                                                                                                                                                                             |
| **CopyToOffset(src, dst, offset)**                                              | **Mov(\<src type>, src, PseudoMem(dst, offset))**                                                                                                                                                                                                                                                                                                                                                                                                                                              |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator           | Assembly operator |
| ------------------------ | ----------------- |
| Complement               | Not               |
| Negate                   | Neg               |
| Add                      | Add               |
| Subtract                 | Sub               |
| Multiply                 | Mult              |
| Divide (double division) | DivDouble         |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code (signed) | Assembly condition code (unsigned **or pointer** or double ) |
| ---------------- | -------------------------------- | ------------------------------------------------------------ |
| Equal            | E                                | E                                                            |
| NotEqual         | NE                               | NE                                                           |
| LessThan         | L                                | B                                                            |
| LessOrEqual      | LE                               | BE                                                           |
| GreaterThan      | G                                | A                                                            |
| GreaterOrEqual   | GE                               | AE                                                           |

#### Converting TACKY Operands to Assembly

| TACKY operand                         | Assembly operand                                                                                 |
| ------------------------------------- | ------------------------------------------------------------------------------------------------ |
| Constant(ConstInt(int))               | Imm(int)                                                                                         |
| Constant(ConstUInt(int))              | Imm(int)                                                                                         |
| Constant(ConstLong(int))              | Imm(int)                                                                                         |
| Constant(ConstULong(int))             | Imm(int)                                                                                         |
| Constant(ConstDouble(double))         | Data(\<ident>)<br>And add top-level constant:<br>StaticConstant(\<ident>, 8, DoubleInit(Double)) |
| Var(identifier) **(Scalar value)**    | Pseudo(identifier)                                                                               |
| **Var(identifier) (Aggregate value)** | **PseudoMem(identifier, 0)**                                                                     |

#### Converting Types to Assembly

| Source type                                                      | Assembly type                                                       | Alignment                      |
| ---------------------------------------------------------------- | ------------------------------------------------------------------- | ------------------------------ |
| Int                                                              | Longword                                                            | 4                              |
| UInt                                                             | Longword                                                            | 4                              |
| Long                                                             | Quadword                                                            | 8                              |
| ULong                                                            | Quadword                                                            | 8                              |
| Double                                                           | Double                                                              | 8                              |
| Pointer(referenced_t)                                            | Quadword                                                            | 8                              |
| **Array(element, size) (Variables that are 16 bytes or larger)** | **ByteArray(\<size of element> \* size, 16)**                       | **16**                         |
| **Array(element, size) (Everything else)**                       | **ByteArray(\<size of element> \* size, \<alignment of elements>)** | **Same alignment as elements** |

### Replacing Pseudoregisters

Similar to replacing Pseudo with Memory operands, we replace PseudoMem with Memory, but add both the offset from the start of the array with the concrete address of the array from RBP.
For example, suppose "arr" is assigned the stack address -12(%rbp), we compute -12 + 4 to determine a new memory operand -8(%rbp).

```
Mov(Longword, Imm(3), PseudoMem("arr", 4))
```

is replaced into

```
Mov(Longword, Imm(3), Memory(BP, -8))
```

To access an array with static storage duration, we use the Data operand.

For example: suppose "arr" is a static array

```
PseudoMem("arr", 0)
```

is converted to:

```
Data("arr")
```

### Fixing Up Instructions

We didn't introduce any new instructions, so we don't need any new fix-up rules here.
This pass must recognize that the new Indexed operand specifies a memory address and therefore can't be used where a register or immediate value is required.

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                                                                      | Output                                                                                                                                                                                                                                                                                                           |
| --------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                                                                               | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                                                                              | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, **init_list**) (Initialized to zero **, or any variable initialized only with ZeroInit**) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;**<init_list>**                                                                                                                             |
| StaticVariable(name, global, alignment, init) **(All other variables)**                                                           | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;**<init_list>**                                                                                                                            |
| StaticConstant(name, alignment, init) (Linux)                                                                                     | &nbsp;&nbsp;&nbsp;&nbsp;.section .rodata<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                                                         |
| StaticConstant(name, alignment, init) (MacOS 8-byte aligned constants)                                                            | &nbsp;&nbsp;&nbsp;&nbsp;.literal8<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 8<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init>                                                                                                                                                                                             |
| StaticConstant(name, alignment, init) (MacOS 16-byte aligned constants)                                                           | &nbsp;&nbsp;&nbsp;&nbsp;.literal16<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 16<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;<init><br>&nbsp;&nbsp;&nbsp;&nbsp;.quad 0                                                                                                                                                        |
| Global directive                                                                                                                  | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                                                                               | For Linux only: .align <alignment><br>For macOS and Linux: .balign <alignment>                                                                                                                                                                                                                                   |

#### Formatting Static Initializers

| Static Initializer | Output                                               |
| ------------------ | ---------------------------------------------------- |
| IntInit(0)         | .zero 4                                              |
| IntInit(i)         | .long \<i>                                           |
| LongInit(0)        | .zero 8                                              |
| LongInit(i)        | .quad \<i>                                           |
| UIntInit(0)        | .zero 4                                              |
| UIntInit(i)        | .long \<i>                                           |
| ULongInit(0)       | .zero 8                                              |
| ULongInit(i)       | .quad \<i>                                           |
| DoubleInit(d)      | .double \<d><br>OR<br>.quad \<d-interpreted-as-long> |
| **ZeroInit(n)**    | **.zero \<n>**                                       |

#### Formatting Assembly Instructions

| Assembly instruction                 | Output                                                                                 |
| ------------------------------------ | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                     | mov \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                         |
| Movsx(src, dst)                      | movslq &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| Cvtsi2sd(t, src, dst)                | cvtsi2sd\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                    |
| Cvttsd2si(t, src, dst)               | cvttsd2si\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                   |
| Ret                                  | ret                                                                                    |
| Unary(unary_operator, t, operand)    | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst) | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| Binary(Xor, Double, src, dst)        | xorpd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Binary(Mult, Double, src, dst)       | mulsd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Idiv(t, operand)                     | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| Div(t, operand)                      | div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                             |
| Cdq(Longword)                        | cdq                                                                                    |
| Cdq(Quadword)                        | cdo                                                                                    |
| Cmp(t, operand, operand)             | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| Cmp(Double, operand, operand)        | comisd&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                   |
| Jmp(label)                           | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)              | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)            | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                         | .L\<label>:                                                                            |
| Push(operand)                        | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                          | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |
| Lea(src, dst)                        | leaq&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                             |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |
| Shr               | shr              |
| DivDouble         | div              |
| And               | and              |
| Or                | or               |

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
| Double        | sd                 |

#### Formatting Assembly Operands

| Assembly operand             | Output                         |
| ---------------------------- | ------------------------------ |
| Reg(AX) 8-byte               | %rax                           |
| Reg(AX) 4-byte               | %eax                           |
| Reg(AX) 1-byte               | %al                            |
| Reg(DX) 8-byte               | %rdx                           |
| Reg(DX) 4-byte               | %edx                           |
| Reg(DX) 1-byte               | %dl                            |
| Reg(CX) 8-byte               | %rcx                           |
| Reg(CX) 4-byte               | %ecx                           |
| Reg(CX) 1-byte               | %cl                            |
| Reg(DI) 8-byte               | %rdi                           |
| Reg(DI) 4-byte               | %edi                           |
| Reg(DI) 1-byte               | %dil                           |
| Reg(SI) 8-byte               | %rsi                           |
| Reg(SI) 4-byte               | %esi                           |
| Reg(SI) 1-byte               | %sil                           |
| Reg(R8) 8-byte               | %r8                            |
| Reg(R8) 4-byte               | %r8d                           |
| Reg(R8) 1-byte               | %r8b                           |
| Reg(R9) 8-byte               | %r9                            |
| Reg(R9) 4-byte               | %r9d                           |
| Reg(R9) 1-byte               | %r9b                           |
| Reg(R10) 8-byte              | %r10                           |
| Reg(R10) 4-byte              | %r10d                          |
| Reg(R10) 1-byte              | %r10b                          |
| Reg(R11) 8-byte              | %r11                           |
| Reg(R11) 4-byte              | %r11d                          |
| Reg(R11) 1-byte              | %r11b                          |
| Reg(SP)                      | %rsp                           |
| Reg(BP)                      | %rbp                           |
| Reg(XMM0)                    | %xmm0                          |
| Reg(XMM1)                    | %xmm1                          |
| Reg(XMM2)                    | %xmm2                          |
| Reg(XMM3)                    | %xmm3                          |
| Reg(XMM4)                    | %xmm4                          |
| Reg(XMM5)                    | %xmm5                          |
| Reg(XMM6)                    | %xmm6                          |
| Reg(XMM7)                    | %xmm7                          |
| Reg(XMM8)                    | %xmm8                          |
| Reg(XMM9)                    | %xmm9                          |
| Reg(XMM10)                   | %xmm10                         |
| Reg(XMM11)                   | %xmm11                         |
| Reg(XMM12)                   | %xmm12                         |
| Reg(XMM13)                   | %xmm13                         |
| Reg(XMM14)                   | %xmm14                         |
| Reg(XMM15)                   | %xmm15                         |
| Memory(reg, int)             | \<int>(reg)                    |
| Imm(int)                     | $\<int>                        |
| **Indexed(reg1, reg2, int)** | **(\<reg1>, \<reg2>, \<int>)** |

## Summary

We've just completed our first aggregate type.
In the next chapter, we'll implement three more integer types: char, signed char, unsigned char.
We'll also implement string literals, which can be either array initializers or char arrays that decay to pointers.

### Reference Implementation Analysis

[Chapter 15 Code Analysis](./code_analysis/chapter_15.md)

---

# Chapter 16: Characters and Strings

## Stages of a Compiler

1. **Lexer**
   - Input: Source code
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

We implement _char_, _signed char_ and _unsigned char_.
All of the characters types have a size of 1 byte.

We also add support for string literals and character contants.
String literals are sometimes like compound initializers, but other times they are like constant char arrays.

### Character Traits

In C, `int c` and `signed int c` are identical as _int_ is signed.
However, `char c` and `signed char c` are distinct, even though they behave the same. Thus, the 2 declarations of _c_ conflict each other.
A character type is promoted to int type (integer promotions) when it's in:

- Unary +, -, ~
- Bitwise operation
- Usual arithmetic operation

In C17, there are no scalar constants of character type. So the token `'a'` has type _int_.

### String Literals

In this book, we'll distinguish between _string literals_ and _strings_.
String Literals are expressions in source code like "abc".
Strings are objects that live in memory, such as a null-terminated char array.

Strings that can't be modified at runtime are called Constant Strings.

There are 2 ways to use String Literals:

First, they are used to initialze an array of any character type.

```C
signed char array[4] = "abc";		// We'll include a terminating byte if there's space, omit it if there isn't
signed char array[4] = {'a', 'b', 'c', 0}; // This is equivalent to the one above

signed char array[3] = "abc";			// No space left for terminating byte
signed char array[3] = {'a', 'b', 'c'};	// so we omit it in the char array
```

Second, they become constant strings and act like other expressions of array type.
They decay to pointers so we can subscript them or assign them to char \* objects.
String Literals are lvalues so we can use the & operator.

```C
char *str_ptr = "abc";				// "abc" is decayed to a pointer to char
char (*array_ptr)[4] = &"abc";		// "abc" is not decayed to a pointer,
									// 	so & result is a pointer to a whole string with type char(*)[4]
```

Constant Strings live in a Read-only section in memory. So any attempt to modify its value at runtime result in undefined behavior.

Well, are you confused yet? Let's compare both cases of string literals: one initializing a char array, and one designating a constant string.

```C
// String literals initializing an array, and we can modify the char elements in it
char arr[3] = "abc";
arr[0] = 'x';

// Constant strings cannot be modified. This will compile, but will result in undefined behavior.
char *ptr = "abc";
ptr[0] = 'x';
```

### Working with Strings in Assembly

We use two directives:

- `asciz` includes a terminating byte
- `ascii` doens't include a terminating byte

```C
static char null_terminated[4] = "abc";
```

is compiled to

```asm
	.data
null_terminated:
	.asciz "abc"
```

```C
static char not_null_terminated[3] = "abc";
```

is compiled to

```asm
	.data
not_null_terminated:
	.ascii "abc"
```

and

```C
static char extra_padding[5] = "abc";
```

is compiled to

```asm
	.data
extra_padding:
	.asciz "abc"
	.zero 1
```

We don't need any alignment directive as all the character types are 1-byte aligned. So are the array of characters, except if they are larger than 16 bytes.

How about for non static arrays of characters?

```C
int main(void) {
	char letters[6] = "abcde";
	return 0;
}
```

has the letters initialized like this on stack

```asm
movb 	$97, -8(%rbp)
movb 	$98, -7(%rbp)
movb 	$99, -6(%rbp)
movb 	$100, -5(%rbp)
movb 	$101, -4(%rbp)
movb 	$0, -3(%rbp)
```

This works, but can be improved:

```asm
movl 	$1684234849, -8(%rbp)
movb 	$101, -4(%rbp)
movb 	$0, -3(%rbp)
```

The immediate `1684234849` is used as we interpret the first 4 bytes of our strings as an integer.

Ok, we're done with the first use case of String Literals in initializing char arrays.
Let's move to Constant Strings.
Constant strings are always null-terminated.

```C
return "A profound statement.";
```

is compiled to

```asm
	.section .rodata
.Lstring.0:
	.asciz "A profound statement."
```

We can access to it iwht RIP-relative addressing like any other static objects.

```asm
leaq	.LString.0(%rip), %rax
```

We need some more work when using constant strings to initialize a static pointer.

```C
static char *ptr = "A profound statement.";
```

We still define the constant string as before, but we can load it to _ptr_ with the _lea_ instruction as the pointer is static, so it must be initialized before the program starts.

So we then need to add this:

```asm
	.data
	.align 8
ptr:
	.quad .LString.0
```

In fact, this works for any static pointer initialization, not just string literals. But our compiler implementation only supports constants for static objects.

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordChar | char |
| Character constants | '([^'\\\n]|\\['"?\\abfnrtv])' |
| String literals | "([^"\\\n]|\\['"\\?abfnrtv])\*" |

After lexing a string literal or character token, convert every escape sequence to the corresponding ASCII character.

## The Parser

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer*)
type = <strong>Char | SChar | UChar</strong> Int | Long | UInt | ULong | Double 
	| FunType(type* params, type ret)
	| Pointer(type referenced)
	| Array(type element, int size)
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
	<strong>String(string)</strong>
	| Var(identifier) 
	| Cast(type target_type, exp)
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
	| Dereference(exp)
	| AddrOf(exp)
	| Subscript(exp, exp)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
	<strong>| ConstChar(int) | ConstUChar(int)</strong></pre></code>

The new constant constructors are a little unusual. Character constant like 'a' have type _int_, and our parser will convert them to ConstInt nodes.
Useless as they might seem, we'll need them later when we pad out partially initialized character arrays.

String literals are constants, but they will be handled differently when we process initializers, so it's easier for us to treat String literals as expressions.

### EBNF

<pre><code>&lt;program&gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; [ "=" &lt;initializer&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;declarator&gt; ::= "*" &lt;declarator&gt; | &lt;direct-declarator&gt;
&lt;direct-declarator&gt; ::= &lt;simple-declarator&gt; [ &lt;declarator-suffix&gt; ]
&lt;declarator-suffix&gt; ::= &lt;param-list&gt; | { "[" &lt;const&gt; "]" }+ 
&lt;param-list&gt; ::= "(" "void" ")" | "(" &lt;param&gt; { "," &lt;param&gt; } ")"
&lt;param&gt; ::= { &lt;type-specifier&gt; }+ &lt;declarator&gt;
&lt;simple-declarator&gt; ::= &lt;identifier&gt; | "(" &lt;declarator&gt; ")"
&lt;type-specifier&gt; ::= "int" | "long" | "unsigned" | "signed" | "double" <strong>| "char"</strong>
&lt;specifier&gt; ::= &lt;type-specifier&gt; | "static" | "extern"
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;initializer&gt; ::= &lt;exp&gt; | "{" &lt;initializer&gt; { "," &lt;initializer&gt; } [ "," ] "}"
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
&lt;exp&gt; ::= &lt;unary-exp&gt; | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
&lt;unary-exp&gt; ::= &lt;unop&gt; &lt;unary-exp&gt;
	| "(" { &lt;type-specifier&gt; }+ ")" [ &lt;abstract-declarator&gt; ] &lt;unary-exp&gt;
	| &lt;postfix-exp&gt;
&lt;postfix-exp&gt; ::= &lt;primary-exp&gt; { "[" &lt;exp&gt; "]" }
&lt;primary-exp&gt; ::= &lt;const&gt; | &lt;identifier&gt; | "(" &lt;exp&gt; ")" <strong>| { &lt;string&gt; }+</strong>
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;abstract-declarator&gt; ::= "*" [ &lt;abstract-declarator&gt; ]
	| &lt;direct-abstract-declarator&gt;
&lt;direct-abstract-declarator&gt; ::= "(" &lt;abstract-declarator&gt; ")" { "[" &lt;const&gt; "]" }
	| { "[" &lt;const&gt; "]" }+
&lt;unop&gt; ::= "-" | "~" | "!" | "*" | "&"
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; | &lt;uint&gt; | &lt;ulong&gt; | &lt;double&gt; <strong>| &lt;char&gt;</strong>
&lt;identifier&gt; ::= ? An identifier token ?
<strong>&lt;string&gt; ::= ? A string token ?</strong>
&lt;int&gt; ::= ? An int token ?
<strong>&lt;char&gt; ::= ? A char token ?</strong>
&lt;long&gt; ::= ? An int or long token ?
&lt;uint&gt; ::= ? An unsigned int token ?
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?
&lt;double&gt; ::= ? A floating-point constant token ?</pre></code>

### Parser

#### Parsing Type Specifiers

The extended rule is simple:

- If only the keyword _char_ appears, it's type _char_
- If _char_ with _signed_, it's type _signed char_
- If _char_ with _unsigned_, it's type _unsigned char_
- If _char_ with any other type specifier, throw error

#### Parsing Character Constants

Character constants are converted to ConstInt.
For example:

- 'a' -> ConstInt(97)
- '\n' -> ConstInt(10)

#### Parsing String Literals

- If your lexer hasn't unescape string literals, the parser should do it now
- Each character in string literals must be represented as a single byte, because we need to calculate the length of strings with Type Checker
- Adjacent string literal tokens should be concatenated into a single String AST node. For example: `return "foo" "bar";` -> Return(String("foobar"))

## Semantic Analysis

### Type Checker

Character types are treated the same as other integer types. We only concern the integer promotions and introduce static initializers for the character types.
String literals are more challenging because we need to track if each string should be used directly or converted to a pointer, and which string should be null-terminated.

#### Characters

We promote char types to int when we get the common type.

```
get_common_type(type1, type2):
	if type1 is a character type:
		type1 = Int
	if type2 is a character type:
		type2 = Int
	--snip--
```

We also handle the integer promotions in Negate, also with Complement.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Unary(Negate, inner) ->
		typed_inner = typecheck_and_convert(inner, symbols)
		inner_t = get_type(typed_inner)
		if inner_t is a pointer type:
			fail("Can't negate a pointer")
		if inner_t is a character type:
			typed_inner = convert_to(typed_inner, Int)
		unary_exp = Unary(Negate, typed_inner)
		return set_type(unary_exp, get_type(typed_inner))
```

We'll always recognize characters as integer types. That means we accept characters as operands in ~ and %.
We allow implicit conversions from chars to any other arithmetic types in `convert_by_assignment`.

We add two static initializers for the character types.

```
static_init = IntInit(int) | LongInit(int) | UIntInit(int) | ULongInit(int)
	| CharInit(int) | UCharInit(int)
	| DoubleInit(double)
```

We need to update the way we type check compound initializers for non-static arrays because string literals is a special case.
In the previous chapter, we dealt with partly initialized arrays by padding out the remaining elements with zeros.
I suggested using a zero_initializer helper function. We can extend the function to emit ConstChar and ConstUChar to zero out elements of character type.

#### String Literal in Expressions

When we encounter a string literal in an expression, rather than an array initializer, we'll annotate it as a char array of the appropriate size.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| String(s) -> return set_type(e, Array(Char, length(s)+ 1))
```

We add 1 to the lenght of string to account for a terminating null byte.
Our `typecheck_and_convert` already handles implicit conversions from arrays to pointers. We extend it to convert string literals to pointers too.

String expressions should be recognized as lvalues, along with variables, subscript, and dereference.

#### String Literals Initializing Non-static Variables

```
typecheck_init(target_type, init, symbols):
	match target_type, init with
	| Array(elem_t, size), SingleInit(String(s))
		if elem_t is not a character type:
			fail("Can't initialize a non-character type with a string literal")
		if length(s) > size:
			fail("Too many characters in string literal")
		return set_type(init, target_type)
	| --snip--
```

#### String Literals Initializing Static Variables

```
static_init = IntInit(int) | LongInit(int) | UIntInit(int) | ULongInit(int)
	| CharInit(int) | UCharInit(int)
	| DoubleInit(double)
	| StringInit(string, bool null_terminated)
	| PointerInit(string name)
```

```
identifier_attrs = FunAttr(bool defined, bool global)
	| StaticAttr(initial_value init, bool global)
	| ConstantAttr(static_init init)
	| LocalAttr
```

StaticAttr indicates a variable which may be uninitialized, tentatively initialized or initialized with a list of values.
However, our new ConstantAttr means the constant is initialized with a single value.
We don't need global flag, since we'll never define a global constant.

##### Initializing a Static Array with a String Literal

We first validate the array's type:

- Array elements have character type
- Array is long enough to contain the string

Then we convert the string literal to StringInit initializer
, setting the _null_terminated_ flag if the array has enough space for the terminating null byte.
We add ZeroInit to the initializer list if we need tp pad it out.

```C
static char letters[10] = "abc";
```

is converted to a symbol entry as:

```
name="letters"
type=Array(Char, 10)
attrs=StaticAttr(init=Initial([StringInit("abc", True), ZeroInit(6)]),
				 global=False)
```

##### Initializing a Static Pointer with a String Literal

We create 2 entries in the symbol table:

- The string itself
- The variable that points to that string

```C
static char *message = "Hello!";
```

First, we generate an identifer for the constant string "Hello!"; let's call this "string.0".
Then, we add the entry as the previous example.

```
name="string.0"
type=Array(Char, 7)
attrs=ConstantAttr(StringInit("Hello!", True))
```

Finally, we add _message_ itself to the symbol table, initializing it with a pointer to the "string.0"

```
name="message"
type=Pointer(Char)
attrs=StaticAttr(init=Initial([PointerInit("string.0")]), global=False)
```

## TACKY Generation

We handle casts to and from character types with the existing type instructions.
For example, we'll implement casts from _double_ to _unsigned char_ with _DoubleToUInt_,
and well implement casts from _char_ to _int_ with _SignExtend_.

Processing string literals requires more work.

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, static_init* init_list)
		<strong>| StaticConstant(identifier, type t, static_init init)</strong>
instruction = Return(val) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	| ZeroExtend(val src, val dst)
	| DoubleToInt(val src, val dst)
	| DoubleToUInt(val src, val dst)
	| IntToDouble(val src, val dst)
	| UIntToDouble(val src, val dst)
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| GetAddress(val src, val dst)
	| Load(val src_ptr, val dst)
	| Store(val src, val dst_ptr)
	| Addptr(val src, val index, int scale, val dst)
	| CopyToOffset(val src, identifier dst, int offset)
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

#### String Literals as Array Initializers

In the Type Checker, we dealt with string literals that initialized static arrays.
Now we do the same for arrays with automatic storage duration.

We have 2 ways:

- Simple way is to initialize these arrays onbe character at a time
- More efficient way is to initialize entire 4- or 8-byte chunks at once with _CopyToOffset_ instructions.

Back to the example we had before:

```C
int main(void) {
	char letters[6] = "abcde";
	return 0;
}
```

Using the simple way, we would have something like this:

```
CopyToOffset(Constant(ConstChar(97)),  "letters", 0)
CopyToOffset(Constant(ConstChar(98)),  "letters", 1)
CopyToOffset(Constant(ConstChar(99)),  "letters", 2)
CopyToOffset(Constant(ConstChar(100)), "letters", 3)
CopyToOffset(Constant(ConstChar(101)), "letters", 4)
CopyToOffset(Constant(ConstChar(0)),   "letters", 5)
```

Using the more efficient way, we would initialize letters with a single 4-byte integer, followed by 2 individual bytes:

```
CopyToOffset(Constant(ConstInt(1684234849)), "letters", 0)
CopyToOffset(Constant(ConstChar(101)), 		 "letters", 4)
CopyToOffset(Constant(ConstChar(0)), 		 "letters", 5)
```

How to get the number `1684234849`?
We take the first 4 bytes of 'a', 'b', 'c' and 'd': 97, 98, 99, 100.
Then we interpret them as a single little-endian integer.

In hexadecimal, these bytes are 0x61, 0x62, 0x63, and 0x64. So we'll have 0x64636261, or 1684234849 in decimal.

To initialize eight characters at once, we'll use ConstLong instead of ConstInt, but we need to be cautiousnot to overrun the bounds of the array.

It's up to you which approach to use. In either case, make sure to initialize the correct number of null bytes at the end of the string.
If a string literal is longer than the array, copy in as many characters as the array can hold, in other words we leave off the null byte.
If the string literal is to short, copy zeros into the rest of the array.

#### String Literals in Expressions

When we encounter a string literal outside of an array initializer, we'll add it to the symbol table as a constant string.
Then we'll use its identifier as a TACKY var.

Back to an example before

```C
return "A profound statement.";
```

The parser and type checker will result this into:

```
Return(AddrOf(String("A profound statement.")))
```

In TACKY, we first define the string to symbol table

```
name="string.1"
type=Array(Char, 22)
attrs=ConstantAttr(StringInit("A profound statement.", True))
```

This entry is no different from the constant strings we defined in the type checker.

Now in TACKY:

```
GetAddress(Var("string.1"), Var("tmp2"))
Return(Var("tmp2"))
```

## Assembly Generation

- We convert operations on individual characters to assembly.
- Then we handle TACKY StaticConstant constructs and add constant strings to the backend symbol table.
- When processing a TACKY StaticConstant, just convert it to an corresponding assembly StaticConstant.
- Each constant string in the symbol table needs to be converted to an equivalent entry in the backend symbol table: set the is_static to True, and is_constant to True.

Most instructions support 1-byte operands as well as longwords and quadwords.
We'll convert _char_, _signed char_ and _unsigned char_ to _Byte_ Asm type.
Converting between double and character types require an intermediate step of converting the character type to an _int_ first.

### Assembly

<pre><code>program = Program(top_level*)
assembly_type = <strong>Byte |</strong> LongWord | Quadword | Double | ByteArray(int size, int alignment)
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, static_init* init_list)
	| StaticConstant(identifier name, int alignment, static_init init)
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(<strong>assembly_type src_type, assembly_type dst_type,</strong> operand src, operand dst)
		| MovZeroExtend(<strong>assembly_type src_type, assembly_type dst_type</strong>operand src, operand dst)
		| Lea(operand src, operand dst)
		| Cvttsd2si(assembly_type dst_type, operand src, operand dst)
		| Cvtsi2sd(assembly_type src_type, operand src, operand dst)
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
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier) | PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP
	| XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15</pre></code>

### Converting TACKY to Assembly

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| -------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs + \<all StaticConstant constructs for floating-point constants>)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Function(name, global, params, instructions) | Function(name, global, [<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first int param type>, Reg(DI), \<first int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second int param type>, Reg(SI), \<second int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four integer parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first double param type>, Reg(XMM0), \<first double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second double param type>, Reg(XMM1), \<second double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next six double parameters from registers><br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first stack param type>, Memory(BP, 16), \<first stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second stack param type>, Memory(BP, 24), \<second stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, init_list)   | StaticVariable(name, global, \<alignment of t, init_list)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **StaticConstant(name, t, init)**            | **StaticConstant(name, \<alignment of t>, init)**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                                                           | Assembly instructions                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val) (Integer)                                                       | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Return(val) (Double)                                                        | Mov(\<Double>, val, Reg(XMM0))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Unary(Not, src, dst) (Integer)                                              | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                                                                |
| Unary(Not, src, dst) (Double)                                               | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, src, Reg(\<X>))<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                     |
| Unary(Negate, src, dst) (Double negation)                                   | Mov(Double, src, dst)<br>Binary(Xor, Double, Data(\<negative-zero>), dst)<br>And add a top-level constant:<br>StaticConstant(\<negative-zero>, 16, DoubleInit(-0.0))                                                                                                                                                                                                                                                                                                                           |
| Unary(unary_operator, src, dst)                                             | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Binary(Divide, src1, src2, dst) (Signed)                                    | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Divide, src1, src2, dst) (Unsigned)                                  | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Signed)                                 | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Unsigned)                               | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(arithmetic_operator, src1, src2, dst)                                | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(relational_operator, src1, src2, dst)                                | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                                                                                                                                                                                                                                                                                                                                                                              |
| Jump(target)                                                                | Jmp(target)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| JumpIfZero(condition, target) (Integer)                                     | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| JumpIfZero(condition, target) (Double)                                      | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                             |
| JumpIfNotZero(condition, target)                                            | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| JumpIfNotZero(condition, target) (Double)                                   | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                            |
| Copy(src, dst)                                                              | Mov(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Label(identifier)                                                           | Label(identifier)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| FunCall(fun_name, args, dst)                                                | \<fix stack alignment><br>\<move arguments to general-purpose registers><br>\<move arguments to XMM registers><br>\<push arguments onto the stack><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, \<dst register>, dst)                                                                                                                                                                                                                                             |
| SignExtend(src, dst)                                                        | Movsx(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Truncate(src, dst)                                                          | Mov(Longword, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ZeroExtend(src, dst)                                                        | MovZeroExtend(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| IntToDouble(src, dst) **(char or signed char)**                             | **Movsx(Byte, Longword, src, Reg(\<R>))<br>Cvtsi2sd(Longword, Reg(\<R>, dst))**                                                                                                                                                                                                                                                                                                                                                                                                                |
| IntToDouble(src, dst) (int or long)                                         | Cvtsi2sd(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| DoubleToInt(src, dst) **(char or signed char)**                             | **Cvttsd2si(\<dst type>, src, Reg(\<R>))<br>Mov(Byte, Reg(\<R>), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                        |
| DoubleToInt(src, dst) (int or long)                                         | Cvttsd2si(\<dst type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| UIntToDouble(src, dst) **(unsigned char)**                                  | **MovZeroExtend(Byte, Longword, src, Reg(\<R>))<br>Cvtsi2sd(Longword, Reg(\<R>), dst)**                                                                                                                                                                                                                                                                                                                                                                                                        |
| UIntToDouble(src, dst) (unsigned int)                                       | MovZeroExtend(**Longword, Quadword,** src, Reg(\<R>))<br>Cvtsi2s(Quadword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                     |
| UIntToDouble(src, dst) (unsigned long)                                      | Cmp(Quadword, Imm(0), src)<br>JmpCC(L, \<label1>)<br>Cvtsi2sd(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Quadword, src, Reg(\<R1>))<br>Mov(Quadword, Reg(\<R1>), Reg(\<R2>))<br>Unary(Shr, Quadword, Reg(\<R2>))<br>Binary(And, Quadword, Imm(1), Reg(\<R1>))<br>Binary(Or, Quadword, Reg(\<R1>), Reg(\<R2>))<br>Cvtsi2sd(Quadword, Reg(\<R2>), dst)<br>Binary(Add, Double, dst, dst)<br>Label(\<label2>)                                                                |
| DoubleToUInt(src, dst) **(unsigned char)**                                  | **Cvttsd2si(Longword, src, Reg(\<R>))<br>Mov(Byte, Reg(\<R>), dst)**                                                                                                                                                                                                                                                                                                                                                                                                                           |
| DoubleToUInt(src, dst) (unsigned int)                                       | Cvttsd2si(Quadword, src, Reg(\<R>))<br>Mov(Longword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                           |
| DoubleToUInt(src, dst) (unsigned long)                                      | Cmp(Double, Data(\<upper-bound>), src)<br>JmpCC(AE, \<label1>)<br>Cvttsd2si(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Double, src, Reg(\<X>))<br>Binary(Sub, Double, Data(\<upper-bound>),Reg(\<X>))<br>Cvttsd2si(Quadword, Reg(\<X>), dst)<br>Mov(Quadword, Imm(9223372036854775808), Reg(\<R>))<br>Binary(Add, Quadword, Reg(\<R>), dst)<br>Label(\<label2>)<br>And add a top-level constant:<br>StaticConstant(\<upper-bound>, 8, DoubleInit(9223372036854775808.0)) |
| Load(ptr, dst)                                                              | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<dst type>, Memory(\<R>, 0), dst)                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Store(src, ptr)                                                             | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<src type>, src, Memory(\<R>, 0))                                                                                                                                                                                                                                                                                                                                                                                                                        |
| GetAddress(src, dst)                                                        | Lea(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| AddPtr(ptr, index, scale, dst) (Constant index)                             | Mov(Quadword, ptr, Reg(\<R>))<br>Lea(Memory(\<R>, index \* scale), dst)                                                                                                                                                                                                                                                                                                                                                                                                                        |
| AddPtr(ptr, index, scale, dst) (Variable index and scale of 1, 2, 4, or 8 ) | Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)                                                                                                                                                                                                                                                                                                                                                                                   |
| AddPtr(ptr, index, scale, dst) (Variable index and other scale )            | Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Binary(Mult, Quadword, Imm(scale), Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)                                                                                                                                                                                                                                                                                                                                 |
| CopyToOffset(src, dst, offset)                                              | Mov(\<src type>, src, PseudoMem(dst, offset))                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator           | Assembly operator |
| ------------------------ | ----------------- |
| Complement               | Not               |
| Negate                   | Neg               |
| Add                      | Add               |
| Subtract                 | Sub               |
| Multiply                 | Mult              |
| Divide (double division) | DivDouble         |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code (signed) | Assembly condition code (unsigned or pointer or double ) |
| ---------------- | -------------------------------- | -------------------------------------------------------- |
| Equal            | E                                | E                                                        |
| NotEqual         | NE                               | NE                                                       |
| LessThan         | L                                | B                                                        |
| LessOrEqual      | LE                               | BE                                                       |
| GreaterThan      | G                                | A                                                        |
| GreaterOrEqual   | GE                               | AE                                                       |

#### Converting TACKY Operands to Assembly

| TACKY operand                     | Assembly operand                                                                                 |
| --------------------------------- | ------------------------------------------------------------------------------------------------ |
| Constant(ConstInt(int))           | Imm(int)                                                                                         |
| Constant(ConstUInt(int))          | Imm(int)                                                                                         |
| Constant(ConstLong(int))          | Imm(int)                                                                                         |
| Constant(ConstULong(int))         | Imm(int)                                                                                         |
| Constant(ConstDouble(double))     | Data(\<ident>)<br>And add top-level constant:<br>StaticConstant(\<ident>, 8, DoubleInit(Double)) |
| **Constant(ConstChar(int))**      | **Imm(int)**                                                                                     |
| **Constant(ConstUChar(int))**     | **Imm(int)**                                                                                     |
| Var(identifier) (Scalar value)    | Pseudo(identifier)                                                                               |
| Var(identifier) (Aggregate value) | PseudoMem(identifier, 0)                                                                         |

#### Converting Types to Assembly

| Source type                                                  | Assembly type                                                   | Alignment                  |
| ------------------------------------------------------------ | --------------------------------------------------------------- | -------------------------- |
| **Char**                                                     | **Byte**                                                        | **1**                      |
| **SChar**                                                    | **Byte**                                                        | **1**                      |
| **UChar**                                                    | **Byte**                                                        | **1**                      |
| Int                                                          | Longword                                                        | 4                          |
| UInt                                                         | Longword                                                        | 4                          |
| Long                                                         | Quadword                                                        | 8                          |
| ULong                                                        | Quadword                                                        | 8                          |
| Double                                                       | Double                                                          | 8                          |
| Pointer(referenced_t)                                        | Quadword                                                        | 8                          |
| Array(element, size) (Variables that are 16 bytes or larger) | ByteArray(\<size of element> \* size, 16)                       | 16                         |
| Array(element, size) (Everything else)                       | ByteArray(\<size of element> \* size, \<alignment of elements>) | Same alignment as elements |

### Replacing Pseudo-Operands

We'll allocate 1 byte on the stack for each Byte object.
We don't need to worry about rounding these to the right alignment, because they're all 1-byte aligned.
And we don't deal with constant strings here, as they are now static constants, recorded in the backend symbol table.

### Fixing Up Instructions

The _movz_ instruction must have a register as its destination, and its source must not be an immediate value.

```asm
movzbl	$10, -4(%rbp)
```

is rewritten as:

```asm
movb 	$10, %r10b
movzbl 	%r10b, %r11b
movl 	%r11b, -4(%rbp)
```

If the source of a _movb_ instruction is an immediate value that can't fit in a single byte, we'll reduce it modulo 256.

```asm
movb	$258, %al
```

is rewritten as:

```asm
movb	$2, %al
```

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                                                              | Output                                                                                                                                                                                                                                                                                                           |
| ------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                                                                       | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                                                                      | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, init_list) (Initialized to zero , or any variable initialized only with ZeroInit) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init_list>                                                                                                                                |
| StaticVariable(name, global, alignment, init) (All other variables)                                                       | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init_list>                                                                                                                               |
| StaticConstant(name, alignment, init) (Linux)                                                                             | &nbsp;&nbsp;&nbsp;&nbsp;.section .rodata<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>                                                                                                                                                                        |
| StaticConstant(name, alignment, init) (MacOS 8-byte aligned constants)                                                    | &nbsp;&nbsp;&nbsp;&nbsp;.literal8<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 8<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>                                                                                                                                                                                            |
| StaticConstant(name, alignment, init) (MacOS 16-byte aligned constants)                                                   | &nbsp;&nbsp;&nbsp;&nbsp;.literal16<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 16<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init><br>&nbsp;&nbsp;&nbsp;&nbsp;.quad 0                                                                                                                                                       |
| **StaticConstant(name, alignment, init) (MacOS string constants)**                                                        | **&nbsp;&nbsp;&nbsp;&nbsp;.cstring<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>**                                                                                                                                                                                                                              |
| Global directive                                                                                                          | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                                                                       | For Linux only: .align <alignment><br>For macOS and Linux: .balign \<alignment>                                                                                                                                                                                                                                  |

#### Formatting Static Initializers

| Static Initializer       | Output                                               |
| ------------------------ | ---------------------------------------------------- |
| IntInit(0)               | .zero 4                                              |
| IntInit(i)               | .long \<i>                                           |
| LongInit(0)              | .zero 8                                              |
| LongInit(i)              | .quad \<i>                                           |
| UIntInit(0)              | .zero 4                                              |
| UIntInit(i)              | .long \<i>                                           |
| ULongInit(0)             | .zero 8                                              |
| ULongInit(i)             | .quad \<i>                                           |
| DoubleInit(d)            | .double \<d><br>OR<br>.quad \<d-interpreted-as-long> |
| ZeroInit(n)              | .zero \<n>                                           |
| **CharInit(0)**          | **.zero 1**                                          |
| **CharInit(i)**          | **.byte \<i>**                                       |
| **UCharInit(0)**         | **.zero 1**                                          |
| **UCharInit(i)**         | **.byte \<i>**                                       |
| **StringInit(s, True)**  | **.asciz "\<s>"**                                    |
| **StringInit(s, False)** | **.ascii "\<s>"**                                    |
| **PointerInit(label)**   | **.quad \<label>**                                   |

#### Formatting Assembly Instructions

| Assembly instruction                      | Output                                                                                 |
| ----------------------------------------- | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                          | mov\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| Movsx(**src_t, dst_t,** src, dst)         | movs **\<src_t>\<dst_t>** &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                       |
| **MovZeroExtend(src_t, dst_t, src, dst)** | **movz\<src_t>\<dst_t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>**                         |
| Cvtsi2sd(t, src, dst)                     | cvtsi2sd\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                    |
| Cvttsd2si(t, src, dst)                    | cvttsd2si\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                   |
| Ret                                       | ret                                                                                    |
| Unary(unary_operator, t, operand)         | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst)      | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| Binary(Xor, Double, src, dst)             | xorpd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Binary(Mult, Double, src, dst)            | mulsd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Idiv(t, operand)                          | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| Div(t, operand)                           | div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                             |
| Cdq(Longword)                             | cdq                                                                                    |
| Cdq(Quadword)                             | cdo                                                                                    |
| Cmp(t, operand, operand)                  | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| Cmp(Double, operand, operand)             | comisd&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                   |
| Jmp(label)                                | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)                   | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)                 | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                              | .L\<label>:                                                                            |
| Push(operand)                             | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                               | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |
| Lea(src, dst)                             | leaq&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                             |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |
| Shr               | shr              |
| DivDouble         | div              |
| And               | and              |
| Or                | or               |

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
| Double        | sd                 |
| **Byte**      | **b**              |

#### Formatting Assembly Operands

| Assembly operand         | Output                     |
| ------------------------ | -------------------------- |
| Reg(AX) 8-byte           | %rax                       |
| Reg(AX) 4-byte           | %eax                       |
| Reg(AX) 1-byte           | %al                        |
| Reg(DX) 8-byte           | %rdx                       |
| Reg(DX) 4-byte           | %edx                       |
| Reg(DX) 1-byte           | %dl                        |
| Reg(CX) 8-byte           | %rcx                       |
| Reg(CX) 4-byte           | %ecx                       |
| Reg(CX) 1-byte           | %cl                        |
| Reg(DI) 8-byte           | %rdi                       |
| Reg(DI) 4-byte           | %edi                       |
| Reg(DI) 1-byte           | %dil                       |
| Reg(SI) 8-byte           | %rsi                       |
| Reg(SI) 4-byte           | %esi                       |
| Reg(SI) 1-byte           | %sil                       |
| Reg(R8) 8-byte           | %r8                        |
| Reg(R8) 4-byte           | %r8d                       |
| Reg(R8) 1-byte           | %r8b                       |
| Reg(R9) 8-byte           | %r9                        |
| Reg(R9) 4-byte           | %r9d                       |
| Reg(R9) 1-byte           | %r9b                       |
| Reg(R10) 8-byte          | %r10                       |
| Reg(R10) 4-byte          | %r10d                      |
| Reg(R10) 1-byte          | %r10b                      |
| Reg(R11) 8-byte          | %r11                       |
| Reg(R11) 4-byte          | %r11d                      |
| Reg(R11) 1-byte          | %r11b                      |
| Reg(SP)                  | %rsp                       |
| Reg(BP)                  | %rbp                       |
| Reg(XMM0)                | %xmm0                      |
| Reg(XMM1)                | %xmm1                      |
| Reg(XMM2)                | %xmm2                      |
| Reg(XMM3)                | %xmm3                      |
| Reg(XMM4)                | %xmm4                      |
| Reg(XMM5)                | %xmm5                      |
| Reg(XMM6)                | %xmm6                      |
| Reg(XMM7)                | %xmm7                      |
| Reg(XMM8)                | %xmm8                      |
| Reg(XMM9)                | %xmm9                      |
| Reg(XMM10)               | %xmm10                     |
| Reg(XMM11)               | %xmm11                     |
| Reg(XMM12)               | %xmm12                     |
| Reg(XMM13)               | %xmm13                     |
| Reg(XMM14)               | %xmm14                     |
| Reg(XMM15)               | %xmm15                     |
| Memory(reg, int)         | \<int>(reg)                |
| Imm(int)                 | $\<int>                    |
| Indexed(reg1, reg2, int) | (\<reg1>, \<reg2>, \<int>) |

## Hello Again, World!

Now we should be able to compile a program that prints a whole strin "Hello, World!", not a character at a time in Chapter 9.

```C
int puts(char *c);
int main(void) {
	puts("Hello, World!");
	return 0;
}
```

To be wild, try compiling this.

```C
#include <stdio.h>
#include <string.h>

int getchar(void);
int puts(char *c);
char *strncat(char *s1, char *s2, unsigned long n);
char *strcat(char *s1, char *s2);
unsigned long strlen(char *s);

static char name[30];
static char message[40] = "Hello, ";

int main(void) {
	puts("Please enter your name: ");
	int idx = 0;
	while (idx < 29) {
		int c = getchar();
		// Treat EOF, null byte, or line break as end of input!
		if (c <= 0 || c == '\n') {
			break;
		}
		name[idx] = c;
		idx = idx + 1;
	}
	name[idx] = '\0';  // add terminating null byte to name

	// Append name to message, leaving space for null byte and exclamation point
	strncat(message, name, 40 - strlen(message) - 2);

	// Append exclamation point
	strcat(message, "!");
	puts(message);
	return 0;
}
```

## Summary

Our compiler now allows programs that work with text.
We learned about string literals and character constants,
and figured out a new way to define constants in the symbol table and the TACKY IR.

In the next chapter, you'll introduce 2 features to dynamically allocate memory: _sizeof_ operator and _void_ type.

### Reference Implementation Analysis

[Chapter 16 Code Analysis](./code_analysis/chapter_16.md)

---

# Chapter 17: Supporting Dynamic Memory Allocation

## Stages of a Compiler

1. **Lexer**
   - Input: Source code
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

We'll implement features to call _malloc_, _calloc_ and _aligned_alloc_, which allocate memory dynamically.
Also, we'll support _free_, which deallocates dynamically allocated memory, and _realloc_ which deallocates one block of memory and reallocates another with the same contents.

So we'll need to support _void_ type.

### The void Type

It comprises an empty set of values; it is an incomplete object type that cannot be completed.

Give a function a _void_ type if it doesn't return anything.

```C
void return_nothing(void) {
	return;
}

void perform_side_effect(void) {
	extern int some_variable;
	some_variable = 100;
}
```

A _void expression_ is an expression whose type is _void_; it has no value, but you can evaluate it for its side effects.

```
flag ? perform_side_effect() : return_nothing();
```

```C
(void) (1 + 1)  // tells the compiler, and human readers, that the value of the
				// expression should be discarded
```

Where to use a void expression?

- As a clause in a conditonal expression
- As a stand-alone expression
- As the first or third clause of a for loop header
- Cast it to a void (not very useful, but still legal)
- Special case: void keyword to specify an empty parameter list in a function declaration, but there's no expression, object or return value of type void.

### Memory Management with void \*

```C
void *malloc(size_t size);
```

The size argument specifies the number of bytes to allocate.
Under the System V x64 ABI, size_t is an alias for unsigned long.

In our compiler, we'll write:

```C
void *malloc(unsigned long size);
```

The _malloc_ doesn't know what type of object will be stored, so it doesn't return a pointer to int, or char, or any other types,
but a pointer to void, so we can cast to any other type before reading or writing memory.
Implicit casts work too:

```C
int *many_ints = malloc(100 * sizeof (int));
```

The _free_ function accepts a _void \*_ argument.

```C
void free(void *ptr);
free(many_ints);		// implicily converting int* back to void* to pass to free.
```

The _calloc_ and _aligned_alloc_ are similar to _malloc_.
Meanwhile _realloc_ accepts a size and a void \* pointer to copy the original contents to a new allocated memory, free the old one, and return the new pointer.

These blocks of memory are objects that we can read and write, much like variables, but the lifetimes are different.

We learned that variables are either automatic storage duration, or static storage duration.
But a block of allocated memory has allocated storage duration, and its lifetime starts when it's allocated and ends when it's deallocated.

Our compiler needs to track all variables with static or automatic storage duration, their size and lifetime in symbol table to preserve space in data section or on the stack.
But for objects wiht allocated storage duration, programmers (and memory management library) are responsible for that.

### Complete and Incomplete Types

An object is _complete_ if we know its size, and _incomplete_ if we don't.
The _void_ type cannot be completed, but incomplete structure types (which we'll implement in Chapter 18) can be.

We use _void_ with pointer since we already know the size of pointer is always 8 bytes, it's an address, so it doens't matter what type of the object it points to.

### The sizeof Operator

The _sizeof_ operator accepts either an expression or the name of a type.

```C
sizeof (long);	// return 8 for long type, the name of type must be parenthesized
sizeof 10.0;	// return 8 for double type, the expression doesn't need to be parentesized

int array[3];
return sizeof array;	// the array doesn't decay to a pointer, so this returns 12 (3-int array)
						// 	rather than 8, the size of a pointer
```

The C standard requires that we **dont** evaludate the operand of a _sizeof_ expression.
Instead, we infer the operand's type and evaluate sizeof at compile time.

```C
// This won't actually call puts, but return 4 because the puts function's return type is int
return sizeof puts("Shouting into the void");

double *null_ptr = 0;	// a null pointer
return sizeof *null_ptr;	// Normally dereferencing null pointers will lead to undefined behavior,
							// but this will never evaluate *null_ptr, so it returns 8 as the type of double
```

## The Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordSizeof | sizeof |

Our lexer already recognizes the _void_ keyword.

## The Parser

### AST

<pre><code>program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer*)
type = Char | SChar | UChar Int | Long | UInt | ULong | Double <strong>| Void</strong>
	| FunType(type* params, type ret)
	| Pointer(type referenced)
	| Array(type element, int size)
storage_class = Static | Extern
block_item = S(statement) | D(declaration)
block = Block(block_item*)
for_init = InitDecl(variable_declaration) | InitExp(exp?)
statement = Return(<strong>exp?</strong>) 
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
	String(string)
	| Var(identifier) 
	| Cast(type target_type, exp)
	| Unary(unary_operator, exp)
	| Binary(binary_operator, exp, exp)
	| Assignment(exp, exp) 
	| Conditional(exp condition, exp, exp)
	| FunctionCall(identifier, exp* args)
	| Dereference(exp)
	| AddrOf(exp)
	| Subscript(exp, exp)
	<strong>| sizeOf(exp)
	| SizeOfT(type)</strong>
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
				| Equal | NotEqual | LessThan | LessOrEqual
				| GreaterThan | GreaterOrEqual
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
	| ConstChar(int) | ConstUChar(int)</pre></code>

The new constant constructors are a little unusual. Character constant like 'a' have type _int_, and our parser will convert them to ConstInt nodes.
Useless as they might seem, we'll need them later when we pad out partially initialized character arrays.

String literals are constants, but they will be handled differently when we process initializers, so it's easier for us to treat String literals as expressions.

### EBNF

<pre><code>
&lt;program&gt;gt; ::= { &lt;declaration&gt; }
&lt;declaration&gt; ::= &lt;variable-declaration&gt; | &lt;function-declaration&gt;
&lt;variable-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; [ "=" &lt;initializer&gt; ] ";"
&lt;function-declaration&gt; ::= { &lt;specifier&gt; }+ &lt;declarator&gt; "(" &lt;param-list&gt; ")" (&lt;block&gt; | ";")
&lt;declarator&gt; ::= "*" &lt;declarator&gt; | &lt;direct-declarator&gt;
&lt;direct-declarator&gt; ::= &lt;simple-declarator&gt; [ &lt;declarator-suffix&gt; ]
&lt;declarator-suffix&gt; ::= &lt;param-list&gt; | { "[" &lt;const&gt; "]" }+ 
&lt;param-list&gt; ::= "(" "void" ")" | "(" &lt;param&gt; { "," &lt;param&gt; } ")"
&lt;param&gt; ::= { &lt;type-specifier&gt; }+ &lt;declarator&gt;
&lt;simple-declarator&gt; ::= &lt;identifier&gt; | "(" &lt;declarator&gt; ")"
&lt;type-specifier&gt; ::= "int" | "long" | "unsigned" | "signed" | "double" | "char" <strong>| "void"</strong>
&lt;specifier&gt; ::= &lt;type-specifier&gt; | "static" | "extern"
&lt;block&gt; ::= "{" { &lt;block-item&gt; } "}"
&lt;block-item&gt; ::= &lt;statement&gt; | &lt;declaration&gt;
&lt;initializer&gt; ::= &lt;exp&gt; | "{" &lt;initializer&gt; { "," &lt;initializer&gt; } [ "," ] "}"
&lt;for-init&gt; ::= &lt;variable-declaration&gt; | [ &lt;exp&gt; ] ";"
&lt;statement&gt; ::= "return" <strong>[ &lt;exp&gt; ]</strong>";" 
	| &lt;exp&gt; ";" 
	| "if" "(" &lt;exp&gt; ")" &lt;statement&gt; ["else" &lt;statement&gt;]
	| &lt;block&gt;
	| "break" ";"
	| "continue" ";"
	| "while" "(" &lt;exp&gt; ")" &lt;statement&gt;
	| "do" &lt;statement&gt; "while" "(" &lt;exp&gt; ")" ";"
	| "for" "(" &lt;for-init&gt; [ &lt;exp&gt; ] ";" [ &lt;exp&gt; ] ")" &lt;statement&gt;
	| ";"
&lt;exp&gt; ::= <strong>&lt;cast-exp&gt;</strong> | &lt;exp&gt; &lt;binop&gt; &lt;exp&gt; | &lt;exp&gt; "?" &lt;exp&gt; ":" &lt;exp&gt;
<strong>&lt;cast-exp&gt; ::= "(" &lt;typename&gt; ")" &lt;cast-exp&gt;
	| &lt;unary-exp&gt;</strong>
&lt;unary-exp&gt; ::= &lt;unop&gt; <strong>&lt;cast-exp&gt;</strong>
	<strong>| "sizeof" &lt;unary-exp&gt;
	| "sizeof" "(" &lt;type-name&gt; ")"</strong>
	| &lt;postfix-exp&gt;
<strong>&lt;type-name&gt; ::= { &lt;type-specifier&gt; }+ [ &lt;abstract-declarator&gt; ] </strong>
&lt;postfix-exp&gt; ::= &lt;primary-exp&gt; { "[" &lt;exp&gt; "]" }
&lt;primary-exp&gt; ::= &lt;const&gt; | &lt;identifier&gt; | "(" &lt;exp&gt; ")" | { &lt;string&gt; }+
	| &lt;identifier&gt; "(" [ &lt;argument-list&gt; ] ")"
&lt;argument-list&gt; ::= &lt;exp&gt; { "," &lt;exp&gt; }
&lt;abstract-declarator&gt; ::= "*" [ &lt;abstract-declarator&gt; ]
	| &lt;direct-abstract-declarator&gt;
&lt;direct-abstract-declarator&gt; ::= "(" &lt;abstract-declarator&gt; ")" { "[" &lt;const&gt; "]" }
	| { "[" &lt;const&gt; "]" }+
&lt;unop&gt; ::= "-" | "~" | "!" | "*" | "&"
&lt;binop&gt; :: = "+" | "-" | "\*" | "/" | "%" | "&&" | "||"
				| "==" | "!=" | "&lt;" | "&lt;=" | "&gt;" | "&gt;=" | "="
&lt;const&gt; ::= &lt;int&gt; | &lt;long&gt; | &lt;uint&gt; | &lt;ulong&gt; | &lt;double&gt; | &lt;char&gt;
&lt;identifier&gt; ::= ? An identifier token ?
&lt;string&gt; ::= ? A string token ?
&lt;int&gt; ::= ? An int token ?
&lt;char&gt; ::= ? A char token ?
&lt;long&gt; ::= ? An int or long token ?
&lt;uint&gt; ::= ? An unsigned int token ?
&lt;ulong&gt; ::= ? An unsigned int or unsigned long token ?
&lt;double&gt; ::= ? A floating-point constant token ?
</pre></code>

### Parser

The _parse_type_ helper function should reject any declaration where the _void_ specifier appears with other type like _long_, or _unsigned_.
The void type can be modified by pointer, array, and function declarators; pointers to void and functions returning void are both perfectly legal, while other ways of using void
are syntactically valid but semantically illegal.

We need to change the parsing logic for \<param-list> as well because the void keyword can be void \*, so you need to look ahead one more token to check if the token right after void is a ).

## Semantic Analysis

### Type Checker

#### Conversions to and from void \*

There are 3 cases where implicit conversions between void \* and other pointer types are legal.

```C
/*
First, you can compare a value of type
void * with another pointer type using == or !=.
The non-void pointer is converted to void *.
*/
int *a;
void *b;
// --snip--
return a == b;

/*
Second, in conditional expression, one clause can be void *
and the other clause have another pointer type.
The non-void pointer is converted to void *.
*/
int *a;
void *b;
// --snip--
return flag ? a : b;

/*
Third, you can implicitly convert to and from void * during assignment.
It covers all the conversions "as if by assignment".
*/
int *a = 0;
void *b = a;

int use_int_pointer(int *a);
void *ptr = 0;
use_int_pionter(ptr);
```

To support implicit conversions to and from void \*,
we extend the two hlper functions get_common_pointer_type and convert_by_assignment.

```
get_common_pointer_type(e1, e2):
	e1_t = get_type(e1)
	e2_t = get_type(e2)
	if e1_t == e2_t:
		return e1_t
	else if is_null_pointer_constant(e1):
		return e2_t
	else if is_null_pointer_constant(e2):
		return e1_t
	else if e1_t == Pointer(Void) and e2_t is a pointer type:
		return Pointer(Void)
	else if e2_t == Pointer(Void) and e1_t is a pointer type:
		return Pointer(Void)
	else:
		fail("Expressions have incompatible types")
```

```
convert_by_assignment(e, target_type):
	if get_type(e) == target_type:
		return e
	if get_type(e) is arithmetic and target_type is arithmetic:
		return convert_to(e, target_type)
	if is_null_pointer_constant(e) and target_type is a pointer type:
		return convert_to(e, target_type)
	if target_type == Pointer(Void) and get_type(e) is a pointer type:
		return convert_to(e, target_type)
	if target_type is a pointer type and get_type(e) == Pointer(Void):
		return convert_to(e, target_type)
	else:
		fail("Cannot convert type for assignment")
```

#### Functions with void Return Types

A function with a void return type must not return an expression.
A function with any other return type must include an expression
when it returns.

```C
// legal
int return_int(void) {
	return 1;
}

// legal
void return_void(void) {
	return;
}

// illegal
int return_int(void) {
	return;
}

// illegal
void return_void(void) {
	return 1;
}

// illegal
void return_void(void) {
	return (void) 1;
}
```

#### Scalar and Non-scalar Types

The AST constructs like the operands of &&, ||, and ! expressions, or the first
operand of a conditional expression, the controlling expressions in loops and if statements, are required
to be scalar types because we need to compare them with 0.

Comparing a pointer or arithmetic values to 0 makes sense, but a non-scalar values doesn't not.

In ealier chapters, we didn't worry about this because array is the only non-scalar type, and we decay them to pointer.
But now we have _void_, so let's enforce these constraints explicitly.

```
is_scalar(t):
	match t with
	| Void -> return False
	| Array(elem_t, size) -> return False
	| FunType(param_ts, ret_t) -> return False
	| _ -> return True
```

Now we can use this to validate controlling conditions and logical operands.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Unary(Not, inner) ->
		typed_inner = typecheck_and_convert(inner, symbols)
		if not is_scalar(get_type(typed_inner)):
		fail("Logical operators only apply to scalar expressions")
		--snip--
```

Cast expressions are a bit different.
We can cast a scalar expression to any scalar type, except for casts between double and pointers (we already tackle this).
And we can cast any type to void.

```
	| Cast(t, inner) ->
		typed_inner = typecheck_and_convert(inner, symbols)
			--snip--
		if t == Void:
			return set_type(Cast(t, typed_inner), Void)
		if not is_scalar(t):
			fail("Can only cast to scalar type or void")
		if not is_scalar(get_type(typed_inner)):
			fail("Cannot cast non-scalar expression to scalar type")
		else:
			return set_type(Cast(t, typed_inner), t)
```

#### Restrictions on Incomplete Types

There are 3 cases we require complete types:

- Add, subtract or subscript pointers.
- Use sizeof operator
- Specify a element type for array.

```
is_complete(t):
	return t != Void

is_pointer_to_complete(t):
	match t with
	| Pointer(t) -> return is_complete(t)
	| _ -> return False
```

```
	| Binary(Add, e1, e2) ->
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 and t2 are arithmetic:
			--snip--
		else if is_pointer_to_complete(t1) and t2 is an integer type:
			--snip--
		else if is_pointer_to_complete(t2) and t1 is an integer type:
			--snip--
		else:
			fail("Invalid operands for addition")
```

We define one more helper function to catch invalid type specifiers that require complete types.

```
validate_type_specifier(t):
	match t with
	| Array(elem_t, size) ->
		if not is_complete(elem_t):
			fail("Illegal array of incomplete type")
		validate_type_specifier(elem_t)
	| Pointer(referenced_t) -> validate_type_specifier(referenced_t)
	| FunType(param_ts, ret_t) ->
		for param_t in param_ts:
			validate_type_specifier(param_t)
		validate_type_specifier(ret_t)
	| _ -> return
```

We’ll call validate_type_specifier to validate type specifiers everywhere they appear: in variable declarations, function declarations, sizeof expressions, and cast expressions.

#### Extra Restrictions on void

- We can't declare void variables or parameters
- We can't dereference pointers to void

#### Conditional Expressions with void Operands

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Conditional(condition, e1, e2) ->
		typed_cond = typecheck_and_convert(condition, symbols)
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		if not is_scalar(get_type(typed_cond)):
			fail("Condition in conditional operator must be scalar")
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 == Void and t2 == Void:
			result_type = Void
		else if t1 and t2 are arithmetic types:
			result_type = get_common_type(t1, t2)
		else if t1 or t2 is a pointer type:
			result_type = get_common_pointer_type(typed_e1, typed_e2)
		else:
			fail("Cannot convert branches of conditional to a common type")
		--snip--
```

#### Existing Validation for Arithmetic Expressions and Comparisons

Earlier, we could assume that every expression had either arithmetic or pointer type.
Now we throw in the void type to the mix.

Here is an example to type check equal expressions to explicitly check that they’re either pointer or
arithmetic types; if they’re anything else, we’ll fail.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| Binary(Equal, e1, e2) ->
		typed_e1 = typecheck_and_convert(e1, symbols)
		typed_e2 = typecheck_and_convert(e2, symbols)
		t1 = get_type(typed_e1)
		t2 = get_type(typed_e2)
		if t1 or t2 is a pointer type:
			common_type = get_common_pointer_type(typed_e1, typed_e2)
		else if t1 and t2 are arithmetic types:
			common_type = get_common_type(t1, t2)
		else:
			fail("Invalid operands to equality expression")
		--snip-
```

We should type check each expression’s operands by accepting valid types instead of rejecting invalid ones.
For example, we should validate that the operands to Multiply and Divide are arithmetic values, instead of making sure they aren’t pointers.

#### sizeof Expressions

A sizeof expression has type size_t, which is unsigned long in our implementation.

```
typecheck_exp(e, symbols):
	match e with
	| --snip--
	| SizeOfT(t) ->
		validate_type_specifier(t)
		if not is_complete(t):
			fail("Can't get the size of an incomplete type")
		return set_type(e, ULong)
	| SizeOf(inner) ->
		typed_inner = typecheck_exp(inner, symbols)
		if not is_complete(get_type(typed_inner)):
			fail("Can't get the size of an incomplete type")
		return set_type(SizeOf(typed_inner), ULong)
```

## TACKY Generation

### TACKY

<pre><code>
program = Program(top_level*)
top_level = Function(identifier, bool global, identifier* params, instruction* body)
		| StaticVariable(identifier, bool global, type t, static_init* init_list)
		| StaticConstant(identifier, type t, static_init init)
instruction = Return(<strong>val?</strong>) 
	| SignExtend(val src, val dst)
	| Truncate(val src, val dst)
	| ZeroExtend(val src, val dst)
	| DoubleToInt(val src, val dst)
	| DoubleToUInt(val src, val dst)
	| IntToDouble(val src, val dst)
	| UIntToDouble(val src, val dst)
	| Unary(unary_operator, val src, val dst)
	| Binary(binary_operator, val src1, val src2, val dst)
	| Copy(val src, val dst)
	| GetAddress(val src, val dst)
	| Load(val src_ptr, val dst)
	| Store(val src, val dst_ptr)
	| Addptr(val src, val index, int scale, val dst)
	| CopyToOffset(val src, identifier dst, int offset)
	| Jump(identifier target)
	| JumpIfZero(val condition, identifier target)
	| JumpIfNotZero(val condition, identifier target)
	| Label(identifier)
	| FunCall(identifier fun_name, val* args, <strong>val?</strong> dst)
val = Constant(const) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | Equal | Not Equal
				| LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
</pre></code>

### Generating TACKY

#### Functions with void Return Types

- We make the destination of the FunCall instruction optional
- We make the value for Return optional

#### Casts to void

We just process the inner expression without emitting any other instructions.
You can return whatever operand you want here; the caller won't use it.

```
emit_tacky(e, instructions, symbols):
	match e with
	| --snip--
	| Cast(void, inner) ->
		emit_tacky_and_convert(inner, instructions, symbols)
		return PlainOperand(Var("DUMMY"))
```

#### Conditional Expressions with void Operands

Let's review our current version

```
	| Conditional(condition, e1, e2) ->
		--snip--
		cond = emit_tacky_and_convert(condition, instructions, symbols)
		instructions.append(JumpIfZero(cond, e2_label))
		dst = make_tacky_variabel(get_type(e), symbols)
		v1 = emit_tacky_and_convert(e1, instructions, symbols)
		instructions.append_all(
			[ Copy(v1, dst),
			  Jump(end),
			  Label(e2_label) ])
		v2 = emit_tacky_and_convert(e2, instructions, symbols)
		instructions.append_all(
			[ Copy(v2, dst),
			  Label(end) ])
		return PlainOperand(dst)
```

This is problematic, as if _e1_ and _e2_ are void expressions, the _e_ has void type as well.
And we shouldn't create _dst_ temporary vairable with type void, and copy anything to it.
Here is the updated pseudocode:

```
	| Conditional(condition, e1, e2) ->
		--snip--
		cond = emit_tacky_and_convert(condition, instructions, symbols)
		instructions.append(JumpIfZero(cond, e2_label))
		if get_type(e) == Void:
			emit_tacky_and_convert(e1, instructions, symbols)
			instructions.append_all(
				[ Jump(end),
				  Label(e2_label) ])
			emit_tacky_and_convert(e2, instructions, symbols)
			instructions.append(Label(end))
			return PlainOperand(Var("DUMMY"))
		else:
			--snip--
```

#### sizeof Expressions

```
	| SizeOf(inner) ->
		t = get_type(inner)
		result = size(t)
		return PlainOperand(Constant(ConstULong(result)))
	| SizeOfT(t) ->
		result = size(t)
		return PlainOperand(Constant(ConstULong(result)))
```

## Assembly Generation

To finish off, we'll generate assembly for Return instructions with no value and FunCall instructions with no destination.

### Assembly

<pre><code>program = Program(top_level*)
assembly_type = Byte | LongWord | Quadword | Double | ByteArray(int size, int alignment)
top_level = Function(identifier name, bool global, instruction* instructions)
	| StaticVariable(identifier name, bool global, int alignment, static_init* init_list)
	| StaticConstant(identifier name, int alignment, static_init init)
instruction = Mov(assembly_type, operand src, operand dst)
		| Movsx(assembly_type src_type, assembly_type dst_type, operand src, operand dst)
		| MovZeroExtend(assembly_type src_type, assembly_type dst_typeoperand src, operand dst)
		| Lea(operand src, operand dst)
		| Cvttsd2si(assembly_type dst_type, operand src, operand dst)
		| Cvtsi2sd(assembly_type src_type, operand src, operand dst)
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
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier) | PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP
	| XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15</pre></code>

### Converting TACKY to Assembly

#### Converting Top-Level TACKY Constructs to Assembly

| TACKY top-level construct                    | Assembly top-level construct                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| -------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_level_defs)                      | Program(top_level_defs + \<all StaticConstant constructs for floating-point constants>)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Function(name, global, params, instructions) | Function(name, global, [<br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first int param type>, Reg(DI), \<first int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second int param type>, Reg(SI), \<second int param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next four integer parameters from registers>, <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first double param type>, Reg(XMM0), \<first double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second double param type>, Reg(XMM1), \<second double param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy next six double parameters from registers><br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<first stack param type>, Memory(BP, 16), \<first stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;Mov(\<second stack param type>, Memory(BP, 24), \<second stack param>), <br>&nbsp;&nbsp;&nbsp;&nbsp;\<copy remaining parameters from stack>] +<br>&nbsp;&nbsp;&nbsp;&nbsp; instructions) |
| StaticVariable(name, global, t, init_list)   | StaticVariable(name, global, \<alignment of t, init_list)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| StaticConstant(name, t, init)                | StaticConstant(name, \<alignment of t>, init)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |

#### Converting TACKY Instructions to Assembly

| TACKY instruction                                                           | Assembly instructions                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Return(val) (Integer)                                                       | Mov(\<val type>, val, Reg(AX))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Return(val) (Double)                                                        | Mov(\<Double>, val, Reg(XMM0))<br>Ret                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Return() **(void)**                                                         | **Ret**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Unary(Not, src, dst) (Integer)                                              | Cmp(\<src type>, Imm(0), src)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                                                                |
| Unary(Not, src, dst) (Double)                                               | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, src, Reg(\<X>))<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(E, dst)                                                                                                                                                                                                                                                                                                                                                                     |
| Unary(Negate, src, dst) (Double negation)                                   | Mov(Double, src, dst)<br>Binary(Xor, Double, Data(\<negative-zero>), dst)<br>And add a top-level constant:<br>StaticConstant(\<negative-zero>, 16, DoubleInit(-0.0))                                                                                                                                                                                                                                                                                                                           |
| Unary(unary_operator, src, dst)                                             | Mov(\<src type>, src, dst)<br>Unary(unary_operator, \<src type>, dst)                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Binary(Divide, src1, src2, dst) (Signed)                                    | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Divide, src1, src2, dst) (Unsigned)                                  | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(AX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Signed)                                 | Mov(\<src1 type>, src1, Reg(AX))<br>Cdq(\<src1 type>)<br>Idiv(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(Remainder, src1, src2, dst) (Unsigned)                               | Mov(\<src1 type>, src1, Reg(AX))<br>Mov(\<src1 type>, Imm(0), Reg(DX))<br>Div(\<src1 type>, src2)<br>Mov(\<src1 type>, Reg(DX), dst)                                                                                                                                                                                                                                                                                                                                                           |
| Binary(arithmetic_operator, src1, src2, dst)                                | Mov(\<src1 type>, src1, dst)<br>Binary(arithmetic_operator, \<src1 type>, src2, dst)                                                                                                                                                                                                                                                                                                                                                                                                           |
| Binary(relational_operator, src1, src2, dst)                                | Cmp(\<src1 type>, src1, src2)<br>Mov(\<dst type>, Imm(0), dst)<br>SetCC(relational_operator, dst)                                                                                                                                                                                                                                                                                                                                                                                              |
| Jump(target)                                                                | Jmp(target)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| JumpIfZero(condition, target) (Integer)                                     | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| JumpIfZero(condition, target) (Double)                                      | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(E, target)                                                                                                                                                                                                                                                                                                                                                                                             |
| JumpIfNotZero(condition, target)                                            | Cmp(\<condition type>, Imm(0), condition)<br>SetCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| JumpIfNotZero(condition, target) (Double)                                   | Binary(Xor, Double, Reg(\<X>), Reg(\<X>))<br>Cmp(Double, condition, Reg(\<X>))<br>JmpCC(NE, target)                                                                                                                                                                                                                                                                                                                                                                                            |
| Copy(src, dst)                                                              | Mov(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Label(identifier)                                                           | Label(identifier)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| FunCall(fun_name, args, dst) (dst is present)                               | \<fix stack alignment><br>\<move arguments to general-purpose registers><br>\<move arguments to XMM registers><br>\<push arguments onto the stack><br>Call(fun_name)<br>\<deallocate arguments\/padding><br>Mov(\<dst type>, \<dst register>, dst)                                                                                                                                                                                                                                             |
| FunCall(fun_name, args, dst) **(dst is absent)**                            | **\<fix stack alignment><br>\<move arguments to general-purpose registers><br>\<move arguments to XMM registers><br>\<push arguments onto the stack><br>Call(fun_name)<br>\<deallocate arguments\/padding>**                                                                                                                                                                                                                                                                                   |
| SignExtend(src, dst)                                                        | Movsx(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Truncate(src, dst)                                                          | Mov(Longword, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ZeroExtend(src, dst)                                                        | MovZeroExtend(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| IntToDouble(src, dst) (char or signed char)                                 | Movsx(Byte, Longword, src, Reg(\<R>))<br>Cvtsi2sd(Longword, Reg(\<R>, dst))                                                                                                                                                                                                                                                                                                                                                                                                                    |
| IntToDouble(src, dst) (int or long)                                         | Cvtsi2sd(\<src type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| DoubleToInt(src, dst) (char or signed char)                                 | Cvttsd2si(\<dst type>, src, Reg(\<R>))<br>Mov(Byte, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                            |
| DoubleToInt(src, dst) (int or long)                                         | Cvttsd2si(\<dst type>, src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| UIntToDouble(src, dst) (unsigned char)                                      | MovZeroExtend(Byte, Longword, src, Reg(\<R>))<br>Cvtsi2sd(Longword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                            |
| UIntToDouble(src, dst) (unsigned int)                                       | MovZeroExtend(Longword, Quadword, src, Reg(\<R>))<br>Cvtsi2s(Quadword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                         |
| UIntToDouble(src, dst) (unsigned long)                                      | Cmp(Quadword, Imm(0), src)<br>JmpCC(L, \<label1>)<br>Cvtsi2sd(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Quadword, src, Reg(\<R1>))<br>Mov(Quadword, Reg(\<R1>), Reg(\<R2>))<br>Unary(Shr, Quadword, Reg(\<R2>))<br>Binary(And, Quadword, Imm(1), Reg(\<R1>))<br>Binary(Or, Quadword, Reg(\<R1>), Reg(\<R2>))<br>Cvtsi2sd(Quadword, Reg(\<R2>), dst)<br>Binary(Add, Double, dst, dst)<br>Label(\<label2>)                                                                |
| DoubleToUInt(src, dst) (unsigned char)                                      | Cvttsd2si(Longword, src, Reg(\<R>))<br>Mov(Byte, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                               |
| DoubleToUInt(src, dst) (unsigned int)                                       | Cvttsd2si(Quadword, src, Reg(\<R>))<br>Mov(Longword, Reg(\<R>), dst)                                                                                                                                                                                                                                                                                                                                                                                                                           |
| DoubleToUInt(src, dst) (unsigned long)                                      | Cmp(Double, Data(\<upper-bound>), src)<br>JmpCC(AE, \<label1>)<br>Cvttsd2si(Quadword, src, dst)<br>Jmp(\<label2>)<br>Label(\<label1>)<br>Mov(Double, src, Reg(\<X>))<br>Binary(Sub, Double, Data(\<upper-bound>),Reg(\<X>))<br>Cvttsd2si(Quadword, Reg(\<X>), dst)<br>Mov(Quadword, Imm(9223372036854775808), Reg(\<R>))<br>Binary(Add, Quadword, Reg(\<R>), dst)<br>Label(\<label2>)<br>And add a top-level constant:<br>StaticConstant(\<upper-bound>, 8, DoubleInit(9223372036854775808.0)) |
| Load(ptr, dst)                                                              | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<dst type>, Memory(\<R>, 0), dst)                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Store(src, ptr)                                                             | Mov(Quadword, ptr, Reg(\<R>))<br>Mov(\<src type>, src, Memory(\<R>, 0))                                                                                                                                                                                                                                                                                                                                                                                                                        |
| GetAddress(src, dst)                                                        | Lea(src, dst)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| AddPtr(ptr, index, scale, dst) (Constant index)                             | Mov(Quadword, ptr, Reg(\<R>))<br>Lea(Memory(\<R>, index \* scale), dst)                                                                                                                                                                                                                                                                                                                                                                                                                        |
| AddPtr(ptr, index, scale, dst) (Variable index and scale of 1, 2, 4, or 8 ) | Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)                                                                                                                                                                                                                                                                                                                                                                                   |
| AddPtr(ptr, index, scale, dst) (Variable index and other scale )            | Mov(Quadword, ptr, Reg(\<R1>))<br>Mov(Quadword, index, Reg(\<R2>))<br>Binary(Mult, Quadword, Imm(scale), Reg(\<R2>))<br>Lea(Indexed(\<R1>, \<R2>, scale), dst)                                                                                                                                                                                                                                                                                                                                 |
| CopyToOffset(src, dst, offset)                                              | Mov(\<src type>, src, PseudoMem(dst, offset))                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator           | Assembly operator |
| ------------------------ | ----------------- |
| Complement               | Not               |
| Negate                   | Neg               |
| Add                      | Add               |
| Subtract                 | Sub               |
| Multiply                 | Mult              |
| Divide (double division) | DivDouble         |

#### Converting TACKY Comparisons to Assembly

| TACKY comparison | Assembly condition code (signed) | Assembly condition code (unsigned or pointer or double ) |
| ---------------- | -------------------------------- | -------------------------------------------------------- |
| Equal            | E                                | E                                                        |
| NotEqual         | NE                               | NE                                                       |
| LessThan         | L                                | B                                                        |
| LessOrEqual      | LE                               | BE                                                       |
| GreaterThan      | G                                | A                                                        |
| GreaterOrEqual   | GE                               | AE                                                       |

#### Converting TACKY Operands to Assembly

| TACKY operand                     | Assembly operand                                                                                 |
| --------------------------------- | ------------------------------------------------------------------------------------------------ |
| Constant(ConstInt(int))           | Imm(int)                                                                                         |
| Constant(ConstUInt(int))          | Imm(int)                                                                                         |
| Constant(ConstLong(int))          | Imm(int)                                                                                         |
| Constant(ConstULong(int))         | Imm(int)                                                                                         |
| Constant(ConstDouble(double))     | Data(\<ident>)<br>And add top-level constant:<br>StaticConstant(\<ident>, 8, DoubleInit(Double)) |
| Constant(ConstChar(int))          | Imm(int)                                                                                         |
| Constant(ConstUChar(int))         | Imm(int)                                                                                         |
| Var(identifier) (Scalar value)    | Pseudo(identifier)                                                                               |
| Var(identifier) (Aggregate value) | PseudoMem(identifier, 0)                                                                         |

#### Converting Types to Assembly

| Source type                                                  | Assembly type                                                   | Alignment                  |
| ------------------------------------------------------------ | --------------------------------------------------------------- | -------------------------- |
| Char                                                         | Byte                                                            | 1                          |
| SChar                                                        | Byte                                                            | 1                          |
| UChar                                                        | Byte                                                            | 1                          |
| Int                                                          | Longword                                                        | 4                          |
| UInt                                                         | Longword                                                        | 4                          |
| Long                                                         | Quadword                                                        | 8                          |
| ULong                                                        | Quadword                                                        | 8                          |
| Double                                                       | Double                                                          | 8                          |
| Pointer(referenced_t)                                        | Quadword                                                        | 8                          |
| Array(element, size) (Variables that are 16 bytes or larger) | ByteArray(\<size of element> \* size, 16)                       | 16                         |
| Array(element, size) (Everything else)                       | ByteArray(\<size of element> \* size, \<alignment of elements>) | Same alignment as elements |

### Replacing Pseudo-Operands

We'll allocate 1 byte on the stack for each Byte object.
We don't need to worry about rounding these to the right alignment, because they're all 1-byte aligned.
And we don't deal with constant strings here, as they are now static constants, recorded in the backend symbol table.

### Fixing Up Instructions

The _movz_ instruction must have a register as its destination, and its source must not be an immediate value.

```asm
movzbl	$10, -4(%rbp)
```

is rewritten as:

```asm
movb 	$10, %r10b
movzbl 	%r10b, %r11b
movl 	%r11b, -4(%rbp)
```

If the source of a _movb_ instruction is an immediate value that can't fit in a single byte, we'll reduce it modulo 256.

```asm
movb	$258, %al
```

is rewritten as:

```asm
movb	$2, %al
```

## Code Emission

#### Formatting Top-Level Assembly Constructs

| Assembly top-level construct                                                                                              | Output                                                                                                                                                                                                                                                                                                           |
| ------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Program(top_levels)                                                                                                       | Printout each top-level construct. <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;_.section .note.GNU-stack,"",@progbits_                                                                                                                                                                    |
| Function(name, global, instructions)                                                                                      | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive>\<br>&nbsp;&nbsp;&nbsp;&nbsp;.text\<br>&nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |
| StaticVariable(name, global, alignment, init_list) (Initialized to zero , or any variable initialized only with ZeroInit) | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.bss<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init_list>                                                                                                                                |
| StaticVariable(name, global, alignment, init) (All other variables)                                                       | &nbsp;&nbsp;&nbsp;&nbsp;\<global-directive><br>&nbsp;&nbsp;&nbsp;&nbsp;.data<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init_list>                                                                                                                               |
| StaticConstant(name, alignment, init) (Linux)                                                                             | &nbsp;&nbsp;&nbsp;&nbsp;.section .rodata<br>&nbsp;&nbsp;&nbsp;&nbsp;\<alignment-directive><br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>                                                                                                                                                                        |
| StaticConstant(name, alignment, init) (MacOS 8-byte aligned constants)                                                    | &nbsp;&nbsp;&nbsp;&nbsp;.literal8<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 8<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>                                                                                                                                                                                            |
| StaticConstant(name, alignment, init) (MacOS 16-byte aligned constants)                                                   | &nbsp;&nbsp;&nbsp;&nbsp;.literal16<br>&nbsp;&nbsp;&nbsp;&nbsp;.balign 16<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init><br>&nbsp;&nbsp;&nbsp;&nbsp;.quad 0                                                                                                                                                       |
| StaticConstant(name, alignment, init) (MacOS string constants)                                                            | **&nbsp;&nbsp;&nbsp;&nbsp;.cstring<br>\<name>:<br>&nbsp;&nbsp;&nbsp;&nbsp;\<init>**                                                                                                                                                                                                                              |
| Global directive                                                                                                          | if global is true:<br>.globl \<identifier><br>Otherwise, omit this directive.                                                                                                                                                                                                                                    |
| Alignment directive                                                                                                       | For Linux only: .align <alignment><br>For macOS and Linux: .balign \<alignment>                                                                                                                                                                                                                                  |

#### Formatting Static Initializers

| Static Initializer   | Output                                               |
| -------------------- | ---------------------------------------------------- |
| IntInit(0)           | .zero 4                                              |
| IntInit(i)           | .long \<i>                                           |
| LongInit(0)          | .zero 8                                              |
| LongInit(i)          | .quad \<i>                                           |
| UIntInit(0)          | .zero 4                                              |
| UIntInit(i)          | .long \<i>                                           |
| ULongInit(0)         | .zero 8                                              |
| ULongInit(i)         | .quad \<i>                                           |
| DoubleInit(d)        | .double \<d><br>OR<br>.quad \<d-interpreted-as-long> |
| ZeroInit(n)          | .zero \<n>                                           |
| CharInit(0)          | .zero 1                                              |
| CharInit(i)          | .byte \<i>                                           |
| UCharInit(0)         | .zero 1                                              |
| UCharInit(i)         | .byte \<i>                                           |
| StringInit(s, True)  | .asciz "\<s>"                                        |
| StringInit(s, False) | .ascii "\<s>"                                        |
| PointerInit(label)   | .quad \<label>                                       |

#### Formatting Assembly Instructions

| Assembly instruction                  | Output                                                                                 |
| ------------------------------------- | -------------------------------------------------------------------------------------- |
| Mov(t, src, dst)                      | mov\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                          |
| Movsx(src_t, dst_t, src, dst)         | movs \<src_t>\<dst_t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| MovZeroExtend(src_t, dst_t, src, dst) | movz\<src_t>\<dst_t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                             |
| Cvtsi2sd(t, src, dst)                 | cvtsi2sd\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                    |
| Cvttsd2si(t, src, dst)                | cvttsd2si\<t> &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                   |
| Ret                                   | ret                                                                                    |
| Unary(unary_operator, t, operand)     | \<unary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                |
| Binary(binary_operator, t, src, dst)  | \<binary_operator>\<t>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                           |
| Binary(Xor, Double, src, dst)         | xorpd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Binary(Mult, Double, src, dst)        | mulsd&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                            |
| Idiv(t, operand)                      | idiv \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                            |
| Div(t, operand)                       | div \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                             |
| Cdq(Longword)                         | cdq                                                                                    |
| Cdq(Quadword)                         | cdo                                                                                    |
| Cmp(t, operand, operand)              | cmp \<t>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                 |
| Cmp(Double, operand, operand)         | comisd&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>                                   |
| Jmp(label)                            | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                                  |
| JmpCC(cond_code, label)               | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>                                        |
| SetCC(cond_code, operand)             | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                      |
| Label(label)                          | .L\<label>:                                                                            |
| Push(operand)                         | pushq&nbsp;&nbsp;&nbsp;&nbsp;\<operand>                                                |
| Call(label)                           | call&nbsp;&nbsp;&nbsp;&nbsp;\<label><br>or<br>call&nbsp;&nbsp;&nbsp;&nbsp;\<label>@PLT |
| Lea(src, dst)                         | leaq&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>                                             |

#### Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ----------------- | ---------------- |
| Neg               | neg              |
| Not               | not              |
| Add               | add              |
| Sub               | sub              |
| Mult              | imul             |
| Shr               | shr              |
| DivDouble         | div              |
| And               | and              |
| Or                | or               |

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
| Double        | sd                 |
| Byte          | b                  |

#### Formatting Assembly Operands

| Assembly operand         | Output                     |
| ------------------------ | -------------------------- |
| Reg(AX) 8-byte           | %rax                       |
| Reg(AX) 4-byte           | %eax                       |
| Reg(AX) 1-byte           | %al                        |
| Reg(DX) 8-byte           | %rdx                       |
| Reg(DX) 4-byte           | %edx                       |
| Reg(DX) 1-byte           | %dl                        |
| Reg(CX) 8-byte           | %rcx                       |
| Reg(CX) 4-byte           | %ecx                       |
| Reg(CX) 1-byte           | %cl                        |
| Reg(DI) 8-byte           | %rdi                       |
| Reg(DI) 4-byte           | %edi                       |
| Reg(DI) 1-byte           | %dil                       |
| Reg(SI) 8-byte           | %rsi                       |
| Reg(SI) 4-byte           | %esi                       |
| Reg(SI) 1-byte           | %sil                       |
| Reg(R8) 8-byte           | %r8                        |
| Reg(R8) 4-byte           | %r8d                       |
| Reg(R8) 1-byte           | %r8b                       |
| Reg(R9) 8-byte           | %r9                        |
| Reg(R9) 4-byte           | %r9d                       |
| Reg(R9) 1-byte           | %r9b                       |
| Reg(R10) 8-byte          | %r10                       |
| Reg(R10) 4-byte          | %r10d                      |
| Reg(R10) 1-byte          | %r10b                      |
| Reg(R11) 8-byte          | %r11                       |
| Reg(R11) 4-byte          | %r11d                      |
| Reg(R11) 1-byte          | %r11b                      |
| Reg(SP)                  | %rsp                       |
| Reg(BP)                  | %rbp                       |
| Reg(XMM0)                | %xmm0                      |
| Reg(XMM1)                | %xmm1                      |
| Reg(XMM2)                | %xmm2                      |
| Reg(XMM3)                | %xmm3                      |
| Reg(XMM4)                | %xmm4                      |
| Reg(XMM5)                | %xmm5                      |
| Reg(XMM6)                | %xmm6                      |
| Reg(XMM7)                | %xmm7                      |
| Reg(XMM8)                | %xmm8                      |
| Reg(XMM9)                | %xmm9                      |
| Reg(XMM10)               | %xmm10                     |
| Reg(XMM11)               | %xmm11                     |
| Reg(XMM12)               | %xmm12                     |
| Reg(XMM13)               | %xmm13                     |
| Reg(XMM14)               | %xmm14                     |
| Reg(XMM15)               | %xmm15                     |
| Memory(reg, int)         | \<int>(reg)                |
| Imm(int)                 | $\<int>                    |
| Indexed(reg1, reg2, int) | (\<reg1>, \<reg2>, \<int>) |

## Summary

In this chapter, we implemented the void type and the sizeof operator.
We learned about the difference between complete and incomplete types and the ways that C programs can use void expressions.

The next chapter, chapter 18, will marks our finish line upon Part II by adding structure types.
Structures are the last language feature you'll implement in the book, and perhaps the most challenging.

### Reference Implementation Analysis

[Chapter 17 Code Analysis](./code_analysis/chapter_17.md)
