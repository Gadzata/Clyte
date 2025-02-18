#include "Clyte.h"

Bindable *locals;

static Node *compound_statement(Token **rest, Token *tok);
static Node *expression(Token **rest, Token *token);
static Node *expression_statement(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
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

static Bindable *find_variable(Token *token)
{
    for (Bindable *var = locals; var; var = var->next)
        if (strlen(var->var_name) == token->length && !strncmp(token->location, var->var_name, token->length))
            return var;
    return NULL;
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
    Node *node = new_node(NODE_NUM);
    node->value = value;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

static Node *new_var_node(Bindable *variable)
{
    Node *node = new_node(NODE_VAR);
    node->var = variable;
    return node;
}

static Bindable *new_lvar(char *name)
{
    Bindable *var = calloc(1, sizeof(Bindable));
    var->var_name = name;
    var->next = locals;
    locals = var;
    return var;
}

static Node *get_comparison_node(Token **rest, Token *token, Node *node)
{
    for (;;)
    {
        if (token_equal(token, "<"))
        {
            node = new_binary(NODE_LT, node, add(&token, token->next));
            continue;
        }

        if (token_equal(token, "<="))
        {
            node = new_binary(NODE_LE, node, add(&token, token->next));
            continue;
        }

        if (token_equal(token, ">"))
        {
            node = new_binary(NODE_LT, add(&token, token->next), node);
            continue;
        }

        if (token_equal(token, ">="))
        {
            node = new_binary(NODE_LE, add(&token, token->next), node);
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
    if (token_equal(tok, "return"))
    {
        Node *node = new_unary(NODE_RETURN, expression(&tok, tok->next));
        *rest = skip(tok, ";");
        return node;
    }

    if (token_equal(tok, "if"))
    {
        Node *node = new_node(NODE_IF);
        tok = skip(tok->next, "(");
        node->cond = expression(&tok, tok);
        tok = skip(tok, ")");
        node->then = statment(&tok, tok);
        if (token_equal(tok, "else"))
            node->els = statment(&tok, tok->next);
        *rest = tok;
        return node;
    }

    if (token_equal(tok, "for"))
    {
        Node *node = new_node(NODE_FOR);
        tok = skip(tok->next, "(");

        node->init = expression_statement(&tok, tok);

        if (!token_equal(tok, ";"))
            node->cond = expression(&tok, tok);
        tok = skip(tok, ";");

        if (!token_equal(tok, ")"))
            node->increment = expression(&tok, tok);
        tok = skip(tok, ")");

        node->then = statment(rest, tok);
        return node;
    }
    if (token_equal(tok, "while"))
    {
        Node *node = new_node(NODE_FOR);
        tok = skip(tok->next, "(");
        node->cond = expression(&tok, tok);
        tok = skip(tok, ")");
        node->then = statment(rest, tok);
        return node;
    }

    if (token_equal(tok, "{"))
        return compound_statement(rest, tok->next);

    return expression_statement(rest, tok);
}

static Node *compound_statement(Token **rest, Token *tok)
{
    Node head = {};
    Node *cur = &head;
    while (!token_equal(tok, "}"))
        cur = cur->next = statment(&tok, tok);

    Node *node = new_node(NODE_BLOCK);
    node->body = head.next;
    *rest = tok->next;
    return node;
}

static Node *expression_statement(Token **rest, Token *tok)
{
    if (token_equal(tok, ";"))
    {
        *rest = tok->next;
        return new_node(NODE_BLOCK);
    }

    Node *node = new_unary(NODE_EXPR_STMT, expression(&tok, tok));
    *rest = skip(tok, ";");
    return node;
}

static Node *expression(Token **rest, Token *token)
{
    return assign(rest, token);
}

static Node *assign(Token **rest, Token *tok)
{
    Node *node = equality(&tok, tok);
    if (token_equal(tok, "="))
        node = new_binary(NODE_ASSIGN, node, assign(&tok, tok->next));
    *rest = tok;
    return node;
}

static Node *equality(Token **rest, Token *token)
{
    Node *node = relational(&token, token);

    while (token_equal(token, "==") || token_equal(token, "!="))
    {
        NodeKind kind = token_equal(token, "==") ? NODE_EQ : NODE_NE;
        node = new_binary(kind, node, relational(&token, token->next));
    }

    *rest = token;
    return node;
}

static Node *relational(Token **rest, Token *token)
{
    Node *node = add(&token, token);

    return get_comparison_node(rest, token, node);
}

static Node *add(Token **rest, Token *token)
{
    Node *node = multiply(&token, token);

    while (token_equal(token, "+") || token_equal(token, "-"))
    {
        NodeKind kind = token_equal(token, "+") ? NODE_ADD : NODE_SUB;
        node = new_binary(kind, node, multiply(&token, token->next));
    }

    *rest = token;
    return node;
}
static Node *multiply(Token **rest, Token *token)
{
    Node *node = unary(&token, token);
    while (token_equal(token, "*") || token_equal(token, "/"))
    {
        NodeKind kind = token_equal(token, "*") ? NODE_MUL : NODE_DIV;
        node = new_binary(kind, node, unary(&token, token->next));
    }
    *rest = token;
    return node;
}

static Node *unary(Token **rest, Token *tok)
{
    if (token_equal(tok, "+"))
        return unary(rest, tok->next);

    if (token_equal(tok, "-"))
        return new_unary(NODE_NEG, unary(rest, tok->next));

    return primary(rest, tok);
}

static Node *primary(Token **rest, Token *token)
{
    if (token_equal(token, "("))
    {
        Node *node = expression(&token, token->next);
        *rest = skip(token, ")");
        return node;
    }
    if (token->kind == TOK_IDENT)
    {
        Bindable *var = find_variable(token);
        if (!var)
            var = new_lvar(strndup(token->location, token->length));
        *rest = token->next;
        return new_var_node(var);
    }
    if (token->kind == TOK_NUM)
    {
        Node *node = new_number(token->value);
        *rest = token->next;
        return node;
    }
    error_at(token->location, "expected an expression");
}

Function *parse(Token *token)
{
    token = skip(token, "{");

    Function *prog = calloc(1, sizeof(Function));
    prog->body = compound_statement(&token, token);
    prog->locals = locals;
    return prog;
}