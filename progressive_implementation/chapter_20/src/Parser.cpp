#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <variant>

#include "Token.h"
#include "Lexer.h"
#include "AST.h"
#include "Types.h"
#include "Parser.h"

/*
EBNF for a subset of C:

<program> ::= { <declaration> }
<declaration> ::= <variable-declaration> | <function-declaration> | <struct-declaration>
<variable-declaration> ::= { <specifier> }+ <declarator> [ "=" <initializer> ] ";"
<function-declaration> ::= { <specifier> }+ <declarator> "(" <param-list> ")" (<block> | ";")
<struct-declaration> ::= "struct" <identifier> [ "{" { <member-declaration> }+ "}" ] ";"
<member-declaration> ::= { <type-specifier> }+ <declarator> ";"
<initializer> ::= <exp> | "{" <initializer> { "," <initializer> } [ "," ] "}"
<declarator> ::= "*" <declarator> | <direct-declarator>
<direct-declarator> ::= <simple-declarator> [ <declarator-suffix> ]
<declarator-suffix> ::= <param-list> | { "[" <const> "]" }+
<param-list> ::= "(" "void" ")" | "(" <param> { "," <param> } ")"
<param> ::= { <type-specifier> }+ <declarator>
<simple-declarator> ::= <identifier> | "(" <declarator> ")"
<type-specifier> ::= "int" | "long" | "signed" | "unsigned" | "double" | "char" | "void"
    | "struct" <identifier>
<specifier> ::= <type-specifier> | "static" | "extern"
<block> ::= "{" { <block-item> } "}"
<block-item> ::= <statement> | <declaration>
<for-init> ::= <variable-declaration> | [ <exp> ] ";"
<statement> ::= "return" [ <exp> ] ";"
            | <exp> ";"
            | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
            | <block>
            | "break" ";"
            | "continue" ";"
            | "while" "(" <exp> ")" <statement>
            | "do" <statement> "while" "(" <exp> ")" ";"
            | "for" "(" <for-init> [ <exp> ] ";" [ <exp> ] ")" <statement>
            | ";"
            | <identifier> ":" <statement>
            | "goto" <identifier> ";"
            | "switch" "(" <exp> ")" <statement>
            | "case" <exp> ":" <statement>
            | "default" ":" <statement>
<exp> ::= <cast-exp> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
<cast-exp> ::= "(" <type-name> ")" <cast-exp>
    | <unary-exp>
<unary-exp> ::= <unop> <cast-exp>
    | "sizeof" <unary-exp>
    | "sizeof" "(" <type-name> ")"
    | <postfix-exp>
<type-name> ::= { <type-specifier> }+ [ <abstract-declarator> ]
<postfix-exp> ::= <primary-exp> { <postfix-op> }
<postfix-op> ::= "[" <exp> "]"
    | "." <identifier>
    | "->" <identifier>
<primary-exp> ::= <const>
    | <identifier>
    | "(" <exp> ")"
    | { <string> }+
    | <identifier> "(" [ <argument-list> ] ")"
<argument-list> ::= <exp> { "," <exp> }
<abstract-declarator> ::= "*" [ <abstract-declarator> ]
    | <direct-abstract-declarator>
<direct-abstract-declarator> ::= "(" <abstract-declarator> ")" { "[" <const> "]" }
    | { "[" <const> "]" }+<unop> ::= "-" | "~" | "!" | "++" | "--" | "*" | "&"
<binop> ::= "+" | "-" | "*" | "/" | "%"
        | "&&" | "||"
        | "==" | "!=" | "<" | "<="
        | ">" | ">="
        | "&" | "^" | "|" | "<<" | ">>"
        | "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
<const> ::= <int> | <long> | <uint> | <ulong> | <double> | <char>
<identifier> ::= ? An identifier token ?
<string> ::= ? A string token ?
<int> ::= ? An integer token ?
<char> ::= ? A char token ?
<long> ::= ? An int or long token ?
<uint> ::= ? An unsigned int token ?
<ulong> ::= ? An unsigned int or unsigned long token ?
<double> ::= ? A floating-point constant token ?
*/

/*
    Defining the Declarators
*/

struct Ident;
struct PointerDeclarator;
struct ArrayDeclarator;
struct FunDeclarator;
struct Param;

using Declarator = std::variant<Ident, PointerDeclarator, ArrayDeclarator, FunDeclarator>;
std::string declaratorToString(const Declarator &declarator);

struct Param
{
    std::shared_ptr<Types::DataType> type;
    std::shared_ptr<Declarator> declarator;
    std::string toString() const;
};

struct Ident
{
    std::string name;
    Ident(const std::string &name) : name{name} {}

    std::string toString() const
    {
        return "Ident(" + name + ")\n";
    }
};

struct PointerDeclarator
{
    std::shared_ptr<Declarator> declarator;
    PointerDeclarator(const std::shared_ptr<Declarator> &declarator) : declarator{declarator} {}

    std::string toString() const;
};

struct ArrayDeclarator
{
    std::shared_ptr<Declarator> declarator;
    std::shared_ptr<Constants::Const> sizeConst;
    ArrayDeclarator(const std::shared_ptr<Declarator> &declarator, const std::shared_ptr<Constants::Const> &sizeConst) : declarator{declarator}, sizeConst{sizeConst} {}

    std::string toString() const;
};

struct FunDeclarator
{
    std::vector<std::shared_ptr<Param>> params;
    std::shared_ptr<Declarator> declarator;
    FunDeclarator(const std::vector<std::shared_ptr<Param>> &params, const std::shared_ptr<Declarator> &declarator)
        : params{params}, declarator{declarator} {}

    std::string toString() const;
};

std::string PointerDeclarator::toString() const
{
    return "PointerDeclarator(" + declaratorToString(*declarator) + ")\n";
}

std::string ArrayDeclarator::toString() const
{
    return "ArrayDeclarator(" + declaratorToString(*declarator) + ", size: " + Constants::toString(*sizeConst) + ")\n";
}

std::string FunDeclarator::toString() const
{
    std::string result = "FunDeclarator(";
    result += "declarator: " + declaratorToString(*declarator);
    result += ", params: [";
    for (int i = 0; i < params.size(); i++)
    {
        result += params[i]->toString();
    }
    result += "])\n";
    return result;
}

std::string Param::toString() const
{
    return "Param(type: " + Types::dataTypeToString(type) + ", declarator: " + declaratorToString(*declarator) + ")\n";
}

std::string declaratorToString(const Declarator &declarator)
{
    return std::visit([](auto &&arg) -> std::string
                      { return arg.toString(); }, declarator);
}

/*
    Done Defining the Declarators
*/

/*
    Defining Specifiers
*/

struct StructTag
{
    std::string tag;
    StructTag(std::string tag) : tag{tag} {}
};
struct UnionTag
{
    std::string tag;
    UnionTag(std::string tag) : tag{tag} {}
};
struct OtherSpec
{
    Token tok; // this could be a type or storage class specifier
    OtherSpec(Token tok) : tok{tok} {}
};

using Specifier = std::variant<StructTag, UnionTag, OtherSpec>;

/*
    Done Defining Specifiers
*/

size_t
Parser::constToDim(const std::shared_ptr<Constants::Const> &c)
{
    size_t i = 0;

    if (auto constInt = Constants::getConstInt(*c))
    {
        i = static_cast<size_t>(constInt->val);
    }
    else if (auto constLong = Constants::getConstLong(*c))
    {
        i = static_cast<size_t>(constLong->val);
    }
    else if (auto constUInt = Constants::getConstUInt(*c))
    {
        i = static_cast<size_t>(constUInt->val);
    }
    else if (auto constULong = Constants::getConstULong(*c))
    {
        i = static_cast<size_t>(constULong->val);
    }
    else if (auto constDouble = Constants::getConstDouble(*c))
    {
        throw std::runtime_error("Array dimensions must have integer type");
    }
    else if (Constants::isConstChar(*c) || Constants::isConstUChar(*c))
    {
        throw std::runtime_error("Internal error, we're not using these yet");
    }

    if (i > 0)
        return i;

    throw std::runtime_error("Array dimensions must be greater than zero");
}

