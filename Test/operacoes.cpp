void logicos(bool log)
{
    bool nao = !log;
    bool e = log && true;
    bool ou = log || false;
}

void relacionais(int valor)
{
    bool igual = valor == 5;
    bool diferente = valor != 5;
    bool ehMaior = valor > 5;
    bool ehMenor = valor < 5;
    bool ehMaiorIgual = valor <= 5;
    bool ehMenorIgual = valor >= 5;
}

void aritmeticos(int valor)
{
    int soma = valor + 10;
    int subt = valor - 10;
    int mult = valor * 10;
    int devi = valor / 10;
    int rest = valor % 10;
}

int main()
{
    bool log = true;
    logicos(log);

    int valor = 10;
    relacionais(valor);
    aritmeticos(valor);

    int inve = -valor;
}