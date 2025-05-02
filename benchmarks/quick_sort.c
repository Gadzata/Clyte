int sum_up_to(int n)
{
    int sum = 0;
    int i = 0;
    for (i = 0; i <= n; i = i + 1)
    {
        sum = sum + i;
    }
    return sum;
}

int multiply(int a, int b)
{
    return a * b;
}

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int square(int x)
{
    return x * x;
}

int main()
{
    int result = 0;
    int i = 0;
    for (i = 0; i < 1000; i = i + 1)
    {
        result = result + sum_up_to(i);
        result = result + multiply(i, i);
        result = result + square(i);
        result = result + fib(i / 10);
    }

    for (i = 0; i < 200; i = i + 1)
    {
        result = result + i * 3 - i / 2 + i;
    }

    return result;
}