#include "lexer.h"
#include <iostream>
#include <unordered_set>
#include <vector>
#include <cctype>

using namespace std;

static const unordered_set<string> KEYWORDS =
{
    "int", "double", "float", "bool", "char", "void",
    "return", "if", "else", "for", "while",
    "using", "namespace", "const"
};

static const unordered_set<string> BOOL_CONSTANTS = {"true", "false"};

static const vector<string> DOUBLE_OPERATORS =
{
    "+=", "-=", "*=", "/=", "%=",
    "++", "--", "<=", ">=", "==", "!=",
    "&&", "||", "<<", ">>", "::"
};

static const unordered_set<char> SINGLE_OPERATORS =
{
    '=', '+', '-', '*', '/', '%', '<', '>', '!', '&'
};

static const unordered_set<char> DELIMITERS =
{
    '(', ')', '{', '}', '[', ']', ';', ',', ':', '.'
};

string tokenTypeName(TokenType type)
{
    switch (type)
    {
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::CONSTANT_INT: return "CONSTANT_INT";
        case TokenType::CONSTANT_FLOAT: return "CONSTANT_FLOAT";
        case TokenType::CONSTANT_STRING: return "CONSTANT_STRING";
        case TokenType::CONSTANT_CHAR: return "CONSTANT_CHAR";
        case TokenType::CONSTANT_BOOL: return "CONSTANT_BOOL";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::DELIMITER: return "DELIMITER";
        case TokenType::PREPROCESSOR: return "PREPROCESSOR";
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(const string& source)
    : source(source), position(0), currentLine(1) {}

const vector<Token>& Lexer::getTokens() const { return tokens; }
const vector<LexError>& Lexer::getErrors() const { return errors; }

void Lexer::tokenize()
{
    while (position < source.size())
    {
        char current = source[position];

        if (current == '\n')
        {
            ++currentLine;
            ++position;
            continue;
        }

        if (isspace(static_cast<unsigned char>(current)))
        {
            ++position;
            continue;
        }

        if (current == '#') { readPreprocessor(); continue; }
        if (current == '"') { readString(); continue; }
        if (current == '\'') { readChar(); continue; }
        if (isdigit(static_cast<unsigned char>(current))) { readNumber(); continue; }

        if (isalpha(static_cast<unsigned char>(current)) || current == '_')
        {
            readIdentifier();
            continue;
        }

        if (tryReadDoubleOperator()) continue;

        if (SINGLE_OPERATORS.count(current))
        {
            tokens.push_back({TokenType::OPERATOR, string(1, current), currentLine});
            ++position;
            continue;
        }

        if (DELIMITERS.count(current))
        {
            tokens.push_back({TokenType::DELIMITER, string(1, current), currentLine});
            ++position;
            continue;
        }

        errors.push_back({"Invalid character: '" + string(1, current) + "'", currentLine});
        ++position;
    }
}

void Lexer::readPreprocessor()
{
    size_t start = position;
    int line = currentLine;

    while (position < source.size() && source[position] != '\n')
        ++position;

    tokens.push_back({
        TokenType::PREPROCESSOR,
        source.substr(start, position - start),
        line
    });
}

void Lexer::readString()
{
    int line = currentLine;
    string value = "\"";
    ++position;

    while (position < source.size())
    {
        char c = source[position];

        if (c == '\\' && position + 1 < source.size())
        {
            value += c;
            value += source[position + 1];
            position += 2;
            continue;
        }

        if (c == '"')
        {
            value += c;
            ++position;
            tokens.push_back({TokenType::CONSTANT_STRING, value, line});
            return;
        }

        if (c == '\n')
        {
            errors.push_back({"Unclosed string literal", line});
            return;
        }

        value += c;
        ++position;
    }

    errors.push_back({"Unclosed string literal", line});
}

void Lexer::readChar()
{
    int line = currentLine;
    string value = "'";
    ++position;

    while (position < source.size())
    {
        char c = source[position];

        if (c == '\\' && position + 1 < source.size())
        {
            value += c;
            value += source[position + 1];
            position += 2;
            continue;
        }

        if (c == '\'')
        {
            value += c;
            ++position;
            tokens.push_back({TokenType::CONSTANT_CHAR, value, line});
            return;
        }

        if (c == '\n')
        {
            errors.push_back({"Unclosed character literal", line});
            return;
        }

        value += c;
        ++position;
    }

    errors.push_back({"Unclosed character literal", line});
}

void Lexer::readNumber()
{
    int line = currentLine;
    size_t start = position;
    bool hasPoint = false;
    bool malformed = false;

    while (position < source.size())
    {
        char c = source[position];

        if (isdigit(static_cast<unsigned char>(c)))
        {
            ++position;
            continue;
        }

        if (c == '.')
        {
            if (hasPoint) malformed = true;
            hasPoint = true;
            ++position;
            continue;
        }

        if (isalpha(static_cast<unsigned char>(c)) || c == '_')
        {
            ++position;
            while (position < source.size() &&
                  (isalnum(static_cast<unsigned char>(source[position])) ||
                   source[position] == '_'))
                ++position;

            string bad = source.substr(start, position - start);
            errors.push_back({"Invalid numeric lexeme: '" + bad + "'", line});
            return;
        }

        break;
    }

    string value = source.substr(start, position - start);

    if (malformed)
    {
        errors.push_back({"Invalid number: '" + value + "'", line});
        return;
    }

    tokens.push_back({
        hasPoint ? TokenType::CONSTANT_FLOAT : TokenType::CONSTANT_INT,
        value,
        line
    });
}

void Lexer::readIdentifier()
{
    int line = currentLine;
    size_t start = position;

    while (position < source.size() &&
          (isalnum(static_cast<unsigned char>(source[position])) ||
           source[position] == '_'))
        ++position;

    string value = source.substr(start, position - start);

    if (BOOL_CONSTANTS.count(value))
        tokens.push_back({TokenType::CONSTANT_BOOL, value, line});
    else if (KEYWORDS.count(value))
        tokens.push_back({TokenType::KEYWORD, value, line});
    else
        tokens.push_back({TokenType::IDENTIFIER, value, line});
}

bool Lexer::tryReadDoubleOperator()
{
    if (position + 1 >= source.size()) return false;

    string candidate = source.substr(position, 2);

    for (const string& op : DOUBLE_OPERATORS)
    {
        if (candidate == op)
        {
            tokens.push_back({TokenType::OPERATOR, op, currentLine});
            position += 2;
            return true;
        }
    }

    return false;
}

void printLexicalErrors(const vector<LexError>& errors)
{
    if (errors.empty())
    {
        cout << "Lexical analysis completed successfully. No errors found.\n";
        return;
    }

    cout << "LEXICAL ERRORS\n";
    for (const auto& error : errors)
        cout << "[Line " << error.line << "] " << error.message << '\n';
}
