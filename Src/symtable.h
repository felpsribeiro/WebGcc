#ifndef COMPILER_SYMTABLE
#define COMPILER_SYMTABLE

#include <string>
#include <map>
using std::string;
using std::map;

// modelo para símbolos
struct Symbol
{
	int type;
	string name;
	unsigned int depth;
};

typedef map<string, Symbol> SymMap;

// tabela de símbolos
class SymTable
{
private:
	SymMap table;
	SymTable *prev;

public:
	SymTable();
	SymTable(SymTable *t);
	~SymTable();

	bool Insert(string s, Symbol symb);
	Symbol *Find(string s);
	SymMap Table() { return table; };
	
	static unsigned int depth;
};

#endif