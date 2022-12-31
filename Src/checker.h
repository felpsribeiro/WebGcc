#ifndef COMPILER_CHECKER
#define COMPILER_CHECKER

#include <string>
using namespace std;
#include "ast.h"

string Tab();
string ConvertType(int);
void TestLexer();
void TestParser(Node *);

struct InstructionCounter
{
    unsigned int block;
    unsigned int loop;
    unsigned int depth;

    InstructionCounter();
    string Tab();
    string Tab(unsigned int d);
};

#endif