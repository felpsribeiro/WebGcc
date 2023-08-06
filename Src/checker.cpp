#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "checker.h"
#include "error.h"
using namespace std;

extern ifstream fin;
ofstream fout;

unsigned int Counter::block = 0;
unsigned int Counter::loop = 0;
unsigned int Counter::depth = 0;

void Counter::ftab(int add = 0)
{
    depth += add;
    stringstream ss;
    for (int i = 0; i < depth; i++)
        fout << "   ";
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
            Counter::ftab(1);
            fout << "(import \"console\" \"log\" (func $log (param i32)))" << endl;
            Counter::ftab();
            fout << "(memory 1)";
            Traverse(p->seq);
            fout << ")" << endl;
            break;
        }
        case COUT:
        {
            // Counter::ftab();
            // fout << "(import \"console\" \"log\" (func $log (param i32)))" << endl
            //      << endl;
            break;
        }
        case FUNC:
        {
            Function *f = (Function *)n;

            fout << endl;
            Counter::ftab();
            // declara função e seu nome
            fout << "(func $" << f->info->key << " (export \"" << f->info->name << "\")";

            // parâmetros da função
            for (auto &local : f->info->params)
            {
                fout << " (param $" << local.first << " " << ConvertType(local.second.type) << ")";
            }

            if (f->info->rtr != ExprType::VOID)
                fout << " (result " << ConvertType(f->info->rtr) << ")";

            Counter::depth++;
            if (!f->info->locals.empty())
            {
                fout << endl;
                Counter::ftab();
                for (auto &local : f->info->locals)
                {
                    fout << "(local $" << local.first << " " << ConvertType(local.second.type) << ") ";
                }
            }

            Traverse(f->seq);
            fout << ")";
            Counter::depth--;
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
            // fout << " ";
            Traverse(s->elemts);
            break;
        }
        case ASSIGN:
        {
            Assign *a = (Assign *)n;

            fout << endl;
            if (a->id->node_type == NodeType::ACCESS)
            {
                Access *v = (Access *)a->id;
                Counter::ftab();
                fout << "(i32.store " << endl;
                Traverse(v->expr);
                Traverse(a->expr);
            }
            else
            {
                Identifier *i = (Identifier *)a->id;
                Counter::ftab();
                fout << "(local.set $" << i->key << " ";
                Counter::depth++;
                Traverse(a->expr);
                Counter::depth--;
                fout << ")";
            }
            break;
        }
        case REL:
        {
            Relational *r = (Relational *)n;

            fout << endl;
            Counter::ftab();
            switch (r->token->tag)
            {
            case Tag::EQ:
                fout << "(i32.eq ";
                break;
            case Tag::NEQ:
                fout << "(i32.ne ";
                break;
            case '<':
                fout << "(i32.lt_s ";
                break;
            case Tag::LTE:
                fout << "(i32.le_s ";
                break;
            case '>':
                fout << "(i32.gt_s ";
                break;
            case Tag::GTE:
                fout << "(i32.ge_s ";
                break;
            }

            Traverse(r->expr1);
            fout << " ";
            Traverse(r->expr2);
            fout << ")";

            break;
        }
        case LOGI:
        {
            Logical *l = (Logical *)n;

            fout << endl;
            Counter::ftab();
            switch (l->token->tag)
            {
            case Tag::OR:
            case Tag::ATTOR:
                fout << "(i32.or ";
                break;
            case Tag::AND:
            case Tag::ATTAND:
                fout << "(i32.and ";
                break;
            default:
                fout << "erro na compilação" << endl;
                break;
            }

            Traverse(l->expr1);
            fout << " ";
            Traverse(l->expr2);
            fout << ")";

            break;
        }
        case ARI:
        {
            Arithmetic *a = (Arithmetic *)n;

            fout << endl;
            Counter::ftab();
            switch (a->token->tag)
            {
            case '+':
            case Tag::PLUSPLUS:
            case Tag::ATTADD:
                fout << "(i32.add ";
                break;
            case '-':
            case Tag::LESSLESS:
            case Tag::ATTSUB:
                fout << "(i32.sub ";
                break;
            case '*':
            case Tag::ATTMUL:
                fout << "(i32.mul ";
                break;
            case '/':
            case Tag::ATTDIV:
                fout << "(i32.div_s ";
                break;
            case '%':
                fout << "(i32.rem_s ";
                break;
            default:
                fout << "erro na compilação" << endl;
                break;
            }

            Traverse(a->expr1);
            fout << " ";
            Traverse(a->expr2);
            fout << ")";

            break;
        }
        case UNARY:
        {
            UnaryExpr *u = (UnaryExpr *)n;

            fout << endl;
            switch (u->type)
            {
            case ExprType::INT:
            case ExprType::FLOAT:
                Counter::ftab();
                fout << "(i32.mul (i32.const -1) ";
                break;
            case ExprType::BOOL:
                Counter::ftab();
                fout << "(i32.eqz ";
                break;
            }

            Traverse(u->expr);
            fout << ")";

            break;
        }
        case CONSTANT:
        {
            Constant *c = (Constant *)n;
            fout << "(i32.const " << c->Name() << ")";
            break;
        }
        case IDENTIFIER:
        {
            Identifier *i = (Identifier *)n;
            fout << "(local.get $" << i->key << ")";
            break;
        }
        case ACCESS:
        {
            Access *a = (Access *)n;

            Counter::ftab();
            fout << "(i32.add" << endl;
            Traverse(a->addres);
            Counter::ftab();
            fout << "(i32.load" << endl;
            Counter::ftab();
            fout << "(i32.mul (i32.const 4) " << endl;
            Traverse(a->expr);
            fout << ")" << endl;
            break;
        }
        case EXPR_ASSIGN:
        {
            ExprPlusAssign *ea = (ExprPlusAssign *)n;

            Traverse(ea->expr);
            fout << endl;
            Counter::ftab();
            Traverse(ea->assign);
            break;
        }
        case ASSIGN_EXPR:
        {
            AssignPlusExpr *ea = (AssignPlusExpr *)n;

            Traverse(ea->assign);
            fout << endl;
            Counter::ftab();
            Traverse(ea->expr);
            break;
        }
        case LOG:
        {
            Print *p = (Print *)n;

            fout << endl;
            Counter::ftab();
            fout << "(call $log ";
            Counter::depth++;
            Traverse(p->args);
            Counter::depth--;
            fout << ")";
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

            fout << endl;
            Counter::ftab();
            fout << "(call $" << a->Name() << " ";
            Counter::depth++;
            Traverse(a->args);
            Counter::depth--;
            fout << ")";
            break;
        }
        case RETURN_STMT:
        {
            Return *r = (Return *)n;
            // Counter::ftab();
            fout << "(return ";
            Counter::depth++;
            Traverse(r->expr);
            Counter::depth--;
            fout << ")";
            break;
        }
        case IF_STMT:
        {
            If *i = (If *)n;

            Traverse(i->expr);

            fout << endl;
            Counter::ftab();
            fout << "(if (result i32)" << endl;

            Counter::ftab(1);
            fout << "(then";
            Counter::depth++;
            Traverse(i->stmt);
            Counter::depth--;
            fout << ")";

            if (i->stmtElse)
            {
                fout << endl;
                Counter::ftab();
                fout << "(else";
                Counter::depth++;
                Traverse(i->stmtElse);
                Counter::depth--;
                fout << ")";
            }

            Counter::depth--;
            fout << ")";

            break;
        }
        case WHILE_STMT:
        {
            While *w = (While *)n;

            Counter::ftab(1);
            fout << "(block $block_" << Counter::block << endl;
            Counter::ftab(1);
            fout << "(loop $loop_" << Counter::loop << endl;

            // inverte o resultado da condição
            Counter::ftab();
            fout << "(br_if $block_" << Counter::block << endl;
            Counter::ftab();
            fout << "(i32.eqz" << endl;
            Traverse(w->expr);
            fout << endl;

            Traverse(w->stmt);

            Counter::ftab();
            fout << "(br $loop_" << Counter::loop << ")" << endl;
            Counter::ftab(-1);
            Counter::ftab();
            fout << ")" << endl;
            Counter::ftab(-1);
            Counter::ftab();
            fout << ")" << endl;

            break;
        }
        case DOWHILE_STMT:
        {
            DoWhile *dw = (DoWhile *)n;
            Counter::block++;
            Counter::loop++;

            Counter::ftab(1);
            fout << "(loop $loop_" << Counter::loop << endl;
            Traverse(dw->stmt);
            fout << endl;
            Traverse(dw->expr);
            Counter::ftab();
            fout << "(br_if $loop_" << Counter::loop << endl;
            Counter::ftab(-1);
            Counter::ftab();
            fout << ")" << endl;

            break;
        }
        case FOR_STMT:
        {
            For *f = (For *)n;
            Counter::block++;
            Counter::loop++;

            Traverse(f->ctrl);
            fout << endl;
            Counter::ftab();
            fout << "(block $block_" << Counter::block << endl;
            Counter::ftab(1);
            fout << "(loop $loop_" << Counter::loop << endl;

            // inverte o resultado da condição
            Counter::ftab(1);
            fout << "(br_if $block_" << Counter::block << endl;
            Counter::ftab(1);
            fout << "(i32.eqz";
            Counter::depth++;
            Traverse(f->cond);
            Counter::depth -= 2;
            fout << "))" << endl;

            Traverse(f->stmt);
            fout << endl;

            Traverse(f->icrmt);
            fout << endl;
            Counter::ftab();
            fout << "(br $loop_" << Counter::loop << "))";
            Counter::depth -= 3;

            break;
        }
        }
    }
}

void TestParser(Node *n, string filename)
{
    fout.open(filename);
    if (!fout.good())
        throw OpenFileError(filename);

    Traverse(n);
    fout.close();
    cout << "Arquivo salvo!" << endl;
}
