#include "functable.h"
#include <sstream>
using std::stringstream;

Fun::Fun(int r, string n, Seq *pp) : rtr(r), name(n), params(pp)
{
	stringstream ss;
	ss << n;

	Seq *seq = pp;
	while (seq != nullptr)
	{
		Param *param = (Param *)(seq->elemt);
		ss << param->type;
		seq = seq->elemts;
	}

	key = ss.str();
}

// construtor para a primeira tabela
FuncTable::FuncTable() : prev(nullptr)
{
}

// construtor para novas tabelas
FuncTable::FuncTable(FuncTable *t) : prev(t)
{
}

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

string FuncTable::Key(string name, Seq *args)
{
	stringstream ss;
	ss << name;
	Seq *seq = args;
	while (seq != nullptr)
	{
		Expression *param = (Expression *)(seq->elemt);
		ss << param->type;
		seq = seq->elemts;
	}
	return ss.str();
}
