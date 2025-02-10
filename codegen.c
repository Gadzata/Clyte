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

static void generate_address(Node *node)
{
    if (node->kind == NODE_VAR)
    {
        int offset = (node->name - 'a' + 1) * 8;
        printf("  lea %d(%%rbp), %%rax\n", -offset);
        return;
    }

    error("not an lvalue");
}

// Main assembly code generation from Nodes
void generate_expression(Node *node)
{
    if (!node)
        error("invalid expression");

    switch (node->kind)
    {
    case NODE_NUM:
        printf("  mov $%d, %%rax\n", node->value);
        return;
    case NODE_NEG:
        generate_expression(node->lhs);
        printf("  neg %%rax\n");
        return;
    case NODE_VAR:
        generate_address(node);
        printf("  mov (%%rax), %%rax\n");
        return;
    case NODE_ASSIGN:
        generate_address(node->lhs);
        push();
        generate_expression(node->rhs);
        pop("%rdi");
        printf("  mov %%rax, (%%rdi)\n");
        return;
    }

    generate_expression(node->rhs);
    push();
    generate_expression(node->lhs);
    pop("%rdi");

    static const char *binary_ops[] = {
        [NODE_ADD] = "add",
        [NODE_SUB] = "sub",
        [NODE_MUL] = "imul",
        [NODE_DIV] = "idiv"};

    if (node->kind == NODE_DIV)
        printf("  cqo\n");

    if (node->kind >= NODE_ADD && node->kind <= NODE_DIV)
    {
        printf("  %s %%rdi, %%rax\n", binary_ops[node->kind]);
        return;
    }

    printf("  cmp %%rdi, %%rax\n");

    static const char *cmp_ops[] = {
        [NODE_EQ] = "sete",
        [NODE_NE] = "setne",
        [NODE_LT] = "setl",
        [NODE_LE] = "setle"};

    if (node->kind >= NODE_EQ && node->kind <= NODE_LE)
    {
        printf("  %s %%al\n", cmp_ops[node->kind]);
        printf("  movzb %%al, %%rax\n");
        return;
    }

    error("invalid expression");
}

static void generate_statment(Node *node)
{
    if (node->kind == NODE_EXPR_STMT)
    {
        generate_expression(node->lhs);
        return;
    }

    error("invalid statement");
}

static void starting_code()
{
    printf("  .globl main\n");
    printf("main:\n");
    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $208, %%rsp\n");
}

void codegen(Node *node)
{
    starting_code();

    for (Node *n = node; n; n = n->next)
    {
        generate_statment(n);
        assert(depth == 0);
    }

    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
}