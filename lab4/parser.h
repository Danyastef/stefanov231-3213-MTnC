#pragma once
#include "lexer.h"
#include <memory>
#include <string>
#include <vector>

enum class NodeType
{
    PROGRAM,
    FUNCTION_DECL,
    PARAM_DECL,
    BLOCK_STMT,
    VAR_DECL_STMT,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    EXPR_STMT,
    IDENTIFIER_EXPR,
    LITERAL_EXPR,
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR
};

struct ASTNode
{
    NodeType type;
    int line;

    explicit ASTNode(NodeType type, int line = 0)
        : type(type), line(line) {}

    virtual ~ASTNode() = default;
    virtual void print(const std::string& prefix = "",
                       bool isLast = true) const = 0;
};

using NodePtr = std::shared_ptr<ASTNode>;

struct IdentifierExpr : ASTNode
{
    std::string name;
    IdentifierExpr(const std::string& name, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct LiteralExpr : ASTNode
{
    std::string value;
    LiteralExpr(const std::string& value, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct BinaryExpr : ASTNode
{
    std::string op;
    NodePtr left;
    NodePtr right;

    BinaryExpr(const std::string& op, NodePtr left, NodePtr right, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct UnaryExpr : ASTNode
{
    std::string op;
    NodePtr operand;

    UnaryExpr(const std::string& op, NodePtr operand, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct CallExpr : ASTNode
{
    std::string callee;
    std::vector<NodePtr> arguments;

    CallExpr(const std::string& callee,
             std::vector<NodePtr> arguments,
             int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct BlockStmt : ASTNode
{
    std::vector<NodePtr> statements;

    explicit BlockStmt(int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct VarDeclStmt : ASTNode
{
    std::string varType;
    std::string name;
    NodePtr initExpr;

    VarDeclStmt(const std::string& varType,
                const std::string& name,
                NodePtr initExpr,
                int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct IfStmt : ASTNode
{
    NodePtr condition;
    NodePtr thenBranch;
    NodePtr elseBranch;

    IfStmt(NodePtr condition,
           NodePtr thenBranch,
           NodePtr elseBranch,
           int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct WhileStmt : ASTNode
{
    NodePtr condition;
    NodePtr body;

    WhileStmt(NodePtr condition, NodePtr body, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct ForStmt : ASTNode
{
    NodePtr init;
    NodePtr condition;
    NodePtr update;
    NodePtr body;

    ForStmt(NodePtr init,
            NodePtr condition,
            NodePtr update,
            NodePtr body,
            int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct ReturnStmt : ASTNode
{
    NodePtr value;

    ReturnStmt(NodePtr value, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct ExprStmt : ASTNode
{
    NodePtr expression;

    ExprStmt(NodePtr expression, int line);
    void print(const std::string& prefix, bool isLast) const override;
};

struct ParamDecl : ASTNode
{
    std::string paramType;
    std::string name;

    ParamDecl(const std::string& paramType,
              const std::string& name,
              int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct FunctionDecl : ASTNode
{
    std::string returnType;
    std::string name;
    std::vector<NodePtr> parameters;
    NodePtr body;

    FunctionDecl(const std::string& returnType,
                 const std::string& name,
                 std::vector<NodePtr> parameters,
                 NodePtr body,
                 int line);

    void print(const std::string& prefix, bool isLast) const override;
};

struct ProgramNode : ASTNode
{
    std::vector<NodePtr> functions;

    ProgramNode();
    void print(const std::string& prefix, bool isLast) const override;
};

struct ParseError
{
    std::string message;
    int line;
};

class Parser
{
public:
    explicit Parser(const std::vector<Token>& tokens);

    NodePtr parse();
    const std::vector<ParseError>& getErrors() const;

private:
    const std::vector<Token>& tokens;
    size_t pos;
    std::vector<ParseError> errors;

    bool atEnd() const;
    const Token& current() const;

    bool check(TokenType type, const std::string& value = "") const;
    bool accept(TokenType type, const std::string& value = "");
    Token expect(TokenType type,
                 const std::string& value,
                 const std::string& description);

    void addError(const std::string& message, int line);
    bool isTypeToken() const;

    NodePtr parseProgram();
    NodePtr parseFunction();
    NodePtr parseParameter();
    NodePtr parseBlock();
    NodePtr parseStatement();
    NodePtr parseVariableDeclaration(bool requireSemicolon = true);
    NodePtr parseIf();
    NodePtr parseWhile();
    NodePtr parseFor();
    NodePtr parseReturn();
    NodePtr parseExpressionStatement();

    NodePtr parseExpression();
    NodePtr parseAssignment();
    NodePtr parseLogicalOr();
    NodePtr parseLogicalAnd();
    NodePtr parseEquality();
    NodePtr parseComparison();
    NodePtr parseTerm();
    NodePtr parseFactor();
    NodePtr parseUnary();
    NodePtr parsePrimary();

    std::vector<NodePtr> parseArguments();
};
