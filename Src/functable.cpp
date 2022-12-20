#include "functable.h"

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
