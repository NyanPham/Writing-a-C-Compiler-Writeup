#ifndef PARSER_H
#define PARSER_H

#include <optional>
#include <memory>
#include <stdexcept>
#include <string>

#include "Token.h"
#include "Lexer.h"
#include "AST.h"

class Parser
{
public:
    Parser()
    {
        _lexer.defineTokenDefs();
    }

    int getPrecedence(TokenType tokenType);
    bool isBinop(TokenType tokenType);
    AST::UnaryOp parseUnop();
    AST::BinaryOp parseBinop();
    std::shared_ptr<AST::Constant> parseConst();
    std::string parseIdentifier();
    std::shared_ptr<AST::Expression> parseFactor();
    std::shared_ptr<AST::Expression> parseExp(int minPrec = 0);
    std::shared_ptr<AST::Statement> parseStatement();
    std::shared_ptr<AST::Declaration> parseDeclaration();
    std::shared_ptr<AST::BlockItem> parseBlockItem();
    std::shared_ptr<AST::FunctionDefinition> parseFunctionDefinition();
    std::shared_ptr<AST::Program> parseProgram();
    std::shared_ptr<AST::Program> parse(const std::string &input);

    void raiseError(const std::string &expected, const std::string &actual);
    std::optional<Token> takeToken();
    std::optional<Token> peekToken();
    std::vector<Token> peekTokens(int n);
    void expect(TokenType expected);

private:
    Lexer _lexer;
    std::optional<Token> _currToken;
};

class ParseError : public std::runtime_error
{
public:
    ParseError(const std::string &message)
        : std::runtime_error("Parse error: " + message)
    {
    }
};

#endif