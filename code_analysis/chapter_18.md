# Table of Contents

- [Token, Lexer](#token-lexer)
- [Types](#types)
- [Const](#const)
- [ConstConvert](#ConstConvert)
- [AST](#ast)
- [Parser](#parser)
- [Initializers](#initializers)
- [IdentifierResolution](#identifier-resolution)
- [ValidateLabels](#validatelabels)
- [LoopLabeling](#looplabeling)
- [CollectSwitchCases](#collectswitchcases)
- [TypeUtils](#typeutils)
- [Symbols](#symbols)
- [TypeCheck](#typecheck)
- [TACKY](#tacky)
- [TACKYGEN](#tackygen)
- [Assembly](#assembly)
- [AssemblySymbols](#assemblysymbols)
- [CodeGen](#codegen)
- [ReplacePseudo](#replacepseudo)
- [Instruction Fixup](#instruction-fixup)
- [Emit](#emit)
- [Output](#output)
- [Extra Credit: Unions](#extra-credit-unions)
  - [Token & Lexer](#token-and-lexer)
  - [AST](#ast-1)
  - [Parser](#parser-1)
  - [Tacky](#tacky-1)
  - [TackyGen](#tackygen-1)
  - [Assembly](#assembly-1)
  - [CodeGen](#codegen-1)
  - [Replace Pseudo](#replace-pseudo)
  - [Instruction Fixup](#instruction-fixup-1)
  - [Emit](#emit-1)
  - [Output](#output-1)

---

# Token, Lexer

New tokens to recognize
| Token | Regular expression |
| ------- | -------------------- |
| KeywordStruct | struct |
| Dot | . |
| Arrow | -> |

# Types

```
type Char {}
type SChar {}
type UChar {}
type Int {}
type Long {}
type UInt {}
type ULong {}
type Double {}
type Pointer {
    referenced_t: t,
}
type Void {}
type Array {
    elem_type: t,
    size: int,
}
type Structure {
    tag: string,
}
type FunType {
    param_types: t[],
    ret_type: t,
}

t = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Pointer | Void | Array | Structure | FunType
```

# Const

_No changes_

# ConstConvert

_Actually no changes_

The default case in const_convert already accounts for the Structure case.

```
// create different for each type of v, better use template if your implementation language supports it
cast(int32/uint32/int64/uint64 v, Types.t targetType):
    switch targetType:
        case Types.Char:
        case Types.SChar: return Const.ConstChar(to_int8(v))
        case Types.UChar: return Const.ConstUChar(to_int8(v))
        case Types.Int: return Const.ConstInt(to_int32(v))
        case Types.UInt: return Const.ConstUInt(to_uint32(v))
        case Types.Long: return Const.ConstLong(to_int64(v))
        case Types.ULong: return Const.ConstULong(to_uint64(v))
        case Types.Pointer: return Const.ConstULong(to_uint64(v))
        case Types.Double: return Const.Double(to_double(v))
        default: // targetType is FunType or Array or Void or Structure
            fail("Internal error: cannot cast constant to non-scalar type")
```

# AST

We add struct_declaration, member_declaration, Dot and Arrow expressions.

```
program = Program(declaration*)
declaration = FunDecl(function_declaration)
    | VarDecl(variable_declaration)
    | StructDecl(struct_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
struct_declaration = (identifier tag, member_declaration* members)
member_declaration = (identifier member_name, type member_type)
initializer = SingleInit(exp) | CompoundInit(initializer* list)
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Void
    | Pointer(type referenced)
    | Array(type elem_type, int size)
    | FunType(type* params, type ret)
    | Structure(identifier tag)
storage_class = Static | Extern
block_item = S(statement) | D(declaration)
block = Block(block_item*)
for_init = InitDecl(variable_declaration) | InitExp(exp?)
statement = Return(exp)
    | Expression(exp)
    | If(exp condition, statement then, statement else)
    | Compound(block)
    | Break
    | Continue
    | While(exp condition, statement body, identifier id)
    | DoWhile(statement body, exp condition, identifier id)
    | For(for_init init, exp? condition, exp? post, statement body, identifier id)
    | Switch(exp control, statement body, string* cases, identifier id)
    | Case(exp, statement body, identifier id)
    | Default(statement body, identifier id)
    | Null
    | LabeledStatement(indentifier label, statement)
    | Goto(identifier label)
exp = Constant(const, type)
    | String(string)
    | Cast(target_type, exp, type)
    | Var(identifier, type)
    | Unary(unary_operator, exp, type)
    | Binary(binary_operator, exp, exp, type)
    | Assignment(exp, exp, type)
    | CompoundAssignment(binary_operator, exp, exp, type)
    | PostfixIncr(exp, type)
    | PostfixDecr(exp, type)
    | Conditional(exp condition, exp then, exp else, type)
    | FunctionCall(identifier, exp* args, type)
    | Dereference(exp, type)
    | AddrOf(exp, type)
    | Subscript(exp, exp, type)
    | SizeOf(exp, type)
    | SizeOfT(ofType, type)
    | Dot(exp structure, identifier member, type)
    | Arrow(exp pointer, identifier member, type)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(double)
    | ConstChar(int) | ConstUChar(int)
```

# Parser

```
is_type_specifier(token):
    switch token type:
        case "int":
        case "long":
        case "double":
        case "signed":
        case "unsigned":
        case "char":
        case "void":
        case "struct":
            return true
        default:
            return false
```

```
// NEW
parse_type_specifier(tokens):
    next_tok = peek(tokens)
    if next_tok is "struct":
        // if the specifier is a struct kw, we actually care about the tag that follows it
        take(tokens)
        // struct keyword must be followed by tag
        tok = take(tokens)
        if tok is Identifier(tag):
            return tag
        else:
            raise_error("a structure tag", tok)
    else if is_type_specifier(next_tok):
        take(tokens)
        return next_tok
    else:
        fail("Internal error: called parse_type_specifier on non-type specifier token")
```

```
// New
parse_specifier(tokens):
    next_tok = peek(tokens)
    if next_tok is "static" or "extern":
        take(tokens)
        return next_tok
    return parse_type_specifier(tokens)
```

```
// call parse_type_specifier
parse_type_specifier_list(tokens):
    type_specifiers = []
    next_token = peek(tokens)

    while is_type_specifier(next_token):
        spec = parse_type_specifier(tokens)
        type_specifiers.append(spec)
        next_token = peek(tokens)

    return type_specifiers
```

```
// call parse_specifier
parse_specifier_list(tokens):
    specifiers = []
    next_token = peek(tokens)

    while is_specifier(next_token):
        spec = parse_specifier(tokens)
        specifiers.append(spec)
        next_token = peek(tokens)

    return specifiers
```

```
parse_type(specifier_list):
    /*
        sort specifiers so we don't need to check for different
        orderings of same specifiers
    /*

    specifier_list = sort(specifier_list)
    is_ident = (tok) -> tok is Identifier

    if specifier_list == [ identifier(tag) ]:
        return Types.Structure(tag)
    else if specifier_list == ["double]:
        return Types.Double
    else if specifier_list == ["char"]:
        return Types.Char
    else if specifier_list == ["char", "signed"]:
        return Types.SChar
    else if specifier_list == ["char", "unsigned"]:
        return Types.UChar

    if (len(specifier_list) == 0 or
        len(set(specifier_list)) != len(specifier_list) or
        ("double in specifier_list) or
        ("char" in specifier_list) or
        (any item in specifier_list is is_ident(item)) or
        (("signed" in specifier_list) and ("unsigned" in specifier_list))):
        fail("Invalid type specifier")
    else if "unsigned" in specifier_list and "long" in specifier_list:
        return Types.ULong
    else if "unsigned" in specifier_list:
        return Types.UInt
    else if "long" in specifier_list:
        return Types.Long
    else:
        return Types.Int
```

```
// Change a bit in the splitting the specifier_list to types and storage_classes
parse_type_and_storage_class(specifier_list):
    types = []
    storage_classes = []

    for tok in specifier_list:
        if tok is "extern" or "static":
            storage_classes.append(tok)
        else:
            types.append(tok)

    type = parse_type(types)

    storage_class = null

    if storage_classes is empty:
        // do nothing, storage_class is null
    else if length(storage_classes) == 1:
        storage_class = parse_storage_class(storage_classes[0])
    else:
        fail("Invalid storage class")

    return (type, storage_class)
```

```
// update the else case: fail -> raise_error
parse_constant(tokens):
    tok = take_token(tokens)

    if tok is ConstChar:
        s_prime = unescape(tok.value)
        if len(s_prime) == 1:
            return Const.ConstInt(to_int32(s_prime[0]))
        else:
            fail("Internal error: Character token contains multiple characters, lexer should have rejected this")

    if (tok is signed and tok.value > MAX_INT64) OR (tok is unsigned and tok.value > MAX_UINT64):
        fail("Constant is too large to fit in an int or long with given signedness")

    if tok is Token.Double:
        return Const.Double(tok.value)

    if tok is Token.ConstInt:
        if tok.value <= MAX_INT32:
            return Const.ConstInt(cast_to_int_32(tok.value))
        else:
            return Const.ConstLong(cast_to_int_64(tok.value))
    else if tok is Token.ConstLong:
        return Const.ConstLong(cast_to_int_64(tok.value))
    else if tok is Token.ConstUInt:
        if tok.value <= MAX_UINT32 :
            return Const.ConstUInt(cast_to_uint_32(tok.value))
        else:
            return Const.ConstULong(cast_to_uint_64(tok.value))
    else if tok is Token.ConstULong:
        return Const.ConstULong(cast_to_uint_64(tok.value))
    else:
        raise_error("a constant", tok)
```

```
// Add dot and arrow cases
parse_postfix_helper(primary, tokens):
	next_token = peek(tokens)
	if next_token is "--":
		decr_exp = PostfixDecr(primary)
		return parse_postfix_helper(decr_exp, tokens)
	else if next_token is "++":
		incr_exp = PostfixDecr(primary)
		return parse_postfix_helper(incr_exp, tokens)
	else if next_token is "[":
        take(tokens)
        index = parse_exp(tokens, 0)
        expect("]", tokens)
        subscript_exp = Subscript(primary, index)
        return parse_postfix_helper(subscript_exp, tokens)
    else if next_token is ".":
        take(tokens)
        member = parse_id(tokens)
        member_exp = Dot(strct=primary, member)
        return parse_postfix_helper(member_exp, tokens)
    else if next_token is "->":
        take(tokens)
        member = parse_id(tokens)
        arrow_exp = Arrow(strct=primary, member)
        return parse_postfix_helper(arrow_exp, tokens)
    else:
		return primary
```

```
parse_declaration(tokens):
    // first figure out whether this is a struct declaration
    toks = npeek(3, tokens)

    if (toks[0] == "struct" and
        toks[1] == Identifier and
        toks[2] == "{" or ";"
    ):
        return parse_structure_declaration(tokens)
    else:
        specifiers = parse_specifier_list(tokens)
        base_typ, storage_class = parse_type_and_storage_class(specifiers)

        // parse until declarator, then call appropriate function to finish parsing
        declarator = parse_declarator(tokens)
        name, typ, params = process_declarator(declarator, base_typ)

        if typ is Types.FunType:
            return finish_parsing_function_declaration(name, storage_class, typ, params, tokens)
        else:
            if params is empty:
                return finish_parsing_variable_declaration(name, storage_class, typ, tokens)
            else:
                fail("Internal error: declarator has parameters but object type")
```

```
// NEW
parse_structure_declaration(tokens):
    expect("struct", tokens)
    tag = parse_id(tokens)

    next_tok = take(tokens)
    if next_tok == ";":
        members = []
    else if next_tok = "{":
        members = parse_member_list(tokens)
        expect("}", tokens)
        expect(";", tokens)
    else:
        fail("Internal error: shouldn't have called parse_structure_declaration here")

    return StructDecl(tag, members)
```

```
// NEW
// parse a non-empty member list
parse_member_list(tokens):
    m = parse_member(tokens)
    members = [m]

    while peek(tokens) is not "}":
        members.append(parse_member(tokens))
    return members
```

```
// NEW
// <member-declaration> ::= { <type-specifier> }+ <declarator> ";"
parse_member(tokens):
    specifiers = parse_type_specifier_list(tokens)
    t = parse_type(specifiers)
    member_decl = parse_declarator(tokens)
    if member_decl is FunDeclarator:
        raise_error("found function declarator in struct member list")
    else:
        expect(";", tokens)
        member_name, member_type, _params = process_declarator(member_decl, t)

        return member_declaration(member_name, member_type)
```

```
// Change the error msg
parse_variable_declaration(tokens):
    decl = parse_declaration(tokens)
    if decl is VarDecl:
        return decl
    else: // is FunDecl or StructDecl
        fail("Expected variable declaration but found function or structure declaration")
```

# Initializers

_No changes_

# Identifier Resolution

```
// NEW
type struct_entry = {
    unique_tag: string,
    struct_from_current_scope: bool,
}
```

```
// NEW
copy_struct_map(m):
    // return a copy of the map with from_current_block set to false for every entry
    copied = {}
    for key, entry in m:
        copied[key] = {...entry, struct_from_current_scope: false}
    return copied
```

```
resolve_type(type, struct_map):
    if type is Structure(tag):
        if struct_map.has(tag):
            unique_tag = struct_map.get(tag)
            return Structure(unique_tag)
        else:
            fail("specified undeclared structure type")
    else if type is Pointer(referenced_t):
        return Pointer(resolve_type(referenced_t, struct_map))
    else if type is Array(elem_type, size):
        resolved_elem_type = resolve_type(elem_type, struct_map)
        return Array(resolved_elem_type, size)
    else if type is FunType(param_types, ret_type):
        resolved_param_types = []
        for param_type in param_types:
            resolved_param_types.append(resolve_type(param_type, struct_map))
        resolved_ret_type = resolve_type(ret_type, struct_map)
        return FunType(resolved_param_types, resolved_ret_type)
    else:
        return type
```

```
// add struct_map and 2 cases for Dot and Arrow
// and resolve_type to the cast.target_type
resolve_exp(exp, id_map, struct_map):
    switch exp.type:
        case Assignment:
            return Assignment(
                resolve_exp(exp.left, id_map, struct_map),
                resolve_exp(exp.right, id_map, struct_map),
                exp.type
            )
        case CompoundAssignment:
            return CompoundAssignment(
                exp.op,
                resolve_exp(exp.left, id_map, struct_map),
                resolve_exp(exp.right, id_map, struct_map),
                exp.type
            )
        case PostfixIncr:
            return PostfixIncr(
                resolve_exp(exp.inner, id_map, struct_map),
                exp.type
            )
        case PostfixDecr:
            return PostfixDecr(
                resolve_exp(exp.inner, id_map, struct_map),
                exp.type
            )
        case Var:
            if exp.name exists in id_map:                       // rename var from map
                return Var( id_map.get(exp.name).uniqueName, exp.type )
            else:
                fail("Undeclared variable: " + exp.name)
        // recursively process operands for other expressions
        case Cast:
            resolved_type = resolve_type(exp.target_type, struct_map)
            return Cast(
                resolved_type,
                resolve_exp(exp.exp, id_map, struct_map),
                exp.type
            )
        case Unary:
            return Unary(
                exp.op,
                resolve_exp(exp.exp, id_map, struct_map),
                resolve_exp(exp.inner, id_map, ),
                exp.type
            )
        case Binary:
            return Binary(
                exp.op,
                resolve_exp(exp.left, id_map, struct_map),
                resolve_exp(exp.right, id_map, struct_map),
                exp.type
            )
        case SizeOf:
            return SizeOf(resolve_exp(exp.exp, id_map, struct_map))
        case SizeOfT:
            return SizeOfT(resolve_type(exp.ofType, struct_map))
        case Constant:
        case String:
            return exp
        case Conditional:
            return Conditional(
                resolve_exp(exp.condition, id_map, struct_map),
                resolve_exp(exp.then, id_map, struct_map),
                resolve_exp(exp.else, id_map, struct_map),
                exp.type
            )
        case FunctionCall(fun_name, args):
            if fun_name is in id_map:
                new_fun_name = id_map.get(fun_name).new_name
                new_args = []
                for arg in args:
                    new_args.append(resolve_exp(arg, id_map, struct_map))

                return FunctionCall(new_fun_name, new_args, exp.type)
            else:
                fail("Undeclared function!")
        case Dereferene(inner):
            return Dereference(resolve_exp(inner, id_map, struct_map))
        case AddOf(inner):
            return AddrOf(resolve_exp(inner, id_map, struct_map))
        case Subscript(ptr, index):
            return Subscript(resolve_exp(ptr, id_map, struct_map), resolve_exp(index, id_map, struct_map))
        case Dot(strct, member):
            return Dot(resolve_exp(strct, id_map, struct_map), member)
        case Arrow(strct, member):
            return Arrow(resolve_exp(strct, id_map, struct_map), member)
        default:
            fail("Internal error: Unknown expression")
```

```
resolve_optional_exp(opt_exp, id_map, struct_map):
    if opt_exp is not null:
        resolve_exp(opt_exp, id_map, struct_map)
```

```
resolve_initializer(init, id_map, struct_map):
    if init is SingleInit(e):
        return SingleInit(resolve_exp(e, id_map, struct_map))
    else if init is CompoundInit(inits):
        resolved_inits = []
        for init in inits:
            resolved_inits.append(resolve_initializer(init, id_map, struct_map))
        return CompoundInit(resolved_inits)
```

```
// Use resolve_initializer for var declarations
resolve_local_var_declaration(var_decl, id_map, struct_map):
    new_id_map, unique_name = resolve_local_var_helper(id_map, var_decl.name, var_decl.storage_class)

    resolved_type = resolve_type(var_decl.var_type, struct_map)
    resolved_init = null
    if var_decl has init:
        resolved_init = resolve_initializer(var_decl.init, id_map, sturct_map)

    return (new_id_map, (name = unique_name, init=resolved_init, resolved_type, var_decl.storage_class))
```

```
resolve_for_init(init, id_map, struct_map):
	match init with
	case InitExp(e):
        return (id_map, InitExp(resolve_optional_exp(e, id_map, struct_map)))
	case InitDecl(d):
        new_id_map, resolved_decl = resolve_local_var_declaration(d, id_map, struct_map)
        return (new_id_map, InitDecl(resolved_decl))
```

```
// mainly add struct_map to resolving functions
resolve_statement(statement, id_map, struct_map):
    switch (statement) do

        case Return(e):
            // If 'e' exists, resolve it; otherwise, leave it as null.
            if (e is not null) then
                resolved_e = resolve_exp(e, id_map, struct_map)
            else
                resolved_e = null
            end if
            return Return(resolved_e)

        case Expression(e):
            return Expression(resolve_exp(e, id_map, struct_map))

        case If { condition, then_clause, else_clause }:
            resolved_condition = resolve_exp(condition, id_map, struct_map)
            resolved_then_clause = resolve_statement(then_clause, id_map, struct_map)
            if (else_clause exists) then
                resolved_else_clause = resolve_statement(else_clause, id_map, struct_map)
            else
                resolved_else_clause = null
            end if
            return If {
                condition: resolved_condition,
                then_clause: resolved_then_clause,
                else_clause: resolved_else_clause
            }

        case LabeledStatement(label, stmt):
            return LabeledStatement(label, resolve_statement(stmt, id_map, struct_map))

        case Goto(label):
            return Goto(label)

        case While { condition, body, id }:
            resolved_condition = resolve_exp(condition, id_map, struct_map)
            resolved_body = resolve_statement(body, id_map, struct_map)
            return While {
                condition: resolved_condition,
                body: resolved_body,
                id: id
            }

        case DoWhile { body, condition, id }:
            resolved_body = resolve_statement(body, id_map, struct_map)
            resolved_condition = resolve_exp(condition, id_map, struct_map)
            return DoWhile {
                body: resolved_body,
                condition: resolved_condition,
                id: id
            }

        case For { init, condition, post, body, id }:
            // Create copies to preserve current scope for 'For'
            id_map1 = copy_identifier_map(id_map)
            struct_map1 = copy_struct_map(struct_map)
            // Resolve initializer: returns an updated id_map along with the resolved initializer.
            (id_map2, resolved_init) = resolve_for_init(init, id_map1, struct_map1)
            resolved_condition = resolve_optional_exp(condition, id_map2, struct_map1)
            resolved_post = resolve_optional_exp(post, id_map2, struct_map1)
            resolved_body = resolve_statement(body, id_map2, struct_map1)
            return For {
                init: resolved_init,
                condition: resolved_condition,
                post: resolved_post,
                body: resolved_body,
                id: id
            }

        case Compound(block):
            // In a new compound block, create new variable & structure maps to enforce scope.
            new_variable_map = copy_identifier_map(id_map)
            new_struct_map = copy_struct_map(struct_map)
            resolved_block = resolve_block(block, new_variable_map, new_struct_map)
            return Compound(resolved_block)

        case Switch(s):
            s.control = resolve_exp(s.control, id_map, struct_map)
            s.body = resolve_statement(s.body, id_map, struct_map)
            return Switch(s)

        case Case(value, stmt, id):
            return Case(value, resolve_statement(stmt, id_map, struct_map), id)

        case Default(stmt, id):
            return Default(resolve_statement(stmt, id_map, struct_map), id)

        // For statements that do not require resolution, we simply return them as is.
        case Null, Break, Continue:
            return statement

```

```
resolve_block_item(block_item, id_map, struct_map):
	if block_item is a declaration:
        // resolving a declaration can change the structure or variable map
		return resolve_declaration(block_item, id_map, struct_map)
	else:
        // resolving a statement doesn't change the struct or variable map
		return resolve_statement(block_item, id_map, struct_map)
```

```
resolve_block(block, id_map, struct_map):
	resolved_block = []

	for block_item in block:
		resolved_item = resolve_block_item(block_item, id_map, struct_map)
		resolved_block.append(resolved_item)

	return resolved_block
```

```
resolve_local_declaration(decl, id_map, struct_map):
    if decl is VarDecl:
        new_id_map, resolved_vd = resolve_local_var_declaration(decl.decl, id_map, struct_map)
        return ((new_id_map, struct_map), VarDecl(resolved_vd))
    else if decl is FunDecl and has body:
        fail("Nested function defintiions are not allowed")
    else if  decl is FunDecl and decl.storage_class is Static:
        fail("Static keyword not allowed on local function declarations")
    else if decl is FunDecl:
        new_id_map, resolved_fd = resolve_function_declaration(decl.decl, id_map, struct_map)
        return ((new_id_map, struct_map), FunDecl(resolved_fd))
    else if decl is StructDecl:
        new_struct_map, resolved_sd = resolve_structure_declaration(decl.decl, struct_map)
        return ((id_map, new_struct_map), StructDecl(resolved_sd))
```

```
resolve_function_declaration(fun_decl, id_map, struct_map):
    entry = id_map.find(fun_decl.name)

    if entry exists and entry.from_current_scope == True and entry.has_linkage == False:
        fail("Duplicate declaration")
    else:
        resolved_type = resolve_type(fun_dec .fun_type, struct_map)
        new_entry = (
            unique_name=fun_decl.name,
            from_current_scope=True,
            has_linkage=True,
        )

        new_id_map = id_map.add(fun_decl.name, new_entry)
        inner_id_map = copy_identifier_map(new_id_map)
        inner_id_map1, resolved_params = resolve_params(inner_id_map, fun_decl.params)
        inner_struct_map = copy_struct_map(struct_map)
        resolved_body = null

        if fun_decl has body:
            resolved_body = resolve_block(fun_decl.body, inner_id_map1, inner_struct_map)

        return (
            new_id_map,
            (
                fun_decl.name,
                resolved_type,
                resolved_params,
                resolved_body
            )
        )
```

```
// NEW
resolve_structure_declaration(struct_decl, struct_map):
    prev_entry = struct_map.find(struct_decl.tag)

    new_map = null
    resolved_tag = null

    if prev_entry is not null and prev_entry.struct_from_current_scope:
        // this refers to the same struct we've already declared, don't update the map
        new_map = struct_map
        resolved_tag = prev_entry.unique_tag
    else:
        // this declare a new type, generate a tag and update the map
        unique_tag = UniqueIds.make_named_temporary(tag)
        entry = (unique_tag, struct_from_current_scope = true)
        struct_map.add(tag, entry)

        new_map = struct_map
        resolved_tag = unique_tag

    resolved_members = []
    for member in struct_decl.members:
        member_type = resolve_type(new_map, m.member_type)
        resolved_members.append((...m, member_type))

    return (
        new_map,
        (resolved_tag, resolved_members)
    )
```

```
resolve_file_scope_variable_declaration(var_decl, id_map, struct_map):
    resolved_vd = (...var_decl, var_type=resolve_type(var_decl.var_type, struct_map))
    new_map = id_map.add(resolved_vd.name, (
        unique_name=resolved_vd.name,
        from_current_scope=true,
        has_linkage=true
    ))

    return (new_map, resolved_vd)
```

```
resolve_global_declaration(decl, id_map, struct_map):
    if decl is FunDecl:
        id_map1, resolved_fd = resolve_function_declaration(decl, id_map, struct_map)
        return ((id_map1, struct_map), resolved_fd)
    else if decl is VarDecl:
        id_map1, resolved_vd = resolve_file_scope_variable_declaration(decl, id_map, struct_map)
        return ((id_map1, struct_map), resolved_vd)
    else if decl is StructDecl:
        struct_map1, resolved_sd = resolve_structure_declaration(decl, struct_map)
        return ((id_map, struct_map_1), resolved_sd)
```

```
resolve(Program prog):
    resolved_decls = []

    for decl in prog.decls:
        resolved_decls.append(
            resolve_global_declaration(decl, {}, {} )
        )

    return Program(resolved_decls)
```

# ValidateLabels

_No Changes_

# LoopLabeling

_No Changes_

# CollectSwitchCases

_No Changes_

# TypeUtils

```
// Add case for Structure
get_size(Types.t type):
    switch type:
        case Types.Char:
        case Types.SChar:
        case Types.UChar:
            return 1
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8
        case Types.Array:
            return type.size * get_size(type.elem_type)
        case Types.Structure:
            return type_table.find(type.tag).size
        default:
            fail("Internal error: function type doesn't have size")
```

```
// Add case for Structure
get_alignment(Types.t type):
    switch type:
        case Types.Char:
        case Types.UChar:
        case Types.SChar:
            return 1
        case Types.Int:
        case Types.UInt:
            return 4
        case Types.Long:
        case Types.ULong:
        case Types.Double:
        case Types.Pointer:
            return 8:
        case Types.Array:
            return get_alignment(type.elem_type)
        case Types.Structure:
            return type_table.find(type.tag).alignment
        case Types.FunType:
            fail("Internal error: function type doesn't have alignment")
```

```
// Default case already handles Structure
is_signed(Types.t type):
    switch type:
        case Types.Int:
        case Types.Long:
        case Types.Char:
        case Types.SChar:
            return true
        case Types.UInt:
        case Types.ULong:
        case Types.Pointer:
        case Types.UChar:
            return false
        default:
            fail("Internal error: signedness doesn't make sense for function, double type, and array type")
```

```
// default case already handles Structures
is_integer(type):
    switch (type):
        case Char:
        case UChar:
        case SChar:
        case Int:
        case UInt:
        case Long:
        case ULong:
            return true
        default:
            return false
```

```
is_character(type):
    return type is Char or SChar or UChar
```

```
// default case already handles Structures
is_arithmetic(type):
    switch (type):
        case Int:
        case UInt:
        case Long:
        case ULong:
        case Double:
        case Char:
        case SChar:
        case UChar:
            return true
        default:
            return false
```

```
// Add case for Structure
is_scalar(type):
    switch (type):
        case Array:
        case Void:
        case FunType:
        case Structure:
            return false
        default:
            return true
```

```
// Add case for Structure
is_complete(type):
    switch type:
        case Void:
            return false
        case Structure(tag):
            return type_table.has(tag)
        default:
            return true
```

# Symbols

_No changes_

# TypeTable

```
type member_entry = {
    member_type: Types.t,
    offset: int
}
```

```
type struct_entry = {
    alignment: int;
    size: int;
    members: Map<string, member_entry>
}
```

```
type_table = Map<string, struct_entry>
```

```
add_struct_definition(tag, struct_def):
    type_table.add(tag, struct_def)
```

```
get_members(tag):
    struct_def = type_table.find(tag)
    member_list = values(struct_def.members)
    sorted_members = sort(member_list, key = member->member.offset)
    return sorted_members
```

```
get_member_types(tag):
    members = get_members(tag)
    types = []
    for member in members:
        types.append(member.member_type)
    return types
```

# TypeCheck

```
is_lvalue(AST.Expression e):
    switch e:
        case Deference:
        case Subscript :
        case Var:
        case String:
        case Arrow:
            return true
        case Dot(strct, _):
            return is_lvalue(strct)
        default:
            return false
```

```
// Structure for the last check
validate_type(type):
    if type is Array(elem_type, _):
        if is_complete(elem_type):
            validate_type(elem_type)
        else:
            fail("Array of incomplete type")

    else if type is Pointer(t):
        validate_type(t)

    else if type is FunType(param_types, ret_type):
        for param_t in param_types:
            validate_type(param_t)
        validate_type(ret_type)

    else if type is in (Char, SChar, UChar, Int, Long, UInt, ULong, Double, Void, Structure):
        return
```

```
validate_struct_definition(struct_def):
    // make sure it's not already in the type table
    tag, members = struct_def

    if type_table.has(tag):
        fail("Structure was already declared")
    else:
        // check for duplicate number names
        member_names = Set{}
        for member in members
            member_name, member_type = member
            if member_names.has(member_name):
                fail("Duplicate declaration of member {member_name} in structure {tag}")
            else:
                member_names.add(member_name)
            // validate member_type
            validate_type(member_type)
            if member_type is FunType:
                // this is redundant, we'd already reject this in parser
                fail("Can't declare structure member with function type")
            else:
                if is_complete(member_type):
                    // do nothing
                else:
                    fail("Cannot declare structure member with incomplete type")
```

```
typecheck_struct_decl(sd):
    tag, members = sd

    if members.empty():
        // ignore forward declarations
        return
    else:
        // validate the definition, then add it to the type table
        validate_struct_definition(sd)

        member_entries = {}
        struct_size = 0
        struct_alignment = 1
        for member in struct_decl.members:
            member_alignment = alignment(member.member_type, type_table)
            member_offset = Rounding.round_away_from_zero(member_alignment, struct_size)
            m = MemberEntry(member.member_type,
                            member_offset)
            member_entries.insert(member.memberName, m)
            struct_alignment = max(struct_alignment, member_alignment)
            struct_size = member_offset + size(member.member_type, type_table)

        size = Rounding.round_away_from_zero(struct_alignment, struct_size)
        struct_def = struct_entry(struct_alignment, size, member_entries)
        type_table.add_struct_definition(tag, struct_def)
    return (tag, members)
```

```
// Add cases for Dot and Arrow
typecheck_exp(e, symbols):
	match e with:
	case Var(v):
		return typecheck_var(v)
	case Constant(c):
        return typecheck_const(c)
    case String(s):
        return typecheck_string(s)
	case Cast(target_type, inner):
        return typecheck_cast(target_type, inner)
	case Unary(op, inner):
        if op == Not:
            return typecheck_not(inner)
        if op == Complement:
            return typecheck_complement(inner)
        if op == Negate:
            return typecheck_negate(inner)
		return typecheck_incr(op, inner)
	case Binary(op, e1, e2):
        switch op:
            case And:
            case Or:
                return typecheck_logical(op, e1, e2)
            case Add:
                return typecheck_addition(e1, e2)
            case Subtract:
                return typecheck_subtraction(e1, e2)
            case Multiply:
            case Divide:
            case Remainder:
                return typecheck_multiplicative(op, e1, e2)
            case Equal:
            case NotEqual:
                return typecheck_equality(op, e1, e2)
            case GreaterThan:
            case GreaterOrEqual:
            case LessThan:
            case LessOrEqual:
                return typecheck_comparison(op, e1, e2)
            case BitwiseAnd:
            case BitwiseOr:
            case BitwiseXor:
                return typecheck_bitwise(op, e1, e2)
            case BitshiftLeft:
            case BitshiftRight:
                return typecheck_bitshift(op, e1, e2)
	case Assignment(lhs, rhs):
        return typecheck_assignment(lhs, rhs)
    case CompoundAssignment(op, lhs, rhs):
        return typecheck_compound_assignment(op, lhs, rhs)
    case PostfixDecr(e):
        return typecheck_postfix_decr(e)
    case PostfixIncr(e):
        return typecheck_postfix_incr(e)
	case Conditional(control, then_result, else_result):
		return typecheck_conditional(control, then_result, else_result)
	case FunctionCall(f, args):
		return typecheck_fun_call(f, args)
    case Dereference(inner):
        return typecheck_dereference(inner)
    case AddrOf(inner):
        return typecheck_addr_of(inner)
    case Subscript():
        return typecheck_subscript(exp)
    case SizeOfT(t):
        return typecheck_size_of_t(t)
    case SizeOf(e):
        return typecheck_size_of(e)
    case Dot(strct, member):
        return typecheck_dot_operator(strct, member)
    case Arrow(strct, member):
        return typecheck_arrow_operator(strct, member)
```

```
// In the end, typed_inner can be used to construct Cast instead of typcheck_and_convert inner again
typecheck_cast(target_type, inner):
    validate_type(target_type)
    typed_inner = typecheck_and_convert(inner)

    if
        (target_type is Pointer and typed_inner.type is Double)
        OR
        (target_type is Double and typed_inner.type is Pointer):
            fail("Cannot cast between pointer and double")

    if target_type is Void:
        cast_exp = Cast(Void, typed_inner)
        return set_type(cast_exp, Void)

    if not is_scalar(target_type):
        fail("Can only cast to scalar types or void")
    else if not is_scalar(typed_inner.type):
        fail("Can only cast scalar expressions to non-void type")

    cast_exp = Cast(target_type, typed_inner)
    return set_type(cast_exp, target_type)
```

```
// add one case check if then and else have the same types after arithmetic and pointer check.
typecheck_conditional(condition, then_exp, else_exp):
    typed_condition = typecheck_scalar(condition)
    typed_then = typecheck_and_convert(then_exp)
    typed_else = typecheck_and_convert(else_exp)

    if typed_then.type is Void and typed_else.type is Void:
        result_type = Void
    else if is_pointer(typed_then.type) or is_pointer(typed_else.type):
        result_type = get_common_pointer_type(typed_then, typed_else)
    else if is_arithmetic(typed_then.type) and is_arithemtic(typed_else.type):
        result_type = get_common_type(typed_then.type, typed_else.type)
    // only other option is structure typess, this is fine if they're identical
    // (typecheck_and_convert already validated that they're complete)
    else if typed_then.type == typed_else.type:
        result_type = typed_then.else
    else:
        fail("Invalid operands for conditional")

    converted_then = convert_to(typed_then, result_type)
    converted_else = convert_to(typed_else, result_type)

    conditionl_exp = Conditional(
        typed_condition,
        converted_then,
        converted_else
    )

    return set_type(conditional_exp, result_type)
```

```
// Early check for Structure of incomplete type
typecheck_and_convert(e):
    typed_e = typcheck_exp(e)

    if typed_e.type is Structure and not (is_complete(typed_e.type)):
        fail("Incomplete structure type not permitted here")
    else if typed_e.type is Types.Array(elem_type):
        addr_exp = AddrOf(typed_e)
        return set_type(addr_exp, Pointer(elem_type))
    else:
        return typed_e
```

```
typecheck_dot_operator(strct, member):
    typed_strct = typecheck_and_convert(strct)

    if typed_struct.type is Structure(tag):
        // typecheck_and_convert already validated that this structure type is complete
        struct_def = type_table.get(tag)
        member_entry = struct_def.members.find(member)

        if member_entry is not found:
            fail("Struct type {tag} has no member {member}")

        member_typ = member_entry.member_type
        dot_exp = Dot(typed_strct, member)
        return set_type(dot_exp, member_typ)
    else:
        fail("Dot operator can only be applied to expressions with structure type")
```

```
typecheck_arrow_operator(strct_ptr, member):
    typed_strct_ptr = typecheck_and_convert(strct_ptr)

    if typed_strct_ptr.type is Pointer(Structure(tag)):
        // typecheck_and_convert already validated that this structure type is complete
        struct_def = type_table.get(tag)
        member_entry = struct_def.members.find(member)

        if member_entry is not found:
            fail("Struct type {tag} is incomplete or has no member {member}")

        member_typ = member_entry.member_type
        arrow_exp = Arrow(typed_strct_ptr, member)
        return set_type(arrow_exp, member_typ)
    else:
        fail("Arrow operator can only be applied to pointers to structure")
```

```
// add two check for Structure with CompoundInit, and Structure with SingleInit
// and check if var_type is arithmetic for SingleInit(Constant(c))
static_init_helper(var_type, init):
    if var_type is Array(elem_type, size) and init is SingleInit(String(s)):
        if is_character(elem_type):
            n = size - len(s)
            if n == 0:
                return [ Initializers.StringInit(s, False) ]
            else if n == 1:
                return [ Initializers.StringInit(s, True) ]
            else if n > 1:
                return [ Initializers.StringInit(s, True), Initializers.ZeroInit(n - 1) ]
            else:
                fail("string is too long for initialize")
        else:
            fail("Can't initialize array of non-character type with string literal")

    else if var_type is Array and init is SingleInit:
        fail("Can't initialize array from scalar value")

    else if var_type is Pointer(Char) and init is SingleInit(String(s)):
        str_id = symbols.add_string(s)
        return [ Initializers.PointerInit(str_id) ]

    else if init is SingleInit(String):
        fail("String literal can only initialize char or decay to pointer")

    else if var_type is Structure(tag) and init is CompoundInit(inits):
        struct_def = type_table.get(tag)
        members = type_table.get_members(tag)
        if length(inits) > length(members):
            fail("Too many elements in struct initializer")
        else:
            current_offset = 0
            current_inits = []
            i = 0
            for init in inits:
                memb = members[i]
                padding = []
                if current_offset < memb.offset:
                    padding.append(Initializers.ZeroInit(memb.offset - current_offset))

                more_static_inits = static_init_helper(memb.member_type, init)
                current_inits = [...current_inits, ...padding, ...more_static_inits]
                current_offset = memb.offset + get_size(memb.member_type)
                i++

            trailing_padding = []
            if current_offset < struct_def.size:
                trailing_padding.append(Initializers.ZeroInit(struct_def.size - current_offset))

            return [...current_inits, ...trailing_padding]

    else if var_type is Structure and init is SingleInit:
        fail("Can't initialize static structure with scalar value")

    else if init is SingleInit(exp) and exp is Constant(c) and is_zero_int(c):
        return Initializers.zero(var_type)

    else if var_type is Pointer:
        fail("invalid static initializer for pointer")

    else if init is SingleInit(exp):
        if exp is Constant(c):
            if is_arithmetic(var_type):
                converted_c = ConstConvert.convert(var_type, c)
                switch (converted_c);:
                    case ConstChar(c):      init_val = Initializers.CharInit(c)
                    case ConstInt(i):       init_val = Initializers.IntInit(i)
                    case ConstLong(l):      init_val = Initializers.LongInit(l)
                    cast ConstUChar(uc):    init_val = Initializers.UCharInit(uc)
                    case ConstUInt(ui):     init_val = Initializers.UIntInit(ui)
                    case ConstULong(ul):    init_val = Initializers.ULongInit(ul)
                    case ConstDouble(d):    init_val = Initializers.DoubleInit(d)
                    default: fail("invalid static initializer")

                return [ init_val ]
            else:
                /*
                    we already dealt with pointers (can only initialize w/ null constant or string litera
                    and already rejected any declarations with type void and any arrays or structs
                    initialized with scalar expressions
                */
                fail("Internal error: should have already rejected initializer with type")
        else:
            fail("non-constant initializer")

    else if var_type is Array(element, size) and init is CompoundInit(inits):
        static_inits = []
        for init in inits:
            static_inits.append(static_init_helper(elem_type, init))

        n = size - len(inits)

        if n == 0:
            padding = []
        if n > 0:
            zero_bytes = get_size(elem_type) * n
            padding = [ Initializers.ZeroInit(zero_bytes) ]
        else:
            fail("Too many values in static initializer")

        static_inits.append(...padding)

        return static_inits

    else if init is CompoundInit:
        fail("Can't use compound initialzier for object with scalar type")
```

```
// add case Structrue
make_zero_init(type):
    scalar = (Constant.Const c) -> return SingleInit(AST.Constant(c), type)

    switch type:
        case Array(elem_type, size):
            return CompoundInit([make_zero_init(elem_type)] * size, type)
        case Structure(tag):
            members = type_table.get_members(tag)
            zero_inits = []
            for member in members:
                zero_inits.append(make_zero_init(member.member_type))
            return CompoundInit(zero_inits, type)
        case Char:
        case SChar:
            return scalar(Constant.ConstChar(0))
        case Int:
            return scalar(Constant.ConstInt(0))
        case UChar:
            return scalar(Constant.ConstUChar(0))
        case UInt:
            return scalar(Constant.ConstUInt(0))
        case Long:
            return scalar(Constant.ConstLong(0))
        case ULong:
        case Pointer:
            return scalar(Constant.ConstULong(0))
        case Double:
            return scalar(Constant.ConstDouble(0))
        case FunType:
        case Void:
            fail("Internal error: can't create zero initializer with function or void type")
```

```
// add check for target_type is Structure and init is CompoundInit
typecheck_init(target_type, init):
    if target_type is Array(elem_type, size) and init is SingleInit(String(s)):
        if !is_character(elem_type):
            fail("Can't initialize non-character type with string literal")
        else if len(s) > size:
            fail("Too many characters in string literal")
        else:
            return SingleInit(set_type(String(s), target_type))

    else if target_type is Structure(tag) and init is CompoundInit(inits):
        members = type_table.get_members(tag)
        if length(inits) > length(members):
            fail("Too many elements in structure initializer")
        else:
            initialized_members = members[0:length(inits)]
            uninitialized_members = members[length(inits):]

            typechecked_members = []
            i = 0
            for memb in initialized_members:
                init = inits[i]
                typechecked_members.append(typecheck_init(memb.member_type, init))
                i++

            padding = []
            for memb in uninitialized_members:
                padding.append(make_zero_init(memb.member_type))

            return CompoundInit([...typechecked_members, ...padding], target_type)

    else if init is SingleInit(e):
        typedchecked_e = typecheck_and_convert(e)
        cast_exp = convert_by_assignment(typechecked_e, target_type)
        return SingleInit(cast_exp)

    else if var_type is Array(elem_type, size) and init is CompoundInit(inits):
        if inits > size:
            fail("Too many values in initializer")
        else:
            typechecked_inits = []
            for init in inits:
                typechecked_inits.append(typecheck_init(elem_type, init))
            padding = [make_zero_init(elem_type)] * (size - len(inits))
            return CompoundInit(target_type, [...typechecked_inits, ...padding])

    else:
        fail("Can't initialize scalar value from compound initializer")
```

```
typecheck_local_decl(decl):
    if decl is VarDecl(vd):
        return VarDecl(typecheck_local_var_decl(vd))
    else if decl is FunDecl(fd):
        return FunDecl(typecheck_fn_decl(fd))
    else if decl is StructDecl(sd):
        return StructDecl(typecheck_struct_decl(sd))
```

```
// check if the var_type is not complete after Extern, before Static and others
typecheck_local_var_decl(var_decl):
    if var_decl.var_type is Void:
        fail("No void declarations")
    else:
        validate_type(var_decl.var_type)

    if var_decl.storage_class is Extern:
        if var_decl has init:
            fail("Initializer on local extern declaration")
        else:
            entry = symbols.get_opt(var_decl.name)
            if entry is not null:
                // If an external local var is already in the symbol table, don't need to add it
                if entry.t != var_decl.var_type:
                    fail("Variable redeclared with different type")
            else:
                symbols.add_static_var(var_decl.name, t=var_decl.var_type, global=true, init=NoInitializer)

            return var_declaration(var_decl.name, init=null, var_decl.storage_class, var_decl.var_type)

    else if not is_complete(var_decl.var_type):
        // can't define a variable with an incomplete type
        fail("Cannot define a variable with an incomplete type")

    else if var_decl.storage_class is Static:
        zero_init = Symbols.Initial(Initializer.zero(var_decl.var_type))
        static_init = if var_decl has init
                        then to_static_init(var_decl.var_type, var_decl.init)
                        else zero_init

        symbols.add_static_var(var_decl.name, t=var_decl.var_type, global=false, static_init)

        // Note: we won't actually use init inb subsequent passes so we can drop it
        return var_declaration(var_decl.name, init = null, var_decl.storage_class, var_decl.var_type)
    else: // storage_class is null, so it's automatic storage duration
        Symbols.add_automatic_var(var_decl.name, var_decl.var_type)
        new_init = null
        if var_decl has init:
            new_init = typecheck_init(var_decl.var_type, var_decl.init)

        return var_declaration(var_decl.name, init=new_init, storage_class=null, t=var_decl.var_type, )
```

```
// check for valid return_t and param_ts right after has_body check
typecheck_fn_decl(fn_decl):
    validate_type(fn_decl.fun_type)

    param_ts = []
    return_t, fun_type

    if fn_decl.fun_type == FunType:
        if fn_decl.fun_type.ret_type == Array:
            fail("A function cannot return an array")
        else:
            for param_t in fn_decl.fun_type.param_types:
                if param_t is Array(elem_type):
                    param_ts.append(Pointer(elem_type))
                else if param_t is Void:
                    fail("No void params allowed")
                else:
                    param_ts.append(param_t)
        return_t = fn_decl.fun_type.ret_type
        fun_type = FunType(param_ts, fn_decl.fun_type.ret_type)
    else:
        fail("Internal error: function has non function type")

    has_body = fn_decl has body

    if has_body
        and not ((return_t == Void or is_complete(return_t))
                and <all param_ts are is_complete>):
        fail("Can't define a function with incomplete return type or parameter type")
    else:
        global = fn_decl.storage_class != Static
        already_defined = false

        if fn_decl.name is in symbols:
            old_decl = symbols.get(fn_decl.name)
            if old_decl.type != fn_decl.fun_type:
                fail("Incompatiable function declaration")

            if old_decl.attrs is FunAttr:
                if old_decl.attrs.defined and has_body:
                    fail("Function is defined more than once")
                else if old_decl.attrs.global and fn_decl.storage_class == Static:
                    fail("Static function declaration follows non-static")
                else:
                    already_defined = has_body or old_decl.attrs.defined
                    global = old_decl.attrs.global
            else:
                fail("Internal error: symbol has function type but not function attributes")

        symbols.add_fun(fn_decl.name, fn_decl.fun_type, global, already_defined)

        new_body = null

        if has_body:
            for i in length(param_ts):
                param = params[i]
                t = param_ts[i]

                symbols.add_automatic_var(param, t)
            new_body = typecheck_block(fn_decl.body, return_t)

        return function_declaration(fn_decl.name, fn_decl.fun_type, new_body, fn_decl.storage_class)
```

```
// After getting static_init, check either the var_type is complete, or static_init is NoInitializer
typecheck_file_scope_var_decl(var_decl):
    if var_decl.var_type is Void:
        fail("void variables not allowed")
    else:
        validate_type(var_decl.var_type)

    default_init = if var_decl.storage_class exists and is Extern
                    then Symbols.NoInitializer
                    else Symbols.Tentative

    static_init = if var_decl has init
                    then to_static_init(var_decl.var_type, var_decl.init)
                    else default_init

    if not (is_complete(var_decl.var_type) or static_init is NoInitializer):
        // note: some compilers permit tentative definition with incomplete type, if it's completed later in the file. we don't.
        fail("Can't define a variable with an incomplete type")
    else:
        curr_global = var_decl.storage_class != Static

        if symbols has var_decl.name
            old_decl = symbols.get(var_decl.name)
            if old_decl.type != var_decl.var_type:
                fail("Variable redeclared with different type")

            if old_decl.attrs is not StaticAttr:
                fail("Internal error: file-scope variable previsouly declared as local")

            if var_decl.storage_class == Extern:
                curr_global = old_decl.attrs.global
            else if old_decl.attrs.global != curr_global:
                fail("Conflicting variable linkage")

            if old_decl.attrs.init is an Initial(x):
                if static_init is Initial(y):
                    fail("Conflicting global variable definition")
                else:
                    static_init = old_decl.attrs.init
            else if old_delc.attrs.init is Tentative and static_init is either Tentative or NoInitializer:
                static_init = Tentative

        symbols.add_static_var(var_decl.name, t=var_decl.var_type, curr_global, static_init)

        // Ok to drop the initializer as it's never used after this pass
        return var_declaration(var_decl.name, init=null, var_decl.storage_class, var_decl.var_type)
```

```
typecheck_global_decl(decl):
    if decl is FunDecl(fd):
        return FunDecl(typecheck_fn_decl(fd))
    else if decl is VarDecl(vd):
        return VarDecl(typecheck_file_scope_var_decl(vd))
    else if decl is StructDecl(sd):
        return StructDecl(typecheck_struct_decl(sd))
    else:
        fail("Internal error: Unknown declaration type")
```

# TACKY

Add `CopyFromOffset` node

```
program = Program(top_level*)
top_level =
    | Function(string name, bool global, string* params, instruction* body)
    | StaticVariable(string name, bool global, Types.t t, Initializers.static_init* inits)
    | StaticConstant(string name, type t, Initializers.static_init init)
instruction =
    | Return(val?)
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
    | Addptr(val ptr, val index, int scale, val dst)
    | CopyToOffset(val src, string dst, int offset)
    | CopyFromOffset(string src, int offset, val dst)
    | Jump(string target)
    | JumpIfZero(val condition, string target)
    | JumpIfNotZero(val condition, string target)
    | Label(string)
    | FunCall(string fun_name, val* args, val? dst)
val = Constant(const) | Var(string)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Mulitply | Divide | Remainder | Equal | Not Equal
                | LessThan | LessOrEaual | GreaterThan | GreaterOrEqual
```

# TACKYGEN

```
// NEW
get_member_offset(member, type):
    if type is Structure(tag):
        m = type_table.find(tag).members.find(member)
        if m is not found:
            fail("Internal error: failed to find member in struct")
        else:
            return m.offset
    else:
        fail("Internal error: tried to get offset of member within non-structure type")
```

```
// NEW
get_member_pointer_offset(member, type):
    if type is Pointer(t):
        return get_member_offset(member, t)
    else:
        fail("Internal error: trying to get member through pointer but is not a pointer type")
```

```
type exp_result =
    | PlainOperand(Tacky.Val val)
    | DereferencedPointer(Tacky.Val val)
    | SubObject(string base, int offset)
```

```
// Add cases for Dot and Arrow
emit_tacky_for_exp(exp):
	match exp.type:
		case AST.Constant(c):
            return ([], PlainOperand(TACKY.Constant(c)))
		case AST.Var(v):
            return ([], PlainOperand(TACKY.Var(v)))
		case AST.String(s):
            str_id = symbols.add_string(s)
            return ([], PlainOperand(Var(str_id)))
        case Unary:
			if exp.op is increment:
                const_t = if TypeUtils.is_pointer(exp.type) then Types.Long else type
				return emit_compound_expression(AST.Add, exp.exp, mk_ast_const(const_t, 1), exp.type)
			else if exp.op is decrement:
                const_t = if TypeUtils.is_pointer(exp.type) then Types.Long else type
				return emit_compound_expression(AST.Subtract, exp.exp, mk_ast_const(const_t, 1), exp.type)
			else:
				return emit_unary_expression(exp)
        case Cast:
            return emit_cast_expression(exp)
		case Binary:
			if exp.op is And:
				return emit_and_expression(exp)
			else if exp.op is Or:
				return emit_or_expression(exp)
			else:
                if exp.op is Add and TypeUtils.is_pointer(exp.type):
                    return emit_pointer_addition(exp.type, exp.e1, exp.e2)
                else if exp.op is Subtract and TypeUtils.is_pointer(exp.type):
                    return emit_subtraction_from_pointer(exp.type, exp.e1, exp.e2)
                else if exp.op is Subtract and TypeUtils.is_pointer(exp.e1.type):
                    // at least one operand is pointer but result isn't, must be subtracting one pointer from another
                    return emit_pointer_diff(exp.type, exp.e1, exp.e2)

                return emit_binary_expression(exp)
		case Assignment:
                return emit_assignment(exp.left, exp.right)
		case CompoundAssignment:
			return emit_compound_expression(exp.op, exp.left, exp.right, exp.type)
		case PostfixIncr:
			return emit_postfix(AST.Add, exp.exp)
		case PostfixDecr:
			return emit_postfix(AST.Subtract, exp.exp)
        case Conditional:
            return emit_conditional_expression(exp)
        case FunCall:
            return emit_fun_call(exp)
        case Dereference:
            return emit_dereference(exp.exp)
        case AddrOf:
            return emit_addr_of(type, exp.exp)
        case Subscript:
            return emit_subscript(exp.type, exp.e1, exp.e2)
        case SizeOfT:
            return ([], PlainOperand(eval_size(exp.type)))
        case SizeOf:
            return ([], PlainOperand(eval_size(exp.exp.type)))
        case Dot:
            return emit_dot_operator(exp.type, exp.strct, exp.member)
        case Arrow:
            return emit_arrow_operator(exp.type, exp.strct, exp.member)
```

```

// Add check if lval is SubObject
emit_postfix(op, inner):
    /*  If LHS is a variable:
            dst = lhs
            lhs = lhs <op> 1
        If LHS is a pointer:
            dst = load(ptr)
            tmp = dst <op> 1
            store(tmp, ptr)

        If LHS has a pointer type, we implement <op> with AddPtr
        otherwise with Binary instruction
    */

    // define var for result - i.e. value of lval BEFORE incr or decr
	dst = TACKY.Var(create_tmp(inner.t))

    // evaluate inner to get exp_result
    insts, lval = emit_tacky_for_exp(inner)
    /*  Helper to construct Binary or AddPtr instruction from operands, depending on type
        Note that dst is destination of this instruction rather than the whole expression
        (i.e. it's the lvalue we're updating, or a temporary we'll then store to that lvalue)
    */

    do_op(op, src, dst):
        if op is Add:
            index = TACKY.Constant(mk_const(Long, 1))
        else if op is Subtract:
            index = TACKY.Constant(mk_const(Long, -1))
        else:
            fail("Internal error")

        if TypeUtils.is_pointer(inner.type):
            return Addptr(src, index, get_ptr_scale(inner.type), dst)
        else:
            one = TACKY.Constant(mk_const(inner.type, 1))
            return Binary(convert_binop(op), src1=src, src2=one, dst)

    // copy result to dst and perform incr or decr
    if lval is PlainOperand(Var(name)):
        /*
            dst = v
            v = v + 1 // v - 1
        */
        oper_insts = [
            Copy(Var(name), dst),
            do_op(op, Var(name), Var(name)),
        ]
    else if lval is DereferencedPointer(p):
        /*
            dst = Load(p)
            tmp = dst + 1 // dst - 1
            Store(tmp, p)
        */
        tmp = Var(create_tmp(inner.type))
        oper_insts = [
            Load(src_ptr=p, dst),
            do_op(op, dst, tmp)
            Store(tmp, dst_ptr=p),
        ]
    else if lval is SubObject(base, offset):
        /*
            dst = CopyFromOffset(base, offset)
            tmp = dst + 1 // or dst - 1
            CopyToOffset(tmp, base, offset)
        */
        tmp = Var(create_tmp(inner.type))
        oper_insts = [
            CopyFromOffset(src=base, offset,  dst=tmp),
            do_op(op, dst, tmp),
            CopyToOffset(src=tmp, dst=base, offset)
        ]

	return ([...insts, ...oper_insts], PlainOperand(dst))
```

```
// Update the comment
// add case for SubObject
emit_compound_expression(op, lhs, rhs, result_type):
    /*
        if LHS is var w/ same type as result:
            lhs = lhs <op> rval
        if LHS is a var w/ different type:
            tmp = cast(lhs)
            tmp = tmp <op> rval
            lhs = cast(tmp)
        if LHS is pointer w/ same type:
            tmp = load(lhs_ptr)
            tmp = tmp <op> rval
            store(tmp, lhs_ptr)
        if LHS is pointer w/ diff type:
            tmp = load(lhs_ptr)
            tmp2 = cast(tmp)
            tmp2 = tmp2 <op> rval
            tmp = cast(tmp2)
            store(tmp, rhs_ptr)
        if LHS is subobject w/ same type:
            tmp = CopyFromOffset(lhs, offset)
            tmp = tmp <op> rval
            CopyToOffset(tmp, lhs, offset)
        if LHS is suboject w/ different type:
            tmp = CopyFromOffset(lhs, offset)
            tmp2 = cast(tmp)
            tmp2 = tmp2 <op> rval
            tmp = cast(tmp2)
            CopyToOffset(tmp, lhs, offset)
    */
    lhs_type = lhs.type
    // evaluate LHS
    eval_lhs, lhs = emit_tacky_for_exp(lhs)
    // evaluate RHS - type checker already added conversion to common type if one is needed
    eval_rhs, rhs = emit_tacky_and_convert(rhs)
    /*
        If LHS is a variable, we can update it directly.
        If it's a dereferenced pointer, we need to load it into a temporary variable, operate on that, and then store it.
    */
    if lhs is PlainOperand(dst):
        dst = dst
        load_inst = []
        store_inst = []
    else if lhs is DereferencedPointer(p):
        dst = Var(create_tmp(lhs_type))
        load_inst = [ Load(src_ptr=p, dst) ]
        store_inst = [ Store(dst, dst_ptr=p) ]
    else if lhs is SubObject(base, offset):
        dst = Var(create_tmp(lhs_type))
        load_inst = [ CopyFromOffset(src=base, offset, dst) ]
        store_inst = [ CopyToOffset(src=dst, dst=base, offset) ]

    /*
        If LHS type and result type are the same, we can operate on dst directly.
        Otherwise, we need to cast dst to correct type before operation, then cast result
        back and assign to dst
    */
    if lhs_type == result_type:
        result_var = dst
        cast_to = []
        cast_from = []
    else:
        tmp = Var(create_tmp(result_type))
        cast_lhs_to_tmp = get_cast_instruction(dst, tmp, lhs_type, resul_type)
        cast_tmp_to_lhs = get_cast_instruction(tmp, dst, result_type, lhs_type)

        result_var = tmp
        cast_to = [ cast_lhs_to_tmp ]
        cast_from = [ cast_tmp_to_lhs ]

    do_operation():
        if TypeUtils.is_pointer(result_t):
            scale = get_ptr_scale(result_t)
            if op is Add:
                return [ AddPtr(result_var, rhs, scale, result_var) ]
            else if op is Subtract:
                negated_index = Var(create_tmp(Types.Long))
                return [
                    Unary(Negae, rhs, negated_index),
                    AddPtr(result_var, negated_index, scale, result_var),
                ]
            else: fail("Internal error in compound assignment")
        else:
            return [
                Binary(
                op=convert_binop(op),
                src1=result_var,
                src2=rhs,
                dst=result_var)
            ]

    insts = [
        ...eval_lhs,
        ...eval_rhs,
        ...load_inst,
        ...cast_to,
        ...do_operation(),
        ...cast_from,
        ...store_inst,
    ]

    return (insts, PlainOperand(dst))
```

```
// Add case for SubObject
emit_assignment(lhs, rhs):
    lhs_insts, lval = emit_tacky_for_exp(lhs)
    rhs_insts, rval = emit_tacky_and_convert(rhs)
    insts = [
        ...lhs_insts,
        ...rhs_insts,
    ]

    if lval is PlainOperand(o):
        insts.append(
            Copy(rval, o)
        )
        return (insts, lval)
    else if lval is DereferencedPointer(ptr):
        insts.append(
            Store(rval, ptr)
        )
    else if lval is SubObject(base, offset):
        insts.append(
            CopyToOffset(src=rval, offset, dst=base)
        )
    return (insts, PlainOperand(rval))
```

```
// NEW
emit_dot_operator(t, strct, member):
    member_offset = get_member_offset(member, strct.type)
    insts, inner_obj = emit_tacky_for_exp(strct)

    if inner_obj is PlainOperand(Var(v)):
        return (insts, SubObject(v, member_offset))

    if inner_obj is SubObject(base, offset):
        return (insts, SubObject(base, offset + member_offset))

    if inner_obj is DereferencedPointer(ptr):
        if member_offset == 0:
            return (insts, inner_obj)
        else:
            dst = Var(create_tmp(Pointer(t)))
            index = Constant(ConstLong(member_offset))
            add_ptr_inst = AddPtr(ptr, index, scale=1, dst)
            return ([...insts, add_ptr_inst], DereferencedPointer(dst))

    else: // is PlainOperand(Constant):
        fail("Internal error: found dot operator applied to constant")
```

```
// NEW
emit_arrow_operator(t, strct, member):
    member_offset = get_member_pointer_offset(member, strct.type)
    insts, ptr = emit_tacky_and_convert(strct)
    if member_offset == 0:
        return (insts, DereferencedPtr(ptr))
    else:
        dst = Var(create_tmp(Pointer(t)))
        index = Constant(ConstLong(member_offset))
        add_ptr_inst = AddPtr(ptr, index, scale=1, dst)
        return ([...insts, add_ptr_inst], DereferencedPointer(dst))
```

```
// Add case for SubObject
emit_addr_of(Types.t type, inner):
    insts, result = emit_tacky_for_exp(inner)
    if result is PlainOperand(o):
        dst = Var(create_tmp(type))
        insts.append(
            GetAddress(src=o, dst)
        )
        return (insts, PlainOperand(dst))

    else if result is DereferencedPointer(ptr):
        return (insts, PlainOperand(ptr))

    else if result is SubObject(base, offset):
        dst = Var(create_tmp(type))
        get_addr = GetAddress(src=Var(base), dst)
        if offset == 0:
            // skip AddPtr if offset is 0
            return ([...insts, get_addr], PlainOperand(dst))
        else:
            index = Constant(ConstLong(offset))
            return ([...insts, get_addr, AddPtr(ptr=dst, index, scale=1, dst)], PlainOperand(dst))
```

```
// Add case for SubObject
emit_tacky_and_convert(e):
    insts, result = emit_tacky_for_exp(e)
    if result is PlainOperand(o):
        return (insts, o)

    else if result is DereferencedPointer(ptr):
        dst = Var(create_tmp(e.t))
        return (
            [...insts, Load(src_ptr=ptr, dst)],
            dst
        )

    else if result is SubObject(base, offset):
        dst = Var(create_tmp(e.t))
        return (
            [...insts, CopyFromOffset(src=base, offset, dst)],
            dst
        )
```

```
// Add the case of CompoundInit(Structure(tag), inits)
emit_compound_init(init, name, offset):
    if init is SingleInit(String(s), t=Array(size, _)):
        str_bytes = str_to_bytes(s)
        padding_bytes = to_bytes([0] * size-len(s))
        return emit_string_init([...str_bytes, ...padding_bytes], name, offset)
    else if init is SingleInit(e):
        eval_init, v = emit_tacky_and_convert(e)
        return [...eval_init, CopyToOffset(src=v, dst=name, offset)]
    else if init is CompoundInit:
        new_inits = []

        if init.type is Array(elem_type):
            for idx, elem_init in init.inits:
                new_ofset = offset + (idx + TypeUtils.get_size(elem_type))
                new_inits.append(...emit_compound_init(elem_init, name, new_offset))
            return new_inits

        else if init.type is Structure(tag):
            members = type_table.get_members(tag)
            for idx, init in init.inits:
                memb = members[idx]
                mem_offset = offset + memb.offset
                new_inits.append(...emit_compound_init(init, name, mem_offset))
            return new_inits

        else:
            fail("Internal error: compound init has non-array type!")
```

```
// Else branch already handles the StructDecl case
emit_local_declaration(decl):
    if decl is VarDecl:
        if decl.storage_class is not null:
            return []

        return emit_var_declaration(decl.decl)
    else: // FunDecl or StructDecl
        return []
```

# Assembly

Update the Data operand to have offset

```
program = Program(top_level*)
asm_type =
    | Byte
    | Longword
    | Quadword
    | Double
    | ByteArray(int size, int alignment)
top_level =
    | Function(string name, bool global, instruction* instructions)
    | StaticVariable(string name, bool global, int alignment, static_init* inits)
    | StaticConstant(string name, int alignment, static_init init)
instruction =
    | Mov(asm_type, operand src, operand dst)
    | Movsx(asm_type src_type, asm_type, dst_type, operand src, operand dst)
    | MovZeroExtend(asm_type src_type, asm_type, operand src, operand dst)
    | Lea(operand src, operand dst)
    | Cvttsd2si(asm_type, operand src, operand dst)
    | Cvtsi2sd(asm_type, operand src, operand dst)
    | Unary(unary_operator, asm_type, operand dst)
    | Binary(binary_operator, asm_type, operand, operand)
    | Cmp(asm_type, operand, operand)
    | Idiv(asm_type, operand)
    | Div(asm_type, operand)
    | Cdq(asm_type)
    | Jmp(string)
    | JmpCC(cond_code, string)
    | SetCC(cond_code, operand)
    | Label(string)
    | Push(operand)
    | Call(string)
    | Ret
unary_operator = Neg | Not | ShrOneOp
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor | Sal | Sar | Shr | Shl
operand = Imm(int64) | Reg(reg) | Pseudo(string) | Memory(reg, int) | Data(string, int offset) | PseudoMem(string, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
```

# AssemblySymbols

```
// Fun has return_on_stack flag
type entry =
    | Fun(bool defined, int bytes_required, bool return_on_stack)
    | Obj(Assembly.asm_type t, bool is_static, bool is_constant)
```

```
// return_on_stack flag
add_fun(fun_name, defined, return_on_stack):
    symbol_table[fun_name] = Fun(defined, 0, return_on_stack)
```

```
set_bytes_required(fun_name, bytes_required):
    if symbol_table has fun_name:
        old_entry = symbol_table.get(fun_name)
        symbol_table[fun_name] = Fun(...old_entry, bytes_required)
    else:
        fail("Internal error: function is not defined")
```

```
// NEW
get_type(var_name):
    entry = symbol_table.find(var_name)
    if entry is Obj(t, _):
        return t
    else: // is Fun
        fail("Internal error: this is a function, not an object")
```

```
// NEW
returns_on_stack(fun_name):
    entry = symbol_table.find(fun_name)
    if entry is Fun:
        return entry.return_on_stack
    else: // is Obj
        fail("Internal error: this is an object, not a function")
```

# CodeGen

```
// NEW
/*
Get the operand type we should use to move an eightbyte of a struct.
If it contains exactly 8, 4, or 1 bytes, use the corresponding type (note that all but the last
eightbyte of a struct are exactly 8 bytes). If it's an ueven size s
*/
get_eightbyte_type(eightbyte_idx, total_var_size):
    bytes_left = total_var_size - (eightbyte_idx * 8)
    if bytes_left >= 8:
        return Quadword
    if bytes_left == 4:
        return Longword
    if bytes_left == 1:
        return Byte
    else:
        return ByteArray(size=bytes_left, alignment=8)
```

```
// NEW
add_offset(n, operand):
    if operand is PseudoMem(base, off):
        return PseudoMem(base, off + n)

    if operand is Memory(r, off):
        return Memory(r, of + n)

    // you could do pointer arithmetic w/ indexed or data operands but we don't need to
    if operand is Imm, or Reg, or Pseudo, or Indexed, or Data:
        fail("Internal error: trying to copy data to or from non-memory operand")
```

```
// NEW
copy_bytes(src_val, dst_val, byte_count):
    insts = []

    if byte_count == 0:
        return insts

    while byte_count > 0:
        operand_type = null
        operand_size = null

        if byte_count < 4:
            operand_type = Byte
            operand_size = 1
        else if byte_count < 8:
            operand_type = Longword
            operand_size = 4
        else:
            operand_type = Quadword
            operand_size = 8

        insts.append(Mov(operand_type, src_val, dst_val))

        src_val = add_offset(operand_size, src_val)
        dst_val = add_offset(operand_size, dst_val)
        byte_count = byte_count - operand_size

    return insts
```

```
// NEW
/*
copy an uneven, smaller-than-quadword eightbyte from memory into a register:
repeatedly copy byte into register and shift left, starting w/ highest byte and working down to lowest
*/
copy_bytes_to_reg(src_val, dst_reg, byte_count):
    insts = []

    for i from byte_count - 1 down to 0:
        mv = Mov(Byte, add_offset(i, src_val), Reg(dst_reg))
        insts.append(mv)

        if i != 0:
            insts.append(Binary(Shl, Quadword, Imm(8), Reg(dst_reg)))

    return insts
```

```
// NEW
/*
copy an uneven, smaller-than-quadword eightbyte from a register into memory;
repeatedly copy byte into register and shift right, starting w/ byte 0  and working up
*/
copy_bytes_from_reg(src_reg, dst_val, byte_count):
    insts = []

    for i from 0 to byte_count - 1:
        mv = Mov(Byte, Reg(src_reg), add_offset(i, dst_val))
        insts.append(mv)

        if i < byte_count - 1:
            insts.append(Binary(Shr, Quadword, Imm(8), Reg(src_reg)))

    return insts
```

```
// Data operand has new field of int
// and invert the logic to get PseudoMem from Var
convert_val(Tacky.Val val):
    match val.type:
        case Tacky.Constant(ConstChar(c)):
            return Assembly.Imm((int64) c)
        case Tacky.Constant(ConstUChar(uc)):
            return Assembly.Imm((int64) uc)
        case Tacky.Constant(ConstInt(i)):
            return Assembly.Imm((int64) i)
        case Tacky.Constant(ConstLong(l)):
            return Assembly.Imm(l)
        case Tacky.Constant(ConstUInt(u)):
            return Assembly.Imm((uint64) u)
        case Tacky.Constant(ConstULong(ul)):
            return Assembly.Imm((uint64) ul)
        case Tacky.Double(ConstDouble(d)):
            return Assembly.Data(add_constant(d), 0)
        case Tacky.Var:
            if TypeUtils.is_scalar(symbols.get(val.name)):
                return Assembly.Pseudo(val.name)
            return Assembly.PseudoMem(val.name, 0)
```

```
// Structure is like Array
convert_type(Types.t type):
    if type is Int or UInt:
        return asm_type.Longword
    else if type is Long or ULong or Pointer:
        return asm_type.Quadword
    else if type is Char or SChar or UChar:
        return asm_type.Byte
    else if type is Double:
        return asm_type.Double
    else if type is Array or Structure:
        return ByteArray(
            size=TypeUtils.get_size(type),
            alignment=TypeUtils.get_alignment(type)
        )
    else:
        fail("Internal error: converting function type to assembly")
```

We define some classes for Eightbytes and create/update helper functions with parameters, arguments and return value from functionc call.

```
// NEW
type cls = Mem | SSE | INTEGER
```

```
// NEW
classify_new_structure(tag):
    size = type_table.find(tag).size

    if size > 16:
        eightbyte_count = (size / 8) + (0 if size % 8 == 0 else 1)
        return <a list of Mem with size of eightbyte_count>

    process_type = (t) ->
    {
        if t is Structure(struct_tag):
            member_types = type_table.get_member_types(struct_tag)
            processed_types = []
            for memb_type in member_types:
                processed_types.append(...process_type(memb_type))
            return processed_types
        elif t is Array:
            processed_types = <create array of calling process_type on elem_type>
            return flatten(processed_types)
        else:
            return [t]
    }

    scalar_types = process_type(Structure(tag))
    first, last = scalar_types[0], scalar_types[-1]

    if size > 8:
        first_class = SSE if first == Double else Integer
        last_class = SSE if last == Double else Integer
        return [first_class, last_class]
    elif first == Double:
        return [SSE]
    else:
        return [Integer]
```

```
// NEW
// memoize results of classify_structure
classified_structures = Map()
```

```
// NEW
classify_structure(tag):
    if classified_structures.has(tag):
        return classified_structures.get(tag) as classes
    else:
        classes = classify_new_structure(tag)
        classified_structures.add(tag, classes)
        return classes
```

```
classify_tacky_val(v):
    if tacky_type(v) is Structure(tag):
        return classify_structure(tag)
    else:
        fail("Internal error: trying to classify non-structure type")
```

```
classify_parameters(tacky_vals, return_on_stack):
    if return_on_stack == true:
        int_regs_available = 5
    else:
        int_regs_available = 6

    int_reg_args = []
    dbl_reg_args = []
    stack_args   = []

    for each v in tacky_vals:
        operand = convert_val(v)
        t = asm_type(v)
        typed_operand = (t, operand)

        if t == Double:
            if length(dbl_reg_args) < 8:
                dbl_reg_args.append(operand)
            else:
                stack_args.append(typed_operand)

        else if t is one of {Byte, Longword, Quadword}:
            if length(int_reg_args) < int_regs_available:
                int_reg_args.append(typed_operand)
            else:
                stack_args.append(typed_operand)

        else if t is a structure (ByteArray):
            // it's a structure
            if v is Tacky.Var(v):
                var_name = v
            else:
                fail("Internal error: constant byte array")

            var_size = get_size(tacky_type(v))
            classes = classify_tacky_val(v)
            use_stack = True

            if classes[0] == Mem:
                // all eightbytes go on the stack
                use_stack = true
            else:
                // tentative assign eigthbytes to registers
                tentative_ints = copy of int_reg_args
                tentative_dbls = copy of dbl_reg_args

                for i, cls in classes:
                    operand = PseudoMem(var_name, i * 8)
                    if cls == SSE:
                        tentative_dbls.append(operand)
                    else if cls == Integer:
                        eightbyte_type = get_eightbyte_type(eightbyte_idx = i, total_var_size = var_size)
                        tentative_ints.append(eightbyte_type, operand)
                    else if cls == Mem:
                        fail("Internal error: found eightbyte in Mem class, but first eighbyte wasn't Mem")

                if length(tentative_ints) ≤ int_regs_available and length(tentative_dbls) ≤ 8:
                    int_reg_args = tentative_ints
                    dbl_reg_args = tentative_dbls
                    use_stack = false
                else:
                    use_stack = true

            if use_stack == true:
                for i from 0 to length(classes) - 1:
                    eightbyte_type = get_eightbyte_type(eightbyte_idx = i, total_var_size = var_size)
                    stack_args.append(eightbyte_type, PseudoMem(var_name, i * 8))

    return (int_reg_args, dbl_reg_args, stack_args)
```

```
classify_return_value(retval):
    retval_type = tacky_type(retval)

    if retval_type is Structure(tag)
        classes = classify_structure(tag)

        if retval is Var(v)
            var_name = v
        else:
            fail("Internal error: constant with structure type")

        if classes[0] == Mem:
            return ([], [], True)
        else:
            // return in registers, can move everything w/ quadword operands
            int_retvals = []
            double_retvals = []

            for i from 0 to length(classes) - 1:
                cls = classes[i]
                operand = PseudoMem(var_name, i * 8)
                if cls == SSE:
                    double_retvals.append(operand)
                else if cls == INTEGER:
                    eightbyte_type = get_eightbyte_type(eightbyte_idx = i, total_var_size = get_size(retval_type))
                    int_retvals.append((eightbyte_type, operand))
                else if cls == Mem:
                    fail("Internal error: found eightbyte in Mem class unexpectedly")
            return (int_retvals, double_retvals, False)

    else if retval_type == Double:
        asm_val = convert_val(retval)
        return ([], [asm_val], False)

    else:
        typed_operand = (asm_type(retval), convert_val(retval))
        return ([typed_operand], [], False)
```

```
// add checking the return value on top.
// update the passing args in int_regs and on stack.
// update how to retrieve value in the end
convert_function_call(TACKY.FunCall fun_call)
    int_retvals = []
    dbl_retvals = []
    return_on_stack = false

    if (fn_call.dst):
        int_retvals, dbl_retvals, return_on_stack = classify_return_value(fun_call.dst)

    // load address of dest into DI
    first_intreg_idx = 0
    load_dst_inst = null

    if return_on_stack:
        first_intreg_idx = 1
        load_dst_inst = Lea(convert_val(fun_call.dst), Reg(DI))

    int_reg_args, dbl_reg_args, stack_args = classify_parameters(fun_call.args, return_on_stack)

    // adjust stack alignment
    stack_padding = length(stack_args) % 2 == 0 ? 0 : 8
    insts = [ load_dst_inst ]

    if stack_padding != 0:
        inst.append(
            Assembly.Binary(
                op=Sub,
                t=Quadword,
                src=Imm((int64) stack_padding)
                dst=Reg(SP)
            )
        )

    // pass args in registers
    reg_idx = 0
	for arg_t, arg in int_reg_args:
		r = int_param_passing_regs[reg_idx + first_intreg_idx]
        if arg_t is ByteArray(size):
            ints.append(...copy_bytes_to_reg(arg, r, size))
        else:
            insts.append(Mov(arg_t, arg, Reg(r)))
		reg_idx += 1

    // pass args in registers
    reg_idx = 0
	for arg in dbl_reg_args:
		r = dbl_param_passing_regs[reg_idx]
        insts.append(Mov(Double, arg, Reg(r)))
		reg_idx += 1

   // pass args on the stack
    for arg_t, arg in reverse(stack_args):
		if arg is (Imm | Reg) or arg_t is (Quadword | Double):
			insts.append(Push(arg))
        else if arg_t is ByteArray(size):
            insts.append(
                Binary(Sub, Quadword, Imm(8), Reg(SP)),
                ...copy_bytes(arg, Memory(Reg(SP), 0), size)
            )
		else:
            // copy into a register before pushing
            insts.append(Mov(arg_t, arg, Reg(AX)))
            insts.append(Push(Reg(AX)))

    insts.append(Call(fun_call.name))

    // adjust stack pointer
    bytes_to_remove = 8 * length(stack_args) + stack_padding
    if bytes_to_remove != 0:
        insts.append(
            Assembly.Binary(
                op=Add,
                t=Quadword,
                src=Imm((int64) bytes_to_remove),
                dst=Reg(SP)
            )
        )

    // Retrieve return value
    int_ret_regs = [AX, DX]
    dbl_ret_regs = [XMM0, XMM1]

    retrieve_result = []
    if fun_call.dst is not null and return_on_stack is false:
        for i from 0 to length(int_retvals)-1:
            r = int_ret_regs[i]
            t, op = int_retvals[i]
            if t is ByteArray(size, _):
                retrieve_result.append(...copy_bytes_from_reg(r, op, size))
            else:
                retrieve_result.append(Mov(t, Reg(r), op))

        for i from 0 to length(dbl_retvals)-1:
            r = dbl_ret_regs[i]
            op = dbl_retvals[i]
            retrieve_result.append(Mov(Double, Reg(r), op))
    else:
        retrieve_result = []

    return [...insts, ...retrieve_result]
```

```
convert_return_instruction(retval):
    if retval is None:
        return [Assembly.Ret]

    int_retvals, dbl_retvals, return_on_stack = classify_return_value(retval)

    if return_on_stack:
        byte_count = get_size(tacky_type(retval))
        get_ptr = Mov(Quadword, Memory(BP, -8), Reg(AX))
        copy_into_ptr = copy_bytes(convert_val(retval), Memory(AX, 0), byte_count)
        return [get_ptr, copy_into_ptr, Ret ]
    else:
        return_ints = []
        for i from 0 to length(int_retvals)-1:
            t, op = int_retvals[i]
            dst_reg = [AX, DX][i]
            if t is ByteArray(size):
                return_ints.append(...copy_bytes_to_reg(op, dst_reg, byte_count=size))
            else:
                return_ints.append(Mov(t, op, Reg(dst_reg))),

        return_dbls = []
        for i from 0 to length(dbl_retvals)-1:
            op = dbl_retvals[i]
            return_dbls.append(Mov(Double, op, Reg([XMM0, XMM1][i])))

        return [...return_ints, ...return_dbls, Ret]
```

```
// Update Copy, Return, Unary negate on double (Data operand), Load, Store, Data operand in DoubleToUInt, CopyToOffset, CopyFromOffset
convert_instruction(Tacky.Instruction inst):
    match inst type:
        case Copy:
            if TypeUtils.is_scalar(tacky_type(inst.src)):
                t = asm_type(inst.src)
                asm_src = convert_val(inst.src)
                asm_dst = convert_Val(inst.dst)

                return [Mov(t, asm_src, asm_dst)]
            else:
                asm_src = convert_val(inst.src)
                asm_dst = convert_Val(inst.dst)
                byte_count = TypeUtils.get_size(tacky_type(inst.src))

                return copy_bytes(asm_src, asm_dst, byte_count)
        case Return:
            return convert_return_instruction(inst)
        case Unary:
            match inst.op:
                case Not:
                    src_t = asm_type(inst.src)
                    dst_t = asm_type(inst.dst)
                    asm_src = convert_val(inst.src)
                    asm_dst = convert_val(inst.dst)

                    if src_t == Double:
                        insts = [
                            Binary(Xor,t=Double src=Reg(XMM0), dst=Reg(XMM0)),
                            Cmp(src_t, asm_src, Reg(XMM0)),
                            Mov(dst_t, zero(), asm_dst),
                            SetCC(E, asm_dst),

                            // cmp with NaN sets both ZF and PF, but !NaN should evaluate to 0,
                            // so we'll calculate:
                            // !x = ZF && !PF

                            SetCC(NP, Reg(R9)),
                            Binary(And, dst_t, Reg(R9), asm_dst),
                        ]

                        return insts
                    else:
                        return [
                            Cmp(src_t, Imm(0), asm_src),
                            Mov(dst_t, Imm(0), asm_dst),
                            SetCC(E, asm_dst)
                        ]
                case Negate:
                    if tacky_type(src) == Double:
                        asm_src = convert_val(inst.src)
                        asm_dst = convert_val(inst.dst)
                        negative_zero = add_constant(-0.0, 16)

                        return [
                            Mov(Double, asm_src, asm_dst),
                            Binary(op=Xor, t=Double, src=Data(negative_zero, 0), dst=asm_dst)
                        ]
                    else:
                        // Fall through to the default case
                default:
                    t = asm_t(inst.src)
                    asm_op = convert_op(inst.op)
                    asm_src = convert_val(inst.src)
                    asm_dst = convert_val(inst.dst)

                    return [
                        Assembly.Mov(t, asm_src, asm_dst),
                        Assembly.Unary(asm_op, t, asm_dst)
                    ]
        case Binary:
            src_t = asm_type(inst.src1)
            dst_t = asm_type(inst.dst)
            asm_src1 = convert_val(inst.src1)
            asm_src2 = convert_val(inst.src2)
            asm_dst = convert_dst(inst.dst)

            match inst.op:
                (* Relational operator *)
                case Equal:
                case NotEqual:
                case GreaterThan:
                case GreaterOrEqual:
                case LessThan:
                case LessOrEqual:
                    if src_t == Double:
                        return convert_dbl_comparison(inst.op, dst_t, asm_src1, asm_src2, asm_dst)
                    else:
                        signed = src_t == Double
                            ? false
                            : TypeUtils.is_signed(tacky_type(inst.src1))

                        cond_code = convert_cond_code(inst.op, signed)

                        return [
                            Cmp(src_t, asm_src2, asm_src1),
                            Mov(dst_t, zero(), asm_dst),
                            SetCC(cond_code, asm_dst),
                        ]

                (* Division/modulo *)
                case Divide:
                case Mod:
                    if src_t != Double:
                        result_reg = op == Divide ? AX : DX

                        if (TypeUtils.is_signed(tacky_type(inst.src1))):
                            return [
                                Mov(src_t, asm_src1, Reg(AX)),
                                Cdq(src_t),
                                Idiv(src_t, asm_src2),
                                Mov(src_t, Reg(result_reg), asm_dst)
                            ]
                        else:
                            return [
                                Mov(src_t, asm_src1, Reg(AX)),
                                Mov(src_t, zero(), Reg(DX)),
                                Div(src_t, asm_src2),
                                Mov(src_t, Reg(result_reg), asm_dst)
                            ]
                    else:
                        // Jump to the default case
                        break
                case BiftShiftLeft:
                case BiftShiftRight:
                    is_signed = TypeUtils.is_signed(tacky_type(inst.src1))
                    asm_op = convert_shift_op(inst.op, is_signed)
                    asm_t = asm_type(inst.src1)

                    match type of asm_src2:
                        case Imm:
                            return [
                                Mov(asm_t, asm_src1, asm_dst),
                                Binary(asm_op, t=asm_t, asm_src2, asm_dst)
                            ]
                        default:
                            // NOTE: only lower byte of cx is used.
                            return [
                                Mov(asm_t, asm_src1, asm_dst),
                                Mov(Byte, asm_src2, Reg(CX)),
                                Binary(asm_op, asm_t, Reg(CX), asm_dst)
                            ]
                (* Addition/subtraction/mutliplication/and/or/xor *)
                default:
                    asm_op = convert_binop(inst.op)

                    return [
                        Mov(src_t, asm_src1, asm_dst),
                        Binary(asm_op, src_t, asm_src2, asm_dst)
                    ]
        case Load:
            if TypeUtils.is_scalar(tacky_type(inst.dst)):
                asm_src_ptr = convert_val(inst.str_ptr)
                asm_dst = convert_val(inst.dst)
                t = asm_type(inst.dst)
                return [
                    Mov(Quadword, asm_src_ptr, Reg(A9)),
                    Mov(t, Memory(Reg(R9), 0), asm_dst),
                ]
            else:
                asm_src_ptr = convert_val(inst.src_ptr)
                asm_dst = convert_val(inst.dst)
                byte_count = TypeUtils.get_size(tacky_type(inst.dst))
                return [
                    Mov(Quadword, asm_src_ptr, Reg(A9)),
                    ...copy_bytes(Memory(Reg(R9), 0), asm_dst, byte_count)
                ]
        case Store:
            if TypeUtils.is_scalar(tacky_type(inst.src)):
                asm_src = convert_val(inst.src)
                t = asm_type(inst.src)
                asm_dst_ptr = convert_val(inst.dst_ptr)
                return [
                    Mov(Quadword, asm_dst_ptr, Reg(R9)),
                    Mov(t, asm_src, Memory(Reg(R9), 0)),
                ]
            else:
                asm_src = convert_val(inst.src)
                asm_dst_ptr = convert_val(inst.dst_ptr)
                byte_count = TypeUtils.get_size(tacky_type(inst.src))
                return [
                    Mov(Quadword, asm_dst_ptr, Reg(R9)),
                    ...copy_bytes(asm_src, Memory(Reg(R9), 0), byte_count)
                ]
        case GetAddress:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            return [ Lea(asm_src, asm_dst) ]
        case Jump:
            return [ Jmp(inst.target) ]

        case JumpIfZero:
            t = asm_type(inst.cond)
            asm_cond = convert_val(inst.cond)

            if t == Double:
                compare_to_zero = [
                    Binary(Xor, Double, Reg(XMM0), Reg(XMM0)),
                    Cmp(t, asm_cond, Reg(XMM0)),
                ]

                lbl = UniqueIds.make_label("nan.jmp.end")
                conditional_jmp = [
                    // Comparison to NaN sets ZF and PF flags;
                    // to treat NaN as nonzero, skip over je instruction if PF flag is set
                    JmpCC(P, lbl),
                    JmpCC(E, inst.target),
                    Label(lbl),
                ]

                return [...compare_to_zero, ...conditional_jmp]
            else:
                return [
                    Cmp(t, zero(), asm_cond),
                    JmpCC(E, inst.target),
                ]
        case JumpIfNotZero:
            t = asm_type(inst.cond)
            asm_cond = convert_val(inst.cond)

            if t == Double:
                return [
                    Binary(Xor, Double, Reg(XMM0), Reg(XMM0)),
                    Cmp(t, asm_cond, Reg(XMM0)),
                    JmpCC(NE, inst.target),

                    // Also jumpt to target on Nan, which is nonzero
                    JmpCC(P, target),
                ]

            else:
                return [
                    Cmp(t, zero(), asm_cond),
                    JmpCC(NE, inst.target),
                ]

        case Label:
            return [ Assembly.Label(inst.name) ]
        case FunCall:
            return convert_function_call(inst)
        case SignExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ Movsx(asm_type(src), asm_type(dst), asm_src, asm_dst) ]
        case Truncate:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ Mov(asm_type(dst), asm_src, asm_dst) ]
        case ZeroExtend:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            return [ MovZeroExtend(asm_type(src), asm_type(dst), asm_src, asm_dst) ]
        case IntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(src)

            if t is Byte:
                return [
                    Movsx(Byte, Longword, asm_src, Reg(R9)),
                    Cvtsi2sd(Longword, Reg(R9), asm_dst)
                ]

            return [ Cvtsi2sd(t, asm_src, asm_dst) ]
        case DoubleToInt:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)
            t = asm_type(asm_dst)

            if t is Byte:
                return [
                    Cvttsd2si(Longword, asm_src, Reg(R9)),
                    Mov(Byte, Reg(R9), asm_dst)
                ]

            return [
                Cvttsd2si(t, asm_src, asm_dst)
            ]
        case UIntToDouble:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            if tacky_type(src) == Types.UChar:
                return [
                    MovZeroExtend(Byte, Longword, asm_src, Reg(R9)),
                    Cvtsi2sd(Longword, Reg(R9), asm_dst)
                ]
            else if tacky_type(src) == Types.UInt:
                return [
                    MovZeroExtend(Longword, Quadword, asm_src, Reg(R9)),
                    Cvtsi2sd(Quadword, Reg(R9), asm_dst)
                ]
            else:
                out_of_bound = UniqueIds.make_label("ulong2dbl.oob")
                end_lbl = UniqueIds.make_label("ulong2dbl.end")
                r1 = Reg(R8)
                r2 = Reg(R9)

                return [
                    Cmp(Quadword, zero(), asm_src),         // check whether asm_src is w/in range of long
                    JmpCC(L, out_of_bound),
                    Cvtsi2sd(Quadword, asm_src, asm_dst),   // it's in range, just use normal cvtsi2sd then jump to end
                    Jmp(end_lbl),
                    Label(out_of_bound),                    // it's out of bound
                    Mov(Quadword, asm_src, r1),             // halve source and round to dd
                    Mov(Quadword, r1, r2),
                    Unary(ShrOneOp, Quadword, r2),
                    Binary(And, Quadword, Imm(one), r1),
                    Binary(Or, Quadword, r1, r2),
                    Cvtsi2sd(Quadword, r2, asm_dst),        // convert to double, then double it
                    Binary(Add, Double, asm_dst, asm_dst),
                    Label(end_lbl)
                ]
        case DoubleToUInt:
            asm_src = convert_val(inst.src)
            asm_dst = convert_val(inst.dst)

            if tacky_type(dst) == Types.UChar:
                return [
                    Cvttsd2si(Longword, asm_src, Reg(R9)),
                    Mov(Byte, Reg(R9), asm_dst)
                ]
            else if tacky_type(dst) == Types.UInt:
                return [
                    Cvttsd2si(Quadword, asm_src, Reg(R9)),
                    Mov(Longword, Reg(R9), asm_dst)
                ]
            else:
                out_of_bounds = UniqueIds.make_label("dbl2ulong.oob")
                end_lbl = UniqueIds.make_label(dbl2ulong.end)
                upper_bound = add_constant(9223372036854775808.0)
                upper_bound_as_int = Imm(Int64.min_int)
                r = Reg(R9)
                x = Reg(XMM7)

                return [
                    Cmp(Double, Data(upper_bound, 0), asm_src),
                    JmpCC(AE, out_of_bound),
                    Cvttsd2si(Quadword, asm_src, asm_dst),
                    Jmp(end_lbl),
                    Label(out_of_bounds),
                    Mov(Double, asm_src, x),
                    Binary(Sub, Double, Data(upper_bound, 0), x),
                    Cvttsd2si(Quadword, x, asm_dst),
                    Mov(Quadword, upper_bound_as_int, r),
                    Binary(Add, Quadword, r, asm_dst),
                    Label(end_lbl)
                ]
        case CopyToOffset:
            if TypeUtils.is_scalar(tacky_type(inst.src)):
                return [
                    Mov(asm_type(inst.src), convert_val(inst.src), PseudoMem(inst.dst, inst.offset))
                ]
            else:
                asm_src = convert_val(inst.src)
                asm_dst = PseudoMem(inst.dst)
                byte_count = TypeUtils.get_size(tack_type(inst.src))

                return copy_bytes(asm_src, asm_dst, byte_count)
        case CopyFromOffset:
            if TypeUtils.is_scalar(tacky_type(inst.dst)):
                return [
                    Mov(asm_type(inst.dst), PseudoMem(inst.src, offset), convert_val(inst.dst))
                ]
            else:
                asm_src = PseudoMem(inst.src)
                asm_dst = convert_val(inst.dst)
                byte_count = TypeUtils.get_size(tack_type(inst.src))

                return copy_bytes(asm_src, asm_dst, byte_count)
        case AddPtr:
            if inst.index is Constant(ConstLong(c)):
                // note that typechecker converts index to long
                // QUESTION: what's the largest offset we should support?
                i = to_int64(c)
                return [
                    Mov(Quadword, convert_val(inst.ptr), Reg(R9)),
                    Lea(Memory(Reg(R9), i*inst.scale), convert_val(inst.dst)),
                ]
            else:
                if inst.scale is 1, or 2, or 4, or 8:
                    return [
                        Mov(Quadword, convert_val(inst.ptr), Reg(R8)),
                        Mov(Quadword, convert_val(inst.index), Reg(R9)),
                        Lea(Indexed(base=Reg(R8), index=Reg(R9), scale), convert_val(dst)),
                    ]
                else:
                    return [
                        Mov(Quadword, convert_val(inst.ptr), Reg(R8)),
                        Mov(Quadword, convert_val(inst.index), Reg(R9)),
                        Binary(op=Mult, type=Quadword, src=Imm(to_in64(scale), dst=Reg(R9))),
                        Lea(Indexed(base=Reg(R8), index=Reg(R9), scale=1), convert_val(dst)),
                    ]
```

```
// add return_on_stack, and some setting up for remaining_int_regs on top.
// then change the passing params in registers and on stack
pass_params(param_list, return_on_stack):
    int_reg_params, dbl_reg_params, stack_params = classify_parameters(param_list, return_on_stack)
    insts = []

    copy_dst_ptr = null
    remaining_int_regs = int_param_passing_regs

    if return_on_stack:
        copy_dst_ptr = Mov(Quadword, Reg(DI), Memory(Reg(BP), -8))
        remaining_int_regs = int_param_passing_regs[1:]

    // pass params in registers
    reg_idx = 0
    for param_t, param in int_reg_params:
        r = remaining_int_regs[reg_idx]
        if param_t is ByteArray(size):
            insts.append(...copy_bytes_from_reg(r, param, size))
        else:
            insts.append(Mov(param_t, Reg(r), param))
        reg_idx++

    // pass params in registers
    reg_idx = 0
    for param in dbl_reg_params:
        r = dbl_param_passing_regs[reg_idx]
        insts.append(Mov(Double, Reg(r), param))
        reg_idx++

    // pass params on the stack
    // first param passed on stack has idx0 and is passed at Stack(16)
    stk_idx = 0
    for param_t, param in stack_params:
        stk = Memory(Reg(BP), 16 + (8*stk_idx))
        if param_t is ByteArray(size):
            insts.append(...copy_bytes(stk, param, size))
        else:
            insts.append(Mov(param_t, stk, param))
        stk_idx++

    return [copy_dst_ptr, ...insts]
```

```
// NEW
returns_on_stack(fn_name):
    fn_type = symbols.get(fn_name).type
    if fn_type is FunType:
        if fn_type.ret_type is Structure(tag):
            classes = classify_structure(tag)
            return classes[0] == Mem
        else:
            return false
    else:
        fail("Internal error: not a function name")
```

```
// call returns_on_stack and pass the flag in pass_params
convert_top_level(Tacky.top_level top_level):
    if top_level is Function:
        return_on_stack = returns_on_stack(top_level.name)
        params_as_tacky = []
        for param in fun_def.params:
            params_as_tacky.append(Tacky.Var(param))

        params_insts = pass_params(params_as_tacky, return_on_stack)
        insts = [...params_insts]

        for inst in fun_def.body:
            insts.append(...convert_instruction(inst))

        return Function(fun_def.name, fun_decl.global, insts)
    else if top_level is StaticVariable:
        return Assembly.StaticVariable(
            name=top_level.name,
            global=top_level.global,
            alignment=get_var_alignment(top_level.t),
            init=top_level.inits
        )
    else: // is StaticConstant
        return Assembly.StaticConstant(top_level.name, TypeUtils.get_elignment(top_level.t), top_level.init)
```

```
convert_symbol(name, symbol):
    if symbol.attrs is Types.FunAttr:
        // If this function has incomplete return type (implying we don't define or call it in this translation unit)
        // use a dummy value for fun_returns_on_stack
        fun_returns_on_stack = if TypeUtils.is_complete(symbol.type.ret_type) or symbol.type.ret_type is Void
                                then returns_on_stack(name)
                                else false

        return asmSymbols.add_fun(name, symbol.attrs.defined, fun_returns_on_stack)
    else if symbol.attrs is ConstAttr:
        return asmSymbols.add_constant(name, convert_type(symbol.t))
    else if symbol.attrs is Types.StaticAttr:
        if not TypeUtils.is_complete(symbol.type):
            // use dummy type for static variables of incomplete type:
            return asmSymbols.add_var(name, Byte, true)
        else:
            return asmSymbols.add_var(name, convert_var_type(symbol.t), true)
    else:
        return asmSymbols.add_var(name, convert_var_type(symbol.t), false)
```

# ReplacePseudo

```
// Data operand has new field offset
// in PseudoMem static case, Data can now receive offset
replace_operand(Assembly.Operand operand, state):
	match operand type:
		case Pseudo:
            if AsmSymbols.is_static(operand.name):
                return (state, Assembly.Data(operand.name, 0))
			else:
                if state.offset_map.has(operand.name):
                    return (state, Assembly.Memory(Reg(BP), state.offset_map.get(operand.name)))
                else:
                    new_state, new_offset = calculate_offset(state, operand.name)
				    return (new_state, Assembly.Memory(Reg(BP), new_offset))
		case PseudoMem:
            if AsmSymbols.is_static(operand.name):
                return (state, Data(operand.name, operand.offset))
            else:
                if state.offset_map.has(operand.name):
                    // We've already assigned this operand a stack slot
                    v = state.offset_map.get(operand.name)
                    return (state, Assembly.Memory(Reg(BP), operand.offset + v.var_offset))
                else:
                    // assign operand name a stack slot, and add its offset to the offset w/tin operand.name to get new operand
                    new_state, new_var_offset = calculate_offset(state, operand.name)
                    return (new_state,  Assembly.Memory(Reg(BP), operand.offset + new_var_offset))
        default:
			return (state, operand)
```

```
replace_pseudos_in_top_level(Assembly.top_level top_level):
	if top_level is Function:
        // should we stick returns_on_stack in the AST or symbol table?
        if asmSymbol.returns_on_stack(top_level.name):
            starting_offset = -8
        else:
            starting_offset = 0

        curr_state = {
            current_offset = starting_offset,
            offset_map = {}
        }
        fixed_instructions = []

        for inst in fn.instrustions:
            new_state, new_inst = replace_pseudos_in_instruction(inst, curr_state)
            curr_state = new_state
            fixed_instructions.push(new_inst)

        AsmSymbols.set_bytes_required(fn.name, curr_state.current_offset)

        new_fn = Assembly.Function(fn.name, fn.global, fixed_instructions)
        return new_fn, curr_state.current_offset
    else: // is StaticVariable
        return top_level
```

# Instruction Fixup

_No changes_

# Emit

```
// Data operand now has offset field
show_operand(asm_type, operand):
    if operand is Reg:
        if asm_type is Byte:
            return show_byte_reg(operand)
        else if asm_type is Longword:
            return show_long_reg(operand)
        else if asm_type is Quadword:
            return show_quadword_reg(operand)
        else if asm_type is Double:
            return show_double_reg(operand)
        else: // is ByteArray
            fail("Internal error: can't store non-scalar operand in register")
    else if operand is Imm:
        return "${operand.value}"
    else if operand is Memory:
        if operand.offset = 0:
            return "({show_quadword_reg(operand.reg)})"
        return "%{operand.offset}({show_quadword_reg(operand.reg)})"
    else if operand is Data:
        lbl =
            AssemblySymbols.is_constant(operand.name)
                ? show_local_label(operand.name)
                : show_label(operand.name)

        if operand.offset == 0:
            return "{lbl}(%rip)"
        else:
            return "{lbl}+{operand.offset}(%rip)"
    else if operand is Indexed:
        return "({show_quadword_reg(operand.base)}, {show_quadword_reg(operand.index)}, {operand.scale})"
    else if operand is Pseudo: // For debugging
        return operand.name
    else if operand is PseudoMem:
        return "{operand.offset}(%{operand.name})"
```

# Output

From C:

```C
struct pair {
    int x;
    char y;
};

struct pair2 {
    double d;
    long l;
};

struct pair2 double_members(struct pair p) {
    struct pair2 retval = {p.x * 2, p.y * 2};
    return retval;
}

int main(void) {
    struct pair arg = {1, 4};
    struct pair2 result = double_members(arg);

    if (result.d != 2.0 || result.l != 8) {
        return 1;
    }
    return 0;
}
```

To x64 Assembly on Linux:

```asm
	.section .rodata
	.align 8
.Ldbl.24:
	.quad 2

	.global double_members
double_members:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$80, %rsp
	movb	%dil, -16(%rbp)
	shrq	$8, %rdi
	movb	%dil, -15(%rbp)
	shrq	$8, %rdi
	movb	%dil, -14(%rbp)
	shrq	$8, %rdi
	movb	%dil, -13(%rbp)
	shrq	$8, %rdi
	movb	%dil, -12(%rbp)
	shrq	$8, %rdi
	movb	%dil, -11(%rbp)
	shrq	$8, %rdi
	movb	%dil, -10(%rbp)
	shrq	$8, %rdi
	movb	%dil, -9(%rbp)
	shrq	$8, %rdi
	movb	%dil, -8(%rbp)
	shrq	$8, %rdi
	movb	%dil, -7(%rbp)
	shrq	$8, %rdi
	movb	%dil, -6(%rbp)
	shrq	$8, %rdi
	movb	%dil, -5(%rbp)
	shrq	$8, %rdi
	movb	%dil, -4(%rbp)
	shrq	$8, %rdi
	movb	%dil, -3(%rbp)
	shrq	$8, %rdi
	movb	%dil, -2(%rbp)
	shrq	$8, %rdi
	movb	%dil, -1(%rbp)
	movb	%sil, -8(%rbp)
	shrq	$8, %rsi
	movb	%sil, -7(%rbp)
	shrq	$8, %rsi
	movb	%sil, -6(%rbp)
	shrq	$8, %rsi
	movb	%sil, -5(%rbp)
	shrq	$8, %rsi
	movb	%sil, -4(%rbp)
	shrq	$8, %rsi
	movb	%sil, -3(%rbp)
	shrq	$8, %rsi
	movb	%sil, -2(%rbp)
	shrq	$8, %rsi
	movb	%sil, -1(%rbp)
	shrq	$8, %rsi
	movb	%sil, (%rbp)
	shrq	$8, %rsi
	movb	%sil, 1(%rbp)
	shrq	$8, %rsi
	movb	%sil, 2(%rbp)
	shrq	$8, %rsi
	movb	%sil, 3(%rbp)
	shrq	$8, %rsi
	movb	%sil, 4(%rbp)
	shrq	$8, %rsi
	movb	%sil, 5(%rbp)
	shrq	$8, %rsi
	movb	%sil, 6(%rbp)
	shrq	$8, %rsi
	movb	%sil, 7(%rbp)
	movl	-16(%rbp), %r10d
	movl	%r10d, -20(%rbp)
	movl	-20(%rbp), %r10d
	movl	%r10d, -24(%rbp)
	movl	-24(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -24(%rbp)
	cvtsi2sdl	-24(%rbp), %xmm15
	movsd	%xmm15, -32(%rbp)
	movsd	-32(%rbp), %xmm14
	movsd	%xmm14, -48(%rbp)
	movb	-8(%rbp), %r10b
	movb	%r10b, -49(%rbp)
	movsbl 	-49(%rbp), %r11d
	movl	%r11d, -56(%rbp)
	movl	-56(%rbp), %r10d
	movl	%r10d, -60(%rbp)
	movl	-60(%rbp), %r11d
	imull	$2, %r11d
	movl	%r11d, -60(%rbp)
	movslq 	-60(%rbp), %r11
	movq	%r11, -72(%rbp)
	movq	-72(%rbp), %r10
	movq	%r10, -40(%rbp)
	movq	-40(%rbp), %rax
	movsd	-48(%rbp), %xmm0
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.global main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$96, %rsp
	movl	$1, -16(%rbp)
	movb	$4, -17(%rbp)
	movb	-17(%rbp), %r10b
	movb	%r10b, -8(%rbp)
	movb	-1(%rbp), %dil
	shlq	$8, %rdi
	movb	-2(%rbp), %dil
	shlq	$8, %rdi
	movb	-3(%rbp), %dil
	shlq	$8, %rdi
	movb	-4(%rbp), %dil
	shlq	$8, %rdi
	movb	-5(%rbp), %dil
	shlq	$8, %rdi
	movb	-6(%rbp), %dil
	shlq	$8, %rdi
	movb	-7(%rbp), %dil
	shlq	$8, %rdi
	movb	-8(%rbp), %dil
	shlq	$8, %rdi
	movb	-9(%rbp), %dil
	shlq	$8, %rdi
	movb	-10(%rbp), %dil
	shlq	$8, %rdi
	movb	-11(%rbp), %dil
	shlq	$8, %rdi
	movb	-12(%rbp), %dil
	shlq	$8, %rdi
	movb	-13(%rbp), %dil
	shlq	$8, %rdi
	movb	-14(%rbp), %dil
	shlq	$8, %rdi
	movb	-15(%rbp), %dil
	shlq	$8, %rdi
	movb	-16(%rbp), %dil
	movb	7(%rbp), %sil
	shlq	$8, %rsi
	movb	6(%rbp), %sil
	shlq	$8, %rsi
	movb	5(%rbp), %sil
	shlq	$8, %rsi
	movb	4(%rbp), %sil
	shlq	$8, %rsi
	movb	3(%rbp), %sil
	shlq	$8, %rsi
	movb	2(%rbp), %sil
	shlq	$8, %rsi
	movb	1(%rbp), %sil
	shlq	$8, %rsi
	movb	(%rbp), %sil
	shlq	$8, %rsi
	movb	-1(%rbp), %sil
	shlq	$8, %rsi
	movb	-2(%rbp), %sil
	shlq	$8, %rsi
	movb	-3(%rbp), %sil
	shlq	$8, %rsi
	movb	-4(%rbp), %sil
	shlq	$8, %rsi
	movb	-5(%rbp), %sil
	shlq	$8, %rsi
	movb	-6(%rbp), %sil
	shlq	$8, %rsi
	movb	-7(%rbp), %sil
	shlq	$8, %rsi
	movb	-8(%rbp), %sil
	call	double_members
	movq	%rax, -32(%rbp)
	movsd	%xmm0, -40(%rbp)
	movq	-40(%rbp), %r10
	movq	%r10, -56(%rbp)
	movq	-32(%rbp), %r10
	movq	%r10, -48(%rbp)
	movsd	-56(%rbp), %xmm14
	movsd	%xmm14, -64(%rbp)
	movsd	-64(%rbp), %xmm15
	comisd	.Ldbl.24(%rip), %xmm15
	movl	$0, -68(%rbp)
	setne	-68(%rbp)
	movl	$0, %r9d
	setp	%r9b
	orl	%r9d, -68(%rbp)
	cmpl	$0, -68(%rbp)
	je	.Lor_true.21
	movq	-48(%rbp), %r10
	movq	%r10, -80(%rbp)
	movl	$8, %r10d
	movslq 	%r10d, %r11
	movq	%r11, -88(%rbp)
	movq	-80(%rbp), %r10
	cmpq	%r10, -88(%rbp)
	movl	$0, -92(%rbp)
	setne	-92(%rbp)
	cmpl	$0, -92(%rbp)
	je	.Lor_true.21
	movl	$0, -96(%rbp)
	jmp	.Lor_end.22
.Lor_true.21:
	movl	$1, -96(%rbp)
.Lor_end.22:
	cmpl	$0, -96(%rbp)
	je	.Lif_end.15
	movl	$1, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
.Lif_end.15:
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret
	movl	$0, %eax
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.section .note.GNU-stack,"",@progbits
```
