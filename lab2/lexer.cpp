#include "lexer.h"

#include <iostream>
#include <cctype>
#include <iomanip>

using namespace std;


// ======================================================
// LEXICAL TABLES
// ======================================================

static const unordered_set<string> KEYWORDS =
{
    "int",
    "double",
    "return",
    "if",
    "else",
    "for",
    "do",
    "while",
    "using",
    "namespace",
    "const"
};


static const unordered_set<string> BOOL_CONSTANTS =
{
    "true",
    "false"
};


static const unordered_set<string> VALID_IDENTIFIERS =
{
    "calculateSum",
    "findMinimum",
    "findMaximum",
    "countPositive",
    "main",

    "numbers",
    "sum",
    "value",
    "minimum",
    "maximum",
    "counter",
    "count",
    "i",
    "average",
    "positiveCount",

    "vector",
    "size_t",

    "cout",
    "cin",
    "endl",

    "setlocale",
    "LC_ALL",

    "numeric_limits",
    "streamsize",
    "max",

    "clear",
    "ignore",
    "size",

    "std"
};


static const vector<string> DOUBLE_OPERATORS =
{
    "+=",
    "++",
    "--",
    "<=",
    ">=",
    "==",
    "!=",
    "&&",
    "||",
    "<<",
    ">>",
    "::"
};


static const unordered_set<char> SINGLE_OPERATORS =
{
    '=',
    '+',
    '-',
    '*',
    '/',
    '%',
    '<',
    '>',
    '!',
    '&'
};


static const unordered_set<char> DELIMITERS =
{
    '(',
    ')',
    '{',
    '}',
    '[',
    ']',
    ';',
    ',',
    ':',
    '.'
};


// ======================================================
// TOKEN TYPE NAME
// ======================================================

string tokenTypeName(TokenType type)
{
    switch (type)
    {
        case TokenType::KEYWORD:
            return "KEYWORD";

        case TokenType::IDENTIFIER:
            return "IDENTIFIER";

        case TokenType::CONSTANT_INT:
            return "CONSTANT_INT";

        case TokenType::CONSTANT_FLOAT:
            return "CONSTANT_FLOAT";

        case TokenType::CONSTANT_STRING:
            return "CONSTANT_STRING";

        case TokenType::CONSTANT_CHAR:
            return "CONSTANT_CHAR";

        case TokenType::CONSTANT_BOOL:
            return "CONSTANT_BOOL";

        case TokenType::OPERATOR:
            return "OPERATOR";

        case TokenType::DELIMITER:
            return "DELIMITER";

        case TokenType::PREPROCESSOR:
            return "PREPROCESSOR";

        default:
            return "UNKNOWN";
    }
}


// ======================================================
// LEXER CONSTRUCTOR
// ======================================================

Lexer::Lexer(const string& source)
    : source(source),
      position(0),
      currentLine(1)
{
}


const vector<Token>& Lexer::getTokens() const
{
    return tokens;
}


const vector<LexError>& Lexer::getErrors() const
{
    return errors;
}


// ======================================================
// MAIN TOKENIZATION
// ======================================================

void Lexer::tokenize()
{
    while (position < source.size())
    {
        char current = source[position];


        // New line
        if (current == '\n')
        {
            currentLine++;
            position++;
            continue;
        }


        // Spaces and tabs
        if (isspace(static_cast<unsigned char>(current)))
        {
            position++;
            continue;
        }


        // Preprocessor directive
        if (current == '#')
        {
            readPreprocessor();
            continue;
        }


        // String literal
        if (current == '"')
        {
            readString();
            continue;
        }


        // Character literal
        if (current == '\'')
        {
            readChar();
            continue;
        }


        // Number
        if (isdigit(static_cast<unsigned char>(current)))
        {
            readNumber();
            continue;
        }


        // Keyword or identifier
        if (isalpha(static_cast<unsigned char>(current))
            || current == '_')
        {
            readIdentifier();
            continue;
        }


        // Two-character operator
        if (tryReadDoubleOperator())
        {
            continue;
        }


        // Single-character operator
        if (SINGLE_OPERATORS.count(current))
        {
            tokens.push_back(
            {
                TokenType::OPERATOR,
                string(1, current),
                currentLine
            });

            position++;
            continue;
        }


        // Delimiter
        if (DELIMITERS.count(current))
        {
            tokens.push_back(
            {
                TokenType::DELIMITER,
                string(1, current),
                currentLine
            });

            position++;
            continue;
        }


        // Unknown symbol
        errors.push_back(
        {
            "Invalid character: '" +
            string(1, current) + "'",
            currentLine
        });

        position++;
    }
}


