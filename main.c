#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Token types
typedef enum
{
    TK_PUNCT, // Operators (+, -)
    TK_NUM,   // Number literals
    TK_EOF    // End-of-file
} TokenKind;

// Token structure
typedef struct Token
{
    TokenKind kind;
    struct Token *next;
    int val;
    char *loc;
    int len;
} Token;

static char *input; // Input string

// Error reporting
static void error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void error_at(char *loc, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - input;
    fprintf(stderr, "%s\n%*s^ ", input, pos, "");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Token utilities
static bool equal(Token *tok, const char *op)
{
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

static Token *skip(Token *tok, const char *s)
{
    if (!equal(tok, s))
        error_at(tok->loc, "expected '%s'", s);
    return tok->next;
}

static int get_number(Token *tok)
{
    if (tok->kind != TK_NUM)
        error_at(tok->loc, "expected a number");
    return tok->val;
}

static Token *new_token(TokenKind kind, char *start, char *end)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

// Tokenizer
static Token *tokenize(void)
{
    char *p = input;
    Token head = {};
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (isdigit(*p))
        {
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            cur = cur->next = new_token(TK_PUNCT, p, p + 1);
            p++;
            continue;
        }

        error_at(p, "invalid token");
    }

    cur->next = new_token(TK_EOF, p, p);
    return head.next;
}

// Code generation
static void generate_assembly(Token *tok)
{
    printf("  .globl main\nmain:\n");
    printf("  mov $%d, %%rax\n", get_number(tok));
    tok = tok->next;

    while (tok->kind != TK_EOF)
    {
        if (equal(tok, "+"))
        {
            printf("  add $%d, %%rax\n", get_number(tok->next));
            tok = tok->next->next;
            continue;
        }

        tok = skip(tok, "-");
        printf("  sub $%d, %%rax\n", get_number(tok));
        tok = tok->next;
    }

    printf("  ret\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        error("Usage: %s <expression>", argv[0]);

    input = argv[1];
    Token *tok = tokenize();
    generate_assembly(tok);
    return 0;
}
