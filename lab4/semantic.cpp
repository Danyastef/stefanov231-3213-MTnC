#include "semantic.h"
#include <iomanip>
#include <iostream>
#include <unordered_set>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer(const NodePtr& root)
    : root(root), triadCounter(0) {}

const vector<SymbolEntry>& SemanticAnalyzer::getSymbolTable() const { return symbolTable; }
const vector<Triad>& SemanticAnalyzer::getTriads() const { return triads; }
const vector<SemanticError>& SemanticAnalyzer::getErrors() const { return errors; }

void SemanticAnalyzer::addError(const string& message, int line)
{
    errors.push_back({message, line});
}

void SemanticAnalyzer::pushScope(const string& name)
{
    scopes.push_back({name, {}});
}

void SemanticAnalyzer::popScope()
{
    if (!scopes.empty()) scopes.pop_back();
}

string SemanticAnalyzer::currentScopeName() const
{
    return scopes.empty() ? "global" : scopes.back().name;
}

bool SemanticAnalyzer::declareVariable(const string& name,
                                       const string& type,
                                       int line,
                                       bool initialized,
                                       const string& initialValue)
{
    if (scopes.empty()) pushScope("global");

    auto& variables = scopes.back().variables;

    if (variables.count(name))
    {
        addError("Duplicate declaration of variable '" + name +
                 "' in scope '" + currentScopeName() + "'.", line);
        return false;
    }

    variables[name] = type;
    symbolTable.push_back({
        name, type, currentScopeName(), line, initialized, initialValue
    });

    return true;
}

bool SemanticAnalyzer::lookupVariable(const string& name, string& type) const
{
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i)
    {
        auto it = scopes[i].variables.find(name);
        if (it != scopes[i].variables.end())
        {
            type = it->second;
            return true;
        }
    }
    return false;
}

int SemanticAnalyzer::emitTriad(const string& op,
                                const string& arg1,
                                const string& arg2)
{
    int index = ++triadCounter;
    triads.push_back({index, op, arg1, arg2});
    return index;
}

string SemanticAnalyzer::triadReference(int index) const
{
    return "^" + to_string(index);
}

string SemanticAnalyzer::inferType(const NodePtr& node)
{
    if (!node) return "unknown";

    switch (node->type)
    {
        case NodeType::LITERAL_EXPR:
        {
            const auto& literal = static_cast<const LiteralExpr&>(*node);
            if (!literal.value.empty() && literal.value.front() == '"') return "string";
            if (!literal.value.empty() && literal.value.front() == '\'') return "char";
            if (literal.value == "true" || literal.value == "false") return "bool";
            if (literal.value.find('.') != string::npos) return "double";
            return "int";
        }

        case NodeType::IDENTIFIER_EXPR:
        {
            const auto& identifier = static_cast<const IdentifierExpr&>(*node);
            string type;
            if (lookupVariable(identifier.name, type)) return type;
            return "unknown";
        }

        case NodeType::BINARY_EXPR:
        {
            const auto& binary = static_cast<const BinaryExpr&>(*node);

            if (binary.op == "<" || binary.op == ">" ||
                binary.op == "<=" || binary.op == ">=" ||
                binary.op == "==" || binary.op == "!=" ||
                binary.op == "&&" || binary.op == "||")
                return "bool";

            if (binary.op == "=" || binary.op == "+=" ||
                binary.op == "-=" || binary.op == "*=" ||
                binary.op == "/=" || binary.op == "%=")
                return inferType(binary.left);

            string leftType = inferType(binary.left);
            string rightType = inferType(binary.right);

            if (leftType == "double" || rightType == "double") return "double";
            if (leftType == rightType) return leftType;
            return "unknown";
        }

        case NodeType::UNARY_EXPR:
            return inferType(static_cast<const UnaryExpr&>(*node).operand);

        case NodeType::CALL_EXPR:
        {
            const auto& call = static_cast<const CallExpr&>(*node);
            auto it = functionReturnTypes.find(call.callee);
            if (it != functionReturnTypes.end()) return it->second;
            return "unknown";
        }

        default:
            return "unknown";
    }
}

void SemanticAnalyzer::analyze()
{
    if (!root)
    {
        addError("AST is empty.", 0);
        return;
    }

    if (root->type != NodeType::PROGRAM)
    {
        addError("AST root is not Program.", root->line);
        return;
    }

    visitProgram(static_cast<const ProgramNode&>(*root));
}

