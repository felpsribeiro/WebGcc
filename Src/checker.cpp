#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "ast.h"
using namespace std;

extern ifstream fin;
int depth = 0;

void Tab()
{
    for (int i = 0; i < depth; i++)
        cout << "   ";
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
            Traverse(m->funcs);
            cout << ")" << endl;
            break;
        }
        case FUNC:
        {
            Function *f = (Function *)n;

            depth++;
            Tab();

            // declara função e seu nome
            cout << "(func $" << f->name;

            // parâmetros da função
            Traverse(f->params);

            if (f->type != ExprType::VOID)
                cout << " (result " << ConvertType(f->type) << ")";
            cout << endl;

            depth++;
            Traverse(f->block);
            depth--;

            Tab();
            cout << ")" << endl;
            depth--;
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
            Tab();
            cout << "(" << endl;
            depth++;

            Block *b = (Block *)n;
            for (auto &local : b->table)
            {
                Tab();
                cout << "(local $" << local.second.name;
                cout << " " << ConvertType(local.second.type) << ")" << endl;
            }

            Traverse(b->seq);

            depth--;
            Tab();
            cout << ")" << endl;
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
            Tab();
            cout << "local.set $" << a->id->Name() << endl;
            break;
        }
        case REL:
        {
            Relational *r = (Relational *)n;
            Traverse(r->expr1);
            Traverse(r->expr2);

            Tab();
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
            Tab();
            if (l->token->tag == Tag::OR)
                cout << "i32.or" << endl;
            else if (l->token->tag == Tag::AND)
                cout << "i32.and" << endl;
            break;
        }
        case ARI:
        {
            Arithmetic *a = (Arithmetic *)n;
            Traverse(a->expr1);
            Traverse(a->expr2);

            Tab();
            cout << a->Name() << endl;
            break;
        }
        case UNARY:
        {
            UnaryExpr *u = (UnaryExpr *)n;
            cout << "<UNARY> ";
            cout << u->Name() << " ";
            Traverse(u->expr);
            cout << "</UNARY> ";
            break;
        }
        case CONSTANT:
        {
            Constant *c = (Constant *)n;
            Tab();
            cout << "i32.const " << c->token->lexeme << endl;
            break;
        }
        case IDENTIFIER:
        {
            Identifier *i = (Identifier *)n;
            Tab();
            cout << "local.get $" << i->Name() << endl;
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
            Tab();
            cout << "call $" << a->token->lexeme << endl;
            break;
        }
        case RETURN_STMT:
        {
            Return *r = (Return *)n;
            Traverse(r->expr);
            Tab();
            cout << "return" << endl;
            break;
        }
        case IF_STMT:
        {
            If *i = (If *)n;
            cout << "<IF> ";
            Traverse(i->expr);
            cout << "\n";
            Traverse(i->stmt);
            cout << "</IF> ";
            break;
        }
        case WHILE_STMT:
        {
            While *w = (While *)n;
            cout << "<WHILE> ";
            Traverse(w->expr);
            cout << "\n";
            Traverse(w->stmt);
            cout << "</WHILE> ";
            break;
        }
        case DOWHILE_STMT:
        {
            DoWhile *dw = (DoWhile *)n;
            cout << "<DOWHILE> ";
            Traverse(dw->stmt);
            Traverse(dw->expr);
            cout << "</DOWHILE> ";
            break;
        }
        }
    }
}

void TestParser(Node *n)
{
    Traverse(n);
    cout << endl;
}
