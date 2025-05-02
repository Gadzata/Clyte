#include "utils/util_header.h"

int main()
{
    TEST_PROGRAM(0, "int main() { return 0; }");
    TEST_PROGRAM(42, "int main() { return 42; }");
    TEST_PROGRAM(21, "int main() { return 5+20-4; }");
    TEST_PROGRAM(41, "int main() { return 12 + 34 - 5; }");
    TEST_PROGRAM(47, "int main() { return 5+6*7; }");
    TEST_PROGRAM(15, "int main() { return 5*(9-6); }");
    TEST_PROGRAM(4, "int main() { return (3+5)/2; }");
    TEST_PROGRAM(10, "int main() { return -10+20; }");
    TEST_PROGRAM(10, "int main() { return - -10; }");
    TEST_PROGRAM(10, "int main() { return - - +10; }");

    TEST_PROGRAM(1, "int main() { return 42==42; }");
    TEST_PROGRAM(0, "int main() { return 42!=42; }");
    TEST_PROGRAM(1, "int main() { return 0!=1; }");
    TEST_PROGRAM(0, "int main() { return 2<1; }");
    TEST_PROGRAM(1, "int main() { return 1<=1; }");
    TEST_PROGRAM(0, "int main() { return 3<=1; }");

    TEST_PROGRAM(3, "int main() { int a; a=3; return a; }");
    TEST_PROGRAM(8, "int main() { int a=3; int z=5; return a+z; }");
    TEST_PROGRAM(6, "int main() { int a; int b; a=b=3; return a+b; }");

    TEST_PROGRAM(3, "int main() { if (0) return 2; return 3; }");
    TEST_PROGRAM(2, "int main() { if (2-1) return 2; return 3; }");

    TEST_PROGRAM(55, "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }");
    TEST_PROGRAM(10, "int main() { int i=0; while(i<10) i=i+1; return i; }");

    TEST_PROGRAM(5, "int main() { ;;; return 5; }");
    TEST_PROGRAM(3, "int main() { {1; {2;} return 3;} }");
    TEST_PROGRAM(8, "int main() { int x=3, y=5; return x+y; }");

    printf("All TEST_PROGRAMs passed!\n");
    return 0;
}