// ======================================================
// PREPROCESSOR DIRECTIVE
// ======================================================

void Lexer::readPreprocessor()
{
    int startLine = currentLine;
    size_t start = position;


    while (position < source.size()
           && source[position] != '\n')
    {
        position++;
    }


    string directive =
        source.substr(start, position - start);


    if (directive.rfind("#include", 0) != 0)
    {
        errors.push_back(
        {
            "Unknown preprocessor directive: " + directive,
            startLine
        });

        return;
    }


    tokens.push_back(
    {
        TokenType::PREPROCESSOR,
        directive,
        startLine
    });
}


// ======================================================
// STRING LITERAL
// ======================================================

void Lexer::readString()
{
    int startLine = currentLine;

    string value = "\"";

    position++;

    bool closed = false;


    while (position < source.size())
    {
        char current = source[position];


        // Escape sequence
        if (current == '\\'
            && position + 1 < source.size())
        {
            value += current;
            value += source[position + 1];

            position += 2;

            continue;
        }


        if (current == '"')
        {
            value += current;

            position++;

            closed = true;

            break;
        }


        if (current == '\n')
        {
            errors.push_back(
            {
                "Unclosed string literal",
                startLine
            });

            currentLine++;
            position++;

            return;
        }


        value += current;

        position++;
    }


    if (!closed)
    {
        errors.push_back(
        {
            "Unclosed string literal",
            startLine
        });

        return;
    }


    tokens.push_back(
    {
        TokenType::CONSTANT_STRING,
        value,
        startLine
    });
}


// ======================================================
// CHARACTER LITERAL
// ======================================================

void Lexer::readChar()
{
    int startLine = currentLine;

    string value = "'";

    position++;

    bool closed = false;


    while (position < source.size())
    {
        char current = source[position];


        if (current == '\\'
            && position + 1 < source.size())
        {
            value += current;
            value += source[position + 1];

            position += 2;

            continue;
        }


        if (current == '\'')
        {
            value += current;

            position++;

            closed = true;

            break;
        }


        if (current == '\n')
        {
            errors.push_back(
            {
                "Unclosed character literal",
                startLine
            });

            currentLine++;
            position++;

            return;
        }


        value += current;

        position++;
    }


    if (!closed)
    {
        errors.push_back(
        {
            "Unclosed character literal",
            startLine
        });

        return;
    }


    tokens.push_back(
    {
        TokenType::CONSTANT_CHAR,
        value,
        startLine
    });
}


// ======================================================
// NUMBER
// ======================================================

void Lexer::readNumber()
{
    int startLine = currentLine;

    size_t start = position;

    bool hasPoint = false;
    bool malformed = false;


    while (position < source.size())
    {
        char current = source[position];


        if (isdigit(static_cast<unsigned char>(current)))
        {
            position++;
            continue;
        }


        if (current == '.')
        {
            if (hasPoint)
            {
                malformed = true;
            }

            hasPoint = true;

            position++;

            continue;
        }


        // Invalid construction, for example 123abc
        if (isalpha(static_cast<unsigned char>(current))
            || current == '_')
        {
            position++;


            while (position < source.size()
                   &&
                   (isalnum(
                        static_cast<unsigned char>(
                            source[position]))
                    || source[position] == '_'))
            {
                position++;
            }


            string invalid =
                source.substr(
                    start,
                    position - start
                );


            errors.push_back(
            {
                "Invalid lexeme '" +
                invalid +
                "': invalid number format",
                startLine
            });


            return;
        }


        break;
    }


    string value =
        source.substr(
            start,
            position - start
        );


    if (malformed)
    {
        errors.push_back(
        {
            "Invalid number '" +
            value +
            "': multiple decimal points",
            startLine
        });

        return;
    }


    TokenType type;


    if (hasPoint)
    {
        type = TokenType::CONSTANT_FLOAT;
    }
    else
    {
        type = TokenType::CONSTANT_INT;
    }


    tokens.push_back(
    {
        type,
        value,
        startLine
    });
}


