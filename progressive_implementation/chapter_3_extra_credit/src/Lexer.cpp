#include <string>
#include <optional>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <regex>

#include "Lexer.h"
#include "Token.h"

/**
 * Check if the idenfitier is a keyword and convert it to the appropriate token
 * Recognized keywords are: int, void, return
 */
Token convertIdentifer(const std::string &str, long pos)
{
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KEYWORD_INT},
        {"void", TokenType::KEYWORD_VOID},
        {"return", TokenType::KEYWORD_RETURN}};

    auto it = keywords.find(str);
    if (it != keywords.end())
    {
        return Token(it->second, str, pos);
    }

    return Token(TokenType::IDENTIFIER, str, pos);
}

/**
 * For PART I, we only support integer constants and focus on the grammar of C
 */
Token convertConstant(const std::string &str, long pos)
{
    return Token(TokenType::CONSTANT, std::stoi(str), pos);
}

void Lexer::setInput(std::string input)
{
    _input = std::move(input);
}

void Lexer::defineTokenDefs()
{
    _tokenDefs = {
        {std::regex("[A-Za-z_][A-Za-z0-9_]*\\b"), convertIdentifer},
        {std::regex("[0-9]+\\b"), convertConstant},
        // The following 3 keywords match will not be reached after identifier, but still kept here for references.
        {std::regex("int\\b"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::KEYWORD_INT, str, pos);
         }},
        {std::regex("void\\b"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::KEYWORD_VOID, str, pos);
         }},
        {std::regex("return\\b"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::KEYWORD_RETURN, str, pos);
         }},
        {std::regex("\\("), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::OPEN_PAREN, str, pos);
         }},
        {std::regex("\\)"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::CLOSE_PAREN, str, pos);
         }},
        {std::regex("\\{"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::OPEN_BRACE, str, pos);
         }},
        {std::regex("\\}"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::CLOSE_BRACE, str, pos);
         }},
        {std::regex(";"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::SEMICOLON, str, pos);
         }},
        {std::regex("-"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::HYPHEN, str, pos);
         }},
        {std::regex("--"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::DOUBLE_HYPHEN, str, pos);
         }},
        {std::regex("~"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::TILDE, str, pos);
         }},
        {std::regex("\\+"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::PLUS, str, pos);
         }},
        {std::regex("\\*"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::STAR, str, pos);
         }},
        {std::regex("/"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::SLASH, str, pos);
         }},
        {std::regex("%"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::PERCENT, str, pos);
         }},

        {std::regex("\\&"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::AMPERSAND, str, pos);
         }},

        {std::regex("\\^"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::CARET, str, pos);
         }},

        {std::regex("\\|"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::PIPE, str, pos);
         }},
        {std::regex("<<"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::DOUBLE_LEFT_BRACKET, str, pos);
         }},
        {std::regex(">>"), [](const std::string &str, long pos) -> Token
         {
             return Token(TokenType::DOUBLE_RIGHT_BRACKET, str, pos);
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
    long savedPos = _pos;
    std::optional<Token> nextToken = token();
    _pos = savedPos;

    return nextToken;
}

std::vector<Token> Lexer::npeek(int n)
{
    std::vector<Token> tokens;
    long savedPos = _pos;

    for (int i = 0; i < n; ++i)
    {
        std::optional<Token> nextToken = token();
        if (nextToken == std::nullopt)
            break;

        tokens.push_back(*nextToken);
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
