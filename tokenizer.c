#include "Clyte.h"

static char *cur_input;
static char *current_filename;

static const struct
{
    char key;
    char value;
} escape_map[] = {{'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'}, {'r', '\r'}, {'e', 27}};

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
    va_list ap;
    va_start(ap, fmt);

    char *line = location;
    while (cur_input < line && line[-1] != '\n')
        line--;

    char *end = location;
    while (*end != '\n')
        end++;

    int line_no = 1;
    for (char *p = cur_input; p < line; p++)
        if (*p == '\n')
            line_no++;

    int indent = fprintf(stderr, "%s:%d: ", current_filename, line_no);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    int pos = location - line + indent;

    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
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

static bool check_is_letter(char *cur_input)
{
    return 'a' <= *cur_input && *cur_input <= 'z';
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

static bool starts_with(char *cur_input, char *op)
{
    return strncmp(cur_input, op, strlen(op)) == 0;
}

static bool is_comparison_punct(char *cur_input)
{
    if (starts_with(cur_input, "==") || starts_with(cur_input, "!=") ||
        starts_with(cur_input, "<=") || starts_with(cur_input, ">="))
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
        "char",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (token_equal(tok, kw[i]))
            return true;
    return false;
}

// Read a punctuator token from cur_input and returns its length.
static int read_punct(char *cur_input)
{
    if (is_comparison_punct(cur_input))
        return 2;

    return ispunct(*cur_input) ? 1 : 0;
}

static int read_escaped_char(char *cur_input)
{
    for (size_t i = 0; i < sizeof(escape_map) / sizeof(escape_map[0]); i++)
    {
        if (*cur_input == escape_map[i].key)
            return escape_map[i].value;
    }

    return *cur_input;
}

static char *string_literal_end(char *cur_input)
{
    char *start = cur_input;
    for (; *cur_input != '"'; cur_input++)
    {
        if (*cur_input == '\n' || *cur_input == '\0')
            error_at(start, "unclosed string literal");
        if (*cur_input == '\\')
            cur_input++;
    }
    return cur_input;
}

static Token *read_string_literal(char *start)
{
    char *end = string_literal_end(start + 1);
    char *buffuer = calloc(1, end - start);
    int len = 0;

    for (char *cur_input = start + 1; cur_input < end;)
    {
        if (*cur_input == '\\')
        {
            buffuer[len++] = read_escaped_char(cur_input + 1);
            cur_input += 2;
        }
        else
        {
            buffuer[len++] = *cur_input++;
        }
    }

    Token *tok = new_token(TOK_STR, start, end + 1);
    tok->type = array_of(ty_char, len + 1);
    tok->str = buffuer;
    return tok;
}

static void convert_keywords(Token *token)
{
    for (Token *tok = token; tok->kind != TOK_EOF; tok = tok->next)
        if (is_keyword(tok))
            tok->kind = TOK_KEYWORD;
}

// Starting the tokenization of the input
static Token *tokenize(char *filename, char *input)
{
    current_filename = filename;
    cur_input = input;
    Token head = {};
    Token *cur = &head;

    // Looping through the input
    while (*cur_input)
    {
        if (isspace(*cur_input))
        {
            cur_input++;
            continue;
        }
        if (isdigit(*cur_input))
        {
            cur = cur->next = new_token(TOK_NUM, cur_input, cur_input);
            char *q = cur_input;
            cur->value = strtoul(cur_input, &cur_input, 10);
            cur->length = cur_input - q;
            continue;
        }
        if (*cur_input == '"')
        {
            cur = cur->next = read_string_literal(cur_input);
            cur_input += cur->length;
            continue;
        }
        if (is_valid_first_identifier(*cur_input))
        {
            char *start = cur_input;
            do
            {
                cur_input++;
            } while (is_valid_non_first_identifier(*cur_input));
            cur = cur->next = new_token(TOK_IDENT, start, cur_input);
            continue;
        }
        int punct_length = read_punct(cur_input);
        if (punct_length)
        {
            cur = cur->next = new_token(TOK_PUNCT, cur_input, cur_input + punct_length);
            cur_input += cur->length;
            continue;
        }
        error_at(cur_input, "invalid token");
    }
    cur = cur->next = new_token(TOK_EOF, cur_input, cur_input);

    convert_keywords(head.next);
    return head.next;
}

static char *read_file(char *path)
{
    FILE *fp;

    if (strcmp(path, "-") == 0)
    {
        // By convention, read from stdin if a given filename is "-".
        fp = stdin;
    }
    else
    {
        fp = fopen(path, "r");
        if (!fp)
            error("cannot open %s: %s", path, strerror(errno));
    }

    char *buf;
    size_t buflen;
    FILE *out = open_memstream(&buf, &buflen);

    // Read the entire file.
    for (;;)
    {
        char buf2[4096];
        int n = fread(buf2, 1, sizeof(buf2), fp);
        if (n == 0)
            break;
        fwrite(buf2, 1, n, out);
    }

    if (fp != stdin)
        fclose(fp);

    // Make sure that the last line is properly terminated with '\n'.
    fflush(out);
    if (buflen == 0 || buf[buflen - 1] != '\n')
        fputc('\n', out);
    fputc('\0', out);
    fclose(out);
    return buf;
}

Token *tokenize_file(char *path)
{
    return tokenize(path, read_file(path));
}