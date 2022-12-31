#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "checker.h"
using namespace std;

extern ifstream fin;
InstructionCounter *count = nullptr;

InstructionCounter::InstructionCounter() : block(0), loop(0), depth(0) {}

string InstructionCounter::Tab()
{
    stringstream ss;
    for (int i = 0; i < depth; i++)
        ss << "   ";
    return ss.str();
}

string InstructionCounter::Tab(unsigned int d)
{
    stringstream ss;
    for (int i = 0; i < d; i++)
        ss << "   ";
    return ss.str();
}

string ConvertType(int type)
{
    switch (type)
    {
    case ExprType::INT:
    case ExprType::BOOL:
        return "i32";
    case ExprType::FLOAT:
        return "f32";
    default:
        return "nulo";
    }
}

void TestLexer()
{
    Lexer scanner;
    Token *t = nullptr;
    while ((t = scanner.Scan()) && (t->tag != EOF))
    {
        switch (t->tag)
        {
        case ID:
            cout << "<ID," << t->lexeme << "> ";
            break;
        case INTEGER:
            cout << "<INTEGER," << t->lexeme << "> ";
            break;
        case REAL:
            cout << "<REAL," << t->lexeme << "> ";
            break;
        case TYPE:
            cout << "<TYPE," << t->lexeme << "> ";
            break;
        case TRUE:
            cout << "<TRUE," << t->lexeme << "> ";
            break;
        case FALSE:
            cout << "<FALSE," << t->lexeme << "> ";
            break;
        case MAIN:
            cout << "<MAIN," << t->lexeme << "> ";
            break;
        case IF:
            cout << "<IF," << t->lexeme << "> ";
            break;
        case WHILE:
            cout << "<WHILE," << t->lexeme << "> ";
            break;
        case DO:
            cout << "<DO," << t->lexeme << "> ";
            break;
        case OR:
            cout << "<OR," << t->lexeme << "> ";
            break;
        case AND:
            cout << "<AND," << t->lexeme << "> ";
            break;
        case EQ:
            cout << "<EQ," << t->lexeme << "> ";
            break;
        case NEQ:
            cout << "<NEQ," << t->lexeme << "> ";
            break;
        case LTE:
            cout << "<LTE," << t->lexeme << "> ";
            break;
        case GTE:
            cout << "<GTE," << t->lexeme << "> ";
            break;
        default:
            cout << "<" << t->lexeme << "> ";
            break;
        }
    }

    fin.clear();
    fin.seekg(0, ios::beg);
    cout << endl
         << endl;
}

