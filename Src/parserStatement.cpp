#include "parser.h"
#include "error.h"
#include <iostream>
#include <sstream>
using std::stringstream;

extern Lexer *scanner;

Statement *Parser::Stmt()
{
    // stmt -> decl;
    //       | assign;
    //       | void_call;
    //       | return bool;
    //       | block

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
    // stmt -> 'cout' , print , {print} , ';'
    case Tag::PRINT:
    {
        Match(Tag::PRINT);

        if (!Match(Tag::LST))
            throw SyntaxError{scanner->Lineno(), "\'<<\' esperado."};

        Seq *seq = new Seq(Operator());
        if (!Match(';'))
            throw SyntaxError{scanner->Lineno(), "\';\' esperado."};

        return new Print(seq);
    }
    // stmt -> return bool;
    case Tag::RETURN:
    {
        Match(Tag::RETURN);

        Statement *st = new Return(Operator());
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
        return NewScope();
    }
    default:
    {
        stringstream ss;
        ss << "esperado uma instrução no lugar de  \'" << lookahead->lexeme << "\'.";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
    }
}

Statement *Parser::Decl()
{
    // decl   = type , id , position , [list]
    //        | type , id , [value] ;
    // list   = '=' , '{' , [operato , {',' , operator}] '}' ;
    // value  = '=' , operator ;

    Statement *stmt = nullptr;

    // captura o tipo
    int type = StringToExprType(lookahead->lexeme);
    Match(Tag::TYPE);

    // captura nome do identificador
    string name{lookahead->lexeme};
    Match(Tag::ID);

    // verifica se é uma declaração de arranjo
    if (Match('['))
    {
        string sizeString{lookahead->lexeme};
        if (!Match(Tag::INTEGER))
        {
            stringstream ss;
            ss << "o índice de um arranjo deve ser um valor inteiro";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        int size = stoi(sizeString);

        // cria símbolo
        Symbol s{type, name, SymTable::depth, memoryPointer};

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

        if (!Match(']'))
        {
            stringstream ss;
            ss << "esperado ] no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }

        // verifica se tem atribuição
        if (Match('='))
        {
            if (!Match('{'))
            {
                stringstream ss;
                ss << "esperado '{' no lugar de \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }

            if (lookahead->tag != '}')
            {
                Seq *seq = nullptr;

                unsigned int aux = 0;
                do
                {
                    Expression *exp = new Expression(NodeType::CONSTANT, ExprType::INT, new Token{Tag::INTEGER, std::to_string(memoryPointer + aux * 4)});
                    Expression *left = new Access(type, new Token{Tag::ID, name}, memoryPointer, exp);
                    Expression *right = Operator();
                    stmt = new Assign(left, right);
                    seq = new Seq(stmt, seq);

                    aux++;
                    if (aux > size)
                    {
                        stringstream ss;
                        ss << "muitos inicializadores para o vetor \'" << ExprTypeToString(type) << " [" << size << "]\'";
                        throw SyntaxError{scanner->Lineno(), ss.str()};
                    }
                } while (Match(','));

                stmt = new SeqAssign(seq);
            }

            if (!Match('}'))
            {
                stringstream ss;
                ss << "esperado '}' no lugar de \'" << lookahead->lexeme << "\'";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            };

            AllocateMemory(type, size);
        }
    }
    else
    {
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

        // verifica se tem atribuição
        if (Match('='))
        {
            Expression *left = new Identifier(type, new Token{Tag::ID, name});
            Expression *right = Operator();
            stmt = new Assign(left, right);
        }
    }

    return stmt;
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
            right = new Arithmetic(left->type, new Token(t), left, Operator());

            stmt = new Assign(left, right);
            break;
        }
        case Tag::ATTAND:
        case Tag::ATTOR:
        {
            Token t = *lookahead;
            Match(lookahead->tag);
            right = new Logical(new Token(t), left, Operator());

            stmt = new Assign(left, right);
            break;
        }
        case '=':
        {
            Match('=');
            right = Operator();

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

Statement *Parser::NewScope()
{
    // newScope  = '{' , stmt , '}'
    //           | 'if' , '(' , operator , ')' , stmt , ['else' , stmt]
    //           | 'while' , '(' , operator')' , stmt
    //           | 'do' , stmt , 'while' , '(' , operator , ')' , ';'
    //           | 'for' , '(' , (decl | assign) , ';' , operator , ';' , assign , ')' , stmt ;

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
        Expression *cond = Operator();
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
        Expression *cond = Operator();
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

        block = new DoWhile(inst, Operator());

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

        Expression *cond = Operator();
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
