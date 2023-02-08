#ifndef COMPILER_ERROR
#define COMPILER_ERROR

#include <string>
using std::string;

class SyntaxError 
{
private:
	int lineno;
	string desc;
public:
	SyntaxError(int line, string msg);
	void What();
};

class OpenFileError 
{
private:
	string desc;
public:
	OpenFileError(string msg);
	void What();
};

#endif
