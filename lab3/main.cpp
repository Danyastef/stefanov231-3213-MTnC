#include "lexer.h"
#include "parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

int main()
{
    const string inputFileName = "cleaned.cpp";

    ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        cerr << "ERROR: cannot open cleaned.cpp.\n";
        return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();

    Lexer lexer(buffer.str());
    lexer.tokenize();

    cout << "=== LABORATORY WORK 3: SYNTAX ANALYZER ===\n\n";

    if (!lexer.getErrors().empty())
    {
        printLexicalErrors(lexer.getErrors());
        cout << "\nSyntax analysis was not started because lexical errors exist.\n";
        return 1;
    }

    cout << "Input file: cleaned.cpp\n";
    cout << "Tokens received from lexical analyzer: "
         << lexer.getTokens().size() << "\n\n";

    Parser parser(lexer.getTokens());
    NodePtr tree = parser.parse();

    cout << "=== ABSTRACT SYNTAX TREE ===\n";
    tree->print();

    cout << "\n=== SYNTAX ANALYSIS RESULT ===\n";

    if (parser.getErrors().empty())
    {
        cout << "Syntax analysis completed successfully.\n";
        cout << "No syntax errors found.\n";
    }
    else
    {
        cout << "Syntax errors found: "
             << parser.getErrors().size() << "\n";

        for (const auto& error : parser.getErrors())
        {
            cout << "[Line " << error.line << "] "
                 << error.message << '\n';
        }
    }

    return 0;
}
