#include "Clyte.h"

static char *curInput;

// Error handling functions
void error(const char *fmt, ...)
{
    va_list argPrt;
    va_start(argPrt, fmt);
    vfprintf(stderr, fmt, argPrt);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *location, const char *fmt, ...)
{
    va_list argPrt;
    va_start(argPrt, fmt);
    int pos = location - curInput;
    fprintf(stderr, "%s\n%*s^ ", curInput, pos, "");
    vfprintf(stderr, fmt, argPrt);
    fprintf(stderr, "\n");
    exit(1);
}

// Token utility functions
bool equal(Token *token, char *op)
{
    return memcmp(token->location, op, token->length) == 0 && op[token->length] == '\0';
}

Token *skip(Token *token, char *s)
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

static bool isComparisonPunct(char *curInput)
{
    if (startswith(curInput, "==") || startswith(curInput, "!=") ||
        startswith(curInput, "<=") || startswith(curInput, ">="))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *curInput)
{
    if (isComparisonPunct(curInput))
        return 2;

    return ispunct(*curInput) ? 1 : 0;
}

// Starting the tokenization of the input
Token *tokenize(char *argInput)
{
    curInput = argInput;
    Token head = {};
    Token *cur = &head;

    // Looping through the input
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