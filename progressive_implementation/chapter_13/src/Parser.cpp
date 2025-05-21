#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <limits>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"
#include "Types.h"

/*
EBNF for a subset of C:

<program> ::= { <declaration> }
<declaration> ::= <variable-declaration> | <function-declaration>
<variable-declaration> ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
<function-declaration> ::= { <specifier> }+ <identifier> "(" <param-list> ")" (<block> | ";")
<param-list> ::= "void" | { <type-specifier> }+ <identifier> { "," { <type-specifier> }+  <identifier> }
<type-specifier> ::= "int" | "long" | "signed" | "unsigned" | "double"
<specifier> ::= <type-specifier> | "static" | "extern"
<block> ::= "{" { <block-item> } "}"
<block-item> ::= <statement> | <declaration>
<for-init> ::= <variable-declaration> | [ <exp> ] ";"
<statement> ::= "return" <exp> ";"
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
<exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
<factor> ::=  <unop> <factor> | <postfix-exp>
<postfix-exp> ::= <primary-exp> { "++" | "--" }
<primary-exp> ::= <const> | <identifier> | "(" <exp> ")" | <identifier> "(" [ <argument-list> ] ")" | "(" { <type-specifier> }+ ")" <factor>
<argument-list> ::= <exp> { "," <exp> }
<unop> ::= "-" | "~" | "!" | "++" | "--"
<binop> ::= "+" | "-" | "*" | "/" | "%"
        | "&&" | "||"
        | "==" | "!=" | "<" | "<="
        | ">" | ">="
        | "&" | "^" | "|" | "<<" | ">>"
        | "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
<const> ::= <int> | <long> | <uint> | <ulong> | <double>
<identifier> ::= ? An identifier token ?
<int> ::= ? An integer token ?
<long> ::= ? An int or long token ?
<uint> ::= ? An unsigned int token ?
<ulong> ::= ? An unsigned int or unsigned long token ?
<double> ::= ? A floating-point constant token ?
*/

void Parser::raiseError(const std::string &expected, const std::string &actual)
{
    throw ParseError("expected " + expected + " but got " + actual);
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

    const std::set<TokenType> constTokenTypes = {
        TokenType::CONST_INT,
        TokenType::CONST_LONG,
        TokenType::CONST_UINT,
        TokenType::CONST_ULONG,
        TokenType::CONST_DOUBLE,
    };

    std::optional<Token> tokenOpt = takeToken();
    if (!tokenOpt)
    {
        raiseError("a constant", "empty token");
        return nullptr; // unreachable if raiseError exits.
    }

    Token token = tokenOpt.value();

    if (!constTokenTypes.count(token.getType()))
    {
        raiseError("a constant", tokenTypeToString(token.getType()));
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

    throw std::runtime_error("Internal error: unknown token constant type");
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

std::vector<Token> Parser::parseTypeSpecifierList()
{
    std::vector<Token> typeSpecifiers{};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && isTypeSpecifier(nextToken.value()))
    {
        auto spec{takeToken()};
        typeSpecifiers.push_back(spec.value());
        nextToken = peekToken();
    }

    return typeSpecifiers;
}

std::vector<Token> Parser::parseSpecifierList()
{
    std::vector<Token> specifiers{};
    auto nextToken{peekToken()};

    while (nextToken.has_value() && isSpecifier(nextToken.value()))
    {
        auto spec{takeToken()};
        specifiers.push_back(spec.value());
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

Types::DataType Parser::parseType(const std::vector<Token> &typeList)
{
    bool hasDuplicateTokenType(const std::vector<Token> &tokens);
    bool containsUnsignedAndSigned(const std::vector<Token> &tokens);
    bool containsToken(const std::vector<Token> &tokens, TokenType type);

    if (typeList.size() == 1 && containsToken(typeList, TokenType::KEYWORD_DOUBLE))
        return Types::makeDoubleType();

    if (
        typeList.empty() ||
        hasDuplicateTokenType(typeList) ||
        containsToken(typeList, TokenType::KEYWORD_DOUBLE) ||
        containsUnsignedAndSigned(typeList))
    {
        throw std::runtime_error("Invalid type specifier");
    }

    if (containsToken(typeList, TokenType::KEYWORD_UNSIGNED) && containsToken(typeList, TokenType::KEYWORD_LONG))
        return Types::makeULongType();
    else if (containsToken(typeList, TokenType::KEYWORD_UNSIGNED))
        return Types::makeUIntType();
    else if (containsToken(typeList, TokenType::KEYWORD_LONG))
        return Types::makeLongType();
    else
        return Types::makeIntType();
}

std::pair<Types::DataType, std::optional<AST::StorageClass>>
Parser::parseTypeAndStorageClass(const std::vector<Token> &specifierList)
{
    std::vector<Token> types{};
    std::vector<Token> storageClasses{};

    for (const auto &spec : specifierList)
    {
        if (isTypeSpecifier(spec))
            types.push_back(spec);
        else
            storageClasses.push_back(spec);
    }

    Types::DataType type{parseType(types)};
    std::optional<AST::StorageClass> storageClass{};

    if (storageClasses.empty())
        storageClass = std::nullopt;
    else if (storageClasses.size() == 1)
        storageClass = std::make_optional(parseStorageClass(storageClasses[0]));
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

    case TokenType::OPEN_PAREN:
    {
        takeToken();
        std::shared_ptr<AST::Expression> innerExp{parseExp(0)};
        expect(TokenType::CLOSE_PAREN);

        return innerExp;
    }
    default:
        raiseError("an expression", tokenTypeToString(nextToken->getType()));
    }

    return nullptr;
}

std::shared_ptr<AST::Expression> Parser::parseFactor()
{
    auto nextTokens{peekTokens(2)};

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
        std::shared_ptr<AST::Expression> innerExp{parseFactor()};

        return std::make_shared<AST::Unary>(op, innerExp);
    }
    case TokenType::OPEN_PAREN:
    {
        if (nextTokens[1].has_value() && isTypeSpecifier(nextTokens[1].value()))
        {
            // It's a cast, consume the "(", then parse the type specifiers
            takeToken();
            auto typeSpecifiers{parseTypeSpecifierList()};
            auto targetType{parseType(typeSpecifiers)};
            expect(TokenType::CLOSE_PAREN);
            auto innerExp{parseFactor()};

            return std::make_shared<AST::Cast>(targetType, innerExp);
        }
    }
    default:
        return parsePostfixExp();
    }
}

