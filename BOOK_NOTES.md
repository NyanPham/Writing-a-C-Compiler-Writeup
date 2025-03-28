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

| Token | Regular expression |
| ------- | -------------------- |
| Identifier | [a-zA-Z_]\w\*\b|
| Constant | [0-9]+\b |
| *int* keyword | int\b |
| *void* keyword | void\b |
| *return* keyword | return\b |
| Open parenthesis | \( |
| Close parenthesis | \) |
| Open brace | { |
| Close brace | } |
| Semicolon | ; |

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

*Zephyr Abstract Syntax Description Language (ASDL)* is used to represent AST and *extended Backus Naur form (EBNF)* to represent formal grammar.

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

| AST Node | Assembly construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, body) | Function(name, instructions) |
| Return(exp) | Mov(exp, Register) <br> Ret|
| Constant(int) | Imm(int) |

## Code Emission

On macOS, we add _ in front of the function name. For example, *main* becomes *_main*. Don't do this on Linux.

On Linux, add this at the end of the file:
```	
	.section .note.GNU-stack,"",@progbits
```

####  Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

####  Formatting Assembly Instructions

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |

####  Formatting Assembly Operands

| Assembly operand | Output |
| ------------------ | --------- |
| Register | %eax |
| Imm(int) | $\<int> | 

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

### Reference implementation Analysis
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

We will not yet implement the decrement operator *--*, but we need to let the lexer regonize it so it won't mistake -- for -(-

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

***Notes:***
- The first Constant is of AST, and the second is of TACKY.
- The first Unary is of AST, and the second is of TACKY.

### Generating Names for Temporary Variables
We keep a counter, and increment it whenever we use it to create a unique name.
Example: tmp.0
This won't conflict with user-defined identifier

### TACKY representation of Unary Expressions
| AST | TACKY |
| ----- | ----- |
| Return(Constant(3)) | Return (Constant(3)) |
| Return(Unary(Complement, Constant(2))) | Unary(Complement, Constant(2), Var("tmp.0")) <br>Return(Var("tmp.0"))|
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

| TACKY top-level construct | Assembly top-level construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction | Assembly instructions |
| -------- | ------------------ |
| Return(val) | Mov(val, Reg(AX))<br>Ret| 
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst)  |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------- | ------------------ |
| Complement | Not | 
| Negate | Neg  |

#### Converting TACKY Operands to Assembly

| TACKY operand | Assembly operand |
| -------- | ------------------ |
| Constant(int) | Imm(int) | 
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

####  Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> nbsp;&&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

####  Formatting Assembly Instructions

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>|
| AllocateStack(int) | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp |

####  Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ------------------ | --------- |
| Neg | negl |
| Not | notl | 

####  Formatting Assembly Operands

| Assembly operand | Output |
| ------------------ | --------- |
| Reg(AX) | %eax |
| Reg(R10) | %r10d |
| Stack(int) | <int>(%rbp) |
| Imm(int) | $\<int> | 

## Summary
We added one more stage in the compiler: TACKY, our name for TAC IR. 


## Additional Resources

**Two's Complement:**
- [“Two’s Complement”](https://www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html)
- [Chapter 2 of The Elements of Computing Systems](https://www.nand2tetris.org/course)

### Reference implementation Analysis
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
| Star | * |
| Slash | / |
| Percent | / |

We already added HYPHEN for *-* operator in the previous chapter.
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

For our problem about, without any parantehesis,  `1 + 2 * 3`, it's parsed as following:
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
| \* | 50 |
| / | 50 |
| % | 50 |
| + | 45 |
| - | 45 |

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

***Notes:***
In C standard, subexpressions of e1 and e2 are usually unsequenced. They can be evaluated in any order.

## Assembly Generation
We need new instructions to represent binary operators in Assembly.
For add, subtract, multiply, they have the same form:

```
op	src, dst
```

However, divide and remainder don't follow the same form as we're dealing with dividend, divisor, quotient and remainder.
We need to sign extend the *src* into rdx:rax before performing the division using `cdq` instruction.
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

| TACKY top-level construct | Assembly top-level construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction | Assembly instructions |
| -------- | ------------------ |
| Return(val) | Mov(val, Reg(AX))<br>Ret| 
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst)  |
| **Binary(Divide, src1, src2, dst)** | **Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst)** |
| **Binary(Remainder, src1, src2, dst)** | **Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst)** |
| **Binary(binary_operator, src1, src2, dst)** | **Mov(src1, dst)<br>Binary(binary_operator, src2, dst)** |