std::vector<std::shared_ptr<Constants::Const>>
Parser::parseArrayDimensions()
{
    std::vector<std::shared_ptr<Constants::Const>> dimensions{};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && nextToken->getType() == TokenType::OPEN_BRACKET)
    {
        takeToken();
        auto constExp = parseConstant();
        if (!constExp)
            raiseError("a constant expression", "none");

        dimensions.push_back(constExp->getConst());
        expect(TokenType::CLOSE_BRACKET);
        nextToken = peekToken();
    }

    return dimensions;
}

std::vector<std::shared_ptr<AST::Initializer>>
Parser::parseInitList()
{
    auto initList = std::vector<std::shared_ptr<AST::Initializer>>{};

    while (true)
    {
        auto nextInit = parseInitializer();
        initList.push_back(nextInit);

        auto nextTwoTokens = peekTokens(2);

        if (nextTwoTokens[0]->getType() == TokenType::COMMA && nextTwoTokens[1]->getType() == TokenType::CLOSE_BRACE)
        {
            takeToken();
            break;
        }
        else if (nextTwoTokens[0]->getType() == TokenType::COMMA)
        {
            takeToken();
        }
        else
        {
            break;
        }
    }

    return initList;
}

std::shared_ptr<AST::Initializer>
Parser::parseInitializer()
{
    auto nextTok{peekToken()};
    if (nextTok->getType() == TokenType::OPEN_BRACE)
    {
        takeToken();
        auto inits = parseInitList();
        expect(TokenType::CLOSE_BRACE);
        return std::make_shared<AST::CompoundInit>(inits);
    }
    else
    {
        auto exp = parseExp();
        if (!exp)
            raiseError("an expression", "none");
        return std::make_shared<AST::SingleInit>(exp);
    }
}

std::shared_ptr<Declarator> Parser::parseSimpleDeclarator()
{
    auto nextTok{takeToken()};
    switch (nextTok->getType())
    {
    case TokenType::OPEN_PAREN:
    {
        auto decl = parseDeclarator();
        expect(TokenType::CLOSE_PAREN);
        return decl;
    }
    case TokenType::IDENTIFIER:
    {
        return std::make_shared<Declarator>(Ident{std::get<std::string>(nextTok->getValue())});
    }
    default:
        raiseError("a simple declarator", tokenTypeToString(nextTok->getType()));
    }

    return nullptr;
}

std::shared_ptr<Declarator>
Parser::parseDeclarator()
{
    switch (peekToken()->getType())
    {
    case TokenType::STAR:
    {
        takeToken();
        auto inner = parseDeclarator();
        return std::make_shared<Declarator>(
            PointerDeclarator{inner});
    }
    default:
    {
        return parseDirectDeclarator();
    }
    }
}

std::shared_ptr<Declarator>
Parser::parseDirectDeclarator()
{
    auto simpleDecl = parseSimpleDeclarator();
    switch (peekToken()->getType())
    {
    case TokenType::OPEN_BRACKET:
    {
        auto arrayDimensions = parseArrayDimensions();
        for (const auto &dim : arrayDimensions)
        {
            simpleDecl = std::make_shared<Declarator>(
                ArrayDeclarator{simpleDecl, dim});
        }
        return simpleDecl;
    }
    case TokenType::OPEN_PAREN:
    {
        auto params = parseParamList();
        return std::make_shared<Declarator>(FunDeclarator{params, simpleDecl});
    }
    default:
        return simpleDecl;
    }
}

std::vector<std::shared_ptr<Param>>
Parser::parseParamList()
{
    expect(TokenType::OPEN_PAREN);
    auto params = std::vector<std::shared_ptr<Param>>{};

    auto nextToks{peekTokens(2)};

    if (nextToks[0]->getType() == TokenType::KEYWORD_VOID && nextToks[1]->getType() == TokenType::CLOSE_PAREN)
    {
        takeToken();
    }
    else
    {
        params = paramLoop();
    }

    expect(TokenType::CLOSE_PAREN);
    return params;
}

Types::DataType
Parser::parseTypeName()
{
    auto typeSpecifiers{parseTypeSpecifierList()};
    auto baseType{parseType(typeSpecifiers)};

    /*
        check for optional abstract declarator
        note that <type-name> is always followed by close paren,
        although that's not part of the grammar rule
    */

    if (peekToken().has_value() && peekToken()->getType() == TokenType::CLOSE_PAREN)
        return baseType;

    auto abstractDecl{parseAbstractDeclarator()};
    return *processAbstractDeclarator(abstractDecl, std::make_shared<Types::DataType>(baseType));
}

std::vector<std::shared_ptr<Param>>
Parser::paramLoop()
{
    auto params = std::vector<std::shared_ptr<Param>>{};
    auto param = parseParam();
    params.push_back(param);
    while (peekToken()->getType() == TokenType::COMMA)
    {
        takeToken();
        param = parseParam();
        params.push_back(param);
    }
    return params;
}

std::shared_ptr<Param>
Parser::parseParam()
{
    auto specifiers = parseTypeSpecifierList();
    auto paramType = parseType(specifiers);
    auto paramDecl = parseDeclarator();
    return std::make_shared<Param>(std::make_shared<Types::DataType>(paramType), paramDecl);
}

std::tuple<std::string, std::shared_ptr<Types::DataType>, std::vector<std::string>>
Parser::processDeclarator(const std::shared_ptr<Declarator> &decl, const std::shared_ptr<Types::DataType> &baseType)
{
    if (auto ident = std::get_if<Ident>(&*decl))
    {
        return std::make_tuple(ident->name, baseType, std::vector<std::string>{});
    }
    else if (auto pointerDecl = std::get_if<PointerDeclarator>(&*decl))
    {
        auto derivedType = std::make_shared<Types::DataType>(Types::makePointerType(baseType));
        return processDeclarator(pointerDecl->declarator, derivedType);
    }
    else if (auto arrayDecl = std::get_if<ArrayDeclarator>(&*decl))
    {
        auto dim = constToDim(arrayDecl->sizeConst);
        auto derivedType = std::make_shared<Types::DataType>(Types::makeArrayType(baseType, dim));
        return processDeclarator(arrayDecl->declarator, derivedType);
    }
    else if (auto funDecl = std::get_if<FunDeclarator>(&*decl))
    {
        auto ident = std::get_if<Ident>(&*funDecl->declarator);

        if (!ident)
            throw std::runtime_error("Internal error: inner declarator of a function declarator must be an Ident");

        auto paramNames = std::vector<std::string>{};
        auto paramTypes = std::vector<std::shared_ptr<Types::DataType>>{};

        for (const auto &param : funDecl->params)
        {
            auto [paramName, paramType, _] = processDeclarator(param->declarator, param->type);
            if (Types::isFunType(*paramType))
                throw std::runtime_error("Function pointers in parameters are not supported");

            paramNames.push_back(paramName);
            paramTypes.push_back(paramType);
        }

        auto funType = std::make_shared<Types::DataType>(Types::makeFunType(paramTypes, baseType));
        return std::make_tuple(ident->name, funType, paramNames);
    }
    else
        throw std::runtime_error("Internal error: unknown declarator");
}

/*
Abstract Declarator
*/

struct AbstractPointer;
struct AbstractArray;
struct AbstractBase;

using AbstractDeclarator = std::variant<AbstractPointer, AbstractArray, AbstractBase>;
std::string abstractDeclaratorToString(const AbstractDeclarator &declarator);

struct AbstractPointer
{
    std::shared_ptr<AbstractDeclarator> abstractDeclarator;
    std::string toString() const;
};

struct AbstractArray
{
    std::shared_ptr<AbstractDeclarator> abstractDeclarator;
    std::shared_ptr<Constants::Const> sizeConst;
    std::string toString() const;
};

