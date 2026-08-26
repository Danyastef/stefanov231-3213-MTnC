#pragma once
#include <string>
#include <vector>

enum class TokenType
{
    KEYWORD,
    IDENTIFIER,
    CONSTANT_INT,
    CONSTANT_FLOAT,
    CONSTANT_STRING,
    CONSTANT_CHAR,
    CONSTANT_BOOL,
    OPERATOR,
    DELIMITER,
    PREPROCESSOR
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
};

struct LexError
{
    std::string message;
    int line;
};

std::string tokenTypeName(TokenType type);

class Lexer
{
public:
    explicit Lexer(const std::string& source);
    void tokenize();

    const std::vector<Token>& getTokens() const;
    const std::vector<LexError>& getErrors() const;

private:
    std::string source;
    size_t position;
    int currentLine;

    std::vector<Token> tokens;
    std::vector<LexError> errors;

    void readPreprocessor();
    void readString();
    void readChar();
    void readNumber();
    void readIdentifier();
    bool tryReadDoubleOperator();
};

void printLexicalErrors(const std::vector<LexError>& errors);
