
#ifndef COMPILER_AST
#define COMPILER_AST

#include "lexer.h"
#include "symtable.h"

enum NodeType
{
    UNKNOWN,
    PROGRAM,
    FUNC,
    CALL,
    PARAM,
    STMT,
    EXPR,
    CONSTANT,
    IDENTIFIER,
    ACCESS,
    LOG,
    REL,
    ARI,
    UNARY,
    BLOCK,
    STRUC,
    SEQ,
    ASSIGN,
    RETURN_STMT,
    IF_STMT,
    WHILE_STMT,
    DOWHILE_STMT,
    FOR_STMT
};

enum ExprType
{
    VOID,
    INT,
    FLOAT,
    BOOL
};

// retorna o equivalente ExprType para o tipo em string
int ConvertToExprType(string type);

struct Node
{
    int node_type;
    Node();
    Node(int t);
};

struct Seq : public Node
{
    Node *elemt;
    Seq *elemts;
    Seq(Node *e);
    Seq(Node *e, Seq *ee);
};

struct Statement : public Node
{
    Statement();
    Statement(int type);
};

struct Expression : public Node
{
    int type;
    Token *token;

    Expression(Token *t);
    Expression(int ntype, int type, Token *t);
    string Name();
    string Type();
};

struct Constant : public Expression
{
    Constant(int type, Token *t);
};

struct Identifier : public Expression
{
    Identifier(int etype, Token *t);
};

struct Access : public Expression
{
    Expression *id;
    Expression *expr;
    Access(int etype, Token *t, Expression *i, Expression *e);
};

struct Logical : public Expression
{
    Expression *expr1;
    Expression *expr2;
    Logical(Token *t, Expression *e1, Expression *e2);
};

struct Relational : public Expression
{
    Expression *expr1;
    Expression *expr2;
    Relational(Token *t, Expression *e1, Expression *e2);
};

struct Arithmetic : public Expression
{
    Expression *expr1;
    Expression *expr2;
    Arithmetic(int etype, Token *t, Expression *e1, Expression *e2);
};

struct UnaryExpr : public Expression
{
    Expression *expr;
    UnaryExpr(int etype, Token *t, Expression *e);
};

struct CallFunc : public Expression
{
    Seq *args;

    CallFunc(int type, string n, Seq *aa);
};

struct Block : public Statement
{
    Seq *seq;
    SymMap table;
    Block(Seq *s, SymMap t);
};

struct Struc : public Statement
{
    Statement *stmt;
    SymMap table;
    Struc(Statement *s, SymMap t);
};

struct Function : public Node
{
    int type;         // tipo de retorno da função
    string name;      // nome da função
    Seq *params;      // sequência de parâmetros da função
    Statement *block; // instruções do corpo da função

    Function();
    Function(int t, string n, Seq *pp, Statement *b);
};

struct Param : public Node
{
    int type;    // tipo do parametro
    string name; // nome do parametro
    int valor;   // valor do parametro

    Param(int t, string n);
    Param(int t, int v);
};

struct Program : public Node
{
    Seq *funcs;
    Program(Seq *f);
};

struct Assign : public Statement
{
    Expression *id;
    Expression *expr;
    Assign(Expression *i);
    Assign(Expression *i, Expression *e);
};

struct Return : public Statement
{
    Expression *expr;
    Return(Expression *e);
};

struct If : public Statement
{
    Expression *expr;
    Statement *stmt;
    Statement *stmtElse;
    If(Expression *e, Statement *s, Statement *ss);
};

struct While : public Statement
{
    Expression *expr;
    Statement *stmt;
    While(Expression *e, Statement *s);
};

struct DoWhile : public Statement
{
    Statement *stmt;
    Expression *expr;
    DoWhile(Statement *s, Expression *e);
};

struct For : public Statement
{
    Statement *ctrl;
    Expression *cond;
    Statement *icrmt;
    Statement *stmt;
    For(Statement *ct, Expression *co, Statement * ic, Statement *s);
};

#endif