struct AbstractBase
{
    std::string toString() const;
};

std::string AbstractPointer::toString() const
{
    return "AbstractPointer(" + abstractDeclaratorToString(*abstractDeclarator) + ")\n";
}

std::string AbstractArray::toString() const
{
    return "AbstractArray(" + abstractDeclaratorToString(*abstractDeclarator) + ", size: " + Constants::toString(*sizeConst) + ")\n";
}

std::string AbstractBase::toString() const
{
    return "AbstractBase("
           ")\n";
}

std::string abstractDeclaratorToString(const AbstractDeclarator &declarator)
{
    return std::visit([](auto &&arg) -> std::string
                      { return arg.toString(); }, declarator);
}

std::shared_ptr<AbstractDeclarator>
Parser::parseAbstractDeclarator()
{
    if (peekToken()->getType() == TokenType::STAR)
    {
        // it's a pointer declarator
        takeToken();
        auto nextTok{peekToken()};

        auto inner = std::shared_ptr<AbstractDeclarator>{nullptr};

        if (nextTok->getType() == TokenType::STAR || nextTok->getType() == TokenType::OPEN_PAREN || nextTok->getType() == TokenType::OPEN_BRACKET)
        {
            // there's an inner declarator
            inner = parseAbstractDeclarator();
        }
        else if (nextTok->getType() == TokenType::CLOSE_PAREN)
        {
            inner = std::make_shared<AbstractDeclarator>(AbstractBase{});
        }
        else
        {
            raiseError("an abstract declarator", tokenTypeToString(nextTok->getType()));
        }

        return std::make_shared<AbstractDeclarator>(AbstractPointer{inner});
    }
    else
    {
        return parseDirectAbstractDeclarator();
    }
}

std::shared_ptr<AbstractDeclarator>
Parser::parseDirectAbstractDeclarator()
{
    switch (peekToken()->getType())
    {
    case TokenType::OPEN_PAREN:
    {
        takeToken();
        auto abstrDecl = parseAbstractDeclarator();
        expect(TokenType::CLOSE_PAREN);

        // inner declarator is followed by possibly-empty list of aray dimension
        auto arrayDimensions = parseArrayDimensions();
        for (const auto &dim : arrayDimensions)
        {
            abstrDecl = std::make_shared<AbstractDeclarator>(
                AbstractArray{abstrDecl, dim});
        }

        return abstrDecl;
    }
    case TokenType::OPEN_BRACKET:
    {
        auto arrayDimensions = parseArrayDimensions();
        auto abstrDecl = std::make_shared<AbstractDeclarator>(AbstractBase{});
        for (const auto &dim : arrayDimensions)
        {
            abstrDecl = std::make_shared<AbstractDeclarator>(
                AbstractArray{abstrDecl, dim});
        }
        return abstrDecl;
    }
    default:
    {
        raiseError("an abstract declarator", tokenTypeToString(peekToken()->getType()));
        return nullptr;
    }
    }
}

std::shared_ptr<Types::DataType>
Parser::processAbstractDeclarator(const std::shared_ptr<AbstractDeclarator> &decl, const std::shared_ptr<Types::DataType> &baseType)
{
    if (std::holds_alternative<AbstractBase>(*decl))
    {
        return baseType;
    }
    else if (auto abstractArray = std::get_if<AbstractArray>(&*decl))
    {
        auto dim = constToDim(abstractArray->sizeConst);
        auto derivedType = std::make_shared<Types::DataType>(Types::makeArrayType(baseType, dim));
        return processAbstractDeclarator(abstractArray->abstractDeclarator, derivedType);
    }
    else if (auto abstractPointer = std::get_if<AbstractPointer>(&*decl))
    {
        auto derivedType = std::make_shared<Types::DataType>(Types::makePointerType(baseType));
        return processAbstractDeclarator(abstractPointer->abstractDeclarator, derivedType);
    }
    else
    {
        throw std::runtime_error("Internal error: unknown abstract declarator");
    }
}

/*
    Done Abstract Declarator
*/

void Parser::raiseError(const std::string &expected, const std::string &actual)
{
    throw ParseError("expected " + expected + " but got " + actual);
}

std::string Parser::unescape(const std::string &str)
{
    std::string result;
    size_t i = 0;
    size_t length = str.length();

    while (i < length)
    {
        if (str[i] == '\\')
        {
            if (i + 1 >= length)
            {
                throw std::runtime_error("Internal error: not a valid escape sequence; should have been rejected during lexing");
            }
            char next_char = str[i + 1];
            switch (next_char)
            {
            case '\'':
                result.push_back('\'');
                break;
            case '"':
                result.push_back('"');
                break;
            case '?':
                result.push_back('?');
                break;
            case '\\':
                result.push_back('\\');
                break;
            case 'a':
                result.push_back('\a'); // bell (alert)
                break;
            case 'b':
                result.push_back('\b'); // backspace
                break;
            case 'f':
                result.push_back('\f'); // formfeed
                break;
            case 'n':
                result.push_back('\n'); // newline
                break;
            case 'r':
                result.push_back('\r'); // carriage return
                break;
            case 't':
                result.push_back('\t'); // horizontal tab
                break;
            case 'v':
                result.push_back('\v'); // vertical tab
                break;
            default:
                throw std::runtime_error("Internal error: not a valid escape sequence; should have been rejected during lexing");
            }
            i += 2;
        }
        else
        {
            result.push_back(str[i]);
            i++;
        }
    }

    return result;
}

std::string Parser::parseStringLiteral()
{
    std::string result = "";
    auto nextTok{peekToken()};

    while (nextTok.has_value() && nextTok->getType() == TokenType::STRING_LITERAL)
    {
        takeToken();
        result += unescape(std::get<std::string>(nextTok->getValue()));
        nextTok = peekToken();
    }

    return result;
}

int Parser::getPrecedence(TokenType tokenType)
{
    switch (tokenType)
    {
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::PERCENT:
        return 50;
    case TokenType::PLUS:
    case TokenType::HYPHEN:
        return 45;
    case TokenType::DOUBLE_LEFT_BRACKET:
    case TokenType::DOUBLE_RIGHT_BRACKET:
        return 40;
    case TokenType::LESS_THAN:
    case TokenType::LESS_OR_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_OR_EQUAL:
        return 35;
    case TokenType::DOUBLE_EQUAL:
    case TokenType::NOT_EQUAL:
        return 30;
    case TokenType::AMPERSAND:
        return 25;
    case TokenType::CARET:
        return 20;
    case TokenType::PIPE:
        return 15;
    case TokenType::LOGICAL_AND:
        return 10;
    case TokenType::LOGICAL_OR:
        return 5;
    case TokenType::QUESTION_MARK:
        return 3;
    case TokenType::EQUAL_SIGN:
    case TokenType::PLUS_EQUAL:
    case TokenType::HYPHEN_EQUAL:
    case TokenType::STAR_EQUAL:
    case TokenType::SLASH_EQUAL:
    case TokenType::PERCENT_EQUAL:
    case TokenType::AMPERSAND_EQUAL:
    case TokenType::PIPE_EQUAL:
    case TokenType::CARET_EQUAL:
    case TokenType::DOUBLE_LEFT_BRACKET_EQUAL:
    case TokenType::DOUBLE_RIGHT_BRACKET_EQUAL:
        return 1;
    default:
        throw std::runtime_error("Internal Error: Token is not an operator to get precedence!");
    }
}

