#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <optional>
#include <functional>
#include <regex>
#include "Token.h"

struct TokenDef
{
    std::regex reg;
    std::function<Token(const std::string &, int)> converter;
};

class Lexer
{
public:
    Lexer() = default;

    void defineTokenDefs();
    void setInput(std::string input);
    void skipWhitespaceAndComments();
    std::optional<Token> token();
    std::vector<Token> tokens();
    const std::string_view getInput() const;

private:
    std::string _input;
    long _pos = 0;
    std::vector<TokenDef> _tokenDefs;
    const std::string _whitespace = " \t\n\r\v\f ";
};

class LexError : public std::runtime_error
{
public:
    LexError(const std::string &message)
        : std::runtime_error("Lex error: " + message)
    {
    }
};

#endif