#ifndef COMPILER_SYMTABLE
#define COMPILER_SYMTABLE

#include <unordered_map>
#include <string>
using std::string;
using std::unordered_map;

// modelo para símbolos
struct Symbol
{
	string var;
	string type;
	int etype;
};

typedef unordered_map<string, Symbol> SymMap;

// tabela de símbolos
class SymTable
{
private:
	SymMap table;
	SymTable *prev;

public:
	SymTable();
	SymTable(SymTable *t);

	bool Insert(string s, Symbol symb);
	Symbol *Find(string s);
	SymMap Table();
};

inline SymMap SymTable::Table() { return table; }

#endif