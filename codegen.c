#include "Clyte.h"

FILE *output_file = NULL;
static int depth;
static char *arguments_registers_8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *arguments_registers_64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static Bindable *current_function;

static void generate_expression(Node *node);
static void generate_statement(Node *node);

// Stack utility functions
static void push(void)
{
    println("  push %%rax");
    depth++;
}

static void pop(char *arg)
{
    println("  pop %s", arg);
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
            println("  lea %d(%%rbp), %%rax", node->var->offset);
        }
        else
        {
            println("  lea %s(%%rip), %%rax", node->var->name);
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
        println("  movsbq (%%rax), %%rax");
    else
        println("  mov (%%rax), %%rax");
}

static void store(Type *type)
{
    pop("%rdi");

    if (type->size == 1)
        println("  mov %%al, (%%rdi)");
    else
        println("  mov %%rax, (%%rdi)");
}

// Main assembly code generation from Nodes
void generate_expression(Node *node)
{
    if (!node)
        error("invalid expression");

    switch (node->kind)
    {
    case NODE_NUM:
        println("  mov $%d, %%rax", node->value);
        return;
    case NODE_NEG:
        generate_expression(node->lhs);
        println("  neg %%rax");
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
    case NODE_STMT_EXPR:
        for (Node *n = node->body; n; n = n->next)
            generate_statement(n);
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

        println("  mov $0, %%rax");
        println("  call %s", node->function_name);
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
        println("  cqo");

    if (node->kind >= NODE_ADD && node->kind <= NODE_DIV)
    {
        println("  %s %%rdi, %%rax", binary_ops[node->kind]);
        return;
    }

    println("  cmp %%rdi, %%rax");

    static const char *cmp_ops[] = {
        [NODE_EQ] = "sete",
        [NODE_NE] = "setne",
        [NODE_LT] = "setl",
        [NODE_LE] = "setle"};

    if (node->kind >= NODE_EQ && node->kind <= NODE_LE)
    {
        println("  %s %%al", cmp_ops[node->kind]);
        println("  movzb %%al, %%rax");
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
        println("  cmp $0, %%rax");
        println("  je  .L.else.%d", c);
        generate_statement(node->then);
        println("  jmp .L.end.%d", c);
        println(".L.else.%d:", c);
        if (node->els)
            generate_statement(node->els);
        println(".L.end.%d:", c);
        return;
    }
    if (node->kind == NODE_FOR)
    {
        int c = count();
        if (node->init)
            generate_statement(node->init);
        println(".L.begin.%d:", c);
        if (node->cond)
        {
            generate_expression(node->cond);
            println("  cmp $0, %%rax");
            println("  je  .L.end.%d", c);
        }
        generate_statement(node->then);
        if (node->increment)
            generate_expression(node->increment);
        println("  jmp .L.begin.%d", c);
        println(".L.end.%d:", c);
        return;
    }
    if (node->kind == NODE_EXPR_STMT || node->kind == NODE_RETURN)
    {
        generate_expression(node->lhs);
        if (node->kind == NODE_RETURN)
        {
            println("  jmp .L.return.%s", current_function->name);
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

        println("  .data");
        println("  .globl %s", var->name);
        println("%s:", var->name);
        if (var->init_data)
        {
            for (int i = 0; i < var->type->size; i++)
                println("  .byte %d", var->init_data[i]);
        }
        else
        {
            println("  .zero %d", var->type->size);
        }
    }
}

void code_generation(Bindable *function)
{
    println("  .globl %s", function->name);
    println("  .text");
    println("%s:", function->name);
    current_function = function;

    println("  push %%rbp");
    println("  mov %%rsp, %%rbp");
    println("  sub $%d, %%rsp", function->stack_size);

    int i = 0;
    for (Bindable *var = function->parameters; var; var = var->next)
    {
        if (var->type->size == 1)
            println("  mov %s, %d(%%rbp)", arguments_registers_8[i++], var->offset);
        else
            println("  mov %s, %d(%%rbp)", arguments_registers_64[i++], var->offset);
    }

    generate_statement(function->body);
    assert(depth == 0);

    println(".L.return.%s:", function->name);
    println("  mov %%rbp, %%rsp");
    println("  pop %%rbp");
    println("  ret");
}

void codegen(Bindable *prog, FILE *out)
{
    output_file = out;

    assign_local_var_offsets(prog);
    generate_data(prog);
    for (Bindable *function = prog; function; function = function->next)
    {
        if (!function->is_function)
            continue;
        code_generation(function);
    }
}