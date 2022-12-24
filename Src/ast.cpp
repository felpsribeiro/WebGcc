#include <sstream>
#include "ast.h"
#include "error.h"
using std::stringstream;

extern Lexer *scanner;

int ConvertToExprType(string type)
{
    int etype = ExprType::VOID;

    if (type == "int")
        etype = INT;
    else if (type == "float")
        etype = FLOAT;
    else if (type == "bool")
        etype = BOOL;

    return etype;
}

// ----
// Node
// ----

Node::Node() : node_type(UNKNOWN) {}
Node::Node(int t) : node_type(t) {}

// -------
// Program
// -------

Program::Program(Seq *f) : Node(NodeType::PROGRAM), funcs(f) {}

// ---------
// Function
// ---------

Function::Function() : Node(NodeType::FUNC) {}
Function::Function(int t, string n, Seq *pp, Statement *b)
    : Node(NodeType::FUNC), type(t), name(n), params(pp), block(b) {}

// ---------
// Param
// ---------

Param::Param(int t, string n) : Node(NodeType::PARAM), type(t), name(n) {}
Param::Param(int t, int v) : Node(NodeType::PARAM), type(t), valor(v) {}

// ---------
// Statement
// ---------

Statement::Statement() : Node(NodeType::STMT) {}
Statement::Statement(int type) : Node(type) {}

// ----------
// Expression
// ----------

Expression::Expression(Token *t) : Node(NodeType::EXPR), type(ExprType::VOID), token(t) {}
Expression::Expression(int ntype, int type, Token *t) : Node(ntype), type(type), token(t) {}

string Expression::Name()
{
    switch (token->tag)
    {
    case '+':
        return "i32.add ";
    case '-':
        return "i32.sub ";
    case '*':
        return "i32.mul";
    case '/':
        return "i32.div_u";
    default:
        return token->lexeme;
    }
}

string Expression::Type()
{
    switch (type)
    {
    case ExprType::INT:
        return "int";
        break;
    case ExprType::FLOAT:
        return "float";
        break;
    case ExprType::BOOL:
        return "bool";
        break;
    default:
        return "void";
    }
}

// --------
// Constant
// --------

Constant::Constant(int type, Token *t) : Expression(NodeType::CONSTANT, type, t) {}

// ----------
// Identifier
// ----------

Identifier::Identifier(int etype, Token *t) : Expression(NodeType::IDENTIFIER, etype, t) {}

// ------
// Access
// ------

Access::Access(int etype, Token *t, Expression *i, Expression *e) : Expression(NodeType::ACCESS, etype, t), id(i), expr(e) {}

// -------
// Logical
// -------

Logical::Logical(Token *t, Expression *e1, Expression *e2) : Expression(NodeType::LOG, ExprType::BOOL, t), expr1(e1), expr2(e2)
{
    // verificação de tipos
    if (expr1->type != ExprType::BOOL || expr2->type != ExprType::BOOL)
    {
        stringstream ss;
        ss << "\'" << token->lexeme << "\' usado com operandos não booleanos ("
           << expr1->Name() << ":" << expr1->Type() << ") ("
           << expr2->Name() << ":" << expr2->Type() << ") ";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

// ----------
// Relational
// ----------

Relational::Relational(Token *t, Expression *e1, Expression *e2) : Expression(NodeType::REL, ExprType::BOOL, t), expr1(e1), expr2(e2)
{
    // verificação de tipos
    if (expr1->type != expr2->type)
    {
        stringstream ss;
        ss << "\'" << token->lexeme << "\' usado com operandos de tipos diferentes ("
           << expr1->Name() << ":" << expr1->Type() << ") ("
           << expr2->Name() << ":" << expr2->Type() << ") ";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

// ----------
// Arithmetic
// ----------

Arithmetic::Arithmetic(int etype, Token *t, Expression *e1, Expression *e2) : Expression(NodeType::ARI, etype, t), expr1(e1), expr2(e2)
{
    // verificação de tipos
    if (expr1->type != expr2->type)
    {
        stringstream ss;
        ss << "\'" << token->lexeme << "\' usado com operandos de tipos diferentes ("
           << expr1->Name() << ":" << expr1->Type() << ") ("
           << expr2->Name() << ":" << expr2->Type() << ") ";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

// ---------
// UnaryExpr
// ---------

UnaryExpr::UnaryExpr(int etype, Token *t, Expression *e) : Expression(NodeType::UNARY, etype, t), expr(e)
{
    // verificação de tipos
    if (expr->type != ExprType::BOOL)
    {
        stringstream ss;
        ss << "\'" << token->lexeme << "\' usado com operando não booleano ("
           << expr->Name() << ":" << expr->Type() << ")";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

// -------------
// Call Function
// -------------

CallFunc::CallFunc(int type, string n, Seq *aa) : Expression(NodeType::CALL, type, new Token(Tag::ID, n)), args(aa) {}

// -----
// Block
// -----

Block::Block(Seq *s, SymMap t) : Statement(NodeType::BLOCK), seq(s), table(t) {}

// ----
// Seq
// ----

Seq::Seq(Node *e, Seq *ee) : Node(NodeType::SEQ), elemt(e), elemts(ee) {}

// ------
// Assign
// ------

Assign::Assign(Expression *i, Expression *e) : Statement(NodeType::ASSIGN), id(i), expr(e)
{
    // verificação de tipos
    if (id->type != expr->type)
    {
        stringstream ss;
        ss << "\'=\' usado com operandos de tipos diferentes ("
           << id->Name() << ":" << id->Type() << ") ("
           << expr->Name() << ":" << expr->Type() << ") ";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

// ----
// Return
// ----

Return::Return(Expression *e) : Statement(NodeType::RETURN_STMT), expr(e) {}

// ----
// If
// ----

If::If(Expression *e, Statement *s) : Statement(NodeType::IF_STMT), expr(e), stmt(s) {}

// -----
// While
// -----

While::While(Expression *e, Statement *s) : Statement(NodeType::WHILE_STMT), expr(e), stmt(s) {}

// --------
// Do-While
// --------

DoWhile::DoWhile(Statement *s, Expression *e) : Statement(NodeType::DOWHILE_STMT), stmt(s), expr(e) {}