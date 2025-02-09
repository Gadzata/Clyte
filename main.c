#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tokenizer

typedef enum
{
    TK_PUNCT, // Punctuators
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    Token *next;
    int value;
    char *location;
    int length;
};

static char *input;

// Error handling functions
static void error(const char *fmt, ...)
{
    va_list argPrt;
    va_start(argPrt, fmt);
    vfprintf(stderr, fmt, argPrt);
    fprintf(stderr, "\n");
    exit(1);
}

static void error_at(char *location, const char *fmt, ...)
{
    va_list argPrt;
    va_start(argPrt, fmt);
    int pos = location - input;
    fprintf(stderr, "%s\n%*s^ ", input, pos, "");
    vfprintf(stderr, fmt, argPrt);
    fprintf(stderr, "\n");
    exit(1);
}

// Token utility functions
static bool equal(Token *token, char *op)
{
    return memcmp(token->location, op, token->length) == 0 && op[token->length] == '\0';
}

static Token *skip(Token *token, char *s)
{
    if (!equal(token, s))
        error_at(token->location, "expected '%s'", s);
    return token->next;
}

static int get_number(Token *token)
{
    if (token->kind != TK_NUM)
        error_at(token->location, "expected a number");
    return token->value;
}

// Creating a new token
static Token *new_token(TokenKind kind, char *start, char *end)
{
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->location = start;
    token->length = end - start;
    return token;
}

static bool startswith(char *curInput, char *op)
{
    return strncmp(curInput, op, strlen(op)) == 0;
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *curInput)
{
    if (startswith(curInput, "==") || startswith(curInput, "!=") ||
        startswith(curInput, "<=") || startswith(curInput, ">="))
        return 2;

    return ispunct(*curInput) ? 1 : 0;
}

static Token *tokenize(void)
{
    char *curInput = input;
    Token head = {};
    Token *cur = &head;

    while (*curInput)
    {
        if (isspace(*curInput))
        {
            curInput++;
            continue;
        }
        if (isdigit(*curInput))
        {
            cur = cur->next = new_token(TK_NUM, curInput, curInput);
            char *q = curInput;
            cur->value = strtoul(curInput, &curInput, 10);
            cur->length = curInput - q;
            continue;
        }
        int punct_length = read_punct(curInput);
        if (punct_length)
        {
            cur = cur->next = new_token(TK_PUNCT, curInput, curInput + punct_length);
            curInput += cur->length;
            continue;
        }
        error_at(curInput, "invalid token");
    }
    cur = cur->next = new_token(TK_EOF, curInput, curInput);
    return head.next;
}

// Parser

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_NUM
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int value;
};

// Node utility functions
static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_number(int value)
{
    Node *node = new_node(ND_NUM);
    node->value = value;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

// Main parsing logic
// Parsing based of priority
// Higher priority/precedence gets parsed frist
static Node *exprssion(Token **rest, Token *token);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *multiply(Token **rest, Token *token);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *token);

static Node *exprssion(Token **rest, Token *token)
{
    return equality(rest, token);
}

static Node *equality(Token **rest, Token *token)
{
    Node *node = relational(&token, token);

    for (;;)
    {
        if (equal(token, "=="))
        {
            node = new_binary(ND_EQ, node, relational(&token, token->next));
            continue;
        }

        if (equal(token, "!="))
        {
            node = new_binary(ND_NE, node, relational(&token, token->next));
            continue;
        }

        *rest = token;
        return node;
    }
}

static Node *relational(Token **rest, Token *token)
{
    Node *node = add(&token, token);

    for (;;)
    {
        if (equal(token, "<"))
        {
            node = new_binary(ND_LT, node, add(&token, token->next));
            continue;
        }

        if (equal(token, "<="))
        {
            node = new_binary(ND_LE, node, add(&token, token->next));
            continue;
        }

        if (equal(token, ">"))
        {
            node = new_binary(ND_LT, add(&token, token->next), node);
            continue;
        }

        if (equal(token, ">="))
        {
            node = new_binary(ND_LE, add(&token, token->next), node);
            continue;
        }

        *rest = token;
        return node;
    }
}

static Node *add(Token **rest, Token *token)
{
    Node *node = multiply(&token, token);

    for (;;)
    {
        if (equal(token, "+"))
        {
            node = new_binary(ND_ADD, node, multiply(&token, token->next));
            continue;
        }

        if (equal(token, "-"))
        {
            node = new_binary(ND_SUB, node, multiply(&token, token->next));
            continue;
        }

        *rest = token;
        return node;
    }
}
static Node *multiply(Token **rest, Token *token)
{
    Node *node = unary(&token, token);
    while (equal(token, "*") || equal(token, "/"))
    {
        NodeKind kind = equal(token, "*") ? ND_MUL : ND_DIV;
        node = new_binary(kind, node, unary(&token, token->next));
    }
    *rest = token;
    return node;
}

static Node *unary(Token **rest, Token *tok)
{
    if (equal(tok, "+"))
        return unary(rest, tok->next);

    if (equal(tok, "-"))
        return new_unary(ND_NEG, unary(rest, tok->next));

    return primary(rest, tok);
}

static Node *primary(Token **rest, Token *token)
{
    if (equal(token, "("))
    {
        Node *node = exprssion(&token, token->next);
        *rest = skip(token, ")");
        return node;
    }
    if (token->kind == TK_NUM)
    {
        Node *node = new_number(token->value);
        *rest = token->next;
        return node;
    }
    error_at(token->location, "expected an expression");
}

// Code generator
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
static void generate_expression(Node *node)
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

// Starting logic
// Currently processing from input only
int main(int argc, char **argv)
{
    if (argc != 2)
        error("%s: invalid number of arguments", argv[0]);

    input = argv[1];
    Token *token = tokenize();
    Node *node = exprssion(&token, token);

    if (token->kind != TK_EOF)
        error_at(token->location, "extra token");

    printf("  .globl main\nmain:\n");
    generate_expression(node);
    printf("  ret\n");

    assert(depth == 0);
    return 0;
}
