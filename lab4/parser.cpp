#include "parser.h"
#include <iostream>
#include <unordered_set>

using namespace std;

static string branch(bool isLast) { return isLast ? "`-- " : "|-- "; }
static string nextPrefix(const string& p, bool isLast) { return p + (isLast ? "    " : "|   "); }

IdentifierExpr::IdentifierExpr(const string& n, int l) : ASTNode(NodeType::IDENTIFIER_EXPR, l), name(n) {}
void IdentifierExpr::print(const string& p, bool last) const { cout << p << branch(last) << "Identifier: " << name << '\n'; }

LiteralExpr::LiteralExpr(const string& v, int l) : ASTNode(NodeType::LITERAL_EXPR, l), value(v) {}
void LiteralExpr::print(const string& p, bool last) const { cout << p << branch(last) << "Literal: " << value << '\n'; }

BinaryExpr::BinaryExpr(const string& o, NodePtr lft, NodePtr rgt, int l)
    : ASTNode(NodeType::BINARY_EXPR, l), op(o), left(move(lft)), right(move(rgt)) {}
void BinaryExpr::print(const string& p, bool last) const
{
    cout << p << branch(last) << "BinaryExpr [" << op << "]\n";
    string np = nextPrefix(p, last);
    if (left) left->print(np, false);
    if (right) right->print(np, true);
}

UnaryExpr::UnaryExpr(const string& o, NodePtr n, int l)
    : ASTNode(NodeType::UNARY_EXPR, l), op(o), operand(move(n)) {}
void UnaryExpr::print(const string& p, bool last) const
{
    cout << p << branch(last) << "UnaryExpr [" << op << "]\n";
    if (operand) operand->print(nextPrefix(p, last), true);
}

CallExpr::CallExpr(const string& c, vector<NodePtr> a, int l)
    : ASTNode(NodeType::CALL_EXPR, l), callee(c), arguments(move(a)) {}
void CallExpr::print(const string& p, bool last) const
{
    cout << p << branch(last) << "CallExpr: " << callee << '\n';
    string np = nextPrefix(p, last);
    for (size_t i = 0; i < arguments.size(); ++i)
        arguments[i]->print(np, i + 1 == arguments.size());
}

BlockStmt::BlockStmt(int l) : ASTNode(NodeType::BLOCK_STMT, l) {}
void BlockStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "BlockStmt\n";
    string np = nextPrefix(p, last);
    for (size_t i = 0; i < statements.size(); ++i)
        statements[i]->print(np, i + 1 == statements.size());
}

VarDeclStmt::VarDeclStmt(const string& t, const string& n, NodePtr i, int l)
    : ASTNode(NodeType::VAR_DECL_STMT, l), varType(t), name(n), initExpr(move(i)) {}
void VarDeclStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "VarDecl: " << varType << " " << name << '\n';
    if (initExpr) initExpr->print(nextPrefix(p, last), true);
}

IfStmt::IfStmt(NodePtr c, NodePtr t, NodePtr e, int l)
    : ASTNode(NodeType::IF_STMT, l), condition(move(c)), thenBranch(move(t)), elseBranch(move(e)) {}
void IfStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "IfStmt\n";
    string np = nextPrefix(p, last);
    if (condition) condition->print(np, false);
    if (thenBranch) thenBranch->print(np, elseBranch == nullptr);
    if (elseBranch) elseBranch->print(np, true);
}

WhileStmt::WhileStmt(NodePtr c, NodePtr b, int l)
    : ASTNode(NodeType::WHILE_STMT, l), condition(move(c)), body(move(b)) {}
void WhileStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "WhileStmt\n";
    string np = nextPrefix(p, last);
    if (condition) condition->print(np, false);
    if (body) body->print(np, true);
}

ForStmt::ForStmt(NodePtr i, NodePtr c, NodePtr u, NodePtr b, int l)
    : ASTNode(NodeType::FOR_STMT, l), init(move(i)), condition(move(c)), update(move(u)), body(move(b)) {}
void ForStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "ForStmt\n";
    string np = nextPrefix(p, last);
    if (init) init->print(np, false);
    if (condition) condition->print(np, false);
    if (update) update->print(np, false);
    if (body) body->print(np, true);
}

ReturnStmt::ReturnStmt(NodePtr v, int l) : ASTNode(NodeType::RETURN_STMT, l), value(move(v)) {}
void ReturnStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "ReturnStmt\n";
    if (value) value->print(nextPrefix(p, last), true);
}

