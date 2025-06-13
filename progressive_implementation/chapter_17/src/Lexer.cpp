#include <string>
#include <optional>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <regex>
#include <cstdint>

#include "Lexer.h"
#include "Token.h"

/**
 * Check if the idenfitier is a keyword and convert it to the appropriate token
 * Recognized keywords are: int, void, return, if, else, goto, break, continue, while, do, for
 */
Token convertIdentifer(const std::string &str, size_t &pos)
{
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"char", TokenType::KEYWORD_CHAR},
        {"int", TokenType::KEYWORD_INT},
        {"long", TokenType::KEYWORD_LONG},
        {"signed", TokenType::KEYWORD_SIGNED},
        {"unsigned", TokenType::KEYWORD_UNSIGNED},
        {"double", TokenType::KEYWORD_DOUBLE},
        {"void", TokenType::KEYWORD_VOID},
        {"return", TokenType::KEYWORD_RETURN},
        {"if", TokenType::KEYWORD_IF},
        {"else", TokenType::KEYWORD_ELSE},
        {"goto", TokenType::KEYWORD_GOTO},
        {"break", TokenType::KEYWORD_BREAK},
        {"continue", TokenType::KEYWORD_CONTINUE},
        {"while", TokenType::KEYWORD_WHILE},
        {"do", TokenType::KEYWORD_DO},
        {"for", TokenType::KEYWORD_FOR},
        {"switch", TokenType::KEYWORD_SWITCH},
        {"case", TokenType::KEYWORD_CASE},
        {"default", TokenType::KEYWORD_DEFAULT},
        {"static", TokenType::KEYWORD_STATIC},
        {"extern", TokenType::KEYWORD_EXTERN},
        {"sizeof", TokenType::KEYWORD_SIZEOF},
    };

    auto it = keywords.find(str);
    if (it != keywords.end())
    {
        return Token(it->second, str, pos);
    }

    return Token(TokenType::IDENTIFIER, str, pos);
}

Token lexChar(const std::string &str, size_t &pos)
{
    if (str.size() < 3 || str.front() != '\'' || str.back() != '\'')
    {
        throw LexError("Invalid character literal at position " + std::to_string(pos));
    }

    std::string value = str.substr(1, str.size() - 2);
    return Token(TokenType::CONST_CHAR, value, pos);
}

Token lexString(const std::string &str, size_t &pos)
{
    if (str.size() < 2 || str.front() != '\"' || str.back() != '\"')
    {
        throw LexError("Invalid string literal at position " + std::to_string(pos));
    }

    std::string value = str.substr(1, str.size() - 2);
    return Token(TokenType::STRING_LITERAL, value, pos);
}

Token lexInt(const std::string &str, size_t &pos)
{
    size_t len = str.length();
    size_t numLen = len;
    while (numLen > 0 && !isdigit(str[numLen - 1]))
    {
        --numLen;
    }

    std::string numStr = str.substr(0, numLen);

    if (numStr.length() < str.length())
        pos -= 1;

    return Token(TokenType::CONST_INT, std::stoull(numStr), pos);
}

Token lexLong(const std::string &str, size_t &pos)
{
    size_t len = str.length();
    size_t numLen = len;
    while (numLen > 0 && !isdigit(str[numLen - 1]))
    {
        --numLen;
    }

    std::string numStr = str.substr(0, numLen);
    if (numStr.length() + 1 < str.length())
        pos -= 1;
    return Token(TokenType::CONST_LONG, std::stoull(numStr), pos);
}

Token lexUInt(const std::string &str, size_t &pos)
{
    size_t len = str.length();
    size_t numLen = len;
    while (numLen > 0 && !isdigit(str[numLen - 1]))
    {
        --numLen;
    }

    std::string numStr = str.substr(0, numLen);
    if (numStr.length() + 1 < str.length())
        pos -= 1;
    return Token(TokenType::CONST_UINT, std::stoull(numStr), pos);
}

Token lexULong(const std::string &str, size_t &pos)
{
    size_t len = str.length();
    size_t numLen = len;
    while (numLen > 0 && !isdigit(str[numLen - 1]))
    {
        --numLen;
    }

    std::string numStr = str.substr(0, numLen);
    if (numStr.length() + 2 < str.length())
        pos -= 1;
    return Token(TokenType::CONST_ULONG, std::stoull(numStr), pos);
}

Token lexDouble(const std::string &str, size_t &pos)
{
    size_t numLen = 0;
    while (numLen < str.length() &&
           (isdigit(str[numLen]) || str[numLen] == '.' || str[numLen] == 'e' || str[numLen] == 'E' ||
            str[numLen] == '+' || str[numLen] == '-'))
    {
        ++numLen;
    }

    std::string numStr = str.substr(0, numLen);

    if (numStr.length() < str.length())
        pos -= 1;

    return Token(TokenType::CONST_DOUBLE, std::stold(numStr), pos);
}

