#include "utils/util_header.h"

int main()
{
    TEST(8, ({ union { int a; char b[6]; } x; sizeof(x); }));
    TEST(3, ({ union { int a; char b[4]; } x; x.a = 515; x.b[0]; }));
    TEST(2, ({ union { int a; char b[4]; } x; x.a = 515; x.b[1]; }));
    TEST(0, ({ union { int a; char b[4]; } x; x.a = 515; x.b[2]; }));
    TEST(0, ({ union { int a; char b[4]; } x; x.a = 515; x.b[3]; }));
    TEST(3, ({ union {int a,b;} x,y; x.a=3; y.a=5; y=x; y.a; }));
    TEST(3, ({ union {struct {int a,b;} c;} x,y; x.c.b=3; y.c.b=5; y=x; y.c.b; }));

    printf("OK\n");
    return 0;
}