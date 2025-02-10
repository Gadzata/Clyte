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
    char name;
    int value;
};

Node *parse(Token *token);

// Code generation

void codegen(Node *node);