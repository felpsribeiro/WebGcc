#include "parser.h"
#include "error.h"
#include <iostream>
#include <sstream>
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
    // func -> type id(params) { scope }

    // captura nome do tipo de retorno
    int type = ConvertToExprType(lookahead->lexeme);
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

        int type = ConvertToExprType(lookahead->lexeme);
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

// ---------- Statements ----------

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

Statement *Parser::Stmt()
{
    // stmt -> decl;
    //       | assign;
    //       | call;
    //       | return bool;
    //       | block

    Statement *stmt = nullptr;

    switch (lookahead->tag)
    {
    // stmt -> decl
    case Tag::TYPE:
    {
        Statement *st = Decl();
        // verifica ponto e vírgula
        if (!Match(';'))
        {
            stringstream ss;
            ss << "encontrado \'" << lookahead->lexeme << "\' no lugar de ';'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        return st;
    }
    // stmts -> assign;
    case Tag::PLUSPLUS:
    case Tag::LESSLESS:
    {
        Statement *st = Attribution();
        if (!Match(';'))
        {
            stringstream ss;
            ss << "encontrado \'" << lookahead->lexeme << "\' no lugar de ';'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        return st;
    }
    // stmt -> call;
    //       | assign;
    case Tag::ID:
    {
        Statement *st;

        // stmt -> call;
        if (scanner->Peek() == '(')
            st = new Execute(Call());

        // stmt -> assign;
        else
            st = Attribution();

        if (!Match(';'))
        {
            stringstream ss;
            ss << "encontrado \'" << lookahead->lexeme << "\' no lugar de ';'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        return st;
    }
    // stmt -> return bool;
    case Tag::RETURN:
    {
        Match(Tag::RETURN);

        Statement *st = new Return(Bool());
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ';' no lugar de  \'" << lookahead->lexeme << "\'.";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        return st;
    }
    // stmt -> block
    case '{':
    case Tag::IF:
    case Tag::WHILE:
    case Tag::DO:
    case Tag::FOR:
    {
        return Scope();
    }
    default:
    {
        stringstream ss;
        ss << "esperado uma instrução no lugar de  \'" << lookahead->lexeme << "\'.";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
    }
}

Statement *Parser::Attribution()
{
    // assign -> plusplus local
    //        | local plusplus
    //        | local oper = bool

    Statement *stmt = nullptr;

    switch (lookahead->tag)
    {
    // assign -> plusplus local
    case Tag::PLUSPLUS:
    case Tag::LESSLESS:
    {
        Token t = *lookahead;
        Match(lookahead->tag);
        Expression *left = Local();
        Expression *constant = new Constant(ExprType::INT, new Token{'1'});
        Expression *right = new Arithmetic(left->type, new Token(t), left, constant);

        stmt = new Assign(left, right);
        return stmt;
    }

    // assign -> local plusplus
    //         | local oper = bool
    case Tag::ID:
    {
        Expression *left = Local();
        Expression *right = nullptr;

        switch (lookahead->tag)
        {
        // assign -> local plusplus
        case Tag::PLUSPLUS:
        case Tag::LESSLESS:
        {
            Expression *constant = new Constant(ExprType::INT, new Token{'1'});
            right = new Arithmetic(left->type, new Token(*lookahead), left, constant);
            Match(lookahead->tag);

            stmt = new Assign(left, right);
            break;
        }

        // assign -> local oper = bool
        case Tag::ATTADD:
        case Tag::ATTSUB:
        case Tag::ATTMUL:
        case Tag::ATTDIV:
        case Tag::ATTREM:
        {
            Token t = *lookahead;
            Match(lookahead->tag);
            right = new Arithmetic(left->type, new Token(t), left, Bool());

            stmt = new Assign(left, right);
            break;
        }
        case Tag::ATTAND:
        case Tag::ATTOR:
        {
            Token t = *lookahead;
            Match(lookahead->tag);
            right = new Logical(new Token(t), left, Bool());

            stmt = new Assign(left, right);
            break;
        }
        case '=':
        {
            Match('=');
            right = Bool();

            stmt = new Assign(left, right);
            break;
        }
        default:
        {
            stringstream ss;
            ss << "não esperado \'" << lookahead->lexeme << "\' após a chamada de " << left->token->lexeme;
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        }

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

Statement *Parser::Scope()
{
    // block -> { stmts }
    //        | if (bool) stmt
    //        | while (bool) stmt
    //        | do stmt while (bool);
    //        | for (decl ; bool ; assign) stmts
    //        | for (assign ; bool ; assign) stmts

    // ------------------------------------
    // nova tabela de símbolos para o bloco
    // ------------------------------------
    SymTable *saved = symTable;
    symTable = new SymTable(symTable);
    // ------------------------------------

    Statement *block = nullptr;

    switch (lookahead->tag)
    {
    // block -> { stmts }
    case '{':
    {
        Match('{');
        block = new Block(Stmts());
        Match('}');
        break;
    }

    // block -> if (bool) stmt
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

        Seq *inst;
        if (lookahead->tag == '{')
        {
            Match('{');
            inst = Stmts();
            if (!Match('}'))
            {
                stringstream ss;
                ss << "esperado } no lugar de  \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }
        else
        {
            inst = new Seq(Stmt());
        }

        Seq *instElse = nullptr;
        if (Match(Tag::ELSE))
        {
            if (lookahead->tag == '{')
            {
                Match('{');
                instElse = Stmts();
                if (!Match('}'))
                {
                    stringstream ss;
                    ss << "esperado } no lugar de  \'" << lookahead->lexeme << "\'";
                    throw SyntaxError{scanner->Lineno(), ss.str()};
                }
            }
            else
            {
                instElse = new Seq(Stmt());
            }
        }

        block = new If(cond, inst, instElse);
        break;
    }

    // block -> while (bool) stmt
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

        if (lookahead->tag == '{')
        {
            Match('{');
            block = new While(cond, Stmts());
            if (!Match('}'))
            {
                stringstream ss;
                ss << "esperado } no lugar de  \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }
        else
        {
            block = new While(cond, new Seq(Stmt()));
        }

        break;
    }

    // block -> do stmt while (bool);
    case Tag::DO:
    {
        Match(Tag::DO);

        Seq *inst;
        if (lookahead->tag == '{')
        {
            Match('{');
            inst = Stmts();
            if (!Match('}'))
            {
                stringstream ss;
                ss << "esperado } no lugar de  \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }
        else
        {
            inst = new Seq(Stmt());
        }

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

        block = new DoWhile(inst, Bool());

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

        break;
    }

    // block -> for (decl ; bool ; assign) stmts
    // block -> for (assign ; bool ; assign) stmts
    case Tag::FOR:
    {
        Match(Tag::FOR);
        if (!Match('('))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        Statement *ctrl;
        switch (lookahead->tag)
        {
        // block -> for (decl ; bool ; assign) stmts
        case Tag::TYPE:
            ctrl = Decl();
            break;
        // block -> for (assign ; bool ; assign) stmts
        case Tag::ID:
            ctrl = Attribution();
            break;
        default:
            stringstream ss;
            ss << "esperado uma variável de controle no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
            break;
        }
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        Expression *cond = Bool();
        if (!Match(';'))
        {
            stringstream ss;
            ss << "esperado ( no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        Statement *icrmt = Attribution();

        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        if (lookahead->tag == '{')
        {
            Match('{');
            block = new For(ctrl, cond, icrmt, Stmts());
            if (!Match('}'))
            {
                stringstream ss;
                ss << "esperado } no lugar de  \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }
        }
        else
        {
            block = new For(ctrl, cond, icrmt, new Seq(Stmt()));
        }

        break;
    }
    }

    // ------------------------------------------------------
    // tabela do escopo envolvente volta a ser a tabela ativa
    // ------------------------------------------------------
    delete symTable;
    symTable = saved;
    // ------------------------------------------------------

    return block;
}

// ---------- Declaration ----------

Statement *Parser::Decl()
{
    // decl  -> type id index valor
    //
    // index -> [ integer ]
    //        | empty
    //
    // valor -> = bool
    //        | empty

    Statement *stmt = nullptr;

    // captura o tipo
    int type = ConvertToExprType(lookahead->lexeme);
    Match(Tag::TYPE);

    // captura nome do identificador
    string name{lookahead->lexeme};
    Match(Tag::ID);

    // cria símbolo
    Symbol s{type, name, SymTable::depth};

    // insere variável na tabela de símbolos
    if (!symTable->Insert(name, s))
    {
        // a inserção falha quando a variável já está na tabela
        stringstream ss;
        ss << "variável \"" << name << "\" já definida.";
        throw SyntaxError(scanner->Lineno(), ss.str());
    }

    for (int i = 0; i < SymTable::depth; i++)
        name += '_';
    funcInfo->InsertLocal(name, s);

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

    return stmt;
}

// ---------- Expression ----------

Expression *Parser::Local()
{
    // local    -> id position
    // position -> [bool]
    //           | empty

    // local -> id position
    if (lookahead->tag == Tag::ID)
    {
        // verifica se a variável existe na tabela de varaiveis
        Symbol *s = symTable->Find(lookahead->lexeme);
        if (!s)
        {
            stringstream ss;
            ss << "variável \"" << lookahead->lexeme << "\" não declarada";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        // identificador
        Expression *expr = new Identifier(s->type, new Token{*lookahead}, s->depth);
        Match(Tag::ID);

        // position -> [bool]
        //           | empty
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

        return expr;
    }
    else
    {
        stringstream ss;
        ss << "esperado um local de armazenamento (variável ou arranjo)";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
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
    //       | % unary calc
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
        // calc -> % unary calc
        else if (lookahead->tag == '%')
        {
            Match('%');
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
        expr = new Constant(ExprType::BOOL, new Token{lookahead->tag, "1"});
        Match(Tag::TRUE);
        break;
    }

    // factor -> false
    case Tag::FALSE:
    {
        expr = new Constant(ExprType::BOOL, new Token{lookahead->tag, "0"});
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

    stringstream ss;
    ss << name;
    Seq *seq = args;
    while (seq != nullptr)
    {
        Expression *param = (Expression *)(seq->elemt);
        ss << param->type;
        seq = seq->elemts;
    }

    // verifica se a função existe na tabela de varaiveis
    Fun *f = funcTable->Find(ss.str());
    if (!f)
    {
        stringstream ss;
        ss << "função \"" << name << "\" com esses parametros não foi declarada";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }

    CallFunc *func = new CallFunc(f->rtr, ss.str(), args);
    return func;
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