void Lexer::setInput(std::string input)
{
    _input = std::move(input);
}

void Lexer::defineTokenDefs()
{
    _tokenDefs = {
        {std::regex("[A-Za-z_][A-Za-z0-9_]*\\b"), convertIdentifer},
        {std::regex("'([^'\\\\\\n]|\\\\['\"?\\\\abfnrtv])'"), lexChar},
        {std::regex("\"([^\"\\\\\\n]|\\\\['\"\\\\?abfnrtv])*\""), lexString},
        {std::regex("([0-9]+)[^\\w.]"), lexInt},
        {std::regex("([0-9]+[lL])[^\\w.]"), lexLong},
        {std::regex("([0-9]+[uU])[^\\w.]"), lexUInt},
        {std::regex("([0-9]+([lL][uU]|[uU][lL]))[^\\w.]"), lexULong},
        {std::regex("(([0-9]*\\.[0-9]+|[0-9]+\\.?)[Ee][+-]?[0-9]+|[0-9]*\\.[0-9]+|[0-9]+\\.)[^\\w.]"), lexDouble},
        // The following 22 keywords match will not be reached after identifier, but still kept here for references.
        {std::regex("char\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_CHAR, str, pos);
         }},
        {std::regex("int\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_INT, str, pos);
         }},
        {std::regex("long\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_LONG, str, pos);
         }},
        {std::regex("signed\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_SIGNED, str, pos);
         }},
        {std::regex("unsigned\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_UNSIGNED, str, pos);
         }},
        {std::regex("double\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_DOUBLE, str, pos);
         }},
        {std::regex("void\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_VOID, str, pos);
         }},
        {std::regex("return\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_RETURN, str, pos);
         }},
        {std::regex("if\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_IF, str, pos);
         }},
        {std::regex("else\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_ELSE, str, pos);
         }},
        {std::regex("goto\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_GOTO, str, pos);
         }},
        {std::regex("break\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_BREAK, str, pos);
         }},
        {std::regex("continue\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_CONTINUE, str, pos);
         }},
        {std::regex("while\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_WHILE, str, pos);
         }},
        {std::regex("do\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_DO, str, pos);
         }},
        {std::regex("for\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_FOR, str, pos);
         }},
        {std::regex("switch\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_SWITCH, str, pos);
         }},
        {std::regex("case\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_CASE, str, pos);
         }},
        {std::regex("default\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_DEFAULT, str, pos);
         }},
        {std::regex("static\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_STATIC, str, pos);
         }},
        {std::regex("extern\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_EXTERN, str, pos);
         }},
        {std::regex("sizeof\\b"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::KEYWORD_SIZEOF, str, pos);
         }},
        {std::regex("\\("), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::OPEN_PAREN, str, pos);
         }},
        {std::regex("\\)"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::CLOSE_PAREN, str, pos);
         }},
        {std::regex("\\{"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::OPEN_BRACE, str, pos);
         }},
        {std::regex("\\}"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::CLOSE_BRACE, str, pos);
         }},
        {std::regex(";"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::SEMICOLON, str, pos);
         }},
        {std::regex(","), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::COMMA, str, pos);
         }},
        {std::regex("\\["), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::OPEN_BRACKET, str, pos);
         }},
        {std::regex("\\]"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::CLOSE_BRACKET, str, pos);
         }},
        {std::regex("-"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::HYPHEN, str, pos);
         }},
        {std::regex("--"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_HYPHEN, str, pos);
         }},
        {std::regex("~"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::TILDE, str, pos);
         }},
        {std::regex("\\+"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PLUS, str, pos);
         }},
        {std::regex("\\*"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::STAR, str, pos);
         }},
        {std::regex("/"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::SLASH, str, pos);
         }},
        {std::regex("%"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PERCENT, str, pos);
         }},

        {std::regex("\\&"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::AMPERSAND, str, pos);
         }},

        {std::regex("\\^"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::CARET, str, pos);
         }},

        {std::regex("\\|"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PIPE, str, pos);
         }},
        {std::regex("<<"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_LEFT_BRACKET, str, pos);
         }},
        {std::regex(">>"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_RIGHT_BRACKET, str, pos);
         }},
        {std::regex("!"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::BANG, str, pos);
         }},
        {std::regex("\\&\\&"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::LOGICAL_AND, str, pos);
         }},
        {std::regex("\\|\\|"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::LOGICAL_OR, str, pos);
         }},
        {std::regex("=="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_EQUAL, str, pos);
         }},
        {std::regex("!="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::NOT_EQUAL, str, pos);
         }},
        {std::regex("<"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::LESS_THAN, str, pos);
         }},
        {std::regex("<="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::LESS_OR_EQUAL, str, pos);
         }},
        {std::regex(">"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::GREATER_THAN, str, pos);
         }},
        {std::regex(">="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::GREATER_OR_EQUAL, str, pos);
         }},
        {std::regex("="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::EQUAL_SIGN, str, pos);
         }},
        {std::regex("\\+\\+"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_PLUS, str, pos);
         }},
        {std::regex("\\+="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PLUS_EQUAL, str, pos);
         }},
        {std::regex("-="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::HYPHEN_EQUAL, str, pos);
         }},
        {std::regex("\\*="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::STAR_EQUAL, str, pos);
         }},
        {std::regex("/="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::SLASH_EQUAL, str, pos);
         }},
        {std::regex("%="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PERCENT_EQUAL, str, pos);
         }},
        {std::regex("&="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::AMPERSAND_EQUAL, str, pos);
         }},
        {std::regex("\\|="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::PIPE_EQUAL, str, pos);
         }},
        {std::regex("\\^="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::CARET_EQUAL, str, pos);
         }},
        {std::regex("<<="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_LEFT_BRACKET_EQUAL, str, pos);
         }},
        {std::regex(">>="), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::DOUBLE_RIGHT_BRACKET_EQUAL, str, pos);
         }},
        {std::regex("\\?"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::QUESTION_MARK, str, pos);
         }},
        {std::regex("\\:"), [](const std::string &str, size_t &pos) -> Token
         {
             return Token(TokenType::COLON, str, pos);
         }},
    };
};

