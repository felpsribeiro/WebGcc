#include <sstream>
#include "ast.h"
#include "error.h"
using std::stringstream;

extern Lexer *scanner;

int ConvertToExprType(string type)
{
    if (type == "int")
        return ExprType::INT;
    else if (type == "float")
        return ExprType::FLOAT;
    else if (type == "bool")
        return ExprType::BOOL;
    else if (type == "void")
        return ExprType::VOID;
    else
        throw SyntaxError(scanner->Lineno(), "esperado um tipo válido para a função");
}

// ----
// Node
// ----

Node::Node() : node_type(UNKNOWN) {}
Node::Node(int t) : node_type(t) {}

// -------
// Program
// -------

Program::Program(Seq *s) : Node(NodeType::PROGRAM), seq(s) {}

// ---------
// Include
// ---------

Include::Include(string n) : Node(NodeType::COUT), name(n) {}

// ---------
// Using
// ---------

// Using::Using(string s) : Node(NodeType::USING), scope(s) {}
// Using::Using(string s, string m) : Node(NodeType::USING), scope(s), method(m) {}

// ---------
// Function
// ---------

Function::Function() : Node(NodeType::FUNC) {}
Function::Function(Fun *f, Seq *s) : Node(NodeType::FUNC), info(f), seq(s) {}

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

Identifier::Identifier(int etype, Token *t) : Expression(NodeType::IDENTIFIER, etype, t), key(t->lexeme) {}
Identifier::Identifier(int etype, Token *t, unsigned int d) : Expression(NodeType::IDENTIFIER, etype, t)
{
    key = t->lexeme;
    for (int i = 0; i < d; i++)
        key += '_';
}

// ------
// Access
// ------

Access::Access(int etype, Token *t, Expression *i, Expression *e) : Expression(NodeType::ACCESS, etype, t), id(i), expr(e) {}

// -------
// Logical
// -------

Logical::Logical(Token *t, Expression *e1, Expression *e2) : Expression(NodeType::LOGI, ExprType::BOOL, t), expr1(e1), expr2(e2)
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

UnaryExpr::UnaryExpr(int etype, Token *t, Expression *e) : Expression(NodeType::UNARY, etype, t), expr(e) {}

// -------------
// Call Function
// -------------

CallFunc::CallFunc(int type, string n, Seq *aa) : Expression(NodeType::CALL, type, new Token(Tag::ID, n)), args(aa) {}

// -----
// Block
// -----

Block::Block(Seq *s) : Statement(NodeType::BLOCK), seq(s) {}

// ----
// Seq
// ----

Seq::Seq(Node *e) : Node(NodeType::SEQ), elemt(e) {}
Seq::Seq(Node *e, Seq *ee) : Node(NodeType::SEQ), elemt(e), elemts(ee) {}

// ------
// Print
// ------

Print::Print(Seq *a) : Statement(NodeType::LOG), args(a) {}

// ------
// Execute
// ------

Execute::Execute(Expression *f) : Statement(NodeType::EXECUTE), func(f) {}

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

If::If(Expression *e, Seq *s, Seq *ss) : Statement(NodeType::IF_STMT), expr(e), stmt(s), stmtElse(ss) {}

// -----
// While
// -----

While::While(Expression *e, Seq *s) : Statement(NodeType::WHILE_STMT), expr(e), stmt(s) {}

// --------
// Do-While
// --------

DoWhile::DoWhile(Seq *s, Expression *e) : Statement(NodeType::DOWHILE_STMT), stmt(s), expr(e) {}

// --------
// For
// --------

For::For(Statement *ct, Expression *co, Statement *ic, Seq *s) : Statement(NodeType::FOR_STMT), ctrl(ct), cond(co), icrmt(ic), stmt(s) {}