// ======================================================
// IDENTIFIER / KEYWORD
// ======================================================

void Lexer::readIdentifier()
{
    int startLine = currentLine;

    size_t start = position;


    while (position < source.size()
           &&
           (isalnum(
                static_cast<unsigned char>(
                    source[position]))
            || source[position] == '_'))
    {
        position++;
    }


    string value =
        source.substr(
            start,
            position - start
        );


    // Boolean constant
    if (BOOL_CONSTANTS.count(value))
    {
        tokens.push_back(
        {
            TokenType::CONSTANT_BOOL,
            value,
            startLine
        });

        return;
    }


    // Keyword
    if (KEYWORDS.count(value))
    {
        tokens.push_back(
        {
            TokenType::KEYWORD,
            value,
            startLine
        });

        return;
    }


    // Identifier
    if (VALID_IDENTIFIERS.count(value))
    {
        tokens.push_back(
        {
            TokenType::IDENTIFIER,
            value,
            startLine
        });

        return;
    }


    errors.push_back(
    {
        "Unknown identifier: " + value,
        startLine
    });
}


// ======================================================
// DOUBLE OPERATORS
// ======================================================

bool Lexer::tryReadDoubleOperator()
{
    if (position + 1 >= source.size())
    {
        return false;
    }


    string candidate =
        source.substr(position, 2);


    for (const string& op : DOUBLE_OPERATORS)
    {
        if (candidate == op)
        {
            tokens.push_back(
            {
                TokenType::OPERATOR,
                op,
                currentLine
            });

            position += 2;

            return true;
        }
    }


    return false;
}


// ======================================================
// TOKEN TABLE
// ======================================================

void printTokenTable(
    const vector<Token>& tokens)
{
    cout << "\nTOKEN TABLE\n";


    cout << left
         << setw(6) << "No."
         << setw(28) << "Lexeme"
         << "Type"
         << '\n';


    cout << string(60, '-')
         << '\n';


    for (size_t i = 0;
         i < tokens.size();
         i++)
    {
        string value = tokens[i].value;


        if (value.size() > 25)
        {
            value =
                value.substr(0, 22)
                + "...";
        }


        cout << left
             << setw(6) << i + 1
             << setw(28) << value
             << tokenTypeName(tokens[i].type)
             << '\n';
    }
}


// ======================================================
// TOKEN SEQUENCE
// ======================================================

void printTokenSequence(
    const vector<Token>& tokens)
{
    cout << "\nTOKEN SEQUENCE\n[";


    for (size_t i = 0;
         i < tokens.size();
         i++)
    {
        cout << "{"
             << tokenTypeName(tokens[i].type)
             << ", \""
             << tokens[i].value
             << "\"}";


        if (i + 1 < tokens.size())
        {
            cout << ", ";
        }
    }


    cout << "]\n";
}


// ======================================================
// ERROR OUTPUT
// ======================================================

void printLexicalErrors(
    const vector<LexError>& errors)
{
    if (errors.empty())
    {
        cout
            << "\nNo lexical errors found.\n";

        return;
    }


    cout << "\nLEXICAL ERRORS\n";


    for (const LexError& error : errors)
    {
        cout
            << "[Line "
            << error.line
            << "] "
            << error.message
            << '\n';
    }
}