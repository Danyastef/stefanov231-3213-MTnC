#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main()
{
    const string inputFileName = "cleaned.cpp";

    ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        cerr << "Error: cannot open cleaned.cpp." << endl;
        return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();

    cout << "File cleaned.cpp loaded successfully." << endl;

    Lexer lexer(buffer.str());
    lexer.tokenize();

    printTokenTable(lexer.getTokens());
    printTokenSequence(lexer.getTokens());
    printLexicalErrors(lexer.getErrors());

    cout << "\nTotal tokens: "
         << lexer.getTokens().size() << endl;

    return 0;
}