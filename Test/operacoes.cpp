#include <iostream>
using namespace std;

void logicos(bool log)
{
    cout << !log;
    cout << log && true;
    cout << log || false;
}

void relacionais(int valor)
{
    cout << (valor == 5);
    cout << (valor != 5);
    cout << (valor > 5);
    cout << (valor < 5);
    cout << (valor <= 5);
    cout << (valor >= 5);
}

void aritmeticos(int valor)
{
    cout << valor + 10;
    cout << valor - 10;
    cout << valor * 10;
    cout << valor / 10;
    cout << valor % 10;
}

int main()
{
    bool log = true;
    logicos(log);

    int valor = 20;
    relacionais(valor);
    aritmeticos(valor);

    cout << -valor;
}