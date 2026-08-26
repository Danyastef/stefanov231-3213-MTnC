#include "parser.h"
#include <iostream>
#include <unordered_set>

using namespace std;

static string branch(bool isLast)
{
    return isLast ? "`-- " : "|-- ";
}

static string nextPrefix(const string& prefix, bool isLast)
{
    return prefix + (isLast ? "    " : "|   ");
}

IdentifierExpr::IdentifierExpr(const string& name, int line)
    : ASTNode(NodeType::IDENTIFIER_EXPR, line), name(name) {}

void IdentifierExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "Identifier: " << name << '\n';
}

LiteralExpr::LiteralExpr(const string& value, int line)
    : ASTNode(NodeType::LITERAL_EXPR, line), value(value) {}

void LiteralExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "Literal: " << value << '\n';
}

BinaryExpr::BinaryExpr(const string& op, NodePtr left, NodePtr right, int line)
    : ASTNode(NodeType::BINARY_EXPR, line),
      op(op), left(move(left)), right(move(right)) {}

void BinaryExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "BinaryExpr [" << op << "]\n";
    string p = nextPrefix(prefix, isLast);
    if (left) left->print(p, false);
    if (right) right->print(p, true);
}

UnaryExpr::UnaryExpr(const string& op, NodePtr operand, int line)
    : ASTNode(NodeType::UNARY_EXPR, line),
      op(op), operand(move(operand)) {}

void UnaryExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "UnaryExpr [" << op << "]\n";
    if (operand) operand->print(nextPrefix(prefix, isLast), true);
}

CallExpr::CallExpr(const string& callee,
                   vector<NodePtr> arguments,
                   int line)
    : ASTNode(NodeType::CALL_EXPR, line),
      callee(callee), arguments(move(arguments)) {}

void CallExpr::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "CallExpr: " << callee << '\n';
    string p = nextPrefix(prefix, isLast);

    for (size_t i = 0; i < arguments.size(); ++i)
        arguments[i]->print(p, i + 1 == arguments.size());
}

BlockStmt::BlockStmt(int line)
    : ASTNode(NodeType::BLOCK_STMT, line) {}

void BlockStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "BlockStmt\n";
    string p = nextPrefix(prefix, isLast);

    for (size_t i = 0; i < statements.size(); ++i)
        statements[i]->print(p, i + 1 == statements.size());
}

VarDeclStmt::VarDeclStmt(const string& varType,
                         const string& name,
                         NodePtr initExpr,
                         int line)
    : ASTNode(NodeType::VAR_DECL_STMT, line),
      varType(varType), name(name), initExpr(move(initExpr)) {}

void VarDeclStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast)
         << "VarDecl: " << varType << " " << name << '\n';

    if (initExpr)
        initExpr->print(nextPrefix(prefix, isLast), true);
}

IfStmt::IfStmt(NodePtr condition,
               NodePtr thenBranch,
               NodePtr elseBranch,
               int line)
    : ASTNode(NodeType::IF_STMT, line),
      condition(move(condition)),
      thenBranch(move(thenBranch)),
      elseBranch(move(elseBranch)) {}

void IfStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "IfStmt\n";
    string p = nextPrefix(prefix, isLast);

    if (condition) condition->print(p, false);
    if (thenBranch) thenBranch->print(p, elseBranch == nullptr);
    if (elseBranch) elseBranch->print(p, true);
}

WhileStmt::WhileStmt(NodePtr condition, NodePtr body, int line)
    : ASTNode(NodeType::WHILE_STMT, line),
      condition(move(condition)), body(move(body)) {}

void WhileStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "WhileStmt\n";
    string p = nextPrefix(prefix, isLast);

    if (condition) condition->print(p, false);
    if (body) body->print(p, true);
}

ForStmt::ForStmt(NodePtr init,
                 NodePtr condition,
                 NodePtr update,
                 NodePtr body,
                 int line)
    : ASTNode(NodeType::FOR_STMT, line),
      init(move(init)),
      condition(move(condition)),
      update(move(update)),
      body(move(body)) {}

void ForStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "ForStmt\n";
    string p = nextPrefix(prefix, isLast);

    if (init) init->print(p, false);
    if (condition) condition->print(p, false);
    if (update) update->print(p, false);
    if (body) body->print(p, true);
}

ReturnStmt::ReturnStmt(NodePtr value, int line)
    : ASTNode(NodeType::RETURN_STMT, line), value(move(value)) {}

void ReturnStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "ReturnStmt\n";
    if (value) value->print(nextPrefix(prefix, isLast), true);
}

ExprStmt::ExprStmt(NodePtr expression, int line)
    : ASTNode(NodeType::EXPR_STMT, line), expression(move(expression)) {}

void ExprStmt::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast) << "ExprStmt\n";
    if (expression) expression->print(nextPrefix(prefix, isLast), true);
}

ParamDecl::ParamDecl(const string& paramType,
                     const string& name,
                     int line)
    : ASTNode(NodeType::PARAM_DECL, line),
      paramType(paramType), name(name) {}

void ParamDecl::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast)
         << "Param: " << paramType << " " << name << '\n';
}

