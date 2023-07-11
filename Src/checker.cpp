#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "checker.h"
#include "error.h"
using namespace std;

extern ifstream fin;
ofstream fout;

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
            fout << "(module" << endl;
            Program *p = (Program *)n;
            count->depth++;
            fout << count->Tab() << "(import \"console\" \"log\" (func $log (param i32)))" << endl;
            fout << count->Tab() << "(memory 1)" << endl;
            Traverse(p->seq);
            fout << count->Tab() << "(start $main)" << endl;
            count->depth--;
            fout << ")" << endl;
            break;
        }
        case COUT:
        {
            // fout << count->Tab() << "(import \"console\" \"log\" (func $log (param i32)))" << endl
            //      << endl;
            break;
        }
        case FUNC:
        {
            Function *f = (Function *)n;

            // declara função e seu nome
            fout << count->Tab() << "(func $" << f->info->key;

            // parâmetros da função
            for (auto &local : f->info->params)
            {
                fout << " (param $" << local.first << " " << ConvertType(local.second.type) << ")";
            }

            if (f->info->rtr != ExprType::VOID)
                fout << " (result " << ConvertType(f->info->rtr) << ")";

            fout << endl;

            count->depth++;
            fout << count->Tab();
            for (auto &local : f->info->locals)
            {
                fout << "(local $" << local.first;
                fout << " " << ConvertType(local.second.type) << ") ";
            }
            fout << endl;

            Traverse(f->seq);
            count->depth--;

            fout << count->Tab() << ")" << endl;
            break;
        }
        case BLOCK:
        {
            Block *b = (Block *)n;
            Traverse(b->seq);
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

            if (a->id->node_type == NodeType::ACCESS)
            {
                Access *v = (Access *)a->id;
                Traverse(v->expr);
                Traverse(a->expr);
                fout << count->Tab() << "i32.store" << endl;
            }
            else
            {
                Traverse(a->expr);
                Identifier *i = (Identifier *)a->id;
                fout << count->Tab() << "local.set $" << i->key << endl;
            }
            break;
        }
        //case ASSIGN_VECTOR:

        case REL:
        {
            Relational *r = (Relational *)n;
            Traverse(r->expr1);
            Traverse(r->expr2);

            fout << count->Tab();
            switch (r->token->tag)
            {
            case Tag::EQ:
                fout << "i32.eq" << endl;
                break;
            case Tag::NEQ:
                fout << "i32.ne" << endl;
                break;
            case '<':
                fout << "i32.lt_s" << endl;
                break;
            case Tag::LTE:
                fout << "i32.le_s" << endl;
                break;
            case '>':
                fout << "i32.gt_s" << endl;
                break;
            case Tag::GTE:
                fout << "i32.ge_s" << endl;
                break;
            }
            break;
        }
        case LOGI:
        {
            Logical *l = (Logical *)n;
            Traverse(l->expr1);
            Traverse(l->expr2);

            fout << count->Tab();
            switch (l->token->tag)
            {
            case Tag::OR:
            case Tag::ATTOR:
                fout << "i32.or" << endl;
                break;
            case Tag::AND:
            case Tag::ATTAND:
                fout << "i32.and" << endl;
                break;
            default:
                fout << "erro na compilação" << endl;
                break;
            }

            break;
        }
        case ARI:
        {
            Arithmetic *a = (Arithmetic *)n;
            Traverse(a->expr1);
            Traverse(a->expr2);

            fout << count->Tab();
            switch (a->token->tag)
            {
            case '+':
            case Tag::PLUSPLUS:
            case Tag::ATTADD:
                fout << "i32.add" << endl;
                break;
            case '-':
            case Tag::LESSLESS:
            case Tag::ATTSUB:
                fout << "i32.sub" << endl;
                break;
            case '*':
            case Tag::ATTMUL:
                fout << "i32.mul" << endl;
                break;
            case '/':
            case Tag::ATTDIV:
                fout << "i32.div_s" << endl;
                break;
            case '%':
                fout << "i32.rem_s" << endl;
                break;
            default:
                fout << "erro na compilação" << endl;
                break;
            }

            break;
        }
        case UNARY:
        {
            UnaryExpr *u = (UnaryExpr *)n;
            Traverse(u->expr);

            switch (u->type)
            {
            case ExprType::INT:
            case ExprType::FLOAT:
                fout << count->Tab() << "i32.const -1" << endl;
                fout << count->Tab() << "i32.mul" << endl;
                break;
            case ExprType::BOOL:
                fout << count->Tab() << "i32.eqz" << endl;
                break;
            }

            break;
        }
        case CONSTANT:
        {
            Constant *c = (Constant *)n;
            fout << count->Tab() << "i32.const " << c->Name() << endl;
            break;
        }
        case IDENTIFIER:
        {
            Identifier *i = (Identifier *)n;
            fout << count->Tab() << "local.get $" << i->key << endl;
            break;
        }
        case ACCESS:
        {
            Access *a = (Access *)n;

            Traverse(a->expr);
            fout << count->Tab() << "i32.const 4" << endl;
            fout << count->Tab() << "i32.mul" << endl;
            if (a->addres != 0)
            {
                fout << count->Tab() << "i32.const " << a->addres << endl;
                fout << count->Tab() << "i32.add" << endl;
            }

            fout << count->Tab() << "i32.load" << endl;
            break;
        }
        case LOG:
        {
            Print *p = (Print *)n;
            Traverse(p->args);
            fout << count->Tab() << "call $log" << endl;
            break;
        }
        case EXECUTE:
        {
            Execute *e = (Execute *)n;
            Traverse(e->func);
            break;
        }
        case CALL:
        {
            CallFunc *a = (CallFunc *)n;
            Traverse(a->args);
            fout << count->Tab() << "call $" << a->Name() << endl;
            break;
        }
        case RETURN_STMT:
        {
            Return *r = (Return *)n;
            Traverse(r->expr);
            fout << count->Tab() << "return" << endl;
            break;
        }
        case IF_STMT:
        {
            If *i = (If *)n;

            Traverse(i->expr);
            fout << count->Tab(count->depth++) << "(if (result i32)" << endl;

            fout << count->Tab(count->depth++) << "(then" << endl;
            Traverse(i->stmt);
            count->depth--;
            fout << count->Tab() << ")" << endl;

            if (i->stmtElse)
            {
                fout << count->Tab(count->depth++) << "(else" << endl;
                Traverse(i->stmtElse);
                count->depth--;
                fout << count->Tab() << ")" << endl;
            }

            count->depth--;
            fout << count->Tab() << ")" << endl;

            break;
        }
        case WHILE_STMT:
        {
            While *w = (While *)n;

            fout << count->Tab(count->depth++) << "(block $block_" << count->block << endl;
            fout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;

            Traverse(w->expr);
            // inverte o resultado da condição
            fout << count->Tab() << "i32.eqz" << endl;
            fout << count->Tab() << "br_if $block_" << count->block << endl
                 << endl;

            Traverse(w->stmt);
            fout << count->Tab() << "br $loop_" << count->loop << endl;
            count->depth--;
            fout << count->Tab() << ")" << endl;
            count->depth--;
            fout << count->Tab() << ")" << endl;

            break;
        }
        case DOWHILE_STMT:
        {
            DoWhile *dw = (DoWhile *)n;
            count->block++;
            count->loop++;

            fout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;
            Traverse(dw->stmt);
            fout << endl;
            Traverse(dw->expr);
            fout << count->Tab() << "br_if $loop_" << count->loop << endl;
            count->depth--;
            fout << count->Tab() << ")" << endl;

            break;
        }
        case FOR_STMT:
        {
            For *f = (For *)n;
            count->block++;
            count->loop++;

            Traverse(f->ctrl);

            fout << count->Tab(count->depth++) << "(block $block_" << count->block << endl;
            fout << count->Tab(count->depth++) << "(loop $loop_" << count->loop << endl;

            Traverse(f->cond);
            // inverte o resultado da condição
            fout << count->Tab() << "i32.eqz" << endl;
            fout << count->Tab() << "br_if $block_" << count->block << endl
                 << endl;

            Traverse(f->stmt);
            fout << endl;

            Traverse(f->icrmt);
            fout << count->Tab() << "br $loop_" << count->loop << endl;
            count->depth--;
            fout << count->Tab() << ")" << endl;
            count->depth--;
            fout << count->Tab() << ")" << endl;

            break;
        }
        }
    }
}

void TestParser(Node *n, string filename)
{
    count = new InstructionCounter();

    fout.open(filename);
    if (!fout.good())
        throw OpenFileError(filename);

    Traverse(n);
    fout.close();
    cout << "Arquivo salvo!" << endl;
}