void SemanticAnalyzer::visitProgram(const ProgramNode& program)
{
    // Pass 1: register all functions and parameter types.
    for (const auto& node : program.functions)
    {
        if (!node || node->type != NodeType::FUNCTION_DECL) continue;

        const auto& function = static_cast<const FunctionDecl&>(*node);

        if (functionReturnTypes.count(function.name))
        {
            addError("Duplicate function declaration: '" + function.name + "'.",
                     function.line);
            continue;
        }

        functionReturnTypes[function.name] = function.returnType;

        vector<string> parameterTypes;
        for (const auto& parameterNode : function.parameters)
        {
            const auto& parameter = static_cast<const ParamDecl&>(*parameterNode);
            parameterTypes.push_back(parameter.paramType);
        }
        functionParameters[function.name] = parameterTypes;
    }

    // Pass 2: analyze function bodies.
    for (const auto& node : program.functions)
    {
        if (node && node->type == NodeType::FUNCTION_DECL)
            visitFunction(static_cast<const FunctionDecl&>(*node));
    }
}

void SemanticAnalyzer::visitFunction(const FunctionDecl& function)
{
    currentFunction = function.name;
    pushScope(function.name);

    for (const auto& parameterNode : function.parameters)
    {
        const auto& parameter = static_cast<const ParamDecl&>(*parameterNode);

        declareVariable(
            parameter.name,
            parameter.paramType,
            parameter.line,
            true,
            "argument"
        );
    }

    if (function.body)
        visitBlock(static_cast<const BlockStmt&>(*function.body), false);

    popScope();
    currentFunction.clear();
}

void SemanticAnalyzer::visitBlock(const BlockStmt& block, bool createScope)
{
    if (createScope)
        pushScope(currentFunction + "_block");

    for (const auto& statement : block.statements)
        visitStatement(statement);

    if (createScope)
        popScope();
}

void SemanticAnalyzer::visitStatement(const NodePtr& statement)
{
    if (!statement) return;

    switch (statement->type)
    {
        case NodeType::VAR_DECL_STMT:
            visitVariableDeclaration(static_cast<const VarDeclStmt&>(*statement));
            break;

        case NodeType::IF_STMT:
            visitIf(static_cast<const IfStmt&>(*statement));
            break;

        case NodeType::WHILE_STMT:
            visitWhile(static_cast<const WhileStmt&>(*statement));
            break;

        case NodeType::FOR_STMT:
            visitFor(static_cast<const ForStmt&>(*statement));
            break;

        case NodeType::RETURN_STMT:
            visitReturn(static_cast<const ReturnStmt&>(*statement));
            break;

        case NodeType::EXPR_STMT:
            visitExpressionStatement(static_cast<const ExprStmt&>(*statement));
            break;

        case NodeType::BLOCK_STMT:
            visitBlock(static_cast<const BlockStmt&>(*statement), true);
            break;

        default:
            break;
    }
}

void SemanticAnalyzer::visitVariableDeclaration(const VarDeclStmt& declaration)
{
    bool initialized = declaration.initExpr != nullptr;
    string initialValue = "-";
    string expressionRef = "-";

    // Declare first so later expressions in the same scope can resolve it
    // after this statement. Initialization expression itself is evaluated
    // before final assignment triad is emitted.
    if (initialized)
    {
        string initType = inferType(declaration.initExpr);

        if (initType != "unknown" && initType != declaration.varType)
        {
            bool compatible =
                declaration.varType == "double" && initType == "int";

            if (!compatible)
            {
                addError(
                    "Type mismatch while initializing '" + declaration.name +
                    "': variable type is " + declaration.varType +
                    ", expression type is " + initType + ".",
                    declaration.line
                );
            }
        }

        expressionRef = visitExpression(declaration.initExpr);

        if (declaration.initExpr->type == NodeType::LITERAL_EXPR)
            initialValue =
                static_cast<const LiteralExpr&>(*declaration.initExpr).value;
        else
            initialValue = expressionRef;
    }

    bool declared = declareVariable(
        declaration.name,
        declaration.varType,
        declaration.line,
        initialized,
        initialValue
    );

    if (declared && initialized)
        emitTriad("=", declaration.name, expressionRef);
}

void SemanticAnalyzer::visitIf(const IfStmt& statement)
{
    string conditionType = inferType(statement.condition);

    if (conditionType != "bool" && conditionType != "unknown")
    {
        addError("Condition of if statement must have type bool.",
                 statement.line);
    }

    string conditionRef = visitExpression(statement.condition);
    emitTriad("if", conditionRef);

    pushScope(currentFunction + "_if");
    visitStatement(statement.thenBranch);
    popScope();

    if (statement.elseBranch)
    {
        emitTriad("else", "-");
        pushScope(currentFunction + "_else");
        visitStatement(statement.elseBranch);
        popScope();
    }

    emitTriad("end_if", "-");
}

