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

<program> ::= { <function-declaration> }
<declaration> ::= <variable-declaration> | <function-declaration>
<variable-declaration> ::= "int" <identifier> [ "=" <exp> ] ";"
<function-declaration> ::= "int" <identifier> "(" <param-list> ")" (<block> | ";")
<param-list> :: = "void" | "int" <identifier> { "," "int" <identifier> }
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
<primary-exp> ::= <int> | <identifier> | "(" <exp> ")" | <identifier> "(" [ <argument-list> ] ")"
<argument-list> ::= <exp> { "," <exp> }
<unop> ::= "-" | "~" | "!" | "++" | "--"
<binop> ::= "+" | "-" | "*" | "/" | "%"
        | "&&" | "||"
        | "==" | "!=" | "<" | "<="
        | ">" | ">="
        | "&" | "^" | "|" | "<<" | ">>"
        | "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
<int> ::= ? An integer token ?
<identifier> ::= ? An identifier token ?
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

std::shared_ptr<AST::Constant> Parser::parseConst()
{
    std::optional<Token> token{takeToken()};

    if (token.has_value())
    {
        if (token->getType() != TokenType::CONSTANT)
        {
            raiseError("a constant", tokenTypeToString(token->getType()));
        }

        if (std::holds_alternative<int>(token->getValue()))
        {
            int value{std::get<int>(token->getValue())};
            return std::make_shared<AST::Constant>(value);
        }

        raiseError("an integer", "non-integer value");
    }

    raiseError("a constant", "empty token");
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

std::optional<std::shared_ptr<AST::Expression>> Parser::parseOptionalExp(TokenType delim)
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

    if (nextToken->getType() == TokenType::KEYWORD_INT)
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
    case TokenType::CONSTANT:
        return parseConst();

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
    auto nextToken{peekToken()};

    if (!nextToken.has_value())
    {
        raiseError("an expression", "empty token");
    }
    switch (nextToken->getType())
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

std::vector<std::string> Parser::parseParamList()
{
    expect(TokenType::KEYWORD_INT);
    auto nextParam{parseIdentifier()};

    auto args{std::vector<std::string>{nextParam}};

    auto nextToken{peekToken()};

    while (nextToken.has_value())
    {
        if (nextToken->getType() == TokenType::COMMA)
        {
            takeToken();
            expect(TokenType::KEYWORD_INT);
            nextParam = parseIdentifier();
            args.push_back(nextParam);
            nextToken = peekToken();
        }
        else
            break;
    }

    return args;
}

std::shared_ptr<AST::FunctionDeclaration> Parser::finishParsingFunctionDeclaration(const std::string &name)
{
    expect(TokenType::OPEN_PAREN);
    std::vector<std::string> params{};

    if (!peekToken().has_value())
    {
        throw std::runtime_error("Unexpected end of file while parsing function declaration");
    }

    if (peekToken()->getType() == TokenType::KEYWORD_VOID)
    {
        takeToken();
    }
    else if (peekToken()->getType() != TokenType::CLOSE_PAREN)
    {
        params = parseParamList();
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

    return std::make_shared<AST::FunctionDeclaration>(name, params, body);
}

std::shared_ptr<AST::VariableDeclaration> Parser::finishParsingVariableDeclaration(const std::string &name)
{
    auto nextToken{takeToken()};
    if (!nextToken.has_value())
        raiseError("a semicolon or initializer", "empty token");

    switch (nextToken->getType())
    {
    case TokenType::SEMICOLON:
        return std::make_shared<AST::VariableDeclaration>(name, std::nullopt);
    case TokenType::EQUAL_SIGN:
    {
        auto init = parseExp(0);
        expect(TokenType::SEMICOLON);

        return std::make_shared<AST::VariableDeclaration>(name, std::make_optional(init));
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
    expect(TokenType::KEYWORD_INT);
    auto name{parseIdentifier()};

    auto nextToken{peekToken()};

    if (nextToken.has_value() && nextToken->getType() == TokenType::OPEN_PAREN)
        return finishParsingFunctionDeclaration(name);
    else
        return finishParsingVariableDeclaration(name);
}

std::shared_ptr<AST::BlockItem> Parser::parseBlockItem()
{
    auto nextToken{peekToken()};

    if (nextToken->getType() == TokenType::KEYWORD_INT)
    {
        return parseDeclaration();
    }
    else
    {
        return parseStatement();
    }
}

std::vector<std::shared_ptr<AST::FunctionDeclaration>> Parser::parseFunctionDeclarationList()
{
    std::vector<std::shared_ptr<AST::FunctionDeclaration>> fnList{};
    auto nextToken{peekToken()};

    while (nextToken.has_value())
    {
        auto nextDecl{parseDeclaration()};

        if (nextDecl->getType() == AST::NodeType::FunctionDeclaration)
        {
            fnList.push_back(std::dynamic_pointer_cast<AST::FunctionDeclaration>(nextDecl));
        }
        else
        {
            raiseError("a function declaration at top level", "a variable declaration");
        }

        nextToken = peekToken();
    }

    return fnList;
}

std::shared_ptr<AST::Program> Parser::parseProgram()
{
    std::vector<std::shared_ptr<AST::FunctionDeclaration>> fnDecls{parseFunctionDeclarationList()};
    std::optional<Token> nextToken{peekToken()};

    if (nextToken.has_value())
        raiseError("end of input", tokenTypeToString(nextToken->getType()));

    return std::make_shared<AST::Program>(fnDecls);
}

std::shared_ptr<AST::Program> Parser::parse(const std::string &input)
{
    _lexer.setInput(input);

    return parseProgram();
}