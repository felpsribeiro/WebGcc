#ifndef COMPILER_CHECKER
#define COMPILER_CHECKER

#include <string>
#include "ast.h"
using namespace std;

string ConvertType(int);
void TestLexer();
void TestParser(Node *, string filename);

struct Counter
{
    static unsigned int block;
    static unsigned int loop;
    static unsigned int depth;
    static void ftab(int add);
};

#endif