#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------- | ------------------ |
| Complement | Not | 
| Negate | Neg  |
| **Add** | **Add**  |
| **Subtract** | **Sub**  |
| **Multiply** | **Mult** |

#### Converting TACKY Operands to Assembly

| TACKY operand | Assembly operand |
| -------- | ------------------ |
| Constant(int) | Imm(int) | 
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters
We added two instruction that have operands: *Binary* and *Idiv*.
Treat them like Mov, Unary.

### Fixing Up Instructions

The *idiv* can't take a constant operand, so we copy the constant into our scratch register R10.

So this instruction 
```
idivl	$3
```
is fixed up into:
```
movl 	$3, %r10d
idivl	%r10d
```

The *add* and *sub* instructions cannot memory addresses as both operands, similar to the *mov* instruction.
From this:
```
addl	-4(%rbp), -8(%rbp)
```
is fixedup into:
```
movl 	-4(%rbp), %r10d
addl	%r10d, -8(%rbp)
```

The *imul* instruction cannot use memory address as its destination. We'll dedicate R11 to fix up destination, also for later chapters.
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

####  Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> nbsp;&&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

####  Formatting Assembly Instructions

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>|
| **Binary(binary_operator, src, dst)** | **\<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst>** |
| **Idiv(operand)** | **idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>** |
| **Cdq** | **cdq** |
| AllocateStack(int) | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp |

####  Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ------------------ | --------- |
| Neg | negl |
| Not | notl | 
| **Add** | **addl** | 
| **Sub** | **subl** | 
| **Mult** | **imull** | 

####  Formatting Assembly Operands

| Assembly operand | Output |
| ------------------ | --------- |
| Reg(AX) | %eax |
| **Reg(DX)** | **%eax** |
| Reg(R10) | %r10d |
| **Reg(R11)** | **%r11d** |
| Stack(int) | <int>(%rbp) |
| Imm(int) | $\<int> | 

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

### Reference implementation Analysis
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
- Or  (\|\|)

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

This table is spaced enough for other binary operators implemented in Extra Credit section in chapter 3.
Extend both parse_unop and parse_binop.

## TACKY Generation

Relational operators are converted to TACKY the same way as implemented binary operators.
However, for && and \|\|, we cannot do this as our TACKY evaluates both expressions in binary, while && and \|\| sometimes allow to skip the 2nd expression.
That's why we're going to add *unconditional jump* and *conditional jump* to support the two *short-circuit* operators.

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
- **JumpIfNotZero** does the opposite to the *JumpIfZero*
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

| TACKY top-level construct | Assembly top-level construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction | Assembly instructions |
| -------- | ------------------ |
| Return(val) | Mov(val, Reg(AX))<br>Ret| 
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst) |
| **Unary(Not, src, dst)** | **Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst)** |
| Binary(Divide, src1, src2, dst) | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst) |
| Binary(Remainder, src1, src2, dst) | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst) |
| Binary(binary_operator, src1, src2, dst) | Mov(src1, dst)<br>Binary(binary_operator, src2, dst) |
| **Binary(relational_operator, src1, src2, dst)** | **Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst)** |
| **Jump(target)** | **Jmp(target)** |
| **JumpIfZero(condition, target)** | **Cmp(Imm(0), condition)<br>SetCC(E, target)** |
| **JumpIfNotZero(condition, target)** | **Cmp(Imm(0), condition)<br>SetCC(NE, target)** |
| **Copy(src, dst)** | **Mov(src, dst)** |
| **Label(identifier)** | **Label(identifier)** |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------- | ------------------ |
| Complement | Not | 
| Negate | Neg  |
| Add | Add  |
| Subtract | Sub  |
| Multiply | Mult |