bool Parser::isBinop(TokenType tokenType)
{
    switch (tokenType)
    {
    case TokenType::PLUS:
    case TokenType::HYPHEN:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::PERCENT:
    case TokenType::AMPERSAND:
    case TokenType::CARET:
    case TokenType::PIPE:
    case TokenType::DOUBLE_LEFT_BRACKET:
    case TokenType::DOUBLE_RIGHT_BRACKET:
    case TokenType::LESS_THAN:
    case TokenType::LESS_OR_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_OR_EQUAL:
    case TokenType::DOUBLE_EQUAL:
    case TokenType::NOT_EQUAL:
    case TokenType::LOGICAL_AND:
    case TokenType::LOGICAL_OR:
    case TokenType::EQUAL_SIGN:
    case TokenType::PLUS_EQUAL:
    case TokenType::HYPHEN_EQUAL:
    case TokenType::STAR_EQUAL:
    case TokenType::SLASH_EQUAL:
    case TokenType::PERCENT_EQUAL:
    case TokenType::AMPERSAND_EQUAL:
    case TokenType::PIPE_EQUAL:
    case TokenType::CARET_EQUAL:
    case TokenType::DOUBLE_LEFT_BRACKET_EQUAL:
    case TokenType::DOUBLE_RIGHT_BRACKET_EQUAL:
    case TokenType::QUESTION_MARK: // Special case
        return true;
    default:
        return false;
    }
}

std::optional<AST::BinaryOp> Parser::getCompoundOperator(std::optional<Token> token)
{
    if (!token.has_value())
    {
        throw std::runtime_error("Internal error: Empty token to get compound operator!");
    }

    switch (token->getType())
    {
    case TokenType::EQUAL_SIGN:
    {
        return std::nullopt;
    }

    case TokenType::PLUS_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::Add);
    }

    case TokenType::HYPHEN_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::Subtract);
    }

    case TokenType::STAR_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::Multiply);
    }

    case TokenType::SLASH_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::Divide);
    }

    case TokenType::PERCENT_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::Remainder);
    }

    case TokenType::AMPERSAND_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::BitwiseAnd);
    }

    case TokenType::PIPE_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::BitwiseOr);
    }

    case TokenType::CARET_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::BitwiseXor);
    }

    case TokenType::DOUBLE_LEFT_BRACKET_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::BitShiftLeft);
    }

    case TokenType::DOUBLE_RIGHT_BRACKET_EQUAL:
    {
        return std::make_optional(AST::BinaryOp::BitShiftRight);
    }

    default:
    {
        throw std::runtime_error("Internal Error: Unknown token type for compound operator!");
    }
    }
}

bool Parser::isAssignment(std::optional<Token> token)
{
    if (!token.has_value())
    {
        throw std::runtime_error("Internal error: Empty token to get compound operator!");
    }

    switch (token->getType())
    {
    case TokenType::EQUAL_SIGN:
    case TokenType::PLUS_EQUAL:
    case TokenType::HYPHEN_EQUAL:
    case TokenType::STAR_EQUAL:
    case TokenType::SLASH_EQUAL:
    case TokenType::PERCENT_EQUAL:
    case TokenType::AMPERSAND_EQUAL:
    case TokenType::PIPE_EQUAL:
    case TokenType::CARET_EQUAL:
    case TokenType::DOUBLE_LEFT_BRACKET_EQUAL:
    case TokenType::DOUBLE_RIGHT_BRACKET_EQUAL:
        return true;
    default:
        return false;
    }
}

std::optional<Token> Parser::takeToken()
{
    std::optional<Token> token{_lexer.token()};

    if (!token.has_value())
    {
        _currToken = std::nullopt;
    }
    else
    {
        _currToken = token;
    }

    return _currToken;
}

std::optional<Token> Parser::peekToken()
{
    return _lexer.peek();
}

std::vector<std::optional<Token>> Parser::peekTokens(int n)
{
    return _lexer.npeek(n);
}

void Parser::expect(TokenType type)
{
    std::optional<Token> token{takeToken()};

    if (!token.has_value())
    {
        raiseError(tokenTypeToString(type), "empty token");
    }

    if (token->getType() != type)
    {
        raiseError(tokenTypeToString(type), tokenTypeToString(token->getType()));
    }
}

AST::UnaryOp Parser::parseUnop()
{
    std::optional<Token> nextToken{takeToken()};
    if (!nextToken.has_value())
    {
        raiseError("a unary operator", "empty token");
    }

    switch (nextToken->getType())
    {
    case TokenType::HYPHEN:
        return AST::UnaryOp::Negate;
    case TokenType::TILDE:
        return AST::UnaryOp::Complement;
    case TokenType::BANG:
        return AST::UnaryOp::Not;
    case TokenType::DOUBLE_PLUS:
        return AST::UnaryOp::Incr;
    case TokenType::DOUBLE_HYPHEN:
        return AST::UnaryOp::Decr;
    default:
        raiseError("a unary operator", tokenTypeToString(nextToken->getType()));
    }

    return AST::UnaryOp::Negate;
}

AST::BinaryOp Parser::parseBinop()
{
    std::optional<Token> nextToken{takeToken()};

    if (!nextToken.has_value())
    {
        raiseError("a binary operaor", "empty token");
    }

    switch (nextToken->getType())
    {
    case TokenType::PLUS:
        return AST::BinaryOp::Add;
    case TokenType::HYPHEN:
        return AST::BinaryOp::Subtract;
    case TokenType::STAR:
        return AST::BinaryOp::Multiply;
    case TokenType::SLASH:
        return AST::BinaryOp::Divide;
    case TokenType::PERCENT:
        return AST::BinaryOp::Remainder;
    case TokenType::LOGICAL_AND:
        return AST::BinaryOp::And;
    case TokenType::LOGICAL_OR:
        return AST::BinaryOp::Or;
    case TokenType::DOUBLE_EQUAL:
        return AST::BinaryOp::Equal;
    case TokenType::NOT_EQUAL:
        return AST::BinaryOp::NotEqual;
    case TokenType::LESS_THAN:
        return AST::BinaryOp::LessThan;
    case TokenType::LESS_OR_EQUAL:
        return AST::BinaryOp::LessOrEqual;
    case TokenType::GREATER_THAN:
        return AST::BinaryOp::GreaterThan;
    case TokenType::GREATER_OR_EQUAL:
        return AST::BinaryOp::GreaterOrEqual;
    case TokenType::AMPERSAND:
        return AST::BinaryOp::BitwiseAnd;
    case TokenType::CARET:
        return AST::BinaryOp::BitwiseXor;
    case TokenType::PIPE:
        return AST::BinaryOp::BitwiseOr;
    case TokenType::DOUBLE_LEFT_BRACKET:
        return AST::BinaryOp::BitShiftLeft;
    case TokenType::DOUBLE_RIGHT_BRACKET:
        return AST::BinaryOp::BitShiftRight;
    default:
        throw std::runtime_error("Internal Error: Unknown binary operator!");
    }
}

