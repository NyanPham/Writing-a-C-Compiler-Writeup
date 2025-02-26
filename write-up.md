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
operand = Imm(int) | Register
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