void SemanticAnalyzer::visitWhile(const WhileStmt& statement)
{
    string conditionType = inferType(statement.condition);

    if (conditionType != "bool" && conditionType != "unknown")
    {
        addError("Condition of while statement must have type bool.",
                 statement.line);
    }

    string conditionRef = visitExpression(statement.condition);
    emitTriad("while", conditionRef);

    pushScope(currentFunction + "_while");
    visitStatement(statement.body);
    popScope();

    emitTriad("end_while", "-");
}

void SemanticAnalyzer::visitFor(const ForStmt& statement)
{
    pushScope(currentFunction + "_for");

    if (statement.init)
    {
        if (statement.init->type == NodeType::VAR_DECL_STMT)
            visitVariableDeclaration(static_cast<const VarDeclStmt&>(*statement.init));
        else
            visitExpression(statement.init);
    }

    string conditionRef = "-";

    if (statement.condition)
    {
        string conditionType = inferType(statement.condition);

        if (conditionType != "bool" && conditionType != "unknown")
        {
            addError("Condition of for statement must have type bool.",
                     statement.line);
        }

        conditionRef = visitExpression(statement.condition);
    }

    emitTriad("for", conditionRef);

    if (statement.body)
        visitStatement(statement.body);

    if (statement.update)
        visitExpression(statement.update);

    emitTriad("end_for", "-");
    popScope();
}

void SemanticAnalyzer::visitReturn(const ReturnStmt& statement)
{
    string expectedType = "unknown";
    auto it = functionReturnTypes.find(currentFunction);
    if (it != functionReturnTypes.end())
        expectedType = it->second;

    if (!statement.value)
    {
        if (expectedType != "void")
        {
            addError("Return statement in function '" + currentFunction +
                     "' must return " + expectedType + ".",
                     statement.line);
        }

        emitTriad("return", "-");
        return;
    }

    string actualType = inferType(statement.value);

    if (expectedType != "unknown" &&
        actualType != "unknown" &&
        expectedType != actualType)
    {
        bool compatible = expectedType == "double" && actualType == "int";

        if (!compatible)
        {
            addError("Return type mismatch in function '" + currentFunction +
                     "': expected " + expectedType +
                     ", got " + actualType + ".",
                     statement.line);
        }
    }

    string valueRef = visitExpression(statement.value);
    emitTriad("return", valueRef);
}

void SemanticAnalyzer::visitExpressionStatement(const ExprStmt& statement)
{
    if (statement.expression)
        visitExpression(statement.expression);
}

string SemanticAnalyzer::visitExpression(const NodePtr& expression)
{
    if (!expression) return "-";

    switch (expression->type)
    {
        case NodeType::IDENTIFIER_EXPR:
            return visitIdentifier(static_cast<const IdentifierExpr&>(*expression));

        case NodeType::LITERAL_EXPR:
            return visitLiteral(static_cast<const LiteralExpr&>(*expression));

        case NodeType::BINARY_EXPR:
            return visitBinary(static_cast<const BinaryExpr&>(*expression));

        case NodeType::UNARY_EXPR:
            return visitUnary(static_cast<const UnaryExpr&>(*expression));

        case NodeType::CALL_EXPR:
            return visitCall(static_cast<const CallExpr&>(*expression));

        default:
            return "-";
    }
}

string SemanticAnalyzer::visitIdentifier(const IdentifierExpr& identifier)
{
    string type;

    if (!lookupVariable(identifier.name, type))
    {
        addError("Use of undeclared variable '" + identifier.name + "'.",
                 identifier.line);
    }

    return identifier.name;
}

string SemanticAnalyzer::visitLiteral(const LiteralExpr& literal)
{
    return literal.value;
}

string SemanticAnalyzer::visitBinary(const BinaryExpr& binary)
{
    static const unordered_set<string> ASSIGNMENT_OPS =
    {
        "=", "+=", "-=", "*=", "/=", "%="
    };

    if (ASSIGNMENT_OPS.count(binary.op))
    {
        if (!binary.left || binary.left->type != NodeType::IDENTIFIER_EXPR)
        {
            addError("Left side of assignment must be a variable.",
                     binary.line);
            return "-";
        }

        const auto& left =
            static_cast<const IdentifierExpr&>(*binary.left);

        string leftType;
        if (!lookupVariable(left.name, leftType))
        {
            addError("Use of undeclared variable '" + left.name + "'.",
                     binary.line);
        }

        string rightType = inferType(binary.right);

        if (!leftType.empty() &&
            rightType != "unknown" &&
            leftType != rightType)
        {
            bool compatible = leftType == "double" && rightType == "int";

            if (!compatible)
            {
                addError(
                    "Type mismatch in assignment to '" + left.name +
                    "': left side is " + leftType +
                    ", right side is " + rightType + ".",
                    binary.line
                );
            }
        }

        string rightRef = visitExpression(binary.right);

        if (binary.op == "=")
        {
            int index = emitTriad("=", left.name, rightRef);
            return triadReference(index);
        }

        string arithmeticOp = binary.op.substr(0, 1);
        int calculation = emitTriad(arithmeticOp, left.name, rightRef);
        int assignment =
            emitTriad("=", left.name, triadReference(calculation));

        return triadReference(assignment);
    }

    string leftRef = visitExpression(binary.left);
    string rightRef = visitExpression(binary.right);

    string leftType = inferType(binary.left);
    string rightType = inferType(binary.right);

    if ((binary.op == "+" || binary.op == "-" ||
         binary.op == "*" || binary.op == "/") &&
        leftType != "unknown" &&
        rightType != "unknown")
    {
        bool numericLeft = leftType == "int" || leftType == "double";
        bool numericRight = rightType == "int" || rightType == "double";

        if (!numericLeft || !numericRight)
        {
            addError("Operator '" + binary.op +
                     "' requires numeric operands.",
                     binary.line);
        }
    }

    int index = emitTriad(binary.op, leftRef, rightRef);
    return triadReference(index);
}

