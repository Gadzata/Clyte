#include "Clyte.h"

static int depth;

// Stack utility functions
static void push(void)
{
    printf("  push %%rax\n");
    depth++;
}

static void pop(char *arg)
{
    printf("  pop %s\n", arg);
    depth--;
}

// Main assembly code generation from Nodes
void generate_expression(Node *node)
{
    if (!node)
        error("invalid expression");

    switch (node->kind)
    {
    case ND_NUM:
        printf("  mov $%d, %%rax\n", node->value);
        return;
    case ND_NEG:
        generate_expression(node->lhs);
        printf("  neg %%rax\n");
        return;
    }

    generate_expression(node->rhs);
    push();
    generate_expression(node->lhs);
    pop("%rdi");

    static const char *binary_ops[] = {
        [ND_ADD] = "add",
        [ND_SUB] = "sub",
        [ND_MUL] = "imul",
        [ND_DIV] = "idiv"};

    if (node->kind == ND_DIV)
        printf("  cqo\n");

    if (node->kind >= ND_ADD && node->kind <= ND_DIV)
    {
        printf("  %s %%rdi, %%rax\n", binary_ops[node->kind]);
        return;
    }

    printf("  cmp %%rdi, %%rax\n");

    static const char *cmp_ops[] = {
        [ND_EQ] = "sete",
        [ND_NE] = "setne",
        [ND_LT] = "setl",
        [ND_LE] = "setle"};

    if (node->kind >= ND_EQ && node->kind <= ND_LE)
    {
        printf("  %s %%al\n", cmp_ops[node->kind]);
        printf("  movzb %%al, %%rax\n");
        return;
    }

    error("invalid expression");
}

static void generate_statment(Node *node)
{
    if (node->kind == ND_EXPR_STMT)
    {
        generate_expression(node->lhs);
        return;
    }

    error("invalid statement");
}

void codegen(Node *node)
{
    printf("  .globl main\n");
    printf("main:\n");

    for (Node *n = node; n; n = n->next)
    {
        generate_statment(n);
        assert(depth == 0);
    }
    printf("  ret\n");
}