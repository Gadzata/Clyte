// test_small.c

int main()
{
    int sum = 0;
    int i = 0;
    for (i = 0; i < 1000; i = i + 1)
        sum = sum + i;
    return sum;
}