std::shared_ptr<AST::Expression> Parser::parseExp(int minPrec)
{
    auto left{parseFactor()};
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
        auto retVal{parseExp(0)};
        expect(TokenType::SEMICOLON);

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

std::vector<std::pair<Types::DataType, std::string>> Parser::parseParamList()
{
    auto specifiers{parseTypeSpecifierList()};
    auto paramType{parseType(specifiers)};
    auto nextParam{parseIdentifier()};

    auto paramsWithTypes = (std::vector<std::pair<Types::DataType, std::string>>){{paramType, nextParam}};

    auto nextToken{peekToken()};
    while (nextToken.has_value())
    {
        if (nextToken->getType() == TokenType::COMMA)
        {
            takeToken();
            auto specifiers{parseTypeSpecifierList()};
            auto paramType{parseType(specifiers)};
            auto nextParam{parseIdentifier()};
            paramsWithTypes.push_back({paramType, nextParam});
            nextToken = peekToken();
        }
        else
            break;
    }

    return paramsWithTypes;
}

std::shared_ptr<AST::FunctionDeclaration> Parser::finishParsingFunctionDeclaration(const std::string &name, const Types::DataType &retType, std::optional<AST::StorageClass> storageClass)
{
    expect(TokenType::OPEN_PAREN);
    std::vector<std::pair<Types::DataType, std::string>> paramsWithTypes;

    if (!peekToken().has_value())
    {
        throw std::runtime_error("Unexpected end of file while parsing function declaration");
    }

    if (peekToken()->getType() == TokenType::KEYWORD_VOID)
    {
        takeToken();
        paramsWithTypes = {};
    }
    else if (peekToken()->getType() != TokenType::CLOSE_PAREN)
    {
        paramsWithTypes = parseParamList();
    }

    std::vector<std::shared_ptr<Types::DataType>> paramTypes;
    std::vector<std::string> params;

    for (const auto &paramPair : paramsWithTypes)
    {
        paramTypes.push_back(std::make_shared<Types::DataType>(paramPair.first));
        params.push_back(paramPair.second);
    }

    expect(TokenType::CLOSE_PAREN);
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

    auto funType = Types::makeFunType(paramTypes, std::make_shared<Types::DataType>(retType));
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
        auto init = parseExp(0);
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
    else
        throw std::runtime_error("Expected variable declaration but found function declaration");
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
    auto specifiers{parseSpecifierList()};
    auto [type, storageClass] = parseTypeAndStorageClass(specifiers);
    auto name{parseIdentifier()};

    auto nextToken{peekToken()};

    if (nextToken.has_value() && nextToken->getType() == TokenType::OPEN_PAREN)
        return finishParsingFunctionDeclaration(name, type, storageClass);
    else
        return finishParsingVariableDeclaration(name, type, storageClass);
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