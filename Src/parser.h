#ifndef COMPILER_PARSER
#define COMPILER_PARSER

#include "lexer.h"
#include "functable.h"
#include "symtable.h"
#include "ast.h"

class Parser
{
private:
	FuncTable * functable;
	SymTable * symtable;
	Token * lookahead;
	
	Program * Prog();
	Function * Funcs();
	Function * Func();
	Statement * Scope();
	Statement * Decl();
	Statement * Stmts();
	Statement * Stmt();
	Expression * Local();
	Expression * Bool();
	Expression * Join();
	Expression * Equality();
	Expression * Rel();
	Expression * Ari();
	Expression * Term();
	Expression * Unary();
	Expression * Factor();
	bool Match(int tag);

public:
	Parser();
	Node * Start();
	static int LineNo();
};

#endif
