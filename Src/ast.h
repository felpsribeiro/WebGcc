
#ifndef COMPILER_AST
#define COMPILER_AST

#include "lexer.h"
#include "symtable.h"
#include "functable.h"
#include <string>

enum NodeType
{
    UNKNOWN,
    PROGRAM,
    COUT,
    FUNC,
    CALL,
    EXECUTE,
    PARAM,
    STMT,
    EXPR,
    CONSTANT,
    IDENTIFIER,
    ACCESS,
    LOGI,
    REL,
    ARI,
    UNARY,
    BLOCK,
    SEQ,
    ASSIGN,
    RETURN_STMT,
    IF_STMT,
    WHILE_STMT,
    DOWHILE_STMT,
    FOR_STMT,
    LOG,
    EXPR_ASSIGN,
    ASSIGN_EXPR,
};

enum ExprType
{
    VOID,
    INT,
    FLOAT,
    BOOL
};

// retorna o equivalente ExprType para o tipo em string
int StringToExprType(string type);
string ExprTypeToString(int type);

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
    string Name() { return token->lexeme; };
    string Type();
};

struct Constant : public Expression
{
    Constant(int type, Token *t);
};

struct Identifier : public Expression
{
    string key;

    Identifier(int etype, Token *t);
    Identifier(int etype, Token *t, unsigned int d);
};

struct Access : public Expression
{
    Identifier *addres;
    Expression *expr;

    Access(int etype, Token *t, Identifier *a, Expression *e);
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

    Block(Seq *s);
};

struct Include : public Node
{
    string name;

    Include(string n);
};

struct Using : public Node
{
    string scope;
    string method;

    Using(string s);
    Using(string s, string m);
};

struct Function : public Node
{
    Fun *info; // informações da função
    Seq *seq;  // instruções do corpo da função

    Function();
    Function(Fun *f, Seq *s);
};

struct Program : public Node
{
    Seq *seq;

    Program(Seq *s);
};

struct Print : public Statement
{
    Seq *args;

    Print(Seq *a);
};

struct Execute : public Statement
{
    Expression *func;

    Execute(Expression *f);
};

struct Assign : public Statement
{
    Expression *id;
    Expression *expr;

    Assign(Expression *i, Expression *e);
};

struct ExprPlusAssign : public Expression
{
    Expression *expr;
    Assign *assign;

    ExprPlusAssign(Expression *e, Assign *a);
};

struct AssignPlusExpr : public Expression
{
    Assign *assign;
    Expression *expr;

    AssignPlusExpr(Assign *a, Expression *e);
};

struct SeqAssign : public Statement
{
    Seq *seq;

    SeqAssign(Seq *s);
};

struct Return : public Statement
{
    Expression *expr;

    Return(Expression *e);
};

struct If : public Statement
{
    Expression *expr;
    Seq *stmt;
    Seq *stmtElse;

    If(Expression *e, Seq *s, Seq *ss);
};

struct While : public Statement
{
    Expression *expr;
    Seq *stmt;

    While(Expression *e, Seq *s);
};

struct DoWhile : public Statement
{
    Seq *stmt;
    Expression *expr;

    DoWhile(Seq *s, Expression *e);
};

struct For : public Statement
{
    Statement *ctrl;
    Expression *cond;
    Statement *icrmt;
    Seq *stmt;

    For(Statement *ct, Expression *co, Statement *ic, Seq *s);
};

#endif