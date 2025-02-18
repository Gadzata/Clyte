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

static int count(void)
{
    static int i = 1;
    return i++;
}

static int align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}

static void assign_local_var_offsets(Function *prog)
{
    int offset = 0;
    for (Bindable *var = prog->locals; var; var = var->next)
    {
        offset += 8;
        var->offset = -offset;
    }
    prog->stack_size = align_to(offset, 16);
}

static void generate_address(Node *node)
{
    if (node->kind == NODE_VAR)
    {
        printf("  lea %d(%%rbp), %%rax\n", node->var->offset);
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
    if (node->kind == NODE_BLOCK)
    {
        for (Node *n = node->body; n; n = n->next)
            generate_statment(n);
        return;
    }
    if (node->kind == NODE_IF)
    {
        int c = count();
        generate_expression(node->cond);
        printf("  cmp $0, %%rax\n");
        printf("  je  .L.else.%d\n", c);
        generate_statment(node->then);
        printf("  jmp .L.end.%d\n", c);
        printf(".L.else.%d:\n", c);
        if (node->els)
            generate_statment(node->els);
        printf(".L.end.%d:\n", c);
        return;
    }
    if (node->kind == NODE_FOR)
    {
        int c = count();
        if (node->init)
            generate_statment(node->init);
        printf(".L.begin.%d:\n", c);
        if (node->cond)
        {
            generate_expression(node->cond);
            printf("  cmp $0, %%rax\n");
            printf("  je  .L.end.%d\n", c);
        }
        generate_statment(node->then);
        if (node->increment)
            generate_expression(node->increment);
        printf("  jmp .L.begin.%d\n", c);
        printf(".L.end.%d:\n", c);
        return;
    }
    if (node->kind == NODE_EXPR_STMT || node->kind == NODE_RETURN)
    {
        generate_expression(node->lhs);
        if (node->kind == NODE_RETURN)
        {
            printf("  jmp .L.return\n");
        }
        return;
    }

    error("invalid statement");
}

static void starting_code(Function *prog)
{
    printf("  .globl main\n");
    printf("main:\n");
    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $%d, %%rsp\n", prog->stack_size);
}

void codegen(Function *prog)
{
    assign_local_var_offsets(prog);

    starting_code(prog);

    generate_statment(prog->body);
    assert(depth == 0);

    printf(".L.return:\n");
    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
}