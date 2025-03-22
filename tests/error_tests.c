#include "utils/util_header.h"

int main()
{

    TEST_ERROR("int main() { return ; }");
    TEST_ERROR("int main( { return 1; }");
    TEST_ERROR("int main() return 1; }");
    TEST_ERROR("int main() { int x = ; return x; }");
    TEST_ERROR("int main() { int x = 10 return x; }");
    TEST_ERROR("int main() { if (1) return 1 else return 0; }");
    TEST_ERROR("int main() { int x = 5; int y = ; return x + y; }");
    TEST_ERROR("int main() { int x; return x + ; }");
    TEST_ERROR("int main() { int 1x = 5; return 1x; }");
    TEST_ERROR("int main() { char x = \"unterminated string; return 1; }");

    TEST_ERROR("int main() { return (5 + ) 3; }");
    TEST_ERROR("int main() { while return 1; }");
    TEST_ERROR("int main() { for (;;) return 1");
    TEST_ERROR("int main() { int x[3]; x[ = 1; return x[0]; }");
    TEST_ERROR("int main() { return sizeof(int; }");
    TEST_ERROR("int main() { return \"unterminated; }");
    TEST_ERROR("int main() { int x = 5 + ; return x; }");
    TEST_ERROR("int main() { int x = (5 + 2; return x; }");
    TEST_ERROR("int main() { int x = 1 int y = 2; return x + y; }");
    TEST_ERROR("int main() { return */ 3; }");
    TEST_ERROR("int main() { return /+3; }");

    printf("All Errors passed!\n");
    return 0;
}