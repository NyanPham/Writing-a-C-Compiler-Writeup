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

    return "Token(type=" + tokenTypeToString(_type) + ", value=\"" + valueStr + "\", pos=" + std::to_string(_pos) + ")";
}

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::CONST_CHAR:
        return "CONST_CHAR";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::CONST_INT:
        return "CONST_INT";
    case TokenType::CONST_LONG:
        return "CONST_LONG";
    case TokenType::CONST_UINT:
        return "CONST_UINT";
    case TokenType::CONST_ULONG:
        return "CONST_ULONG";
    case TokenType::CONST_DOUBLE:
        return "CONST_DOUBLE";
    case TokenType::KEYWORD_CHAR:
        return "KEYWORD_CHAR";
    case TokenType::KEYWORD_INT:
        return "KEYWORD_INT";
    case TokenType::KEYWORD_LONG:
        return "KEYWORD_LONG";
    case TokenType::KEYWORD_SIGNED:
        return "KEYWORD_SIGNED";
    case TokenType::KEYWORD_UNSIGNED:
        return "KEYWORD_UNSIGNED";
    case TokenType::KEYWORD_DOUBLE:
        return "KEYWORD_DOUBLE";
    case TokenType::KEYWORD_VOID:
        return "KEYWORD_VOID";
    case TokenType::KEYWORD_RETURN:
        return "KEYWORD_RETURN";
    case TokenType::KEYWORD_IF:
        return "KEYWORD_IF";
    case TokenType::KEYWORD_ELSE:
        return "KEYWORD_ELSE";
    case TokenType::KEYWORD_GOTO:
        return "KEYWORD_GOTO";
    case TokenType::KEYWORD_BREAK:
        return "KEYWORD_BREAK";
    case TokenType::KEYWORD_CONTINUE:
        return "KEYWORD_CONTINUE";
    case TokenType::KEYWORD_WHILE:
        return "KEYWORD_WHILE";
    case TokenType::KEYWORD_DO:
        return "KEYWORD_DO";
    case TokenType::KEYWORD_FOR:
        return "KEYWORD_FOR";
    case TokenType::KEYWORD_SWITCH:
        return "KEYWORD_SWITCH";
    case TokenType::KEYWORD_CASE:
        return "KEYWORD_CASE";
    case TokenType::KEYWORD_DEFAULT:
        return "KEYWORD_DEFAULT";
    case TokenType::KEYWORD_STATIC:
        return "KEYWORD_STATIC";
    case TokenType::KEYWORD_EXTERN:
        return "KEYWORD_EXTERN";
    case TokenType::KEYWORD_SIZEOF:
        return "KEYWORD_SIZEOF";
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
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::OPEN_BRACKET:
        return "OPEN_BRACKET";
    case TokenType::CLOSE_BRACKET:
        return "CLOSE_BRACKET";
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
    case TokenType::BANG:
        return "BANG";
    case TokenType::LOGICAL_AND:
        return "LOGICAL_AND";
    case TokenType::LOGICAL_OR:
        return "LOGICAL_OR";
    case TokenType::DOUBLE_EQUAL:
        return "DOUBLE_EQUAL";
    case TokenType::NOT_EQUAL:
        return "NOT_EQUAL";
    case TokenType::LESS_THAN:
        return "LESS_THAN";
    case TokenType::LESS_OR_EQUAL:
        return "LESS_OR_EQUAL";
    case TokenType::GREATER_THAN:
        return "GREATER_THAN";
    case TokenType::GREATER_OR_EQUAL:
        return "GREATER_OR_EQUAL";
    case TokenType::EQUAL_SIGN:
        return "EQUAL_SIGN";
    case TokenType::DOUBLE_PLUS:
        return "DOUBLE_PLUS";
    case TokenType::PLUS_EQUAL:
        return "PLUS_EQUAL";
    case TokenType::HYPHEN_EQUAL:
        return "HYPHEN_EQUAL";
    case TokenType::STAR_EQUAL:
        return "STAR_EQUAL";
    case TokenType::SLASH_EQUAL:
        return "SLASH_EQUAL";
    case TokenType::PERCENT_EQUAL:
        return "PERCENT_EQUAL";
    case TokenType::AMPERSAND_EQUAL:
        return "AMPERSAND_EQUAL";
    case TokenType::PIPE_EQUAL:
        return "PIPE_EQUAL";
    case TokenType::CARET_EQUAL:
        return "CARET_EQUAL";
    case TokenType::DOUBLE_LEFT_BRACKET_EQUAL:
        return "DOUBLE_LEFT_BRACKET_EQUAL";
    case TokenType::DOUBLE_RIGHT_BRACKET_EQUAL:
        return "DOUBLE_RIGHT_BRACKET_EQUAL";
    case TokenType::QUESTION_MARK:
        return "QUESTION_MARK";
    case TokenType::COLON:
        return "COLON";
    default:
        return "UNKNOWN";
    }
}