#include "Clyte.h"

static int depth;
static char *arguments_registers_8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *arguments_registers_64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static Bindable *current_function;

static void generate_expression(Node *node);

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

static void assign_local_var_offsets(Bindable *program)
{
    for (Bindable *funct = program; funct; funct = funct->next)
    {
        if (!funct->is_function)
            continue;

        int offset = 0;
        for (Bindable *var = funct->locals; var; var = var->next)
        {
            offset += var->type->size;
            var->offset = -offset;
        }
        funct->stack_size = align_to(offset, 16);
    }
}

static void generate_address(Node *node)
{
    if (node->kind == NODE_VAR)
    {
        if (node->var->is_local_var)
        {
            printf("  lea %d(%%rbp), %%rax\n", node->var->offset);
        }
        else
        {
            printf("  lea %s(%%rip), %%rax\n", node->var->name);
        }
        return;
    }

    if (node->kind == NODE_DEREF)
    {
        generate_expression(node->lhs);
        return;
    }
    error_at(node->token->location, "not an lvalue");
}

static void load(Type *type)
{
    if (type->kind == TYPE_ARRAY)
    {
        return;
    }

    if (type->size == 1)
        printf("  movsbq (%%rax), %%rax\n");
    else
        printf("  mov (%%rax), %%rax\n");
}

static void store(Type *type)
{
    pop("%rdi");

    if (type->size == 1)
        printf("  mov %%al, (%%rdi)\n");
    else
        printf("  mov %%rax, (%%rdi)\n");
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
        load(node->type);
        return;
    case NODE_DEREF:
        generate_expression(node->lhs);
        load(node->type);
        return;
    case NODE_ADDR:
        generate_address(node->lhs);
        return;
    case NODE_ASSIGN:
        generate_address(node->lhs);
        push();
        generate_expression(node->rhs);
        store(node->type);
        return;
    case NODE_FUNCTION_CALL:
    {
        int nargs = 0;
        for (Node *arg = node->arguments_list; arg; arg = arg->next)
        {
            generate_expression(arg);
            push();
            nargs++;
        }

        for (int i = nargs - 1; i >= 0; i--)
            pop(arguments_registers_64[i]);

        printf("  mov $0, %%rax\n");
        printf("  call %s\n", node->function_name);
        return;
    }
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

    error_at(node->token->location, "invalid expression");
}

static void generate_statement(Node *node)
{
    if (node->kind == NODE_BLOCK)
    {
        for (Node *n = node->body; n; n = n->next)
            generate_statement(n);
        return;
    }
    if (node->kind == NODE_IF)
    {
        int c = count();
        generate_expression(node->cond);
        printf("  cmp $0, %%rax\n");
        printf("  je  .L.else.%d\n", c);
        generate_statement(node->then);
        printf("  jmp .L.end.%d\n", c);
        printf(".L.else.%d:\n", c);
        if (node->els)
            generate_statement(node->els);
        printf(".L.end.%d:\n", c);
        return;
    }
    if (node->kind == NODE_FOR)
    {
        int c = count();
        if (node->init)
            generate_statement(node->init);
        printf(".L.begin.%d:\n", c);
        if (node->cond)
        {
            generate_expression(node->cond);
            printf("  cmp $0, %%rax\n");
            printf("  je  .L.end.%d\n", c);
        }
        generate_statement(node->then);
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
            printf("  jmp .L.return.%s\n", current_function->name);
        }
        return;
    }

    error_at(node->token->location, "invalid statement");
}

static void generate_data(Bindable *funct)
{
    for (Bindable *var = funct; var; var = var->next)
    {
        if (var->is_function)
            continue;

        printf("  .data\n");
        printf("  .globl %s\n", var->name);
        printf("%s:\n", var->name);
        if (var->init_data)
        {
            for (int i = 0; i < var->type->size; i++)
                printf("  .byte %d\n", var->init_data[i]);
        }
        else
        {
            printf("  .zero %d\n", var->type->size);
        }
    }
}

void code_generation(Bindable *function)
{
    printf("  .globl %s\n", function->name);
    printf("  .text\n");
    printf("%s:\n", function->name);
    current_function = function;

    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $%d, %%rsp\n", function->stack_size);

    int i = 0;
    for (Bindable *var = function->parameters; var; var = var->next)
    {
        if (var->type->size == 1)
            printf("  mov %s, %d(%%rbp)\n", arguments_registers_8[i++], var->offset);
        else
            printf("  mov %s, %d(%%rbp)\n", arguments_registers_64[i++], var->offset);
    }

    generate_statement(function->body);
    assert(depth == 0);

    printf(".L.return.%s:\n", function->name);
    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
}

void codegen(Bindable *prog)
{
    assign_local_var_offsets(prog);
    generate_data(prog);
    for (Bindable *function = prog; function; function = function->next)
    {
        if (!function->is_function)
            continue;
        code_generation(function);
    }
}