ExprStmt::ExprStmt(NodePtr e, int l) : ASTNode(NodeType::EXPR_STMT, l), expression(move(e)) {}
void ExprStmt::print(const string& p, bool last) const
{
    cout << p << branch(last) << "ExprStmt\n";
    if (expression) expression->print(nextPrefix(p, last), true);
}

ParamDecl::ParamDecl(const string& t, const string& n, int l)
    : ASTNode(NodeType::PARAM_DECL, l), paramType(t), name(n) {}
void ParamDecl::print(const string& p, bool last) const
{
    cout << p << branch(last) << "Param: " << paramType << " " << name << '\n';
}

FunctionDecl::FunctionDecl(const string& rt, const string& n, vector<NodePtr> ps, NodePtr b, int l)
    : ASTNode(NodeType::FUNCTION_DECL, l), returnType(rt), name(n), parameters(move(ps)), body(move(b)) {}
void FunctionDecl::print(const string& p, bool last) const
{
    cout << p << branch(last) << "FunctionDecl: " << returnType << " " << name << '\n';
    string np = nextPrefix(p, last);
    for (size_t i = 0; i < parameters.size(); ++i)
        parameters[i]->print(np, false);
    if (body) body->print(np, true);
}

ProgramNode::ProgramNode() : ASTNode(NodeType::PROGRAM, 0) {}
void ProgramNode::print(const string&, bool) const
{
    cout << "Program\n";
    for (size_t i = 0; i < functions.size(); ++i)
        functions[i]->print("", i + 1 == functions.size());
}

static const Token EOF_TOKEN{TokenType::DELIMITER, "$EOF$", -1};

Parser::Parser(const vector<Token>& t) : tokens(t), pos(0) {}
const vector<ParseError>& Parser::getErrors() const { return errors; }
bool Parser::atEnd() const { return pos >= tokens.size(); }
const Token& Parser::current() const { return atEnd() ? EOF_TOKEN : tokens[pos]; }

bool Parser::check(TokenType t, const string& v) const
{
    if (atEnd()) return false;
    if (current().type != t) return false;
    return v.empty() || current().value == v;
}

bool Parser::accept(TokenType t, const string& v)
{
    if (!check(t, v)) return false;
    ++pos;
    return true;
}

Token Parser::expect(TokenType t, const string& v, const string& d)
{
    if (check(t, v)) return tokens[pos++];
    string got = atEnd() ? "end of file" : "'" + current().value + "'";
    addError("Expected " + d + ", but got " + got + ".", current().line);
    return {t, v, current().line};
}

void Parser::addError(const string& m, int l) { errors.push_back({m, l}); }

bool Parser::isTypeToken() const
{
    if (!check(TokenType::KEYWORD)) return false;
    static const unordered_set<string> TYPES = {"int", "double", "float", "bool", "char", "void"};
    return TYPES.count(current().value) > 0;
}

NodePtr Parser::parse() { return parseProgram(); }

NodePtr Parser::parseProgram()
{
    auto program = make_shared<ProgramNode>();

    while (check(TokenType::PREPROCESSOR)) ++pos;

    if (check(TokenType::KEYWORD, "using"))
    {
        ++pos;
        expect(TokenType::KEYWORD, "namespace", "'namespace'");
        expect(TokenType::IDENTIFIER, "std", "'std'");
        expect(TokenType::DELIMITER, ";", "';'");
    }

    while (!atEnd())
    {
        if (!isTypeToken())
        {
            addError("Expected a function declaration at global scope.", current().line);
            ++pos;
            continue;
        }
        program->functions.push_back(parseFunction());
    }
    return program;
}

NodePtr Parser::parseFunction()
{
    int line = current().line;
    string returnType = current().value;
    ++pos;

    Token name = expect(TokenType::IDENTIFIER, "", "function name");
    expect(TokenType::DELIMITER, "(", "'('");

    vector<NodePtr> params;
    if (!check(TokenType::DELIMITER, ")"))
    {
        do { params.push_back(parseParameter()); }
        while (accept(TokenType::DELIMITER, ","));
    }

    expect(TokenType::DELIMITER, ")", "')'");
    NodePtr body = parseBlock();

    return make_shared<FunctionDecl>(returnType, name.value, move(params), move(body), line);
}

