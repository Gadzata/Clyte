#include "Clyte.h"

// Starting logic
// Currently processing from input only
int main(int argc, char **argv)
{
    if (argc != 2)
        error("%s: invalid number of arguments", argv[0]);

    char *input = argv[1];
    Token *token = tokenize(input);
    Node *node = parse(token);

    codegen(node);
    return 0;
}
