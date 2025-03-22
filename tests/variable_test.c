#include "utils/util_header.h"

int z, q[4];

int main()
{
    TEST(4, ({ int a; a=4; a; }));
    TEST(4, ({ int a=4; a; }));
    TEST(9, ({ int a=4; int z=5; a+z; }));

    TEST(4, ({ int a=4; a; }));
    TEST(9, ({ int a=4; int z=5; a+z; }));
    TEST(8, ({ int a; int b; a=b=4; a+b; }));
    TEST(4, ({ int foo=4; foo; }));
    TEST(9, ({ int foo123=4; int bar=5; foo123+bar; }));
    TEST(8, ({ int x; sizeof(x); }));
    TEST(8, ({ int x; sizeof x; }));
    TEST(8, ({ int *x; sizeof(x); }));
    TEST(32, ({ int x[4]; sizeof(x); }));
    TEST(96, ({ int x[3][4]; sizeof(x); }));
    TEST(32, ({ int x[3][4]; sizeof(*x); }));
    TEST(8, ({ int x[3][4]; sizeof(**x); }));
    TEST(9, ({ int x[3][4]; sizeof(**x) + 1; }));
    TEST(9, ({ int x[3][4]; sizeof **x + 1; }));
    TEST(8, ({ int x[3][4]; sizeof(**x + 1); }));
    TEST(8, ({ int x=1; sizeof(x=2); }));
    TEST(1, ({ int x=1; sizeof(x=2); x; }));
    TEST(0, z);
    TEST(4, ({ z=4; z; }));
    TEST(0, ({ q[0]=0; q[1]=1; q[2]=2; q[3]=3; q[0]; }));
    TEST(1, ({ q[0]=0; q[1]=1; q[2]=2; q[3]=3; q[1]; }));
    TEST(2, ({ q[0]=0; q[1]=1; q[2]=2; q[3]=3; q[2]; }));
    TEST(3, ({ q[0]=0; q[1]=1; q[2]=2; q[3]=3; q[3]; }));
    TEST(8, sizeof(z));
    TEST(32, sizeof(q));
    TEST(2, ({ char x=2; x; }));
    TEST(2, ({ char x=2; char y=3; x; }));
    TEST(3, ({ char x=2; char y=3; y; }));
    TEST(1, ({ char x; sizeof(x); }));
    TEST(10, ({ char x[10]; sizeof(x); }));
    TEST(4, ({ int x=4; { int x=3; } x; }));
    TEST(4, ({ int x=4; { int x=3; } int y=4; x; }));
    TEST(4, ({ int x=2; { x=4; } x; }));
    TEST(15, ({ int x; int y; char z; char *a=&y; char *b=&z; b-a; }));
    TEST(1, ({ int x; char y; int z; char *a=&y; char *b=&z; b-a; }));

    printf("OK\n");
    return 0;
}