std::shared_ptr<AST::Constant> Parser::parseConstant()
{
    constexpr int64_t MAX_INT64 = std::numeric_limits<int64_t>::max();
    constexpr int32_t MAX_INT32 = std::numeric_limits<int32_t>::max();
    constexpr uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
    constexpr uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();

    std::optional<Token> tokenOpt = takeToken();
    if (!tokenOpt)
    {
        raiseError("a constant", "empty token");
        return nullptr;
    }

    Token token = tokenOpt.value();

    if (std::holds_alternative<std::string>(token.getValue()))
    {
        auto sPrime = unescape(std::get<std::string>(token.getValue()));
        if (sPrime.size() == 1)
            return std::make_shared<AST::Constant>(
                std::make_shared<Constants::Const>(Constants::makeConstInt(static_cast<uint8_t>(sPrime[0]))));
        else
            throw std::runtime_error("Internal error: Character token contains multiple characters, lexer should have rejected this");
    }

    if (std::holds_alternative<long double>(token.getValue()))
    {
        long double value = std::get<long double>(token.getValue());

        return std::make_shared<AST::Constant>(
            std::make_shared<Constants::Const>(Constants::makeConstDouble(value)));
    }

    if (std::holds_alternative<uint64_t>(token.getValue()))
    {
        uint64_t value = std::get<uint64_t>(token.getValue());

        if (token.getType() == TokenType::CONST_INT || token.getType() == TokenType::CONST_LONG)
        {
            if (value > static_cast<uint64_t>(MAX_INT64))
            {
                throw std::runtime_error("Constant too large to fit in an int or long: " + std::to_string(value));
            }

            if (token.getType() == TokenType::CONST_INT)
            {
                if (value <= static_cast<uint64_t>(MAX_INT32))
                {
                    return std::make_shared<AST::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstInt(static_cast<int32_t>(value))));
                }
                else
                {
                    return std::make_shared<AST::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstLong(static_cast<int64_t>(value))));
                }
            }
            else
            {
                return std::make_shared<AST::Constant>(
                    std::make_shared<Constants::Const>(Constants::makeConstLong(static_cast<int64_t>(value))));
            }
        }

        else if (token.getType() == TokenType::CONST_UINT || token.getType() == TokenType::CONST_ULONG)
        {
            if (value > MAX_UINT64) // Already chunked to uint64_t
            {
                throw std::runtime_error("Constant too large to fit in an unsigned int or long: " + std::to_string(value));
            }

            if (token.getType() == TokenType::CONST_UINT)
            {
                if (value <= static_cast<uint64_t>(MAX_UINT32))
                {
                    return std::make_shared<AST::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstUInt(static_cast<uint32_t>(value))));
                }
                else
                {
                    return std::make_shared<AST::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstULong(value)));
                }
            }
            else
            {
                return std::make_shared<AST::Constant>(
                    std::make_shared<Constants::Const>(Constants::makeConstULong(value)));
            }
        }
    }

    raiseError("a constant", tokenTypeToString(token.getType()));
    return nullptr;
}

std::string Parser::parseIdentifier()
{
    std::optional<Token> token{takeToken()};

    if (token.has_value())
    {
        if (token->getType() != TokenType::IDENTIFIER)
        {
            raiseError("an identifier", tokenTypeToString(token->getType()));
        }

        if (std::holds_alternative<std::string>(token->getValue()))
        {
            return std::get<std::string>(token->getValue());
        }

        raiseError("an identifier", tokenTypeToString(token->getType()));
    }

    raiseError("an identifier", "empty token");
    return "";
}

Specifier Parser::parseTypeSpecifier()
{
    auto nextTok{peekToken()};

    if (!nextTok.has_value())
        throw ParseError("Found an empty token while parsing type specifier");

    // if the specifier is a struct or union, we actually care about the tag that follows it

    if (nextTok->getType() == TokenType::KEYWORD_STRUCT)
    {
        takeToken();
        // struct keyword must be followed by tag
        auto tok{takeToken()};
        if (tok->getType() == TokenType::IDENTIFIER)
            return Specifier(StructTag(std::get<std::string>(tok->getValue())));
        else
            raiseError("A structure tag", tokenTypeToString(tok->getType()));
    }
    else if (nextTok->getType() == TokenType::KEYWORD_UNION)
    {
        takeToken();
        // union keyword must be followed by tag
        auto tok{takeToken()};
        if (tok->getType() == TokenType::IDENTIFIER)
            return Specifier(UnionTag(std::get<std::string>(tok->getValue())));
        else
            raiseError("A structure tag", tokenTypeToString(tok->getType()));
    }
    else if (isTypeSpecifier(nextTok.value()))
    {
        takeToken();
        return Specifier(OtherSpec{nextTok.value()});
    }
    else
        throw std::runtime_error("Internal error: called parse_type_specifier on non-type specifier token");

    return Specifier(OtherSpec{nextTok.value()});
}

Specifier Parser::parseSpecifier()
{
    auto nextTok{peekToken()};
    if (!nextTok.has_value())
        throw ParseError("Found an empty token while parsing specifier");

    if (nextTok->getType() == TokenType::KEYWORD_STATIC || nextTok->getType() == TokenType::KEYWORD_EXTERN)
    {
        takeToken();
        return Specifier(OtherSpec{nextTok.value()});
    }

    return parseTypeSpecifier();
}

std::vector<Specifier> Parser::parseTypeSpecifierList()
{
    std::vector<Specifier> typeSpecifiers{};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && isTypeSpecifier(nextToken.value()))
    {
        auto spec{parseTypeSpecifier()};
        typeSpecifiers.push_back(spec);
        nextToken = peekToken();
    }

    return typeSpecifiers;
}

std::vector<Specifier> Parser::parseSpecifierList()
{
    std::vector<Specifier> specifiers{};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && isSpecifier(nextToken.value()))
    {
        auto spec{parseSpecifier()};
        specifiers.push_back(spec);
        nextToken = peekToken();
    }

    return specifiers;
}

AST::StorageClass Parser::parseStorageClass(const Token &spec)
{
    switch (spec.getType())
    {
    case TokenType::KEYWORD_STATIC:
        return AST::StorageClass::Static;
    case TokenType::KEYWORD_EXTERN:
        return AST::StorageClass::Extern;
    default:
        throw std::runtime_error("Internal error: bad storage class");
    }
}

Types::DataType Parser::parseType(const std::vector<Specifier> &typeList)
{
    /*
        sort specifiers so we don't need to check for different
        orderings of same specifiers
    */
    bool hasDuplicateTokenType(const std::vector<Token> &tokens);
    bool containsUnsignedAndSigned(const std::vector<Token> &tokens);
    bool containsToken(const std::vector<Token> &tokens, TokenType type);

    // First handle struct/union tags
    if (typeList.size() == 1)
    {
        if (auto strctTag = std::get_if<StructTag>(&typeList[0]))
            return Types::makeStructType(strctTag->tag);
        if (auto unionTag = std::get_if<UnionTag>(&typeList[0]))
            return Types::makeUnionType(unionTag->tag);
    }
    /*
        Make sure we don't have struct/union specifier combined with other type specifier.
        then convert list of specifiers to list of tokens for easier processing
    */
    std::vector<Token> toks{};
    for (auto &spec : typeList)
    {
        if (auto otherSpec = std::get_if<OtherSpec>(&spec))
        {
            toks.push_back(otherSpec->tok);
        }
        else
        {
            throw std::runtime_error("Found struct or union tag combined with other type specifiers");
        }
    }

    if (toks.size() == 1)
    {
        if (toks[0].getType() == TokenType::KEYWORD_VOID)
            return Types::makeVoidType();
        if (toks[0].getType() == TokenType::KEYWORD_DOUBLE)
            return Types::makeDoubleType();
        if (toks[0].getType() == TokenType::KEYWORD_CHAR)
            return Types::makeCharType();
    }

    if (toks.size() == 2)
    {
        if (containsToken(toks, TokenType::KEYWORD_CHAR) && containsToken(toks, TokenType::KEYWORD_SIGNED))
            return Types::makeSCharType();
        if (containsToken(toks, TokenType::KEYWORD_CHAR) && containsToken(toks, TokenType::KEYWORD_UNSIGNED))
            return Types::makeUCharType();
    }

    if (
        toks.empty() ||
        hasDuplicateTokenType(toks) ||
        containsToken(toks, TokenType::KEYWORD_DOUBLE) ||
        containsToken(toks, TokenType::KEYWORD_CHAR) ||
        containsToken(toks, TokenType::KEYWORD_VOID) ||
        containsToken(toks, TokenType::IDENTIFIER) ||
        containsUnsignedAndSigned(toks))
    {
        throw std::runtime_error("Invalid type specifier");
    }

    if (containsToken(toks, TokenType::KEYWORD_UNSIGNED) && containsToken(toks, TokenType::KEYWORD_LONG))
        return Types::makeULongType();
    else if (containsToken(toks, TokenType::KEYWORD_UNSIGNED))
        return Types::makeUIntType();
    else if (containsToken(toks, TokenType::KEYWORD_LONG))
        return Types::makeLongType();
    else
        return Types::makeIntType();
}