string SemanticAnalyzer::visitUnary(const UnaryExpr& unary)
{
    string operandType = inferType(unary.operand);
    string operandRef = visitExpression(unary.operand);

    if ((unary.op == "++(post)" || unary.op == "--(post)") &&
        operandType != "int" &&
        operandType != "unknown")
    {
        addError("Postfix increment/decrement requires int operand.",
                 unary.line);
    }

    int index = emitTriad(unary.op, operandRef);
    return triadReference(index);
}

string SemanticAnalyzer::visitCall(const CallExpr& call)
{
    auto functionIt = functionReturnTypes.find(call.callee);

    if (functionIt == functionReturnTypes.end())
    {
        addError("Call to undeclared function '" + call.callee + "'.",
                 call.line);
    }

    vector<string> argumentRefs;
    vector<string> actualTypes;

    for (const auto& argument : call.arguments)
    {
        actualTypes.push_back(inferType(argument));
        argumentRefs.push_back(visitExpression(argument));
    }

    auto parameterIt = functionParameters.find(call.callee);

    if (parameterIt != functionParameters.end())
    {
        const auto& expected = parameterIt->second;

        if (expected.size() != actualTypes.size())
        {
            addError("Function '" + call.callee +
                     "' expects " + to_string(expected.size()) +
                     " argument(s), but got " +
                     to_string(actualTypes.size()) + ".",
                     call.line);
        }
        else
        {
            for (size_t i = 0; i < expected.size(); ++i)
            {
                if (actualTypes[i] == "unknown") continue;

                bool compatible =
                    expected[i] == actualTypes[i] ||
                    (expected[i] == "double" && actualTypes[i] == "int");

                if (!compatible)
                {
                    addError(
                        "Argument " + to_string(i + 1) +
                        " of function '" + call.callee +
                        "' must have type " + expected[i] +
                        ", got " + actualTypes[i] + ".",
                        call.line
                    );
                }
            }
        }
    }

    string joined = "-";
    if (!argumentRefs.empty())
    {
        joined = argumentRefs[0];
        for (size_t i = 1; i < argumentRefs.size(); ++i)
            joined += ", " + argumentRefs[i];
    }

    int index = emitTriad("call", call.callee, joined);
    return triadReference(index);
}

void SemanticAnalyzer::printSymbolTable() const
{
    cout << "\n=== SYMBOL TABLE ===\n";
    cout << left
         << setw(16) << "Name"
         << setw(12) << "Type"
         << setw(22) << "Scope"
         << setw(10) << "Line"
         << setw(14) << "Initialized"
         << "Initial value\n";

    cout << string(86, '-') << '\n';

    for (const auto& entry : symbolTable)
    {
        cout << left
             << setw(16) << entry.name
             << setw(12) << entry.type
             << setw(22) << entry.scope
             << setw(10) << entry.declarationLine
             << setw(14) << (entry.initialized ? "yes" : "no")
             << entry.initialValue
             << '\n';
    }
}

void SemanticAnalyzer::printTriads() const
{
    cout << "\n=== TRIAD SEQUENCE ===\n";

    for (const auto& triad : triads)
    {
        cout << setw(3) << triad.index << ") ("
             << triad.op << ", "
             << triad.arg1;

        if (triad.arg2 != "-")
            cout << ", " << triad.arg2;

        cout << ")\n";
    }
}

void SemanticAnalyzer::printErrors() const
{
    if (errors.empty())
    {
        cout << "\nSemantic analysis completed successfully.\n";
        cout << "No semantic errors found.\n";
        return;
    }

    cout << "\n=== SEMANTIC ERRORS ===\n";

    for (const auto& error : errors)
    {
        cout << "[Line " << error.line << "] "
             << error.message << '\n';
    }

    cout << "Total semantic errors: " << errors.size() << '\n';
}
