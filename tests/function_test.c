#include "utils/util_header.h"

int return_test()
{
    return 4;
    return 6;
}

int add(int x, int y)
{
    return x + y;
}

int subtract(int x, int y)
{
    return x - y;
}

int add_all_numbers(int x, int y, int z, int q, int r, int s)
{
    return x + y + z + q + r + s;
}

int addx(int *x, int y)
{
    return *x + y;
}

int subtract_char(char x, char y, char z)
{
    return x - y - z;
}

int fibonacci(int x)
{
    if (x <= 1)
        return 1;
    return fibonacci(x - 1) + fibonacci(x - 2);
}

int main()
{
    TEST(4, return_test());
    TEST(9, add(4, 5));
    TEST(3, subtract(6, 3));
    TEST(27, add_all_numbers(1, 2, 3, 4, 5, 12));
    TEST(67, add_all_numbers(1, 2, add_all_numbers(3, 4, 5, 6, 7, 8), 9, 10, 12));
    TEST(137, add_all_numbers(1, 2, add_all_numbers(3, add_all_numbers(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 17));
    TEST(8, add(3, 5));
    TEST(2, subtract(5, 3));
    TEST(55, fibonacci(9));
    TEST(1, ({ subtract_char(7, 3, 3); }));

    printf("OK\n");
    return 0;
}