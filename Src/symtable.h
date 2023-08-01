#ifndef COMPILER_SYMTABLE
#define COMPILER_SYMTABLE

#include <string>
#include <map>
using std::map;
using std::string;

// modelo para símbolos
struct Symbol
{
	int type;
	string name;
	unsigned int depth;
	bool isArray;

	Symbol(int t, string n, unsigned int d);
	Symbol(int t, string n, unsigned int d, bool a);
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