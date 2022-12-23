#include "parser.h"
#include "error.h"
#include <iostream>
#include <sstream>
#include <cctype>
using std::stringstream;

extern Lexer *scanner;

Program *Parser::Prog()
{
    // prog -> funcs
    return new Program(Funcs());
}

Seq *Parser::Funcs()
{
    // funcs -> func funcs
    //        | empty

    Seq *funcs = nullptr;

    if (lookahead->tag == Tag::TYPE)
    {
        // funcs -> func funcs
        Function *func = Func();
        Seq *seq = Funcs();
        funcs = new Seq(func, seq);
    }

    // funcs -> empty
    return funcs;
}

Function *Parser::Func()
{
    // func  -> type id(params) block

    // captura nome do tipo de retorno
    int type = ConvertToExprType(lookahead->lexeme);
    Match(Tag::TYPE);

    // captura nome da função
    string name{lookahead->lexeme};
    Match(Tag::ID);

    if (!Match('('))
        throw SyntaxError(scanner->Lineno(), "\'(\' esperado");

    Seq *params = Params();

    if (!Match(')'))
        throw SyntaxError(scanner->Lineno(), "\')\' esperado");

    // cria símbolo func
    Fun f{type, name, params};

    // insere função na tabela de funções
    if (!funcTable->Insert(f.key, f))
    {
        // a inserção falha quando a função já está na tabela
        stringstream ss;
        ss << "função \"" << name << "\" já definida";
        throw SyntaxError(scanner->Lineno(), ss.str());
    }

    // ---------------------------------------
    // nova tabela de símbolos para os parametros
    // ---------------------------------------
    paramTable = new SymTable();
    // ---------------------------------------

    // insere variáveis dos parametros na tabela de símbolos
    Seq *seq = params;
    while (seq != nullptr)
    {
        Param *param = (Param *)(seq->elemt);
        if (!paramTable->Insert(param->name, Symbol{param->type, param->name}))
        {
            // a inserção falha quando a variável já está na tabela
            stringstream ss;
            ss << "parâmetro \"" << param->name << "\" já definido";
            throw SyntaxError(scanner->Lineno(), ss.str());
        }
        seq = seq->elemts;
    }

    Statement *block = Scope();

    // ------------------
    // remove tabela
    // ------------------
    delete paramTable;
    // ------------------

    return new Function(type, name, params, block);
}

Seq *Parser::Params()
{
    // params  -> param tail
    //          | empty
    // tail    -> , param tail
    //          | empty

    Seq *p = nullptr;

    if (lookahead->tag != ')')
    {
        // params  -> param tail

        int type = ConvertToExprType(lookahead->lexeme);
        if (!Match(Tag::TYPE))
            throw SyntaxError(scanner->Lineno(), "esperado um tipo de variável válido.");

        string name{lookahead->lexeme};
        if (!Match(Tag::ID))
            throw SyntaxError(scanner->Lineno(), "esperado um nome de variável válido.");

        Param *param = new Param(type, name);
        Seq *seq = nullptr;

        // tail -> , param tail
        if (Match(','))
            seq = Params();

        p = new Seq(param, seq);
    }

    // params -> empty
    return p;
}

Statement *Parser::Scope()
{
    // block -> { stmts }

    // ------------------------------------
    // nova tabela de símbolos para o bloco
    // ------------------------------------
    SymTable *saved = varTable;
    varTable = new SymTable(varTable);
    // ------------------------------------

    if (!Match('{'))
        throw SyntaxError(scanner->Lineno(), "\'{\' esperado");

    Seq *sts = Stmts();
    Statement *block = new Block(sts, varTable->Table());

    if (!Match('}'))
        throw SyntaxError(scanner->Lineno(), "\'}\' esperado");

    // ------------------------------------------------------
    // tabela do escopo envolvente volta a ser a tabela ativa
    // ------------------------------------------------------
    delete varTable;
    varTable = saved;
    // ------------------------------------------------------

    return block;
}

// ---------- Statements ----------

