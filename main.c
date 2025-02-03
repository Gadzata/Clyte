#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }

    char *endptr;
    long num = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "%s: invalid argument: '%s' is not a valid number\n", argv[0], argv[1]);
        return 1;
    }

    // Output assembly code
    printf("  .globl main\n");
    printf("main:\n");
    printf("  mov $%ld, %%rax\n", num);
    printf("  ret\n");

    return 0;
}