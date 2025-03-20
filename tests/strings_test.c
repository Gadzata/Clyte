#include "utils/util_header.h"

int main()
{
    TEST(0, ""[0]);
    TEST(1, sizeof(""));
    TEST(97, "abc"[0]);
    TEST(98, "abc"[1]);
    TEST(99, "abc"[2]);
    TEST(0, "abc"[3]);
    TEST(4, sizeof("abc"));
    TEST(7, "\a"[0]);
    TEST(8, "\b"[0]);
    TEST(9, "\t"[0]);
    TEST(10, "\n"[0]);
    TEST(11, "\v"[0]);
    TEST(12, "\f"[0]);
    TEST(13, "\r"[0]);
    TEST(27, "\e"[0]);
    TEST(106, "\j"[0]);
    TEST(107, "\k"[0]);
    TEST(108, "\l"[0]);
    TEST(7, "\ax\ny"[0]);
    TEST(120, "\ax\ny"[1]);
    TEST(10, "\ax\ny"[2]);
    TEST(121, "\ax\ny"[3]);

    printf("OK\n");
    return 0;
}