#### Converting TACKY Comparisons to Assembly
| TACKY comparison | Assembly condition code |
| -------- | ------------------ |
| **Equal** | **E** | 
| **NotEqual** | **NE**  |
| **LessThan** | **L**  |
| **LessOrEqual** | **LE**  |
| **GreaterThan** | **G** |
| **GreaterOrEqual** | **GE** |

#### Converting TACKY Operands to Assembly

| TACKY operand | Assembly operand |
| -------- | ------------------ |
| Constant(int) | Imm(int) | 
| Var(identifier) | Pseudo(identifier) |

### Replacing Pseudoregisters
We added two instruction that have operands: *Cmp* and *SetCC*.
Update this pass to replace any pseudoregisters used in these.

### Fixing Up Instructions

The *cmp* instructions, like *mov*, *add* and *sub*, cannot have memory addresses for both operands.
So the instruction
```
cmpl	-4(%rbp), -8(%rbp)
```
is fixed up into
```
movl 	-4(%bp), %r10d
cmpl	%r10d, -8(%rbp)
```

Also, *cmp*, similarly to *sub*, cannot have destination as constant.
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

####  Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

####  Formatting Assembly Instructions

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>|
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Idiv(operand) | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand> |
| Cdq | cdq |
| AllocateStack(int) | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp |
| **Cmp(operand, operand)** | **cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand>** |
| **Jmp(label)** | **jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>** |
| **JmpCC(cond_code, label)** | **j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label>** |
| **SetCC(cond_code, operand)** | **set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>** |
| **Label(label)** | **.L\<label>:** |

**Notes**:
- *jmp* and *.L\<label>* don't have size suffixes as they don't take operands.
- *jmpcc* and *setcc* do need suffixes to indicate the size of the condition they test.

####  Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ------------------ | --------- |
| Neg | negl |
| Not | notl | 
| Add | addl | 
| Sub | subl | 
| Mult | imull | 

**Instruction Suffixes for Condition Codes**

| Condition code | Instruction suffix |
| ------------------ | --------- |
| **E** | **e** |
| **NE** | **ne** | 
| **L** | **l** | 
| **LE** | **le** | 
| **G** | **g** | 
| **GE** | **ge** | 

####  Formatting Assembly Operands

| Assembly operand | Output |
| ------------------ | --------- |
| Reg(AX) 4-byte | %eax |
| **Reg(AX) 1-byte** | **%al**|
| Reg(DX) 4-byte | %eax |
| **Reg(DX) 1-byte** | **%dl** |
| Reg(R10) 4-byte | %r10d |
| **Reg(R10) 1-byte** | **%r10b** |
| Reg(R11) 4-byte | %r11d |
| **Reg(R11) 1-byte** | **%r11b** |
| Stack(int) | <int>(%rbp) |
| Imm(int) | $\<int> | 


## Summary
Relational and short-circuiting operators are important to let programs branch and make decisions.
The implementation of this step is a foundation for our *if* statement and loops later chapters.

## Additional Resources

