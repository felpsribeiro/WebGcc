#ifndef COMPILER_FUNCTABLE
#define COMPILER_FUNCTABLE

#include <unordered_map>
#include <string>
using std::unordered_map;
using std::string;


// modelo para símbolos
struct Fun
{
	string name;
	string rtr;
};


// tabela de símbolos
class FuncTable
{
private: 
   	unordered_map<string,Fun> table;    
   	FuncTable * prev;   

public:
	FuncTable();
	FuncTable(FuncTable * t);
	
	bool Insert(string n, Fun fun);
	Fun * Find(string n); 
};

#endif