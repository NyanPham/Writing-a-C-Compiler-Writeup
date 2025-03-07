# Chapter 1: A Minimal Compiler 

### Stages of a Compiler

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

**AST**
```AST
program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int)
```

**EBNF**
```EBNF
<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int>
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?
```

**Parser**
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

**Assembly**
```
program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst) | Ret
operand = Imm(int val) | Register
```

**Converting AST Nodes to Assembly**

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

**Formatting Top-Level Assembly Constructs**

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> &nbsp;&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

**Formatting Assembly Instructions**

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl &nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |

**Formatting Assembly Operands**

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

### Stages of a Compiler

1. **Lexer**
    - Input: Source code (program.c)
    - Output: Token list
2. **Parser**
    - Input: Token list
    - Output: Abstract Syntax Tree (AST)
3. **Tacky Generation**
	- Input: AST
	- Output: TAC IR (Tacky)
4. **Assembly Generation**
    - Input: Tacky
    - Output: Assembly code
	- Passes:
		1. Converting TACKY to Assembly
		2. Replacing pseudoregsiters
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

**AST**
<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int) <strong>| Unary(unary_operator, exp)</strong>
<strong>unary_operator = Complement | Negate</strong></pre></code>

**EBNF**
<pre><code>&lt;program&gt; ::= &lt;function&gt;
&lt;function&gt; ::= "int" &lt;identifier&gt; "(" "void" ")" "{" &lt;statement&gt; "}"
&lt;statement&gt; ::= "return" &lt;exp&gt; ";"
&lt;exp&gt; ::= &lt;int&gt; <strong>| &lt;unop&gt; &lt;exp&gt; | "(" &lt;exp&gt; ")"</strong>
<strong>&lt;unop&gt; ::= "-" | "~"</strong>
&lt;identifier&gt; ::= ? An identifier token ?
&lt;int&gt; ::= ? A constant token ?</pre></code>

**Parser**

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

**TACKY**
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

### TACKY representation of Unary Expressions **
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

**Assembly**
<pre><code>program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
		<strong>| Unary(unary_operator, operand dst)</strong>
		<strong>| AllocateStack(int)</strong>
		| Ret
<strong>unary_operator = Neg | Not</strong>
operand = Imm(int) <strong>| Reg(reg) | Pseudo(identifier) | Stack(int)</strong>
<strong>reg = AX | R10</strong></pre></code>

**Converting Top-Level TACKY Constructs to Assembly**

| TACKY top-level construct | Assembly top-level construct |
| -------- | ------------------ |
| Program(function_definition) | Program(function_definition) | 
| Function(name, instructions) | Function(name, instructions) |

**Converting TACKY Instructions to Assembly**

| TACKY instruction | Assembly instructions |
| -------- | ------------------ |
| Return(val) | Mov(val, Reg(AX))<br>Ret| 
| Unary(unary_operator, src, dst) | Mov(src, dst)<br>Unary(unary_operator, dst)  |

**Converting TACKY Arithmetic Operators to Assembly**

| TACKY operator | Assembly operator |
| -------- | ------------------ |
| Complement | Not | 
| Negate | Neg  |

**Converting TACKY Operands to Assembly**

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

**Formatting Top-Level Assembly Constructs**

| Assembly top-level construct | Ouput |
| --------------------- | ------------ |
| Program(function_definition) |  Printout the function definition <br> On Linux, add at the end of file <br> nbsp;&&nbsp;&nbsp;&nbsp;*.section .note.GNU-stack,"",@progbits* | 
| Function(name, instructions) | &nbsp;&nbsp;&nbsp;&nbsp;.globl \<name> <br> \<name>: <br> &nbsp;&nbsp;&nbsp;&nbsp;push&nbsp;&nbsp;&nbsp;&nbsp;%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;movq&nbsp;&nbsp;&nbsp;&nbsp;%rsp,%rbp<br>&nbsp;&nbsp;&nbsp;&nbsp;\<instructions> |

**Formatting Assembly Instructions**

| Assembly instruction | Output |
| ------------------ | --------- |
| Mov(src, dst) | movl&nbsp;&nbsp;&nbsp;&nbsp;\<src>, \<dst> |
| Ret | ret |
| Unary(unary_operator, operand) | \<unary_operator>&nbsp;&nbsp;&nbsp;&nbsp;\<operand>|
| AllocateStack(int) | subq&nbsp;&nbsp;&nbsp;&nbsp;$\<int>, %rsp |

**Formatting Names for Assembly Operators**

| Assembly operator | Instruction name |
| ------------------ | --------- |
| Neg | negl |
| Not | notl | 

**Formatting Assembly Operands**

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
