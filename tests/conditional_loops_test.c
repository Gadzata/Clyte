#include "utils/util_header.h"

int main()
{
    TEST(4, ({ int x; if (0) x=1; else x=4; x; }));
    TEST(4, ({ int x; if (1-1) x=1; else x=4; x; }));
    TEST(1, ({ int x; if (1) x=1; else x=4; x; }));
    TEST(1, ({ int x; if (2-1) x=1; else x=4; x; }));
    TEST(66, ({ int i=0; int j=0; for (i=0; i<=11; i=i+1) j=i+j; j; }));
    TEST(10, ({ int i=0; while(i<10) i=i+1; i; }));
    TEST(5, ({ 1; {3;} 5; }));
    TEST(6, ({ ;;; 6; }));
    TEST(10, ({ int i=0; while(i<10) i=i+1; i; }));
    TEST(66, ({ int i=0; int j=0; while(i<=11) {j=i+j; i=i+1;} j; }));

    TEST(3, (1, 2, 3));
    TEST(5, ({ int i=2, j=3; (i=5, j=6); i; }));
    TEST(6, ({ int i=2, j=3; (i=5, j=6); j; }));

    printf("OK\n");
    return 0;
}