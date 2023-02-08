#ifndef COMPILER_CHECKER
#define COMPILER_CHECKER

#include <string>
#include "ast.h"
using namespace std;

string ConvertType(int);
void TestLexer();
void TestParser(Node *, string filename);

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