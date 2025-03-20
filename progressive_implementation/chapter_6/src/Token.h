#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <variant>

enum class TokenType
{
    IDENTIFIER,
    CONSTANT,

    // Keywords
    KEYWORD_INT,
    KEYWORD_VOID,
    KEYWORD_RETURN,
    KEYWORD_IF,
    KEYWORD_ELSE,

    // Punctuation
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON,

    // Operators
    HYPHEN,
    DOUBLE_HYPHEN,
    TILDE,
    PLUS,
    STAR,
    SLASH,
    PERCENT,
    BANG,
    LOGICAL_AND,
    LOGICAL_OR,
    DOUBLE_EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_OR_EQUAL,
    GREATER_THAN,
    GREATER_OR_EQUAL,
    AMPERSAND,
    CARET,
    PIPE,
    DOUBLE_LEFT_BRACKET,
    DOUBLE_RIGHT_BRACKET,
    EQUAL_SIGN,
    DOUBLE_PLUS,
    PLUS_EQUAL,
    HYPHEN_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    PERCENT_EQUAL,
    AMPERSAND_EQUAL,
    PIPE_EQUAL,
    CARET_EQUAL,
    DOUBLE_LEFT_BRACKET_EQUAL,
    DOUBLE_RIGHT_BRACKET_EQUAL,
    QUESTION_MARK,
    COLON,
};

using TokenValue = std::variant<std::string, int, long>;

class Token
{
public:
    Token(TokenType type, TokenValue value, int pos)
        : _type{type}, _value{value}, _pos{pos}
    {
    }
    std::string toString() const;

    TokenType getType() const { return _type; }
    TokenValue getValue() const { return _value; }
    int getPos() const { return _pos; }

private:
    TokenType _type;
    TokenValue _value;
    int _pos;
};

std::string tokenTypeToString(TokenType type);

#endif