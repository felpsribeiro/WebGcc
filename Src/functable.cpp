#include "functable.h"
#include <sstream>
using std::stringstream;

Fun::Fun(int r, string n, SymMap p) : rtr(r), name(n), params(p)
{
	key = Key(n, &p);
}

bool Fun::InsertLocal(string name, Symbol var)
{
	const auto &[pos, success] = locals.insert({name, var});
	return success;
}

string Fun::Key(string name, SymMap *p)
{
	stringstream ss;
	ss << name;

	for (SymMap::iterator it = (*p).begin(); it != (*p).end(); ++it)
		ss << it->second.type;

	return ss.str();
}

FuncTable::FuncTable() {}

// insere um símbolo na tabela
bool FuncTable::Insert(string n, Fun fun)
{
	const auto &[pos, success] = table.insert({n, fun});
	return success;
}

// busca uma função na tabela
Fun *FuncTable::Find(string f)
{
	auto found = table.find(f);
	if (found != table.end())
		return &found->second;

	return nullptr;
}
