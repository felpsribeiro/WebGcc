#include "symtable.h"

unsigned int SymTable::depth = 0;

// construtor para a primeira tabela
SymTable::SymTable() : prev(nullptr)
{
}

// construtor para novas tabelas
SymTable::SymTable(SymTable *t) : prev(t)
{
	depth++;
}

SymTable::~SymTable()
{
	if (depth > 0)
		depth--;
}

// insere um símbolo na tabela
bool SymTable::Insert(string s, Symbol symb)
{
	const auto &[pos, success] = table.insert({s, symb});
	return success;
}

// busca um símbolo na tabela atual ou nas dos escopos envolventes
Symbol *SymTable::Find(string s)
{
	for (SymTable *st = this; st != nullptr; st = st->prev)
	{
		auto found = st->table.find(s);
		if (found != st->table.end())
			return &found->second;
	}

	return nullptr;
}
