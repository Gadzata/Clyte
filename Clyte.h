#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;
typedef struct Node Node;

// Tokenizer

typedef enum
{
    TOK_IDENT,
    TOK_PUNCT,
    TOK_KEYWORD,
    TOK_STR,
    TOK_NUM,
    TOK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    Token *next;
    int value;
    char *location;
    int length;
    Type *type;
    char *str;
};

void error(const char *fmt, ...);
void error_at(char *location, const char *fmt, ...);
bool token_equal(Token *token, char *op);
Token *skip(Token *token, char *s);
bool consume_token(Token **rest, Token *tok, char *str);
Token *tokenize(char *input);

// Local Variable Object
typedef struct Bindable Bindable;
struct Bindable
{
    Bindable *next;
    char *name;
    Type *type;
    bool is_local_var;
    int offset;

    bool is_function;

    char *init_data;

    Node *body;
    Bindable *parameters;
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
    NODE_ADDR,
    NODE_DEREF,
    NODE_RETURN,
    NODE_IF,
    NODE_FOR,
    NODE_BLOCK,
    NODE_FUNCTION_CALL,
    NODE_EXPR_STMT,
    NODE_VAR,
    NODE_NUM,
} NodeKind;

struct Node
{
    NodeKind kind;
    Node *next;
    Type *type;
    Token *token;
    Node *lhs;
    Node *rhs;

    Node *body;

    char *function_name;
    Node *arguments_list;

    Node *cond;
    Node *then;
    Node *els;

    Node *init;
    Node *increment;

    Bindable *var;
    int value;
};

Bindable *parse(Token *token);

// Code generation

void codegen(Bindable *prog);

typedef enum
{
    TYPE_CHAR,
    TYPE_INT,
    TYPE_PTR,
    TYPE_FUNC,
    TYPE_ARRAY,
} TypeKind;

struct Type
{
    TypeKind kind;
    int size;
    Type *base;
    Token *name;
    Type *return_type;
    Type *parameters;
    Type *next;
    int array_len;
};

extern Type *ty_char;
extern Type *ty_int;

bool is_integer(Type *type);
Type *copy_type(Type *type);
Type *pointer_to(Type *base);
Type *function_type(Type *return_type);
Type *array_of(Type *base, int size);
void add_node_type(Node *node);