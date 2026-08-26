#pragma once
#include "parser.h"
#include <string>
#include <unordered_map>
#include <vector>

struct SymbolEntry
{
    std::string name;
    std::string type;
    std::string scope;
    int declarationLine;
    bool initialized;
    std::string initialValue;
};

struct Triad
{
    int index;
    std::string op;
    std::string arg1;
    std::string arg2;
};

struct SemanticError
{
    std::string message;
    int line;
};

class SemanticAnalyzer
{
public:
    explicit SemanticAnalyzer(const NodePtr& root);

    void analyze();

    const std::vector<SymbolEntry>& getSymbolTable() const;
    const std::vector<Triad>& getTriads() const;
    const std::vector<SemanticError>& getErrors() const;

    void printSymbolTable() const;
    void printTriads() const;
    void printErrors() const;

private:
    struct Scope
    {
        std::string name;
        std::unordered_map<std::string, std::string> variables;
    };

    const NodePtr& root;

    std::vector<SymbolEntry> symbolTable;
    std::vector<Triad> triads;
    std::vector<SemanticError> errors;
    std::vector<Scope> scopes;

    std::unordered_map<std::string, std::string> functionReturnTypes;
    std::unordered_map<std::string, std::vector<std::string>> functionParameters;

    std::string currentFunction;
    int triadCounter;

    void addError(const std::string& message, int line);

    void pushScope(const std::string& name);
    void popScope();
    std::string currentScopeName() const;

    bool declareVariable(const std::string& name,
                         const std::string& type,
                         int line,
                         bool initialized,
                         const std::string& initialValue);

    bool lookupVariable(const std::string& name,
                        std::string& type) const;

    int emitTriad(const std::string& op,
                  const std::string& arg1,
                  const std::string& arg2 = "-");

    std::string triadReference(int index) const;

    std::string inferType(const NodePtr& node);

    void visitProgram(const ProgramNode& program);
    void visitFunction(const FunctionDecl& function);
    void visitBlock(const BlockStmt& block, bool createScope = false);
    void visitStatement(const NodePtr& statement);

    void visitVariableDeclaration(const VarDeclStmt& declaration);
    void visitIf(const IfStmt& statement);
    void visitWhile(const WhileStmt& statement);
    void visitFor(const ForStmt& statement);
    void visitReturn(const ReturnStmt& statement);
    void visitExpressionStatement(const ExprStmt& statement);

    std::string visitExpression(const NodePtr& expression);
    std::string visitIdentifier(const IdentifierExpr& identifier);
    std::string visitLiteral(const LiteralExpr& literal);
    std::string visitBinary(const BinaryExpr& binary);
    std::string visitUnary(const UnaryExpr& unary);
    std::string visitCall(const CallExpr& call);
};
