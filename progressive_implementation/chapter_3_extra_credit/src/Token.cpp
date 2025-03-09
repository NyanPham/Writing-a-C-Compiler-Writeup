#include "Token.h"

std::string Token::toString() const
{
    std::string valueStr = std::visit([](auto &&arg) -> std::string // auto&& arg uses templates to generate code for each possible type in std::variant
                                      {
        using T = std::decay_t<decltype(arg)>; // decltype queries the type of arg at compile time. std::decay_t removes const, volatile, and references from a type
        if constexpr (std::is_same_v<T, std::string>) // std::is_same_v checks if two types are the same at compile time
            return arg;
        else
            return std::to_string(arg); }, _value);

    return "Token(type=" + tokenTypeToString(_type) + ", value=" + valueStr + ", pos=" + std::to_string(_pos) + ")";
}

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::CONSTANT:
        return "CONSTANT";
    case TokenType::KEYWORD_INT:
        return "KEYWORD_INT";
    case TokenType::KEYWORD_VOID:
        return "KEYWORD_VOID";
    case TokenType::KEYWORD_RETURN:
        return "KEYWORD_RETURN";
    case TokenType::OPEN_PAREN:
        return "OPEN_PAREN";
    case TokenType::CLOSE_PAREN:
        return "CLOSE_PAREN";
    case TokenType::OPEN_BRACE:
        return "OPEN_BRACE";
    case TokenType::CLOSE_BRACE:
        return "CLOSE_BRACE";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::HYPHEN:
        return "HYPHEN";
    case TokenType::DOUBLE_HYPHEN:
        return "DOUBLE_HYPHEN";
    case TokenType::TILDE:
        return "TILDE";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::PERCENT:
        return "PERCENT";
    case TokenType::AMPERSAND:
        return "AMPERSAND";
    case TokenType::CARET:
        return "CARET";
    case TokenType::PIPE:
        return "PIPE";
    case TokenType::DOUBLE_LEFT_BRACKET:
        return "DOUBLE_LEFT_BRACKET";
    case TokenType::DOUBLE_RIGHT_BRACKET:
        return "DOUBLE_RIGHT_BRACKET";
    default:
        return "UNKNOWN";
    }
}