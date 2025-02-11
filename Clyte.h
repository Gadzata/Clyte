#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

// Tokenizer

typedef enum
{
    TOK_IDENT, // Identifiers
    TOK_PUNCT, // Punctuators
    TOK_NUM,   // Numeric literals
    TOK_EOF,   // End-of-file markers
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

void error(const char *fmt, ...);
void error_at(char *location, const char *fmt, ...);
bool token_equal(Token *token, char *op);
Token *skip(Token *token, char *s);
Token *tokenize(char *input);

// Local Variable Object
typedef struct Bindable Bindable;
struct Bindable
{
    Bindable *next;
    char *var_name;
    int offset;
};

// Function Object
typedef struct Function Function;
struct Function
{
    Node *body;
    Bindable *locals;
    int stack_size;
};

// Parser

typedef enum
{
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_NEG,
    NODE_EQ,
    NODE_NE,
    NODE_LT,
    NODE_LE,
    NODE_ASSIGN,
    NODE_EXPR_STMT,
    NODE_VAR,
    NODE_NUM,
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *next;
    Node *lhs;
    Node *rhs;
    Bindable *var;
    int value;
};

Function *parse(Token *token);

// Code generation

void codegen(Function *prog);