int main()
{
}

int calc(long base, long exp, long mod)
{
    long result = 1;
    while (exp > 0)
    {
        if ((exp & 1) == 1)
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}
private
static long powMod(long base, long exp, long mod)
{
    long result = 1;
    while (exp > 0)
    {
        if ((exp & 1) == 1)
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}