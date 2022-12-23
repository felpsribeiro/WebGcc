#ifndef COMPILER_PARSER
#define COMPILER_PARSER

#include "lexer.h"
#include "functable.h"
#include "symtable.h"
#include "ast.h"
#include <vector>
using std::vector;

class Parser
{
private:
	FuncTable *funcTable;
	SymTable *varTable;
	SymTable *paramTable;
	Token *lookahead;

	Program *Prog();
	Seq *Funcs();
	Function *Func();
	Seq *Params();
	Statement *Scope();
	Seq *Stmts();
	Statement *Decl();
	Statement *Stmt();
	Expression *Local();
	Expression *Bool();
	Expression *Join();
	Expression *Equality();
	Expression *Rel();
	Expression *Ari();
	Expression *Term();
	Expression *Unary();
	Expression *Factor();
	Expression *Call();
	Seq *Args();
	bool Match(int tag);

public:
	Parser();
	Node *Start();
	static int LineNo();
};

#endif