void Traverse(Node *n)
{
    if (n)
    {
        switch (n->node_type)
        {
        case PROGRAM:
        {
            cout << "(module" << endl;
            Program *m = (Program *)n;
            count->depth++;
            Traverse(m->funcs);
            count->depth--;
            cout << ")" << endl;
            break;
        }
        case FUNC:
        {
            Function *f = (Function *)n;

            // declara função e seu nome
            cout << count->Tab() << "(func $" << f->name;

            // parâmetros da função
            Traverse(f->params);

            if (f->type != ExprType::VOID)
                cout << " (result " << ConvertType(f->type) << ")";
            cout << endl;

            count->depth++;
            Traverse(f->block);
            count->depth--;

            cout << count->Tab() << ")" << endl;
            break;
        }
        case PARAM:
        {
            Param *p = (Param *)n;
            cout << " (param $" << p->name << " " << ConvertType(p->type) << ")";
            break;
        }
        case BLOCK:
        {
            cout << count->Tab() << "(" << endl;

            Block *b = (Block *)n;
            for (auto &local : b->table)
            {
                cout << count->Tab() << "(local $" << local.second.name;
                cout << " " << ConvertType(local.second.type) << ")" << endl;
            }

            count->depth++;
            Traverse(b->seq);
            count->depth--;

            cout << count->Tab() << ")" << endl;
            break;
        }
        case STRUC:
        {
            cout << count->Tab() << "(" << endl;

            Struc *b = (Struc *)n;
            for (auto &local : b->table)
            {
                cout << count->Tab() << "(local $" << local.second.name;
                cout << " " << ConvertType(local.second.type) << ")" << endl;
            }

            count->depth++;
            Traverse(b->stmt);
            count->depth--;

            cout << count->Tab() << ")" << endl;
            break;
        }
        case SEQ:
        {
            Seq *s = (Seq *)n;
            Traverse(s->elemt);
            Traverse(s->elemts);
            break;
        }
        case ASSIGN:
        {
            Assign *a = (Assign *)n;
            // não utilizo enquanto nao estou lidando com arrays
            // Traverse(a->id);
            Traverse(a->expr);
            cout << count->Tab() << "local.set $" << a->id->Name() << endl;
            break;
        }
        case REL:
        {
            Relational *r = (Relational *)n;
            Traverse(r->expr1);
            Traverse(r->expr2);

            cout << count->Tab();
            switch (r->token->tag)
            {
            case Tag::EQ:
                cout << "i32.eq" << endl;
                break;
            case Tag::NEQ:
                cout << "i32.ne" << endl;
                break;
            case '<':
                cout << "i32.lt" << endl;
                break;
            case Tag::LTE:
                cout << "i32.le" << endl;
                break;
            case '>':
                cout << "i32.gt" << endl;
                break;
            case Tag::GTE:
                cout << "i32.ge" << endl;
                break;
            }
            break;
        }
        case LOG:
        {
            Logical *l = (Logical *)n;
            Traverse(l->expr1);
            Traverse(l->expr2);

            cout << count->Tab();
            switch (l->token->tag)
            {
            case Tag::OR:
            case Tag::ATTOR:
                cout << "i32.or" << endl;
                break;
            case Tag::AND:
            case Tag::ATTAND:
                cout << "i32.and" << endl;
                break;
            default:
                cout << "erro na compilação" << endl;
                break;
            }

            break;
        }
        case ARI:
        {
            Arithmetic *a = (Arithmetic *)n;
            Traverse(a->expr1);
            Traverse(a->expr2);

            cout << count->Tab();
            switch (a->token->tag)
            {
            case '+':
            case Tag::PLUSPLUS:
            case Tag::ATTADD:
                cout << "i32.add" << endl;
                break;
            case '-':
            case Tag::LESSLESS:
            case Tag::ATTSUB:
                cout << "i32.sub" << endl;
                break;
            case '*':
            case Tag::ATTMUL:
                cout << "i32.mul" << endl;
                break;
            case '/':
            case Tag::ATTDIV:
                cout << "i32.div_s" << endl;
                break;
            default:
                cout << "erro na compilação" << endl;
                break;
            }

            break;
        }
        case UNARY:
        {
            UnaryExpr *u = (UnaryExpr *)n;
            Traverse(u->expr);

            cout << count->Tab();
            switch (u->type)
            {
            case ExprType::INT:
            case ExprType::FLOAT:
                cout << "i32.const -1" << endl;
                cout << "i32.mul" << endl;
                break;
            case ExprType::BOOL:
                cout << "i32.eqz" << endl;
                break;
            }

            break;
        }
        case CONSTANT:
        {
            Constant *c = (Constant *)n;
            cout << count->Tab() << "i32.const " << c->token->lexeme << endl;
            break;
        }
        case IDENTIFIER:
        {
            Identifier *i = (Identifier *)n;
            cout << count->Tab() << "local.get $" << i->token->lexeme << endl;
            break;
        }
        case ACCESS:
        {
            Access *a = (Access *)n;
            Traverse(a->id);
            cout << "[ ";
            Traverse(a->expr);
            cout << "] ";
            break;
        }
        case CALL:
        {
            CallFunc *a = (CallFunc *)n;
            Traverse(a->args);
            cout << count->Tab() << "call $" << a->token->lexeme << endl;
            break;
        }
        case RETURN_STMT:
        {
            Return *r = (Return *)n;
            Traverse(r->expr);
            cout << count->Tab() << "return" << endl;
            break;
        }
        case IF_STMT:
        {
            If *i = (If *)n;

            Traverse(i->expr);
            cout << count->Tab(count->depth++) << "(if" << endl;
            cout << count->Tab(count->depth++) << "(then" << endl;
            Traverse(i->stmt);
            cout << count->Tab(count->depth--) << ")" << endl;

            if (i->stmtElse)
            {
                cout << count->Tab(count->depth++) << "(else" << endl;
                Traverse(i->stmtElse);
                cout << count->Tab(count->depth--) << ")" << endl;
            }

            cout << count->Tab(count->depth--) << ")" << endl;

            break;
        }
        case WHILE_STMT:
        {
            While *w = (While *)n;

            cout << count->Tab(count->depth++) << "(block $block_" << count->block << endl;
            cout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;

            Traverse(w->expr);
            // inverte o resultado da condição
            cout << count->Tab() << "i32.eqz" << endl;
            cout << count->Tab() << "br_if $block_" << count->block << endl << endl;

            Traverse(w->stmt);
            cout << count->Tab() << "br $loop_" << count->loop << endl;
            count->depth--;
            cout << count->Tab() << ")" << endl;
            count->depth--;
            cout << count->Tab() << ")" << endl;

            break;
        }
        case DOWHILE_STMT:
        {
            DoWhile *dw = (DoWhile *)n;
            count->block++;
            count->loop++;
            
            cout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;
            Traverse(dw->stmt);
            cout << endl;
            Traverse(dw->expr);
            cout << count->Tab() << "br_if $loop_" << count->loop << endl;
            count->depth--;
            cout << count->Tab() << ")" << endl;

            break;
        }
        case FOR_STMT:
        {
            For *f = (For *)n;
            count->block++;
            count->loop++;

            Traverse(f->ctrl);

            cout << count->Tab(count->depth++) << "(block $block_" << count->block << endl;
            cout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;

            Traverse(f->cond);
            // inverte o resultado da condição
            cout << count->Tab() << "i32.eqz" << endl;
            cout << count->Tab() << "br_if $block_" << count->block << endl << endl;

            Traverse(f->stmt);
            cout << endl;
            
            Traverse(f->icrmt);
            cout << count->Tab() << "br $loop_" << count->loop << endl;
            count->depth--;
            cout << count->Tab() << ")" << endl;
            count->depth--;
            cout << count->Tab() << ")" << endl;

            break;
        }
        }
    }
}

void TestParser(Node *n)
{
    count = new InstructionCounter();
    Traverse(n);
    cout << endl;
}
