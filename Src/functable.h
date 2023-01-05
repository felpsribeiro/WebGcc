#ifndef COMPILER_FUNCTABLE
#define COMPILER_FUNCTABLE

#include "symtable.h"
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
	SymMap params;
	SymMap locals;

	Fun(int r, string n, SymMap p);
	bool InsertLocal(string n, Symbol v);

	static string Key(string name, SymMap *params); // retorna a chave da função
};

// tabela de funções
class FuncTable
{
private:
	unordered_map<string, Fun> table;

public:
	FuncTable();

	bool Insert(string n, Fun fun);
	Fun *Find(string n);
};

#endif