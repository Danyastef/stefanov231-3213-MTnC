#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>

using namespace std;

// Проверка корректности многострочных комментариев
bool checkMultilineComments(const string& source)
{
    int openedComments = 0;

    for (size_t i = 0; i < source.length(); i++)
    {
        if (i + 1 < source.length() &&
            source[i] == '/' &&
            source[i + 1] == '*')
        {
            openedComments++;
            i++;
        }
        else if (i + 1 < source.length() &&
                 source[i] == '*' &&
                 source[i + 1] == '/')
        {
            openedComments--;
            i++;

            if (openedComments < 0)
            {
                cerr << "Ошибка: найдено закрытие комментария без его открытия." << endl;
                return false;
            }
        }
    }

    if (openedComments != 0)
    {
        cerr << "Ошибка: многострочный комментарий не закрыт." << endl;
        return false;
    }

    return true;
}

// Удаление комментариев вида /* ... */
string deleteMultilineComments(const string& source)
{
    regex multilinePattern(R"(/\*[\s\S]*?\*/)");

    return regex_replace(source, multilinePattern, "");
}

// Удаление комментариев вида //
string deleteSingleLineComments(const string& source)
{
    regex singleLinePattern(R"(//[^\n]*)");

    return regex_replace(source, singleLinePattern, "");
}

// Удаление пробелов и табуляций в начале и конце строки
string trimLines(const string& source)
{
    stringstream input(source);

    string line;
    string result;

    while (getline(input, line))
    {
        line = regex_replace(line, regex(R"(^[ \t]+)"), "");
        line = regex_replace(line, regex(R"([ \t]+$)"), "");

        result += line;
        result += '\n';
    }

    return result;
}

// Замена нескольких пробельных символов одним пробелом
string normalizeSpaces(const string& source)
{
    stringstream input(source);

    string line;
    string result;

    while (getline(input, line))
    {
        line = regex_replace(line, regex(R"([ \t]+)"), " ");

        result += line;
        result += '\n';
    }

    return result;
}

// Удаление пустых строк
string deleteEmptyLines(const string& source)
{
    stringstream input(source);

    string line;
    string result;

    regex emptyLine(R"(^\s*$)");

    while (getline(input, line))
    {
        if (!regex_match(line, emptyLine))
        {
            result += line;
            result += '\n';
        }
    }

    return result;
}

// Выполнение полной предварительной обработки
string preprocessSource(const string& source)
{
    if (!checkMultilineComments(source))
    {
        return "";
    }

    string result = source;

    result = deleteMultilineComments(result);
    result = deleteSingleLineComments(result);
    result = trimLines(result);
    result = normalizeSpaces(result);
    result = deleteEmptyLines(result);

    return result;
}

int main()
{
    setlocale(LC_ALL, "ru");

    const string inputFileName = "test.cpp";
    const string outputFileName = "cleaned.cpp";

    ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        cerr << "Ошибка: файл test.cpp не найден." << endl;
        return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();

    string sourceCode = buffer.str();

    inputFile.close();

    cout << "Исходный файл успешно загружен." << endl;

    string cleanedCode = preprocessSource(sourceCode);

    if (cleanedCode.empty())
    {
        cerr << "Предварительная обработка остановлена." << endl;
        return 1;
    }

    ofstream outputFile(outputFileName);

    if (!outputFile.is_open())
    {
        cerr << "Ошибка: невозможно создать cleaned.cpp." << endl;
        return 1;
    }

    outputFile << cleanedCode;
    outputFile.close();

    cout << "Обработка завершена успешно." << endl;
    cout << "Очищенный код сохранён в cleaned.cpp." << endl;

    return 0;
}