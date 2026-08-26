#include "lexer.h"
#include "parser.h"
#include "semantic.h"

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

    cout << "=== LABORATORY WORK 4: SEMANTIC ANALYZER ===\n\n";
    cout << "Stage 1/3: lexical analysis...\n";

    Lexer lexer(buffer.str());
    lexer.tokenize();

    if (!lexer.getErrors().empty())
    {
        printLexicalErrors(lexer.getErrors());
        return 1;
    }

    cout << "Lexical analysis: OK\n";
    cout << "Stage 2/3: syntax analysis...\n";

    Parser parser(lexer.getTokens());
    NodePtr ast = parser.parse();

    if (!parser.getErrors().empty())
    {
        cout << "SYNTAX ERRORS\n";
        for (const auto& error : parser.getErrors())
        {
            cout << "[Line " << error.line << "] "
                 << error.message << '\n';
        }
        return 1;
    }

    cout << "Syntax analysis: OK\n";
    cout << "Stage 3/3: semantic analysis...\n";

    SemanticAnalyzer semantic(ast);
    semantic.analyze();

    semantic.printSymbolTable();
    semantic.printTriads();
    semantic.printErrors();

    return 0;
}