std::pair<Types::DataType, std::optional<AST::StorageClass>>
Parser::parseTypeAndStorageClass(const std::vector<Specifier> &specifierList)
{
    std::vector<Specifier> types{};
    std::vector<Specifier> storageClasses{};

    for (const auto &spec : specifierList)
    {
        if (auto otherSpec = std::get_if<OtherSpec>(&spec))
        {
            if (otherSpec->tok.getType() == TokenType::KEYWORD_EXTERN || otherSpec->tok.getType() == TokenType::KEYWORD_STATIC)
                storageClasses.push_back(spec);
            else
                types.push_back(spec);
        }
        else
        {
            types.push_back(spec);
        }
    }

    Types::DataType type{parseType(types)};
    std::optional<AST::StorageClass> storageClass{};

    if (storageClasses.empty())
        storageClass = std::nullopt;
    else if (storageClasses.size() == 1 && std::holds_alternative<OtherSpec>(storageClasses[0]))
        storageClass = std::make_optional(parseStorageClass(std::get<OtherSpec>(storageClasses[0]).tok));
    else
        throw std::runtime_error("Invalid storage class");

    return {type, storageClass};
}

std::optional<std::shared_ptr<AST::Expression>>
Parser::parseOptionalExp(TokenType delim)
{
    auto nextToken{peekToken()};

    if (nextToken.has_value() && nextToken->getType() == delim)
    {
        takeToken();
        return std::nullopt;
    }
    else
    {
        auto exp{parseExp(0)};
        expect(delim);

        return std::make_optional(exp);
    }
}

std::shared_ptr<AST::Switch> Parser::parseSwitchStatement()
{
    expect(TokenType::KEYWORD_SWITCH);
    expect(TokenType::OPEN_PAREN);
    auto control{parseExp(0)};
    expect(TokenType::CLOSE_PAREN);
    auto body{parseStatement()};

    return std::make_shared<AST::Switch>(control, body, std::optional<AST::CaseMap>(), "");
}

std::shared_ptr<AST::Case> Parser::parseCaseStatement()
{
    expect(TokenType::KEYWORD_CASE);
    auto caseVal{parseExp(0)};
    expect(TokenType::COLON);
    auto body{parseStatement()};

    return std::make_shared<AST::Case>(caseVal, body, "");
}

std::shared_ptr<AST::Default> Parser::parseDefaultStatement()
{
    expect(TokenType::KEYWORD_DEFAULT);
    expect(TokenType::COLON);
    auto body{parseStatement()};

    return std::make_shared<AST::Default>(body, "");
}

std::shared_ptr<AST::ForInit> Parser::parseForInit()
{
    auto nextToken{peekToken()};

    if (!nextToken.has_value())
    {
        raiseError("a for initializer", "empty token");
    }

    if (isSpecifier(nextToken.value()))
    {
        return std::make_shared<AST::InitDecl>(parseVariableDeclaration());
    }
    else
    {
        return std::make_shared<AST::InitExp>(parseOptionalExp(TokenType::SEMICOLON));
    }
}

std::shared_ptr<AST::While> Parser::parseWhileLoop()
{
    expect(TokenType::KEYWORD_WHILE);
    expect(TokenType::OPEN_PAREN);
    auto condition{parseExp(0)};
    expect(TokenType::CLOSE_PAREN);
    auto body{parseStatement()};

    return std::make_shared<AST::While>(condition, body, "");
}

std::shared_ptr<AST::DoWhile> Parser::parseDoLoop()
{
    expect(TokenType::KEYWORD_DO);
    auto body{parseStatement()};
    expect(TokenType::KEYWORD_WHILE);
    expect(TokenType::OPEN_PAREN);
    auto condition{parseExp(0)};
    expect(TokenType::CLOSE_PAREN);
    expect(TokenType::SEMICOLON);

    return std::make_shared<AST::DoWhile>(body, condition, "");
}

std::shared_ptr<AST::For> Parser::parseForLoop()
{
    expect(TokenType::KEYWORD_FOR);
    expect(TokenType::OPEN_PAREN);
    auto init{parseForInit()};
    auto condition{parseOptionalExp(TokenType::SEMICOLON)};
    auto post{parseOptionalExp(TokenType::CLOSE_PAREN)};
    auto body{parseStatement()};

    return std::make_shared<AST::For>(init, condition, post, body, "");
}

std::shared_ptr<AST::Expression> Parser::parseConditionMiddle()
{
    expect(TokenType::QUESTION_MARK);
    auto exp = parseExp(0);
    expect(TokenType::COLON);

    return exp;
}

std::shared_ptr<AST::Expression> Parser::parsePostfixHelper(std::shared_ptr<AST::Expression> primaryExp)
{
    auto nextToken{peekToken()};

    switch (nextToken->getType())
    {
    case TokenType::DOUBLE_HYPHEN:
    {
        takeToken();
        auto decrExp{std::make_shared<AST::PostfixDecr>(primaryExp)};
        return parsePostfixHelper(decrExp);
    }
    case TokenType::DOUBLE_PLUS:
    {
        takeToken();
        auto incrExp{std::make_shared<AST::PostfixIncr>(primaryExp)};
        return parsePostfixHelper(incrExp);
    }
    case TokenType::OPEN_BRACKET:
    {
        takeToken();
        auto index{parseExp(0)};
        expect(TokenType::CLOSE_BRACKET);
        auto subscriptExp = std::make_shared<AST::Subscript>(primaryExp, index);
        return parsePostfixHelper(subscriptExp);
    }
    case TokenType::DOT:
    {
        takeToken();
        auto member{parseIdentifier()};
        auto memberExp = std::make_shared<AST::Dot>(primaryExp, member);
        return parsePostfixHelper(memberExp);
    }
    case TokenType::ARROW:
    {
        takeToken();
        auto member{parseIdentifier()};
        auto arrowExp = std::make_shared<AST::Arrow>(primaryExp, member);
        return parsePostfixHelper(arrowExp);
    }
    default:
        return primaryExp;
    }
}

std::shared_ptr<AST::Expression> Parser::parsePostfixExp()
{
    auto primaryExp{parsePrimaryExp()};
    return parsePostfixHelper(primaryExp);
}

std::shared_ptr<AST::Expression> Parser::parsePrimaryExp()
{
    auto nextToken{peekToken()};

    if (!nextToken.has_value())
    {
        raiseError("an expression", "empty token");
    }

    switch (nextToken->getType())
    {
    case TokenType::CONST_CHAR:
    case TokenType::CONST_INT:
    case TokenType::CONST_LONG:
    case TokenType::CONST_UINT:
    case TokenType::CONST_ULONG:
    case TokenType::CONST_DOUBLE:
    {
        auto c = parseConstant();
        return c;
    }

    case TokenType::IDENTIFIER:
    {
        auto id{parseIdentifier()};

        // Look at the next token to figure out whether this is a variable or a function call
        auto nextToken = peekToken();
        if (!nextToken.has_value())
        {
            raiseError("a delimiter for Var/FunctionCall", "empty token");
        }

        if (nextToken->getType() == TokenType::OPEN_PAREN)
        {
            auto args{parseOptionalArgList()};
            return std::make_shared<AST::FunctionCall>(id, args);
        }
        else
        {
            return std::make_shared<AST::Var>(id);
        }
    }

    case TokenType::STRING_LITERAL:
    {
        auto strExp{parseStringLiteral()};
        return std::make_shared<AST::String>(strExp);
    }

    case TokenType::OPEN_PAREN:
    {
        takeToken();
        std::shared_ptr<AST::Expression> innerExp{parseExp(0)};
        expect(TokenType::CLOSE_PAREN);

        return innerExp;
    }
    default:
        raiseError("an expression", tokenTypeToString(nextToken->getType()));
        return nullptr; // Ensure all control paths return a value
    }
}

