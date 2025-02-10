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
bool token_equal(Token *token, char *op)
{
    return memcmp(token->location, op, token->length) == 0 && op[token->length] == '\0';
}

Token *skip(Token *token, char *s)
{
    if (!token_equal(token, s))
        error_at(token->location, "expected '%s'", s);
    return token->next;
}

static int get_number(Token *token)
{
    if (token->kind != TOK_NUM)
        error_at(token->location, "expected a number");
    return token->value;
}

static bool check_is_letter(char *curInput)
{
    return 'a' <= *curInput && *curInput <= 'z';
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

static bool starts_with(char *curInput, char *op)
{
    return strncmp(curInput, op, strlen(op)) == 0;
}

static bool is_comparison_punct(char *curInput)
{
    if (starts_with(curInput, "==") || starts_with(curInput, "!=") ||
        starts_with(curInput, "<=") || starts_with(curInput, ">="))
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
    if (is_comparison_punct(curInput))
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
            cur = cur->next = new_token(TOK_NUM, curInput, curInput);
            char *q = curInput;
            cur->value = strtoul(curInput, &curInput, 10);
            cur->length = curInput - q;
            continue;
        }
        if (check_is_letter(curInput))
        {
            cur = cur->next = new_token(TOK_IDENT, curInput, curInput + 1);
            curInput++;
            continue;
        }
        int punct_length = read_punct(curInput);
        if (punct_length)
        {
            cur = cur->next = new_token(TOK_PUNCT, curInput, curInput + punct_length);
            curInput += cur->length;
            continue;
        }
        error_at(curInput, "invalid token");
    }
    cur = cur->next = new_token(TOK_EOF, curInput, curInput);
    return head.next;
}