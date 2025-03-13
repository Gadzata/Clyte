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

// static int get_number(Token *token)
// {
//     if (token->kind != TOK_NUM)
//         error_at(token->location, "expected a number");
//     return token->value;
// }

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

// Returns true if the identChar is a value in between first and last
static bool character_between_values(char identChar, char first, char last)
{
    return first <= identChar && identChar <= last;
}

// Returns true if identChar is an alphabetic character or an underscore.
static bool is_alpha_or_underscore(char identChar)
{
    return (character_between_values(identChar, 'a', 'z')) || (character_between_values(identChar, 'A', 'Z')) || identChar == '_';
}

// Returns true if identChar is valid as the first character of an identifier.
static bool is_valid_first_identifier(char identChar)
{
    return is_alpha_or_underscore(identChar);
}

// Returns true if identChar is valid as a non-first character of an identifier.
static bool is_valid_non_first_identifier(char identChar)
{
    return is_valid_first_identifier(identChar) || (character_between_values(identChar, '0', '9'));
}

bool consume_token(Token **rest, Token *token, char *str)
{
    if (token_equal(token, str))
    {
        *rest = token->next;
        return true;
    }
    *rest = token;
    return false;
}

static bool is_keyword(Token *tok)
{
    static char *kw[] = {
        "return",
        "if",
        "else",
        "for",
        "while",
        "int",
        "sizeof",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (token_equal(tok, kw[i]))
            return true;
    return false;
}

// Read a punctuator token from curInput and returns its length.
static int read_punct(char *curInput)
{
    if (is_comparison_punct(curInput))
        return 2;

    return ispunct(*curInput) ? 1 : 0;
}

static void convert_keywords(Token *token)
{
    for (Token *tok = token; tok->kind != TOK_EOF; tok = tok->next)
        if (is_keyword(tok))
            tok->kind = TOK_KEYWORD;
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
        if (is_valid_first_identifier(*curInput))
        {
            char *start = curInput;
            do
            {
                curInput++;
            } while (is_valid_non_first_identifier(*curInput));
            cur = cur->next = new_token(TOK_IDENT, start, curInput);
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

    convert_keywords(head.next);
    return head.next;
}