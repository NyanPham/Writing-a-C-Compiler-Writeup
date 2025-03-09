There are several code that is ommited in the first chapter, mainly to set up our own compiler.

# Table of Contents
- [Compiler Driver](#compiler-driver)
  - [Main driver functions](#main-driver-functions)
  - [Define stages for the compiler](#define-stages-for-the-compiler)
- [Source code](#source-code)
  - [Token](#token)
  - [Lexer](#lexer)
  - [AST](#ast)
  - [Parser](#parser)
  - [Assembly](#assembly)
  - [CodeGen](#codegen)
  - [Emit](#emit)
  - [Optional](#optional)
  - [Output](#output)

--- 

# Compiler Driver

We need to set up our compiler driver to support different stages in command line. For example, we can parse the file with command `compiler.exe program.c --parse`

There are some helper functions in compiler driver file named main

```
get_current_platform:
    return "OS_X" or "Linux"
```

```
validate_extension(filename):
    ext = get the extension of filename
    if ext == '.c' or '.h'
        do nothing
    else:
        fail('Invalid extensions')
```

```
replace_extension(filename, new_ext):
    base = remove the ext from filename
    return base + new_ext
```


```
run_command(cmd, arg_list):
    command_string = combine 'cmd' with 'args_list' into a single command
    execute the command_string
```


## Main driver functions

**Preprocess**
```
preprocess(src):
    validate_extension(src)
    output = replace_extension(src, '.i')
    run_command('gcc', ["-E", "-P", src, "-o", output]
    return output

```

**Compile**
```
compile(stage, preprocessed_src):
    call Compiler.compile(stage, preprocessed_src)
    run_command('rm', [preprocessed_src]) // Remove the preprocessed source file
    return replace_exension(preprocessed_src, '.s')
```

**Assemble and link**

```
assemble_and_link(src, cleanup = true):
    asm_file = replace_extension(src, '.s')
    output_file = remove extension from src
    run_command('gcc', [asm_file, '-o', output_file])
    if cleanup:
        run_command('rm', [asm_file])
```

**Driver**
```
driver(target, debug, stage, src):
    set platform to target (OX_X | Linux)
    preprocessed_name = preprocess(src)
    asm_name = compile(stage, preprocessed_name)
    if stage === Executable
        assemble_and_link(asm_name, true) // cleanup should be false for debugging
```

## Define stages for the compiler
- Lexing: '--lex'
- Parsing: '--parse'
- Code Generation: '--codegen'
- Assembly: '-s'

Without any specified stage, it will be Executable stage

--- 

# Token: 
Define an enum of TokenType. Can also have class Token to provide some methods on token, such as getting value, print out the token or get the position

---

# Lexer: 
- Create a type call token_def with reg as regular expression to recognize a token and convert function to turn string into a Token.
- Function convert_identifier to check whether it's a keyword, else it's an ordinary identifier. Example:
```
convert_identifier(identifier):
    if identifier == "int": return TokenType.KWInt
    else if identifier == "return": return TokenType.KWReturn
    else if identifier == "void": return TokenType.KWVoid
    else: return identifier
```

- We also have a function to convert number int into a Token with type as TokenType.Constant
```
convert_int(n):
    return Token(TypeToken.Constant, n)
```

- A map for the regular expression and its converter should be made. For example:
```
LexRules = [
 {
    reg: /[A-Za-z_][A-Za-z0-9_]*\b/,
    converter: convert_identifier,
 },
 {
    reg: /[0-9]+\b/,
    converter: convert_int,
 },
 {
    reg: /\(/,
    converter: return Token(TokenType.OpenParen),
 },
 ...
]
```

- Function trim_whitespace to eliminate whitespace from the beginning of the input.

- A main lexing function. Here is a more detailed version:
```
token(input):
    if input == "":
        return null

    if input starts with whitespace, trim it
    Otherwise, lex next token:
        For each regex in lexRules, return all matches
        Find the longest match
        Run the converter of that rule to get token
        Add the position of the input to consume the tokenized substring
        
        return token

lex(input): 
    tokens = []

    while true:
        token = token(input)
        if token is not null:
            tokens.push(token)
        else:
            break
    
    return tokens
```

---

# AST
We have a list tokens from lexer. Before parsing, we need to define the node constructs in AST. The first chapter is simple start with a minimal set of node type.

```
enum NodeType = Program, FunctionDefinition, Return, Constant

type Expression = Constant
type Statement = Return

Constant(NodeType.Constant, int value)
Return(NodeType.Return, Expression val)
FunctionDefinition(NodeType.FunctionDefinition, string name, Statement body)
Program(NodeType.Program, FunctionDefinition fn_def)
```

---

# Parser
Optionally, we can define some helper functions. For example: 
**Raise Error**
```
raise_error(expected, actual):
    fail("Expected {expected} but found {actual}")
```

And one core function to remove next token and verifiy it's what we expect, if not, raise and error. Please see the function *expect* in the BOOK_NOTES.

Here are some unmentioned functions in the BOOK_NOTES:

```
take_token():
    token = lexer.token() // The lexer holds the input string, so we process the next token
    if token == null:
        return Token(null, null, null)
    else:
        return token
```

```
parse_identifier():
    curr_token = take_token()    
    if curr_token.type == TokenType.Identifier:
        return curr_token.value
    else:
        raise_error("an identifier", curr_token.type)
```

```
parse_int():
    curr_token = take_token()
    
    if curr_token.type == TokenType.Constant
        return AST.Constant(curr_token.value)
    else:
        raise_error("a constant", curr_token.type)
```

```
parse_exp():
    return parse_int()
```

```
parse_function_definition():
    expect(TokenType.KWInt)
    
    fun_name = parse_identifier()
    
    expect(TokenType.OpenParen)
    expect(TokenType.KWVoid)
    expect(TokenType.CloseParen)
    
    expect(TokenType.OpenBrace)
    
    statement = parse_statement()
    
    expect(TokenType.CloseBrace)

    return AST.FunctionDefinition(fun_name, statement)
```

```
parse_program():
    fn_def = parse_function_definition()
    return AST.Program(fn_def)
```

```
parse(input):
    lexer.input = input
    program = parse_program()
    
    if is_EOF():
        return program
    else:
        fail("Unexpected tokens after function definition")
```

---

# Assembly

```
enum NodeType = Program, Function, Imm, Register, Mov, Ret

type Operand = Imm | Register
type Instruction = Mov | Ret

Mov(NodeType.Mov, Operand src, Operand dst)
Ret(NodeType.Ret)

Imm(NodeType.Imm, int val)
Register(NodeType.Register)
Function(NodeType.Function, string name, Instruction* instructions)
Program(NodeType.Program, Function func)
```

---

# CodeGen

```
convert_exp(AST.Constant constant):
	return Assembly.Imm(constant.val)
	
convert_statement(AST.Return stmt):
	v = convert_exp(stmt.val)
	return [
		Mov(v, Register),
		Ret()
	]

convert_function(AST.FunctionDefinition fn_def):
	instructions = convert_statement(fn_def.body)
	
	return Assembly.Function(fn_def.name, instructions)
	
gen(AST.Program prog):
	return Assembly.Program(convert_function(prog.fn_def))
```


# Emit

```
show_operand(Assembly.Operand operand):
	match operand.type:
		case Register:
			return '%eax'
		case Imm:
			return '$' + operand.val
		
show_label(string name):
	if on OS_X:
		return "_" + name
	else:
		return name
	
emit_instruction(Assembly.Instruction inst):
	match inst.type:
		case Mov:
			return '\tmovl {show_operand(inst.src)}, {show_operand(inst.dst)}\n'
		case Ret:
			return '\tret\n'

emit_function(Assembly.Function func):
	label = show_label(func.name)
	return """
	.globl {label}
{label}:
	{emit_instruction(inst) for inst in func.instructions}
"""

emit_stack_note():
	if on OS_X:
		return ""
	if on Linux:
		return "\t.section .note.GNU-stack,\"\",@progbits\n"

emit_file(Assembly.Program prog):
	content = emit_function(prog.func)
	content += emit_stack_note()
	write content to file
```

# Optional
We can make another 2 classes specialized to show LexError and ParseError.

---

# Output
From C:
```C
int main(void) {
    return 2;
}
```

To x64 Assembly on Linux:
```asm
    .globl main
main:
    movl $2, %eax
    ret
```

