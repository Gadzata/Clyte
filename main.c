#include <stdio.h>
#include <stdlib.h>

void generate_assembly(char **ptrArg, char op)
{
    (*ptrArg)++; // Move past the operator
    printf("  %s $%ld, %%rax\n", (op == '+') ? "add" : "sub", strtol(*ptrArg, ptrArg, 10));
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] == '\0') // Check for empty argument
    {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }

    char *ptrArg = argv[1];

    // Output assembly code
    printf("  .globl main\n");
    printf("main:\n");
    printf("  mov $%ld, %%rax\n", strtol(ptrArg, &ptrArg, 10));

    while (*ptrArg)
    {
        if (*ptrArg == '+' || *ptrArg == '-')
        {
            generate_assembly(&ptrArg, *ptrArg);
            continue;
        }

        fprintf(stderr, "Unexpected character: '%c'\n", *ptrArg);
        return 1;
    }

    return 0;
}