Seq *Parser::Stmts()
{
    // stmts -> decl stmts
    //        | stmt stmts
    //        | empty

    Seq *stmts = nullptr;

    switch (lookahead->tag)
    {
    // stmts -> decl stmts
    case Tag::TYPE:
    {
        Statement *st = Decl();
        Seq *sts = Stmts();
        stmts = new Seq(st, sts);
        break;
    }
    // stmts -> stmt stmts
    case Tag::ID:
    case Tag::RETURN:
    case Tag::IF:
    case Tag::WHILE:
    case Tag::DO:
    case '{':
    {
        Statement *st = Stmt();
        Seq *sts = Stmts();
        stmts = new Seq(st, sts);
        break;
    }
    }

    // stmts -> empty
    return stmts;
}

Statement *Parser::Stmt()
{
    // stmt  -> local = bool;
    //        | return bool;
    //        | if (bool) stmt
    //        | while (bool) stmt
    //        | do stmt while (bool);
    //        | block

    Statement *stmt = nullptr;

    switch (lookahead->tag)
    {

    // stmt -> local = bool;
    case Tag::ID:
    {
        Expression *left = Local();
        if (!Match('='))
        {
            stringstream ss;
            ss << "esperado = no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Expression *right = Bool();
        stmt = new Assign(left, right);
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ; no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        return stmt;
    }

    // stmt -> return bool;
    case Tag::RETURN:
    {
        Match(Tag::RETURN);

        Expression *right = Bool();
        stmt = new Return(right);
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ';' no lugar de  \'" << lookahead->lexeme << "\'.";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        return stmt;
    }

    // stmt -> if (bool) stmt
    case Tag::IF:
    {
        Match(Tag::IF);
        if (!Match('('))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Expression *cond = Bool();
        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Statement *inst = Stmt();
        stmt = new If(cond, inst);
        return stmt;
    }

    // stmt -> while (bool) stmt
    case Tag::WHILE:
    {
        Match(Tag::WHILE);
        if (!Match('('))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Expression *cond = Bool();
        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Statement *inst = Stmt();
        stmt = new While(cond, inst);
        return stmt;
    }

    // stmt -> do stmt while (bool);
    case Tag::DO:
    {
        Match(Tag::DO);
        Statement *inst = Stmt();
        if (!Match(Tag::WHILE))
        {
            stringstream ss;
            ss << "esperado \'while\' no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        if (!Match('('))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        Expression *cond = Bool();
        stmt = new DoWhile(inst, cond);
        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ; no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        return stmt;
    }
    // stmt -> block
    case '{':
    {
        stmt = Scope();
        return stmt;
    }
    default:
    {
        stringstream ss;
        ss << "\'" << lookahead->lexeme << "\' não inicia uma instrução válida";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
    }
}

// ---------- Declaration ----------

Statement *Parser::Decl()
{
    // decl   -> type id index assign;
    //
    // index  -> [ integer ]
    //         | empty
    //
    // assign -> = bool
    //         | empty

    Statement *stmt = nullptr;

    // captura o tipo
    int type = ConvertToExprType(lookahead->lexeme);
    Match(Tag::TYPE);

    // captura nome do identificador
    string name{lookahead->lexeme};
    Match(Tag::ID);

    // verifica se o nome da variável já existe como um parâmetro
    if (paramTable->Find(name))
    {
        stringstream ss;
        ss << "variável \"" << name << "\" já definida como um parâmetro.";
        throw SyntaxError(scanner->Lineno(), ss.str());
    }

    // cria símbolo
    Symbol s{type, name};

    // insere variável na tabela de símbolos
    if (!varTable->Insert(name, s))
    {
        // a inserção falha quando a variável já está na tabela
        stringstream ss;
        ss << "variável \"" << name << "\" já definida.";
        throw SyntaxError(scanner->Lineno(), ss.str());
    }

    // verifica se é uma declaração de arranjo
    if (Match('['))
    {
        if (!Match(Tag::INTEGER))
        {
            stringstream ss;
            ss << "o índice de um arranjo deve ser um valor inteiro";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        if (!Match(']'))
        {
            stringstream ss;
            ss << "esperado ] no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
    }

    // verifica se tem atribuição
    if (Match('='))
    {
        Expression *left = new Identifier(type, new Token{Tag::ID, name});
        Expression *right = Bool();
        stmt = new Assign(left, right);
    }

    // verifica ponto e vírgula
    if (!Match(';'))
    {
        stringstream ss;
        ss << "encontrado \'" << lookahead->lexeme << "\' no lugar de ';'";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }

    return stmt;
}

// ---------- Expression ----------

Expression *Parser::Local()
{
    // local -> local[bool]
    //        | id

    Expression *expr = nullptr;

    switch (lookahead->tag)
    {
    case Tag::ID:
    {
        Symbol *s = nullptr;

        // verifica se a variável existe na tabela de varaiveis
        s = varTable->Find(lookahead->lexeme);
        if (!s)
        {
            // verifica se a variável existe na tabela de parâmetros
            s = paramTable->Find(lookahead->lexeme);
            if (!s)
            {
                stringstream ss;
                ss << "variável \"" << lookahead->lexeme << "\" não declarada";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }

        // identificador
        expr = new Identifier(s->type, new Token{*lookahead});
        Match(Tag::ID);

        // acesso a elemento de um arranjo
        if (Match('['))
        {
            expr = new Access(s->type, new Token{*lookahead}, expr, Bool());

            if (!Match(']'))
            {
                stringstream ss;
                ss << "esperado ] no lugar de  \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }
        break;
    }
    default:
    {
        stringstream ss;
        ss << "esperado um local de armazenamento (variável ou arranjo)";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
    }

    return expr;
}

Expression *Parser::Bool()
{
    // bool -> join lor
    // lor  -> || join lor
    //       | empty

    Expression *expr1 = Join();

    // função Lor()
    while (true)
    {
        Token t = *lookahead;

        if (Match(Tag::OR))
        {
            Expression *expr2 = Join();
            expr1 = new Logical(new Token{t}, expr1, expr2);
        }
        else
        {
            // empty
            break;
        }
    }

    return expr1;
}

Expression *Parser::Join()
{
    // join -> equality land
    // land -> && equality land
    //       | empty

    Expression *expr1 = Equality();

    // função Land()
    while (true)
    {
        Token t = *lookahead;
        if (Match(Tag::AND))
        {
            Expression *expr2 = Equality();
            expr1 = new Logical(new Token{t}, expr1, expr2);
        }
        else
        {
            // empty
            break;
        }
    }

    return expr1;
}

Expression *Parser::Equality()
{
    // equality -> rel eqdif
    // eqdif    -> == rel eqdif
    //           | != rel eqdif
    //           | empty

    Expression *expr1 = Rel();

    // função Eqdif()
    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == Tag::EQ)
        {
            Match(Tag::EQ);
            Expression *expr2 = Rel();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else if (lookahead->tag == Tag::NEQ)
        {
            Match(Tag::NEQ);
            Expression *expr2 = Rel();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else
        {
            // empty
            break;
        }
    }

    return expr1;
}

Expression *Parser::Rel()
{
    // rel  -> ari comp
    // comp -> < ari comp
    //       | <= ari comp
    //       | > ari comp
    //       | >= ari comp
    //       | empty

    Expression *expr1 = Ari();

    // função Comp()
    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == '<')
        {
            Match('<');
            Expression *expr2 = Ari();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else if (lookahead->tag == Tag::LTE)
        {
            Match(Tag::LTE);
            Expression *expr2 = Ari();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else if (lookahead->tag == '>')
        {
            Match('>');
            Expression *expr2 = Ari();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else if (lookahead->tag == Tag::GTE)
        {
            Match(Tag::GTE);
            Expression *expr2 = Ari();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else
        {
            // empty
            break;
        }
    }

    return expr1;
}

Expression *Parser::Ari()
{
    // ari  -> term oper
    // oper -> + term oper
    //       | - term oper
    //       | empty

    Expression *expr1 = Term();

    // função Oper()
    while (true)
    {
        Token t = *lookahead;

        // oper -> + term oper
        if (lookahead->tag == '+')
        {
            Match('+');
            Expression *expr2 = Term();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        // oper -> - term oper
        else if (lookahead->tag == '-')
        {
            Match('-');
            Expression *expr2 = Term();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        // oper -> empty
        else
            break;
    }

    return expr1;
}

Expression *Parser::Term()
{
    // term -> unary calc
    // calc -> * unary calc
    //       | / unary calc
    //       | empty

    Expression *expr1 = Unary();

    // função Calc()
    while (true)
    {
        Token t = *lookahead;

        // calc -> * unary calc
        if (lookahead->tag == '*')
        {
            Match('*');
            Expression *expr2 = Unary();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        // calc -> / unary calc
        else if (lookahead->tag == '/')
        {
            Match('/');
            Expression *expr2 = Unary();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        // calc -> empty
        else
            break;
    }

    return expr1;
}

Expression *Parser::Unary()
{
    // unary -> !unary
    //        | -unary
    //        | factor

    Expression *unary = nullptr;

    // unary -> !unary
    if (lookahead->tag == '!')
    {
        Token t = *lookahead;
        Match('!');
        Expression *expr = Unary();
        unary = new UnaryExpr(ExprType::BOOL, new Token{t}, expr);
    }
    // unary -> -unary
    else if (lookahead->tag == '-')
    {
        Token t = *lookahead;
        Match('-');
        Expression *expr = Unary();
        unary = new UnaryExpr(expr->type, new Token{t}, expr);
    }
    else
    {
        unary = Factor();
    }

    return unary;
}

Expression *Parser::Factor()
{
    // factor -> (bool)
    //         | local
    //         | call
    //         | integer
    //         | real
    //         | true
    //         | false

    Expression *expr = nullptr;

    switch (lookahead->tag)
    {
    // factor -> (bool)
    case '(':
    {
        Match('(');
        expr = Bool();
        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        break;
    }

    case Tag::ID:
    {
        // factor -> call
        if (scanner->Peek() == '(')
            expr = Call();
        // factor -> local
        else
            expr = Local();
        break;
    }

    // factor -> integer
    case Tag::INTEGER:
    {
        expr = new Constant(ExprType::INT, new Token{*lookahead});
        Match(Tag::INTEGER);
        break;
    }

    // factor -> real
    case Tag::REAL:
    {
        expr = new Constant(ExprType::FLOAT, new Token{*lookahead});
        Match(Tag::REAL);
        break;
    }

    // factor -> true
    case Tag::TRUE:
    {
        expr = new Constant(ExprType::BOOL, new Token{*lookahead});
        Match(Tag::TRUE);
        break;
    }

    // factor -> false
    case Tag::FALSE:
    {
        expr = new Constant(ExprType::BOOL, new Token{*lookahead});
        Match(Tag::FALSE);
        break;
    }

    default:
    {
        stringstream ss;
        ss << "uma expressão é esperada no lugar de  \'" << lookahead->lexeme << "\'";
        throw SyntaxError{scanner->Lineno(), ss.str()};
        break;
    }
    }

    return expr;
}

Expression *Parser::Call()
{
    // call   -> id(args)

    // args   -> bool tailAr
    //         | NULL
    // tailAr -> , bool tailAr
    //         | NULL

    string name{lookahead->lexeme};
    Match(Tag::ID);

    Match('(');
    Seq *args = Args();
    if (!Match(')'))
    {
        stringstream ss;
        ss << "esperado ')' no lugar de  \'" << lookahead->lexeme << "\'";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }

    // verifica se a função existe na tabela de varaiveis
    string key{FuncTable::Key(name, args)};
    Fun *f = funcTable->Find(key);
    if (!f)
    {
        stringstream ss;
        ss << "função \"" << name << "\" com esses parametros não foi declarada";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }

    return new CallFunc(f->rtr, name, args);
}

Seq *Parser::Args()
{
    // args   -> bool tailAr
    //         | empty
    // tailAr -> , bool tailAr
    //         | empty

    Seq *args = nullptr;

    // args   -> bool tailAr
    if (lookahead->lexeme != ")")
    {
        Expression *bo = Bool();

        // tailAr -> , bool tailAr
        Seq *tail = nullptr;
        if (Match(','))
        {
            tail = Args();
        }

        args = new Seq(bo, tail);
    }

    // args -> empty
    return args;
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

Parser::Parser()
{
    lookahead = scanner->Scan();
    varTable = nullptr;
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
