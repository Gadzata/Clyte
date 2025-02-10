#include "Clyte.h"

static Node *expression(Token **rest, Token *token);
static Node *expression_statement(Token **rest, Token *tok);
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

static Node *getComparisonNode(Token **rest, Token *token, Node *node)
{
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

// Main parsing logic
// Parsing based of priority
// Higher priority/precedence gets parsed frist
static Node *statment(Token **rest, Token *tok)
{
    return expression_statement(rest, tok);
}

static Node *expression_statement(Token **rest, Token *tok)
{
    Node *node = new_unary(ND_EXPR_STMT, expression(&tok, tok));
    *rest = skip(tok, ";");
    return node;
}

static Node *expression(Token **rest, Token *token)
{
    return equality(rest, token);
}

static Node *equality(Token **rest, Token *token)
{
    Node *node = relational(&token, token);

    while (equal(token, "==") || equal(token, "!="))
    {
        NodeKind kind = equal(token, "==") ? ND_EQ : ND_NE;
        node = new_binary(kind, node, relational(&token, token->next));
    }

    *rest = token;
    return node;
}

static Node *relational(Token **rest, Token *token)
{
    Node *node = add(&token, token);

    return getComparisonNode(rest, token, node);
}

static Node *add(Token **rest, Token *token)
{
    Node *node = multiply(&token, token);

    while (equal(token, "+") || equal(token, "-"))
    {
        NodeKind kind = equal(token, "+") ? ND_ADD : ND_SUB;
        node = new_binary(kind, node, multiply(&token, token->next));
    }

    *rest = token;
    return node;
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
    Node head = {};
    Node *cur = &head;
    while (token->kind != TK_EOF)
        cur = cur->next = statment(&token, token);
    return head.next;
}