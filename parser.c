#include "Clyte.h"

Node *expression(Token **rest, Token *token);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *multiply(Token **rest, Token *token);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *token);

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
Node *expression(Token **rest, Token *token)
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
        Node *node = expression(&token, token->next);
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

Node *parse(Token *token)
{
    Node *node = expression(&token, token);
    if (token->kind != TK_EOF)
        error_at(token->location, "extra token");
    return node;
}