NodePtr Parser::parseParameter()
{
    int line = current().line;
    if (!isTypeToken())
    {
        addError("Expected parameter type.", current().line);
        return make_shared<ParamDecl>("?", "?", line);
    }

    string type = current().value;
    ++pos;
    accept(TokenType::OPERATOR, "&");
    Token name = expect(TokenType::IDENTIFIER, "", "parameter name");
    return make_shared<ParamDecl>(type, name.value, line);
}

NodePtr Parser::parseBlock()
{
    int line = current().line;
    expect(TokenType::DELIMITER, "{", "'{' to start a block");

    auto block = make_shared<BlockStmt>(line);
    while (!atEnd() && !check(TokenType::DELIMITER, "}"))
        block->statements.push_back(parseStatement());

    if (!accept(TokenType::DELIMITER, "}"))
        addError("Unclosed block: expected '}'.", current().line);

    return block;
}

NodePtr Parser::parseStatement()
{
    if (isTypeToken()) return parseVariableDeclaration();
    if (check(TokenType::KEYWORD, "if")) return parseIf();
    if (check(TokenType::KEYWORD, "while")) return parseWhile();
    if (check(TokenType::KEYWORD, "for")) return parseFor();
    if (check(TokenType::KEYWORD, "return")) return parseReturn();
    if (check(TokenType::DELIMITER, "{")) return parseBlock();
    return parseExpressionStatement();
}

NodePtr Parser::parseVariableDeclaration(bool requireSemicolon)
{
    int line = current().line;
    string type = current().value;
    ++pos;

    Token name = expect(TokenType::IDENTIFIER, "", "variable name");
    NodePtr init;
    if (accept(TokenType::OPERATOR, "=")) init = parseExpression();

    if (requireSemicolon)
        expect(TokenType::DELIMITER, ";", "';' after variable declaration");

    return make_shared<VarDeclStmt>(type, name.value, move(init), line);
}

NodePtr Parser::parseIf()
{
    int line = current().line;
    ++pos;
    expect(TokenType::DELIMITER, "(", "'(' after if");
    NodePtr condition = parseExpression();
    expect(TokenType::DELIMITER, ")", "')' after if condition");
    NodePtr thenBranch = parseStatement();
    NodePtr elseBranch;
    if (accept(TokenType::KEYWORD, "else")) elseBranch = parseStatement();
    return make_shared<IfStmt>(move(condition), move(thenBranch), move(elseBranch), line);
}

NodePtr Parser::parseWhile()
{
    int line = current().line;
    ++pos;
    expect(TokenType::DELIMITER, "(", "'(' after while");
    NodePtr condition = parseExpression();
    expect(TokenType::DELIMITER, ")", "')' after while condition");
    NodePtr body = parseStatement();
    return make_shared<WhileStmt>(move(condition), move(body), line);
}

NodePtr Parser::parseFor()
{
    int line = current().line;
    ++pos;
    expect(TokenType::DELIMITER, "(", "'(' after for");

    NodePtr init;
    if (isTypeToken()) init = parseVariableDeclaration(false);
    else if (!check(TokenType::DELIMITER, ";")) init = parseExpression();

    expect(TokenType::DELIMITER, ";", "';' after for initialization");

    NodePtr condition;
    if (!check(TokenType::DELIMITER, ";")) condition = parseExpression();
    expect(TokenType::DELIMITER, ";", "';' after for condition");

    NodePtr update;
    if (!check(TokenType::DELIMITER, ")")) update = parseExpression();
    expect(TokenType::DELIMITER, ")", "')' after for clauses");

    NodePtr body = parseStatement();
    return make_shared<ForStmt>(move(init), move(condition), move(update), move(body), line);
}

NodePtr Parser::parseReturn()
{
    int line = current().line;
    ++pos;
    NodePtr value;
    if (!check(TokenType::DELIMITER, ";")) value = parseExpression();
    expect(TokenType::DELIMITER, ";", "';' after return statement");
    return make_shared<ReturnStmt>(move(value), line);
}

NodePtr Parser::parseExpressionStatement()
{
    int line = current().line;
    NodePtr expression = parseExpression();
    expect(TokenType::DELIMITER, ";", "';' after expression");
    return make_shared<ExprStmt>(move(expression), line);
}

NodePtr Parser::parseExpression() { return parseAssignment(); }

NodePtr Parser::parseAssignment()
{
    NodePtr left = parseLogicalOr();
    static const unordered_set<string> OPS = {"=", "+=", "-=", "*=", "/=", "%="};

    if (!atEnd() && current().type == TokenType::OPERATOR && OPS.count(current().value))
    {
        string op = current().value;
        int line = current().line;
        ++pos;
        NodePtr right = parseAssignment();
        return make_shared<BinaryExpr>(op, move(left), move(right), line);
    }
    return left;
}

