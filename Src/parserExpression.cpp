#include "parser.h"
#include "error.h"
#include <iostream>
#include <sstream>
using std::stringstream;

extern Lexer *scanner;

Expression *Parser::Operator()
{
    // operator    = join , {lor}
    // lor         = '||' , join

    Expression *expr1 = Join();

    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == Tag::OR)
        {
            Match(Tag::OR);
            Expression *expr2 = Join();
            expr1 = new Logical(new Token{t}, expr1, expr2);
        }
        else
            break;
    }

    return expr1;
}

Expression *Parser::Join()
{
    // join        = equality , {land}
    // land        = '&&' , equality

    Expression *expr1 = Equality();

    while (true)
    {
        Token t = *lookahead;
        if (lookahead->tag == Tag::AND)
        {
            Match(Tag::AND);
            Expression *expr2 = Equality();
            expr1 = new Logical(new Token{t}, expr1, expr2);
        }
        else
        {
            break;
        }
    }

    return expr1;
}

Expression *Parser::Equality()
{
    // equality    = rel , {eqdif}
    // eqdif       = ('==' | '!=') , rel

    Expression *expr1 = Rel();

    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == Tag::EQ ||
            lookahead->tag == Tag::NEQ)
        {
            Match(lookahead->tag);
            Expression *expr2 = Rel();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else
        {
            break;
        }
    }

    return expr1;
}

Expression *Parser::Rel()
{
    // rel   = ari , {comp}
    // comp  = ('<' | '<=' | '>' | '>=') , ari

    Expression *expr1 = Ari();
    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == '<' ||
            lookahead->tag == Tag::LTE ||
            lookahead->tag == '>' ||
            lookahead->tag == Tag::GTE)
        {
            Match(lookahead->tag);
            Expression *expr2 = Ari();
            expr1 = new Relational(new Token{t}, expr1, expr2);
        }
        else
            break;
    }

    return expr1;
}

Expression *Parser::Ari()
{
    // ari   = term , {oper}
    // oper  = ('+' | '-') , term

    Expression *expr1 = Term();

    while (true)
    {
        Token t = *lookahead;

        // oper = '+' , term
        if (lookahead->tag == '+' ||
            lookahead->tag == '-')
        {
            Match(lookahead->tag);
            Expression *expr2 = Term();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        else
            break;
    }

    return expr1;
}

Expression *Parser::Term()
{
    // term   = unary , {calc}
    // calc   = ('*' | '/' | '%') , unary

    Expression *expr1 = Unary();

    while (true)
    {
        Token t = *lookahead;

        if (lookahead->tag == '*' ||
            lookahead->tag == '/' ||
            lookahead->tag == '%')
        {
            Match(lookahead->tag);
            Expression *expr2 = Unary();
            expr1 = new Arithmetic(expr1->type, new Token{t}, expr1, expr2);
        }
        else
            break;
    }

    return expr1;
}

Expression *Parser::Unary()
{
    // unary  = '!' , unary
    //        | '-' , unary
    //        | factor ;

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
    // factor  = '(' , operator , ')'
    //         | change_one local
    //         | call
    //         | local {change_one}
    //         | integer
    //         | real
    //         | boolean ;

    Expression *expr = nullptr;

    switch (lookahead->tag)
    {
    // factor = '(' , operator , ')'
    case '(':
    {
        Match('(');
        expr = Operator();
        if (!Match(')'))
        {
            stringstream ss;
            ss << "esperado ) no lugar de  \'" << lookahead->lexeme << "\'";
            throw SyntaxError{scanner->Lineno(), ss.str()};
        }
        break;
    }

    // factor = change_one local
    case Tag::PLUSPLUS:
    case Tag::LESSLESS:
    {
        Token tokenAssignment = *lookahead;
        Match(lookahead->tag);

        expr = Local();

        Expression *constant = new Constant(ExprType::INT, new Token{'1'});
        Expression *right = new Arithmetic(expr->type, new Token(tokenAssignment), expr, constant);
        Assign *assign = new Assign(expr, right);

        expr = new AssignPlusExpr(assign, expr);

        break;
    }

    case Tag::ID:
    {
        // factor = call
        if (scanner->Peek() == '(')
            expr = Call();

        // factor = local {change_one}
        else
        {
            expr = Local();

            if (lookahead->tag == Tag::PLUSPLUS || lookahead->tag == Tag::LESSLESS)
            {
                Token tokenAssignment = *lookahead;
                Match(lookahead->tag);

                Expression *constant = new Constant(ExprType::INT, new Token{'1'});
                Expression *right = new Arithmetic(expr->type, new Token(tokenAssignment), expr, constant);
                Assign *assign = new Assign(expr, right);

                expr = new ExprPlusAssign(expr, assign);
            }
        }

        break;
    }

    // factor = integer
    case Tag::INTEGER:
    {
        expr = new Constant(ExprType::INT, new Token{*lookahead});
        Match(Tag::INTEGER);
        break;
    }

    // factor = real
    case Tag::REAL:
    {
        expr = new Constant(ExprType::FLOAT, new Token{*lookahead});
        Match(Tag::REAL);
        break;
    }

    // factor = true
    case Tag::TRUE:
    {
        expr = new Constant(ExprType::BOOL, new Token{lookahead->tag, "1"});
        Match(Tag::TRUE);
        break;
    }

    // factor = false
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

Expression *Parser::Local()
{
    // local     = id , {position}
    // position  = '[' , operator , ']' ;

    // local     = id , {position}
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

        // position  = '[' , operator , ']' ;
        if (Match('['))
        {
            if (!s->isArray)
            {
                stringstream ss;
                ss << "variável \"" << s->name << "\" não é um array";
                throw SyntaxError{scanner->Lineno(), ss.str()};
            }

            expr = new Access(s->type, expr->token, (Identifier *)expr, Operator());

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

Expression *Parser::Call()
{
    // call    = id , '(' , {args} , ')' ;
    // args    = operator , {tailAr} ;
    // tailAr  = ',' operator ;

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
        Expression *bo = Operator();

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
