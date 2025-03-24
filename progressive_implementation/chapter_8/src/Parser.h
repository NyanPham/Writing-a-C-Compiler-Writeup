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
    std::optional<AST::BinaryOp> getCompoundOperator(std::optional<Token> token);
    bool isAssignment(std::optional<Token> token);
    AST::UnaryOp parseUnop();
    AST::BinaryOp parseBinop();
    std::shared_ptr<AST::Constant> parseConst();
    std::string parseIdentifier();

    std::optional<std::shared_ptr<AST::Expression>> parseOptionalExp(TokenType delim);
    std::shared_ptr<AST::ForInit> parseForInit();
    std::shared_ptr<AST::While> parseWhileLoop();
    std::shared_ptr<AST::DoWhile> parseDoLoop();
    std::shared_ptr<AST::For> parseForLoop();

    std::shared_ptr<AST::Expression> parseConditionMiddle();
    std::shared_ptr<AST::Expression> parsePostfixHelper(std::shared_ptr<AST::Expression> primary);
    std::shared_ptr<AST::Expression> parsePostfixExp();
    std::shared_ptr<AST::Expression> parsePrimaryExp();
    std::shared_ptr<AST::Expression> parseFactor();
    std::shared_ptr<AST::Expression> parseExp(int minPrec = 0);
    AST::Block parseBlock();
    std::shared_ptr<AST::Statement> parseStatement();
    std::shared_ptr<AST::Declaration> parseDeclaration();
    std::shared_ptr<AST::BlockItem> parseBlockItem();
    std::shared_ptr<AST::FunctionDefinition> parseFunctionDefinition();
    std::shared_ptr<AST::Program> parseProgram();
    std::shared_ptr<AST::Program> parse(const std::string &input);

    void raiseError(const std::string &expected, const std::string &actual);
    std::optional<Token> takeToken();
    std::optional<Token> peekToken();
    std::vector<std::optional<Token>> peekTokens(int n);
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