FunctionDecl::FunctionDecl(const string& returnType,
                           const string& name,
                           vector<NodePtr> parameters,
                           NodePtr body,
                           int line)
    : ASTNode(NodeType::FUNCTION_DECL, line),
      returnType(returnType),
      name(name),
      parameters(move(parameters)),
      body(move(body)) {}

void FunctionDecl::print(const string& prefix, bool isLast) const
{
    cout << prefix << branch(isLast)
         << "FunctionDecl: " << returnType << " " << name << '\n';

    string p = nextPrefix(prefix, isLast);

    for (size_t i = 0; i < parameters.size(); ++i)
        parameters[i]->print(p, false);

    if (body) body->print(p, true);
}

ProgramNode::ProgramNode()
    : ASTNode(NodeType::PROGRAM, 0) {}

void ProgramNode::print(const string&, bool) const
{
    cout << "Program\n";

    for (size_t i = 0; i < functions.size(); ++i)
        functions[i]->print("", i + 1 == functions.size());
}

static const Token EOF_TOKEN =
{
    TokenType::DELIMITER,
    "$EOF$",
    -1
};

Parser::Parser(const vector<Token>& tokens)
    : tokens(tokens), pos(0) {}

const vector<ParseError>& Parser::getErrors() const
{
    return errors;
}

bool Parser::atEnd() const
{
    return pos >= tokens.size();
}

const Token& Parser::current() const
{
    return atEnd() ? EOF_TOKEN : tokens[pos];
}

const Token& Parser::peek(size_t offset) const
{
    size_t index = pos + offset;
    return index < tokens.size() ? tokens[index] : EOF_TOKEN;
}

bool Parser::check(TokenType type, const string& value) const
{
    if (atEnd()) return false;
    if (current().type != type) return false;
    return value.empty() || current().value == value;
}

bool Parser::accept(TokenType type, const string& value)
{
    if (!check(type, value)) return false;
    ++pos;
    return true;
}

Token Parser::expect(TokenType type,
                     const string& value,
                     const string& description)
{
    if (check(type, value))
        return tokens[pos++];

    string got = atEnd()
        ? "end of file"
        : "'" + current().value + "'";

    addError("Expected " + description + ", but got " + got + ".",
             current().line);

    return {type, value, current().line};
}

void Parser::addError(const string& message, int line)
{
    errors.push_back({message, line});
}

void Parser::synchronize()
{
    while (!atEnd())
    {
        if (current().value == ";" ||
            current().value == "}" ||
            current().value == "{")
            return;

        ++pos;
    }
}

bool Parser::isTypeToken() const
{
    if (!check(TokenType::KEYWORD)) return false;

    static const unordered_set<string> TYPES =
    {
        "int", "double", "float", "bool", "char", "void"
    };

    return TYPES.count(current().value) > 0;
}

NodePtr Parser::parse()
{
    return parseProgram();
}

NodePtr Parser::parseProgram()
{
    auto program = make_shared<ProgramNode>();

    while (check(TokenType::PREPROCESSOR))
        ++pos;

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
            addError("Expected a function declaration at global scope.",
                     current().line);
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
        do
        {
            params.push_back(parseParameter());
        }
        while (accept(TokenType::DELIMITER, ","));
    }

    expect(TokenType::DELIMITER, ")", "')'");

    NodePtr body = parseBlock();

    return make_shared<FunctionDecl>(
        returnType,
        name.value,
        move(params),
        move(body),
        line
    );
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

    // Optional '&' for reference parameter.
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
    if (isTypeToken())
        return parseVariableDeclaration();

    if (check(TokenType::KEYWORD, "if"))
        return parseIf();

    if (check(TokenType::KEYWORD, "while"))
        return parseWhile();

    if (check(TokenType::KEYWORD, "for"))
        return parseFor();

    if (check(TokenType::KEYWORD, "return"))
        return parseReturn();

    if (check(TokenType::DELIMITER, "{"))
        return parseBlock();

    return parseExpressionStatement();
}

NodePtr Parser::parseVariableDeclaration(bool requireSemicolon)
{
    int line = current().line;
    string type = current().value;
    ++pos;

    Token name = expect(TokenType::IDENTIFIER, "", "variable name");

    NodePtr init;

    if (accept(TokenType::OPERATOR, "="))
        init = parseExpression();

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

    if (accept(TokenType::KEYWORD, "else"))
        elseBranch = parseStatement();

    return make_shared<IfStmt>(
        move(condition),
        move(thenBranch),
        move(elseBranch),
        line
    );
}

NodePtr Parser::parseWhile()
{
    int line = current().line;
    ++pos;

    expect(TokenType::DELIMITER, "(", "'(' after while");
    NodePtr condition = parseExpression();
    expect(TokenType::DELIMITER, ")", "')' after while condition");

    NodePtr body = parseStatement();

    return make_shared<WhileStmt>(
        move(condition),
        move(body),
        line
    );
}

