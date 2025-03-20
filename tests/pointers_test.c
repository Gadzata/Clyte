#include "utils/util_header.h"

int main()
{
    TEST(4, ({ int x=4; *&x; }));
    TEST(4, ({ int x=4; int *y=&x; int **z=&y; **z; }));
    TEST(6, ({ int x=3; int y=6; *(&x+1); }));
    TEST(4, ({ int x=4; int y=5; *(&y-1); }));
    TEST(6, ({ int x=3; int y=6; *(&x-(-1)); }));
    TEST(6, ({ int x=3; int *y=&x; *y=6; x; }));
    TEST(8, ({ int x=3; int y=5; *(&x+1)=8; y; }));
    TEST(8, ({ int x=3; int y=5; *(&y-2+1)=8; x; }));
    TEST(5, ({ int x=3; (&x+2)-&x+3; }));
    TEST(9, ({ int x, y; x=3; y=6; x+y; }));
    TEST(9, ({ int x=4, y=5; x+y; }));

    TEST(4, ({ int x[2]; int *y=&x; *y=4; *x; }));

    TEST(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }));
    TEST(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }));
    TEST(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }));

    TEST(0, ({ int x[2][3]; int *y=x; *y=0; **x; }));
    TEST(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }));
    TEST(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }));
    TEST(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }));
    TEST(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }));
    TEST(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }));

    TEST(2, ({ int x[3]; *x=2; x[1]=4; x[2]=5; *x; }));
    TEST(6, ({ int x[3]; *x=3; x[1]=6; x[2]=5; *(x+1); }));
    TEST(7, ({ int x[3]; *x=3; x[1]=4; x[2]=7; *(x+2); }));
    TEST(7, ({ int x[3]; *x=3; x[1]=4; x[2]=7; *(x+2); }));
    TEST(7, ({ int x[3]; *x=3; x[1]=4; 2[x]=7; *(x+2); }));

    TEST(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }));
    TEST(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }));
    TEST(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }));
    TEST(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }));
    TEST(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }));
    TEST(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }));

    printf("OK\n");
    return 0;
}