std::shared_ptr<AST::Expression>
Parser::parseCastExp()
{
    auto nextToks{peekTokens(2)};

    if (nextToks[0]->getType() == TokenType::OPEN_PAREN && nextToks[1].has_value() && isTypeSpecifier(nextToks[1].value()))
    {
        // this is a cast expression
        takeToken();
        auto targetType{parseTypeName()};
        expect(TokenType::CLOSE_PAREN);
        auto innerExp{parseCastExp()};
        return std::make_shared<AST::Cast>(targetType, innerExp);
    }

    return parseUnaryExp();
}

std::shared_ptr<AST::Expression>
Parser::parseUnaryExp()
{
    auto nextTokens{peekTokens(3)};

    if (!nextTokens[0].has_value())
    {
        raiseError("an expression", "empty token");
    }

    switch (nextTokens[0]->getType())
    {
    case TokenType::HYPHEN:
    case TokenType::TILDE:
    case TokenType::BANG:
    case TokenType::DOUBLE_PLUS:
    case TokenType::DOUBLE_HYPHEN:
    {
        AST::UnaryOp op{parseUnop()};
        std::shared_ptr<AST::Expression> innerExp{parseCastExp()};

        return std::make_shared<AST::Unary>(op, innerExp);
    }
    case TokenType::STAR:
    {
        takeToken();
        auto innerExp{parseCastExp()};

        return std::make_shared<AST::Dereference>(innerExp);
    }
    case TokenType::AMPERSAND:
    {
        takeToken();
        auto innerExp{parseCastExp()};

        return std::make_shared<AST::AddrOf>(innerExp);
    }
    case TokenType::KEYWORD_SIZEOF:
    {
        if (nextTokens[1]->getType() == TokenType::OPEN_PAREN && nextTokens[2].has_value() && isTypeSpecifier(nextTokens[2].value()))
        {
            // this is a size of a type name
            takeToken();
            takeToken();
            auto targetType{parseTypeName()};
            expect(TokenType::CLOSE_PAREN);
            return std::make_shared<AST::SizeOfT>(std::make_shared<Types::DataType>(targetType));
        }
        else
        {
            // size of an expression
            takeToken();
            auto innerExp{parseUnaryExp()};
            return std::make_shared<AST::SizeOf>(innerExp);
        }
    }
    default:
        return parsePostfixExp();
    }
}

std::shared_ptr<AST::Expression> Parser::parseExp(int minPrec)
{
    auto left{parseCastExp()};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && isBinop(nextToken->getType()) && getPrecedence(nextToken->getType()) >= minPrec)
    {
        if (isAssignment(nextToken))
        {
            takeToken();
            auto right{parseExp(getPrecedence(nextToken->getType()))};
            auto op{getCompoundOperator(nextToken)};
            if (!op.has_value())
            {
                left = std::make_shared<AST::Assignment>(left, right);
            }
            else
            {
                left = std::make_shared<AST::CompoundAssignment>(op.value(), left, right);
            }
        }
        else if (nextToken->getType() == TokenType::QUESTION_MARK)
        {
            auto middle = parseConditionMiddle();
            auto right = parseExp(getPrecedence(nextToken->getType()));
            left = std::make_shared<AST::Conditional>(left, middle, right);
        }
        else
        {
            auto binOp{parseBinop()};
            auto right{parseExp(getPrecedence(nextToken->getType()) + 1)};
            left = std::make_shared<AST::Binary>(binOp, left, right);
        }

        nextToken = peekToken();
    }

    return left;
}

AST::Block Parser::parseBlock()
{
    expect(TokenType::OPEN_BRACE);
    AST::Block block{};

    while (peekToken().has_value() && peekToken()->getType() != TokenType::CLOSE_BRACE)
    {
        auto nextBlockItem{parseBlockItem()};
        block.push_back(nextBlockItem);
    }

    expect(TokenType::CLOSE_BRACE);

    return block;
}

std::shared_ptr<AST::Statement> Parser::parseStatement()
{
    std::vector<std::optional<Token>> nextTokens{peekTokens(2)};

    switch (nextTokens[0]->getType())
    {
    case TokenType::KEYWORD_RETURN:
    {
        takeToken();
        auto retVal{parseOptionalExp(TokenType::SEMICOLON)};

        return std::make_shared<AST::Return>(retVal);
    }
    case TokenType::KEYWORD_IF:
    {
        takeToken();
        expect(TokenType::OPEN_PAREN);
        auto condition = parseExp(0);
        expect(TokenType::CLOSE_PAREN);
        auto thenClause = parseStatement();
        std::optional<std::shared_ptr<AST::Statement>> elseClause = std::nullopt;

        if (peekToken()->getType() == TokenType::KEYWORD_ELSE)
        {
            takeToken();
            elseClause = std::make_optional(parseStatement());
        }

        return std::make_shared<AST::If>(condition, thenClause, elseClause);
    }
    case TokenType::OPEN_BRACE:
    {
        return std::make_shared<AST::Compound>(parseBlock());
    }
    case TokenType::KEYWORD_GOTO:
    {
        takeToken();
        auto label{parseIdentifier()};
        expect(TokenType::SEMICOLON);

        return std::make_shared<AST::Goto>(label);
    }

    case TokenType::IDENTIFIER:
    {
        if (nextTokens[1]->getType() == TokenType::COLON)
        {
            auto label{parseIdentifier()}; // Take the label from identifier
            takeToken();                   // Take the colon

            auto stmt{parseStatement()};
            return std::make_shared<AST::LabeledStatement>(label, stmt);
        }
        else
        {
            auto innerExp{parseExp(0)};
            expect(TokenType::SEMICOLON);

            return std::make_shared<AST::ExpressionStmt>(innerExp);
        }
    }
    case TokenType::KEYWORD_BREAK:
    {
        takeToken();
        expect(TokenType::SEMICOLON);
        return std::make_shared<AST::Break>("");
    }
    case TokenType::KEYWORD_CONTINUE:
    {
        takeToken();
        expect(TokenType::SEMICOLON);
        return std::make_shared<AST::Continue>("");
    }
    case TokenType::KEYWORD_WHILE:
    {
        return parseWhileLoop();
    }
    case TokenType::KEYWORD_DO:
    {
        return parseDoLoop();
    }
    case TokenType::KEYWORD_FOR:
    {
        return parseForLoop();
    }
    case TokenType::KEYWORD_SWITCH:
    {
        return parseSwitchStatement();
    }
    case TokenType::KEYWORD_CASE:
    {
        return parseCaseStatement();
    }
    case TokenType::KEYWORD_DEFAULT:
    {
        return parseDefaultStatement();
    }
    default:
        // For Expression and Null statement
        auto optExp{parseOptionalExp(TokenType::SEMICOLON)};
        if (!optExp.has_value())
        {
            return std::make_shared<AST::Null>();
        }
        else
        {
            return std::make_shared<AST::ExpressionStmt>(optExp.value());
        }
    }
}

std::vector<std::shared_ptr<AST::Expression>> Parser::parseOptionalArgList()
{
    expect(TokenType::OPEN_PAREN);

    std::vector<std::shared_ptr<AST::Expression>> args{
        peekToken().has_value() && peekToken()->getType() == TokenType::CLOSE_PAREN
            ? std::vector<std::shared_ptr<AST::Expression>>()
            : parseArgList()};

    expect(TokenType::CLOSE_PAREN);
    return args;
}

std::vector<std::shared_ptr<AST::Expression>> Parser::parseArgList()
{
    std::vector<std::shared_ptr<AST::Expression>> args{};
    auto nextToken{peekToken()};

    while (nextToken.has_value())
    {
        auto arg{parseExp(0)};
        args.push_back(arg);

        nextToken = peekToken();
        if (nextToken.has_value() && nextToken->getType() == TokenType::COMMA)
        {
            takeToken();
            nextToken = peekToken();
        }
        else
            break;
    }

    return args;
}

