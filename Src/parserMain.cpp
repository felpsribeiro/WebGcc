#include "parser.h"
#include "error.h"
#include <iostream>
#include <sstream>
using std::stringstream;

extern Lexer *scanner;
unsigned int Parser::memoryPointer = 0;

Program *Parser::Prog()
{
    // program = {instr} ;

    return new Program(Instr());
}

Seq *Parser::Instr()
{
    // instr   = include
    //         | using
    //         | func ;

    switch (lookahead->tag)
    {
    case '#':
        return new Seq(Incl(), Instr());
    // case Tag::USING:
    //     return new Seq(Usin(), Instr());
    case Tag::TYPE:
        return new Seq(Func(), Instr());
    default:
        return nullptr;
    }
}

Include *Parser::Incl()
{
    // include     = '#include' , lib , 'using' , 'namespace' , 'std' , ';';
    // lib         = '<iostream>' ;
    Match('#');
    if (!Match(Tag::INCLUDE))
        throw SyntaxError(scanner->Lineno(), "\"include\" esperado.");

    if (!Match('<'))
        throw SyntaxError(scanner->Lineno(), "\'<\' esperado.");
    string libName{lookahead->lexeme};
    if (!Match(Tag::LIB))
        throw SyntaxError(scanner->Lineno(), "lib inválida.");
    if (!Match('>'))
        throw SyntaxError(scanner->Lineno(), "\'>\' esperado.");

    if (!Match(Tag::USING))
        throw SyntaxError(scanner->Lineno(), "\"using\" esperado.");
    if (!Match(Tag::NAMESPACE))
        throw SyntaxError(scanner->Lineno(), "\"namespace\" esperado.");
    if (!Match(Tag::ID))
        throw SyntaxError(scanner->Lineno(), "\"std\" esperado.");
    if (!Match(';'))
        throw SyntaxError(scanner->Lineno(), "\';\' esperado.");

    return new Include(libName);
}

Function *Parser::Func()
{
    // func -> type id(params) { scope }

    // captura nome do tipo de retorno
    int type = StringToExprType(lookahead->lexeme);
    Match(Tag::TYPE);

    // captura nome da função
    string name{lookahead->lexeme};
    Match(Tag::ID);

    // ---------------------------------------
    // nova tabela de símbolos para a função
    // ---------------------------------------
    symTable = new SymTable();
    // ---------------------------------------

    if (!Match('('))
        throw SyntaxError(scanner->Lineno(), "\'(\' esperado");

    Params();

    if (!Match(')'))
        throw SyntaxError(scanner->Lineno(), "\')\' esperado");

    if (name.compare("main") == 0)
        type = ExprType::VOID;

    // cria símbolo Fun
    funcInfo = new Fun(type, name, symTable->Table());

    // insere função na tabela de funções
    if (!funcTable->Insert(funcInfo->key, *funcInfo))
    {
        // a inserção falha quando a função já está na tabela
        stringstream ss;
        ss << "função \"" << name << "\" já definida";
        throw SyntaxError(scanner->Lineno(), ss.str());
    }

    if (!Match('{'))
        throw SyntaxError(scanner->Lineno(), "\'{\' esperado");

    Seq *sts = Stmts();

    if (!Match('}'))
        throw SyntaxError(scanner->Lineno(), "\'}\' esperado");

    Function *func = new Function(funcInfo, sts);

    // ------------------
    // remove tabela de parâmetros
    // ------------------
    delete symTable;
    // ------------------

    return func;
}

void Parser::Params()
{
    // params  -> param tail
    //          | empty
    // tail    -> , param tail
    //          | empty

    while (lookahead->tag != ')')
    {
        // params  -> param tail

        int type = StringToExprType(lookahead->lexeme);
        if (!Match(Tag::TYPE))
            throw SyntaxError(scanner->Lineno(), "esperado um tipo de variável válido.");

        string name{lookahead->lexeme};
        if (!Match(Tag::ID))
            throw SyntaxError(scanner->Lineno(), "esperado um nome de variável válido.");

        if (!symTable->Insert(name, Symbol{type, name}))
        {
            // a inserção falha quando a variável já está na tabela
            stringstream ss;
            ss << "parâmetro \"" << name << "\" já definido";
            throw SyntaxError(scanner->Lineno(), ss.str());
        }

        // tail -> , param tail
        if (Match(',') && lookahead->tag != Tag::TYPE)
            throw SyntaxError(scanner->Lineno(), "esperado mais um parâmetro após a virgula.");
    }
}

Seq *Parser::Stmts()
{
    // stmts -> stmt stmts;
    //        | empty;

    Seq *stmts = nullptr;

    if (lookahead->tag != '}')
        stmts = new Seq(Stmt(), Stmts());

    // stmts -> empty
    return stmts;
}

bool Parser::Match(int tag)
{
    if (tag == lookahead->tag)
    {
        lookahead = scanner->Scan();
        return true;
    }

    return false;
}

void Parser::AllocateMemory(int type, int size)
{
    int unitSize;
    switch (type)
    {
    case ExprType::INT:
    case ExprType::FLOAT:
    case ExprType::BOOL:
        unitSize = 4;
        break;
    case ExprType::VOID:
        stringstream ss;
        ss << "matriz de void não é permitida";
        throw SyntaxError(scanner->Lineno(), ss.str());
        break;
    }

    memoryPointer += unitSize * size;
}

Parser::Parser()
{
    lookahead = scanner->Scan();
    funcTable = nullptr;
    globalTable = nullptr;
    symTable = nullptr;
}

Node *Parser::Start()
{
    // ------------------------------------
    // tabela de funções com os seus nomes
    // ------------------------------------
    funcTable = new FuncTable();
    // ------------------------------------

    Program *prog = Prog();

    // verifica se a função main foi declarada
    Fun *f = funcTable->Find("main");
    if (!f)
    {
        stringstream ss;
        ss << "função 'main' não foi declarada";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }

    return prog;
}
