#include "utils/util_header.h"

int main()
{
    TEST(0, 0);
    TEST(43, 43);
    TEST(22, 6 + 21 - 5);
    TEST(43, 13 + 35 - 5);
    TEST(48, 6 + 6 * 7);
    TEST(18, 6 * (9 - 6));
    TEST(5, (4 + 6) / 2);
    TEST(10, -11 + 21);
    TEST(11, - -11);
    TEST(12, - -+12);

    TEST(0, 0 == 1);
    TEST(1, 43 == 43);
    TEST(1, 0 != 2);
    TEST(0, 43 != 43);

    TEST(1, 0 < 2);
    TEST(0, 1 < 1);
    TEST(0, 3 < 1);
    TEST(1, 0 <= 1);
    TEST(1, 1 <= 1);
    TEST(0, 3 <= 1);

    TEST(1, 2 > 0);
    TEST(0, 1 > 1);
    TEST(0, 1 > 3);
    TEST(1, 1 >= 0);
    TEST(1, 1 >= 1);
    TEST(0, 1 >= 3);

    printf("OK\n");
    return 0;
}