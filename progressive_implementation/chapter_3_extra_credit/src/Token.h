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
    AMPERSAND,
    CARET,
    PIPE,
    DOUBLE_LEFT_BRACKET,
    DOUBLE_RIGHT_BRACKET,
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