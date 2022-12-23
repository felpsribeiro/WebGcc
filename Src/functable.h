#ifndef COMPILER_FUNCTABLE
#define COMPILER_FUNCTABLE

#include "symtable.h"
#include "ast.h"
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;

// modelo para funções
struct Fun
{
	string key;
	int rtr;
	string name;
	Seq *params;

	Fun(int r, string n, Seq *pp);
};

// tabela de funções
class FuncTable
{
private:
	unordered_map<string, Fun> table;
	FuncTable *prev;

public:
	FuncTable();
	FuncTable(FuncTable *t);

	bool Insert(string n, Fun fun);
	Fun *Find(string n);

	static string Key(string name, Seq *args); // retorna a chave da função
};

#endif