NodePtr Parser::parseFor()
{
    int line = current().line;
    ++pos;

    expect(TokenType::DELIMITER, "(", "'(' after for");

    NodePtr init;

    if (isTypeToken())
        init = parseVariableDeclaration(false);
    else if (!check(TokenType::DELIMITER, ";"))
        init = parseExpression();

    expect(TokenType::DELIMITER, ";", "';' after for initialization");

    NodePtr condition;
    if (!check(TokenType::DELIMITER, ";"))
        condition = parseExpression();

    expect(TokenType::DELIMITER, ";", "';' after for condition");

    NodePtr update;
    if (!check(TokenType::DELIMITER, ")"))
        update = parseExpression();

    expect(TokenType::DELIMITER, ")", "')' after for clauses");

    NodePtr body = parseStatement();

    return make_shared<ForStmt>(
        move(init),
        move(condition),
        move(update),
        move(body),
        line
    );
}

NodePtr Parser::parseReturn()
{
    int line = current().line;
    ++pos;

    NodePtr value;

    if (!check(TokenType::DELIMITER, ";"))
        value = parseExpression();

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

NodePtr Parser::parseExpression()
{
    return parseAssignment();
}

NodePtr Parser::parseAssignment()
{
    NodePtr left = parseLogicalOr();

    static const unordered_set<string> OPS =
    {
        "=", "+=", "-=", "*=", "/=", "%="
    };

    if (!atEnd() &&
        current().type == TokenType::OPERATOR &&
        OPS.count(current().value))
    {
        int line = current().line;
        string op = current().value;
        ++pos;

        NodePtr right = parseAssignment();

        return make_shared<BinaryExpr>(
            op,
            move(left),
            move(right),
            line
        );
    }

    return left;
}

NodePtr Parser::parseLogicalOr()
{
    NodePtr left = parseLogicalAnd();

    while (accept(TokenType::OPERATOR, "||"))
    {
        int line = current().line;
        NodePtr right = parseLogicalAnd();
        left = make_shared<BinaryExpr>("||", move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseLogicalAnd()
{
    NodePtr left = parseEquality();

    while (accept(TokenType::OPERATOR, "&&"))
    {
        int line = current().line;
        NodePtr right = parseEquality();
        left = make_shared<BinaryExpr>("&&", move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseEquality()
{
    NodePtr left = parseComparison();

    while (!atEnd() &&
          current().type == TokenType::OPERATOR &&
          (current().value == "==" || current().value == "!="))
    {
        string op = current().value;
        int line = current().line;
        ++pos;

        NodePtr right = parseComparison();
        left = make_shared<BinaryExpr>(op, move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseComparison()
{
    NodePtr left = parseTerm();

    while (!atEnd() &&
          current().type == TokenType::OPERATOR &&
          (current().value == "<" ||
           current().value == ">" ||
           current().value == "<=" ||
           current().value == ">="))
    {
        string op = current().value;
        int line = current().line;
        ++pos;

        NodePtr right = parseTerm();
        left = make_shared<BinaryExpr>(op, move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseTerm()
{
    NodePtr left = parseFactor();

    while (!atEnd() &&
          current().type == TokenType::OPERATOR &&
          (current().value == "+" || current().value == "-"))
    {
        string op = current().value;
        int line = current().line;
        ++pos;

        NodePtr right = parseFactor();
        left = make_shared<BinaryExpr>(op, move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseFactor()
{
    NodePtr left = parseUnary();

    while (!atEnd() &&
          current().type == TokenType::OPERATOR &&
          (current().value == "*" ||
           current().value == "/" ||
           current().value == "%"))
    {
        string op = current().value;
        int line = current().line;
        ++pos;

        NodePtr right = parseUnary();
        left = make_shared<BinaryExpr>(op, move(left), move(right), line);
    }

    return left;
}

NodePtr Parser::parseUnary()
{
    if (!atEnd() &&
        current().type == TokenType::OPERATOR &&
        (current().value == "!" ||
         current().value == "-" ||
         current().value == "++" ||
         current().value == "--"))
    {
        string op = current().value;
        int line = current().line;
        ++pos;

        return make_shared<UnaryExpr>(
            op,
            parseUnary(),
            line
        );
    }

    NodePtr node = parsePrimary();

    if (!atEnd() &&
        current().type == TokenType::OPERATOR &&
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

    if (check(TokenType::CONSTANT_INT) ||
        check(TokenType::CONSTANT_FLOAT) ||
        check(TokenType::CONSTANT_STRING) ||
        check(TokenType::CONSTANT_CHAR) ||
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
        {
            vector<NodePtr> args = parseArguments();
            return make_shared<CallExpr>(name, move(args), line);
        }

        return make_shared<IdentifierExpr>(name, line);
    }

    addError("Unexpected token '" + current().value +
             "' in expression.", line);

    if (!atEnd()) ++pos;

    return make_shared<LiteralExpr>("?", line);
}

vector<NodePtr> Parser::parseArguments()
{
    vector<NodePtr> args;

    expect(TokenType::DELIMITER, "(", "'('");

    if (!check(TokenType::DELIMITER, ")"))
    {
        do
        {
            args.push_back(parseExpression());
        }
        while (accept(TokenType::DELIMITER, ","));
    }

    expect(TokenType::DELIMITER, ")", "')' after argument list");

    return args;
}
