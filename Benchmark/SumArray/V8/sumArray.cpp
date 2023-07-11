int main()
{
}

int sum(int init, int size)
{
    int list[5] = {1, 1, 1, 1, 1};
    int sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += list[i];
    }
    return sum;
}
