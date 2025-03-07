#include <iostream>
#include <vector>
#include <optional>
#include <memory>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"

/*
EBNF for a subset of C:

<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int> | <unop> <exp> | "(" <exp> ")"
<unop> ::= "-" | "~"
<int> ::= ? An integer token ?
<identifier> ::= ? An identifier token ?

*/

void Parser::raiseError(const std::string &expected, const std::string &actual)
{
    throw ParseError("expected " + expected + " but got " + actual);
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

std::vector<Token> Parser::peekTokens(int n)
{
    return _lexer.npeek(n);
}

void Parser::expect(TokenType type)
{
    std::optional<Token> tokPtr{takeToken()};

    if (!tokPtr.has_value())
    {
        raiseError(tokenTypeToString(type), "empty token");
    }

    Token tok{*tokPtr};
    if (tok.getType() != type)
    {
        raiseError(tokenTypeToString(type), tokenTypeToString(tok.getType()));
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
    default:
        raiseError("a unary operator", tokenTypeToString(nextToken->getType()));
    }

    return AST::UnaryOp::Negate;
}

std::shared_ptr<AST::Constant> Parser::parseConst()
{
    std::optional<Token> tokPtr{takeToken()};

    if (tokPtr.has_value())
    {
        Token tok{*tokPtr};
        if (tok.getType() != TokenType::CONSTANT)
        {
            raiseError("a constant", tokenTypeToString(tok.getType()));
        }

        if (std::holds_alternative<int>(tok.getValue()))
        {
            int value{std::get<int>(tok.getValue())};
            return std::make_shared<AST::Constant>(value);
        }

        raiseError("an integer", "non-integer value");
    }

    raiseError("a constant", "empty token");
    return nullptr;
}

std::string Parser::parseIdentifier()
{
    std::optional<Token> tokPtr{takeToken()};

    if (tokPtr.has_value())
    {
        Token tok{*tokPtr};
        if (tok.getType() != TokenType::IDENTIFIER)
        {
            raiseError("an identifier", tokenTypeToString(tok.getType()));
        }

        if (std::holds_alternative<std::string>(tok.getValue()))
        {
            return std::get<std::string>(tok.getValue());
        }

        raiseError("an identifier", tokenTypeToString(tok.getType()));
    }

    raiseError("an identifier", "empty token");
    return "";
}

std::shared_ptr<AST::Expression> Parser::parseExp()
{
    std::optional<Token> nextToken{peekToken()};

    if (!nextToken.has_value())
    {
        raiseError("an expression", "empty token");
    }

    switch (nextToken->getType())
    {
    case TokenType::CONSTANT:
        return parseConst();
    case TokenType::HYPHEN:
    case TokenType::TILDE:
    {
        AST::UnaryOp op{parseUnop()};
        std::shared_ptr<AST::Expression> innerExp{parseExp()};

        return std::make_shared<AST::Unary>(op, innerExp);
    }
    case TokenType::OPEN_PAREN:
    {
        takeToken();
        std::shared_ptr<AST::Expression> innerExp{parseExp()};
        expect(TokenType::CLOSE_PAREN);

        return innerExp;
    }
    default:
        raiseError("an expression", tokenTypeToString(nextToken->getType()));
    }

    return nullptr;
}

std::shared_ptr<AST::Statement> Parser::parseStatement()
{
    expect(TokenType::KEYWORD_RETURN);
    std::shared_ptr<AST::Expression> retVal{parseExp()};
    expect(TokenType::SEMICOLON);

    return std::make_shared<AST::Return>(retVal);
}

std::shared_ptr<AST::FunctionDefinition> Parser::parseFunctionDefinition()
{
    expect(TokenType::KEYWORD_INT);
    std::string name{parseIdentifier()};
    expect(TokenType::OPEN_PAREN);
    expect(TokenType::KEYWORD_VOID);
    expect(TokenType::CLOSE_PAREN);
    expect(TokenType::OPEN_BRACE);
    std::shared_ptr<AST::Statement> body{parseStatement()};
    expect(TokenType::CLOSE_BRACE);

    return std::make_shared<AST::FunctionDefinition>(name, body);
}

std::shared_ptr<AST::Program> Parser::parseProgram()
{
    std::shared_ptr<AST::FunctionDefinition> funDef{parseFunctionDefinition()};
    std::optional<Token> nextToken{takeToken()};

    if (nextToken.has_value())
    {
        raiseError("end of input", tokenTypeToString(nextToken->getType()));
    }

    return std::make_shared<AST::Program>(funDef);
}

std::shared_ptr<AST::Program> Parser::parse(const std::string &input)
{
    _lexer.setInput(input);

    return parseProgram();
}