- [A Guide to Undefined Behavior in C and C++, Part 1](https://blog.regehr.org/archives/213)
- [With Undefined Behavior, Anything Is Possible](https://raphlinus.github.io/programming/rust/2018/08/17/undefined-behavior.html)

### Reference implementation Analysis
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
An expression that can appear on the left side of an assignment is called *lvalue*.<br> For now, our only *lvalue* is variable.

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

To parse_block_item, peek the first token. If it's the *int* keyword, it's a declaration, a statement otherwise.

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
| **=**  | **1** |

**Note**:
- Our Parser allows any expression can be on the left handside of an assignment.
- Our Semantic Analysis stage will check if the expression is actually a valid *lvalue* expression.
- We check for the validity of *lvalue* not in Parser but in Semantic Analysis because we will support more complex lvalues later.

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

| TACKY top-level construct | Assembly top-level construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, instructions) | Function(name, instructions) |

#### Converting TACKY Instructions to Assembly

| TACKY instruction | Assembly instructions |
| -------- | ------------------ |
| Return(val) | Mov(val, Reg(AX))<br>Ret| 
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst) |
| Unary(Not, src, dst) | Cmp(Imm(0), src)<br>Mov(Imm(0), dst)<br>SetCC(E, dst) |
| Binary(Divide, src1, src2, dst) | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(AX), dst) |
| Binary(Remainder, src1, src2, dst) | Mov(src1, Reg(AX))<br>Cdq<br>Idiv(src2)<br>Mov(Reg(DX), dst) |
| Binary(binary_operator, src1, src2, dst) | Mov(src1, dst)<br>Binary(binary_operator, src2, dst) |
| Binary(relational_operator, src1, src2, dst) | Cmp(src1, src2)<br>Mov(Imm(0), dst)<br>SetCC(relational_operator, dst) |
| Jump(target) | Jmp(target) |
| JumpIfZero(condition, target) | Cmp(Imm(0), condition)<br>SetCC(E, target) |
| JumpIfNotZero(condition, target) | Cmp(Imm(0), condition)<br>SetCC(NE, target) |
| Copy(src, dst) | Mov(src, dst) |
| Label(identifier) | Label(identifier) |

#### Converting TACKY Arithmetic Operators to Assembly

| TACKY operator | Assembly operator |
| -------- | ------------------ |
| Complement | Not | 
| Negate | Neg  |
| Add | Add  |
| Subtract | Sub  |
| Multiply | Mult |

#### Converting TACKY Comparisons to Assembly
| TACKY comparison | Assembly condition code |
| -------- | ------------------ |
| Equal | E | 
| NotEqual | NE  |
| LessThan | L  |
| LessOrEqual | LE  |
| GreaterThan | G |
| GreaterOrEqual | GE |

#### Converting TACKY Operands to Assembly

| TACKY operand | Assembly operand |
| -------- | ------------------ |
| Constant(int) | Imm(int) | 
| Var(identifier) | Pseudo(identifier) |

## Code Emission
Remains unchanged as there are no modifications in the Assembly stage.

####  Formatting Top-Level Assembly Constructs

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

####  Formatting Assembly Instructions

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>|
| Binary(binary_operator, src, dst) | \<binary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Idiv(operand) | idivl&nbsp;&nbsp;&nbsp;&nbsp;\<operand> |
| Cdq | cdq |
| AllocateStack(int) | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp |
| Cmp(operand, operand) | cmpl&nbsp;&nbsp;&nbsp;&nbsp;\<operand>, \<operand> |
| Jmp(label) | jmp&nbsp;&nbsp;&nbsp;&nbsp;.L\<label> |
| JmpCC(cond_code, label) | j\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;.L\<label> |
| SetCC(cond_code, operand) | set\<cond_code>&nbsp;&nbsp;&nbsp;&nbsp;\<operand> |
| Label(label) | .L\<label>: |

####  Formatting Names for Assembly Operators

| Assembly operator | Instruction name |
| ------------------ | --------- |
| Neg | negl |
| Not | notl | 
| Add | addl | 
| Sub | subl | 
| Mult | imull | 

#### Instruction Suffixes for Condition Codes

| Condition code | Instruction suffix |
| ------------------ | --------- |
| E | e |
| NE | ne | 
| L | l | 
| LE | le | 
| G | g | 
| GE | ge | 

####  Formatting Assembly Operands

| Assembly operand | Output |
| ------------------ | --------- |
| Reg(AX) 4-byte | %eax |
| Reg(AX) 1-byte | %al|
| Reg(DX) 4-byte | %eax |
| Reg(DX) 1-byte | %dl |
| Reg(R10) 4-byte | %r10d |
| Reg(R10) 1-byte | %r10b |
| Reg(R11) 4-byte | %r11d |
| Reg(R11) 1-byte | %r11b |
| Stack(int) | <int>(%rbp) |
| Imm(int) | $\<int> | 

