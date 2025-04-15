#ifndef PARSER_H
#define PARSER_H

#include <optional>
#include <memory>
#include <stdexcept>
#include <string>
#include <set>

#include "Token.h"
#include "Lexer.h"
#include "AST.h"
#include "Types.h"

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

    std::vector<Token> parseSpecifierList();
    AST::StorageClass parseStorageClass(const Token &spec);
    std::pair<Types::DataType, std::optional<AST::StorageClass>> parseTypeAndStorageClass(const std::vector<Token> &specifierList);

    std::optional<std::shared_ptr<AST::Expression>> parseOptionalExp(TokenType delim);
    std::shared_ptr<AST::Switch> parseSwitchStatement();
    std::shared_ptr<AST::Case> parseCaseStatement();
    std::shared_ptr<AST::Default> parseDefaultStatement();
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
    std::vector<std::shared_ptr<AST::Expression>> parseOptionalArgList();
    std::vector<std::shared_ptr<AST::Expression>> parseArgList();
    std::vector<std::string> parseParamList();
    std::shared_ptr<AST::FunctionDeclaration> finishParsingFunctionDeclaration(const std::string &name, std::optional<AST::StorageClass> storageClass);
    std::shared_ptr<AST::VariableDeclaration> finishParsingVariableDeclaration(const std::string &name, std::optional<AST::StorageClass> storageClass);
    std::shared_ptr<AST::VariableDeclaration> parseVariableDeclaration();
    std::shared_ptr<AST::FunctionDeclaration> parseFunctionDeclaration();
    std::vector<std::shared_ptr<AST::Declaration>> parseDeclarationList();
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
    std::set<TokenType> _specifierTypes = {
        TokenType::KEYWORD_INT,
        TokenType::KEYWORD_STATIC,
        TokenType::KEYWORD_EXTERN,
    };
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