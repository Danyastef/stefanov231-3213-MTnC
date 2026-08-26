#pragma once
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

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
    PREPROCESSOR,
    UNKNOWN
};

struct Token
{
    TokenType type;
    string value;
    int line;
};

struct LexError
{
    string message;
    int line;
};

string tokenTypeName(TokenType type);

class Lexer
{
public:
    explicit Lexer(const string& source);

    void tokenize();

    const vector<Token>& getTokens() const;
    const vector<LexError>& getErrors() const;

private:
    string source;
    size_t position;
    int currentLine;

    vector<Token> tokens;
    vector<LexError> errors;

    void readPreprocessor();
    void readString();
    void readChar();
    void readNumber();
    void readIdentifier();

    bool tryReadDoubleOperator();
};

void printTokenTable(const vector<Token>& tokens);
void printTokenSequence(const vector<Token>& tokens);
void printLexicalErrors(const vector<LexError>& errors);
