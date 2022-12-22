#ifndef COMPILER_FUNCTABLE
#define COMPILER_FUNCTABLE

#include "symtable.h"
#include <string>
#include <unordered_map>
#include <vector>
using std::string;
using std::unordered_map;
using std::vector;

// modelo para funções
struct Fun
{
	int rtr;
	string name;
	vector<Symbol> params;
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
};

#endif