std::shared_ptr<AST::TypeDeclaration>
Parser::parseTypeDeclaration()
{
    auto structOrUnionKw = takeToken();
    auto tag{parseIdentifier()};

    auto nextTok{takeToken()};
    if (!nextTok.has_value() || !structOrUnionKw.has_value())
        throw std::runtime_error("Internal error: Invalid type declaration");

    std::vector<std::shared_ptr<AST::MemberDeclaration>> members{};
    if (nextTok->getType() == TokenType::SEMICOLON)
    {
        members = {};
    }
    else if (nextTok->getType() == TokenType::OPEN_BRACE)
    {
        members = parseMemberList();
        expect(TokenType::CLOSE_BRACE);
        expect(TokenType::SEMICOLON);
    }
    else
    {
        throw std::runtime_error("Internal error: shouldn't have called parse_structure_declaration here");
    }

    AST::Which structOrUnion;
    if (structOrUnionKw->getType() == TokenType::KEYWORD_STRUCT)
        structOrUnion = AST::Which::Struct;
    else if (structOrUnionKw->getType() == TokenType::KEYWORD_UNION)
        structOrUnion = AST::Which::Union;
    else
        throw std::runtime_error("Internal error: shouldn't have called parse_structure_declaration here");

    return std::make_shared<AST::TypeDeclaration>(structOrUnion, tag, members);
}

// parse a non-empty member list
std::vector<std::shared_ptr<AST::MemberDeclaration>>
Parser::parseMemberList()
{
    auto m = parseMember();
    std::vector<std::shared_ptr<AST::MemberDeclaration>> members{m};

    auto nextToken{peekToken()};
    while (nextToken.has_value() && nextToken->getType() != TokenType::CLOSE_BRACE)
    {
        members.push_back(parseMember());
        nextToken = peekToken();
    }
    return members;
}

std::shared_ptr<AST::MemberDeclaration>
Parser::parseMember()
{
    auto specifiers{parseTypeSpecifierList()};
    auto t{parseType(specifiers)};
    auto memberDecl{parseDeclarator()};
    if (std::holds_alternative<FunDeclarator>(*memberDecl))
    {
        throw ParseError("found function declarator in struct member list");
    }
    else
    {
        expect(TokenType::SEMICOLON);
        auto [memberName, member_type, _] = processDeclarator(memberDecl, std::make_shared<Types::DataType>(t));

        return std::make_shared<AST::MemberDeclaration>(memberName, member_type);
    }
}

std::shared_ptr<AST::FunctionDeclaration>
Parser::finishParsingFunctionDeclaration(const std::string &name, const Types::DataType &funType, std::vector<std::string> params, std::optional<AST::StorageClass> storageClass)
{
    auto nextToken{peekToken()};

    if (!nextToken.has_value())
        raiseError("a function body or semicolon", "empty token");

    std::optional<AST::Block> body{std::nullopt};

    switch (nextToken->getType())
    {
    case TokenType::OPEN_BRACE:
    {
        body = std::make_optional(parseBlock());
        break;
    }
    case TokenType::SEMICOLON:
    {
        body = std::nullopt;
        takeToken();
        break;
    }
    default:
        raiseError("a function body or semicolon", "invalid token");
    }

    return std::make_shared<AST::FunctionDeclaration>(name, params, body, funType, storageClass);
}

std::shared_ptr<AST::VariableDeclaration> Parser::finishParsingVariableDeclaration(const std::string &name, const Types::DataType &varType, std::optional<AST::StorageClass> storageClass)
{
    auto nextToken{takeToken()};
    if (!nextToken.has_value())
        raiseError("a semicolon or initializer", "empty token");

    switch (nextToken->getType())
    {
    case TokenType::SEMICOLON:
        return std::make_shared<AST::VariableDeclaration>(name, std::nullopt, varType, storageClass);
    case TokenType::EQUAL_SIGN:
    {
        auto init = parseInitializer();
        expect(TokenType::SEMICOLON);

        return std::make_shared<AST::VariableDeclaration>(name, std::make_optional(init), varType, storageClass);
    }
    default:
        raiseError("an initializer or semicolon", tokenTypeToString(nextToken->getType()));
    }

    return nullptr;
}

std::shared_ptr<AST::VariableDeclaration> Parser::parseVariableDeclaration()
{
    auto decl{parseDeclaration()};
    if (decl->getType() == AST::NodeType::VariableDeclaration)
    {
        return std::dynamic_pointer_cast<AST::VariableDeclaration>(decl);
    }
    else // is FunDecl or TypeDecl
        throw std::runtime_error("Expected variable declaration but found function or type declaration");
}

std::shared_ptr<AST::FunctionDeclaration> Parser::parseFunctionDeclaration()
{
    auto decl{parseDeclaration()};
    if (decl->getType() == AST::NodeType::FunctionDeclaration)
    {
        return std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl);
    }
    else
        throw std::runtime_error("Expected function declaration but found variable declaration");
}

std::shared_ptr<AST::Declaration> Parser::parseDeclaration()
{
    // first figure out whether this is a struct declaration
    auto toks{peekTokens(3)};

    if (toks[0].has_value() && toks[1].has_value() && toks[2].has_value() &&
        (toks[0]->getType() == TokenType::KEYWORD_STRUCT || toks[0]->getType() == TokenType::KEYWORD_UNION) &&
        toks[1]->getType() == TokenType::IDENTIFIER &&
        (toks[2]->getType() == TokenType::OPEN_BRACE || toks[2]->getType() == TokenType::SEMICOLON))
    {
        return parseTypeDeclaration();
    }
    else
    {
        auto specifiers{parseSpecifierList()};
        auto [baseTyp, storageClass] = parseTypeAndStorageClass(specifiers);

        // parse until declarator, then call appropriate function to finish parsing
        auto declarator{parseDeclarator()};
        auto [name, typ, params] = processDeclarator(declarator, std::make_shared<Types::DataType>(baseTyp));

        if (Types::isFunType(*typ))
        {
            return finishParsingFunctionDeclaration(name, *typ, params, storageClass);
        }
        else
        {
            if (params.empty())
                return finishParsingVariableDeclaration(name, *typ, storageClass);
            else
                throw std::runtime_error("Internal error: declarator has parameters but object type");
        }
    }
}

std::shared_ptr<AST::BlockItem> Parser::parseBlockItem()
{
    auto nextToken{peekToken()};

    if (!nextToken.has_value())
        raiseError("a token", "end of tokens");

    if (isSpecifier(nextToken.value()))
        return parseDeclaration();
    else
        return parseStatement();
}

std::vector<std::shared_ptr<AST::Declaration>> Parser::parseDeclarationList()
{
    std::vector<std::shared_ptr<AST::Declaration>> declList{};
    auto nextToken{peekToken()};

    while (nextToken.has_value())
    {
        auto nextDecl{parseDeclaration()};
        declList.push_back(nextDecl);
        nextToken = peekToken();
    }

    return declList;
}

std::shared_ptr<AST::Program> Parser::parseProgram()
{
    std::vector<std::shared_ptr<AST::Declaration>> decls{parseDeclarationList()};
    std::optional<Token> nextToken{peekToken()};

    if (nextToken.has_value())
        raiseError("end of input", tokenTypeToString(nextToken->getType()));

    return std::make_shared<AST::Program>(decls);
}

std::shared_ptr<AST::Program> Parser::parse(const std::string &input)
{
    _lexer.setInput(input);

    return parseProgram();
}

bool hasDuplicateTokenType(const std::vector<Token> &tokens)
{
    std::unordered_set<TokenType> seenTypes;

    for (const auto &token : tokens)
    {
        if (seenTypes.find(token.getType()) != seenTypes.end())
        {
            return true;
        }
        seenTypes.insert(token.getType());
    }

    return false;
}

bool containsToken(const std::vector<Token> &tokens, TokenType type)
{
    return std::find_if(tokens.begin(), tokens.end(), [type](const Token &token)
                        { return token.getType() == type; }) != tokens.end();
}

bool containsUnsignedAndSigned(const std::vector<Token> &tokens)
{
    return containsToken(tokens, TokenType::KEYWORD_UNSIGNED) &&
           containsToken(tokens, TokenType::KEYWORD_SIGNED);
}