#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "error.h"
#include "ast.h"
#include "checker.h"
using namespace std;

ifstream fin;
Lexer *scanner;

// programa pode receber nomes de arquivos
int main(int argc, char **argv)
{
	if (argc == 2)
	{
		fin.open(argv[1]);
		if (!fin.is_open())
		{
			cout << "Falha na abertura do arquivo \'" << argv[1] << "\'.\n";
			exit(EXIT_FAILURE);
		}

		Lexer leitor;
		scanner = &leitor;
		Node *ast;
		Parser tradutor;

		string filename = (string)argv[1];
		filename = filename.substr(0, filename.length() - 4);
		string filenameWAT = filename + ".wat";

		try
		{
			ast = tradutor.Start();
			TestParser(ast, filenameWAT);

			stringstream ss;
			ss << "wat2wasm " << filenameWAT << " -o " << filename << ".wasm";
			system(&ss.str()[0]);
		}
		catch (SyntaxError err)
		{
			err.What();
		}
		fin.close();
	}
}