void Lexer::skipWhitespaceAndComments()
{
    while (_pos < _input.size())
    {
        if (_whitespace.find(_input[_pos]) != std::string::npos)
        {
            _pos++;
            continue;
        }

        if (_input[_pos] == '/' && _pos + 1 < _input.size())
        {
            if (_input[_pos + 1] == '/')
            {
                _pos += 2;
                while (_pos < _input.size() && _input[_pos] != '\n')
                {
                    _pos++;
                }
                continue;
            }
            else if (_input[_pos + 1] == '*')
            {
                _pos += 2;
                while (_pos + 1 < _input.size() && (_input[_pos] != '*' || _input[_pos + 1] != '/'))
                {
                    _pos++;
                }
                _pos += 2;
                continue;
            }
        }
        break;
    }
}

std::optional<Token> Lexer::token()
{
    if (_input.empty() || _pos >= _input.size())
        return std::nullopt;

    skipWhitespaceAndComments();

    if (_pos >= _input.size())
        return std::nullopt;

    std::string subInput = std::string(_input.substr(_pos));

    // Find all matches from the rules
    std::vector<TokenDef> matches{};
    std::copy_if(_tokenDefs.begin(), _tokenDefs.end(), std::back_inserter(matches), [this, subInput](TokenDef tokenDef)
                 {
        std::smatch match;
        bool result = std::regex_search(subInput, match, tokenDef.reg) && match.position() == 0;
        return result; });

    if (matches.empty())
    {
        throw LexError("No valid token found at position " + std::to_string(_pos));
    }

    // Find the longest match
    auto longestMatch = std::max_element(matches.begin(), matches.end(), [this, subInput](const TokenDef &a, const TokenDef &b)
                                         {
        std::smatch matchA, matchB;
        std::regex_search(subInput, matchA, a.reg);
        std::regex_search(subInput, matchB, b.reg);
        return matchA.str().length() < matchB.str().length(); });

    // Run converter to produce token and increment the pos based on the matched substring size
    std::smatch match;
    std::regex_search(subInput, match, longestMatch->reg);
    std::string matchedStr = match.str();
    Token token = longestMatch->converter(matchedStr, _pos);
    _pos += matchedStr.length();

    return token;
}

std::optional<Token> Lexer::peek()
{
    size_t savedPos = _pos;
    std::optional<Token> nextToken = token();
    _pos = savedPos;

    return nextToken;
}

std::vector<std::optional<Token>> Lexer::npeek(int n)
{
    std::vector<std::optional<Token>> tokens;
    size_t savedPos = _pos;

    for (int i = 0; i < n; ++i)
    {
        std::optional<Token> nextToken = token();
        tokens.push_back(nextToken);
    }

    _pos = savedPos;
    return tokens;
}

std::vector<Token> Lexer::tokens()
{
    std::vector<Token> tokens{};

    while (true)
    {
        std::optional<Token> nextToken = token();
        if (nextToken == std::nullopt)
            break;

        tokens.push_back(*nextToken);
    }

    return tokens;
}

const std::string_view Lexer::getInput() const
{
    return _input;
}