## Extra Credit: Compound Assignment, Increment, and Decrement
Add support for compound assignments: 
+=, -=, *=, /=, %=

We've also implemented Extra Credit in chapter 3, so we also support: 
&=, |=, ^=, <<=, >>=

Increment and decrement also can be implemented: 
++, --

Note that ++ and -- can be either prefix or postfix their operands.

## Summary
This chapter is a milestone as we added new kind of statement and first construct that has side effect.
Also the Semantic Analysis stage is implemented to make sure the program makes sense. 

### Reference implementation Analysis
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

AST definition of the **If** construct doesn't include *else if* because an if statement can have at most one else clause. An else if clause is an else with another if statement.

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
The grammar itself doesn't tell this. Luckily, this is a famous quirk that has it own name: *dangling else ambiguity*.

The dangling else ambiguity cause problems for parser generators, not our handwritten recursive descent parser. 
When we parse the if statement, if we see an else statement right after it, attach it the the closest if.

Now, how about **condition expression**?
The expression has its form: `condition ? exp : exp`
We treat it as a binary expression, whose operator is the whole *? exp :*.
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
| **?** | **3** |
| =  | 1 |

## Semantic Analysis

### Variable Resolution
Extend the *resolve_statement* and *resolve_exp* to handle new constructs. 

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
If we can support *If Statements* and *Conditional Expressions*, we can move forward to add support for **Goto** and **Labeled Statements**.
Goto is like Jump in Assembly, whereas Labeled Statements specify the target for Goto.
A new pass in Semantic Analysis is needed to check for the errors like using the same label for two labeled statements. 

## Summary
Congratulations! We've just implemented the first control structures in our compiler. However, we are limited by having a single statement in each clause.
We'll extend to use compound statement, which is also a single statement but contains several inner statements in the next chapter.

### Reference implementation Analysis
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
We won't touch *Lexer* or the **CodeGen** stages at all.

Compound statements are to:
- Group statements and declarations into a single unit.
- Delineate the different *scopes* within a function.

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

Finally, the function *copy_variable_map* simply copies the whole variable map, but set from_current_block to false to each entry.

## TACKY Generation

### TACKY
No modifications needed!

### Generating TACKY
Straightforwardly, to emit TACKY for a compound statement, we emit TACKY for each block item in it. This should use the same function with the function body.

## Assembly Generation
We do not change our TACKY AST, so Assembly stage stays intact.

## Summary
We've supported a new kind of statements and extended our Variable Resolution pass to be a little more sophisticated. It's important for us to implement loops in the next chapter. 

### Reference implementation Analysis
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

We need to update our lexer, parser to support all 5 new statements and add a new semantic analysis pass called *loop labeling*.

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
We can write helper function to parse optional expressions, specifically two optional expressions in a *for* loop header.

## Semantic Analysis

### Variable Resolution

For *while* and *do* loops, we can simply recursively resolve_statement of their substatements and subexpressions.
For *break* and *continue*, we don't do anything to them as they have no substatements nor subexpressions.

The *for* loop is more complicated as its header introduces a new variable scope.

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

*resolve_optional_exp* is omitted, as it's simple as: it calls *resolve_exp* if the expression is present, does nothing otherwise.

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
Both *break* and *continue* are just jump instructions in TACKY (and Assembly). Question is where to jump to!
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
*break* can be used to break out of a switch, while *continue* is only for loops.
Let's create another pass in the Semantic Analysis stage to collect all the cases in a switch statements.

## Summary
We added three loop statements, break and continue statements. We also had a new Semantic Analysis pass to associate each *break* and *continue* to their enclosing loops.

### Reference implementation Analysis
[Chapter 8 Code Analysis](./code_analysis/chapter_8.md)