NodePtr Parser::parseLogicalOr()
{
    NodePtr left = parseLogicalAnd();
    while (accept(TokenType::OPERATOR, "||"))
        left = make_shared<BinaryExpr>("||", move(left), parseLogicalAnd(), current().line);
    return left;
}

NodePtr Parser::parseLogicalAnd()
{
    NodePtr left = parseEquality();
    while (accept(TokenType::OPERATOR, "&&"))
        left = make_shared<BinaryExpr>("&&", move(left), parseEquality(), current().line);
    return left;
}

NodePtr Parser::parseEquality()
{
    NodePtr left = parseComparison();
    while (!atEnd() && current().type == TokenType::OPERATOR &&
          (current().value == "==" || current().value == "!="))
    {
        string op = current().value; int line = current().line; ++pos;
        left = make_shared<BinaryExpr>(op, move(left), parseComparison(), line);
    }
    return left;
}

NodePtr Parser::parseComparison()
{
    NodePtr left = parseTerm();
    while (!atEnd() && current().type == TokenType::OPERATOR &&
          (current().value == "<" || current().value == ">" ||
           current().value == "<=" || current().value == ">="))
    {
        string op = current().value; int line = current().line; ++pos;
        left = make_shared<BinaryExpr>(op, move(left), parseTerm(), line);
    }
    return left;
}

NodePtr Parser::parseTerm()
{
    NodePtr left = parseFactor();
    while (!atEnd() && current().type == TokenType::OPERATOR &&
          (current().value == "+" || current().value == "-"))
    {
        string op = current().value; int line = current().line; ++pos;
        left = make_shared<BinaryExpr>(op, move(left), parseFactor(), line);
    }
    return left;
}

NodePtr Parser::parseFactor()
{
    NodePtr left = parseUnary();
    while (!atEnd() && current().type == TokenType::OPERATOR &&
          (current().value == "*" || current().value == "/" || current().value == "%"))
    {
        string op = current().value; int line = current().line; ++pos;
        left = make_shared<BinaryExpr>(op, move(left), parseUnary(), line);
    }
    return left;
}

NodePtr Parser::parseUnary()
{
    if (!atEnd() && current().type == TokenType::OPERATOR &&
        (current().value == "!" || current().value == "-" ||
         current().value == "++" || current().value == "--"))
    {
        string op = current().value; int line = current().line; ++pos;
        return make_shared<UnaryExpr>(op, parseUnary(), line);
    }

    NodePtr node = parsePrimary();

    if (!atEnd() && current().type == TokenType::OPERATOR &&
        (current().value == "++" || current().value == "--"))
    {
        string op = current().value + "(post)";
        int line = current().line;
        ++pos;
        node = make_shared<UnaryExpr>(op, move(node), line);
    }
    return node;
}

NodePtr Parser::parsePrimary()
{
    int line = current().line;

    if (accept(TokenType::DELIMITER, "("))
    {
        NodePtr expression = parseExpression();
        expect(TokenType::DELIMITER, ")", "')'");
        return expression;
    }

    if (check(TokenType::CONSTANT_INT) || check(TokenType::CONSTANT_FLOAT) ||
        check(TokenType::CONSTANT_STRING) || check(TokenType::CONSTANT_CHAR) ||
        check(TokenType::CONSTANT_BOOL))
    {
        string value = current().value;
        ++pos;
        return make_shared<LiteralExpr>(value, line);
    }

    if (check(TokenType::IDENTIFIER))
    {
        string name = current().value;
        ++pos;

        if (check(TokenType::DELIMITER, "("))
            return make_shared<CallExpr>(name, parseArguments(), line);

        return make_shared<IdentifierExpr>(name, line);
    }

    addError("Unexpected token '" + current().value + "' in expression.", line);
    if (!atEnd()) ++pos;
    return make_shared<LiteralExpr>("?", line);
}

vector<NodePtr> Parser::parseArguments()
{
    vector<NodePtr> args;
    expect(TokenType::DELIMITER, "(", "'('");

    if (!check(TokenType::DELIMITER, ")"))
    {
        do { args.push_back(parseExpression()); }
        while (accept(TokenType::DELIMITER, ","));
    }

    expect(TokenType::DELIMITER, ")", "')' after argument list");
    return args;
}
