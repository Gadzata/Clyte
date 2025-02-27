#include "Clyte.h"

Bindable *locals;

static Node *compound_statement(Token **rest, Token *token);
static Node *statment(Token **rest, Token *token);
static Node *expression_statement(Token **rest, Token *token);
static Node *expression(Token **rest, Token *token);
static Node *assign(Token **rest, Token *token);
static Node *equality(Token **rest, Token *token);
static Node *relational(Token **rest, Token *token);
static Node *add(Token **rest, Token *token);
static Node *multiply(Token **rest, Token *token);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *token);

// Node utility functions
static Node *new_node(NodeKind kind, Token *token)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->token = token;
    return node;
}

static Bindable *find_variable(Token *token)
{
    for (Bindable *var = locals; var; var = var->next)
        if (strlen(var->var_name) == token->length && !strncmp(token->location, var->var_name, token->length))
            return var;
    return NULL;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *token)
{
    Node *node = new_node(kind, token);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_number(int value, Token *token)
{
    Node *node = new_node(NODE_NUM, token);
    node->value = value;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr, Token *token)
{
    Node *node = new_node(kind, token);
    node->lhs = expr;
    return node;
}

static Node *new_var_node(Bindable *variable, Token *token)
{
    Node *node = new_node(NODE_VAR, token);
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
            node = new_binary(NODE_LT, node, add(&token, token->next), token);
            continue;
        }

        if (token_equal(token, "<="))
        {
            node = new_binary(NODE_LE, node, add(&token, token->next), token);
            continue;
        }

        if (token_equal(token, ">"))
        {
            node = new_binary(NODE_LT, add(&token, token->next), node, token);
            continue;
        }

        if (token_equal(token, ">="))
        {
            node = new_binary(NODE_LE, add(&token, token->next), node, token);
            continue;
        }

        *rest = token;
        return node;
    }
}

// Main parsing logic
// Parsing based of priority
// Higher priority/precedence gets parsed frist
static Node *statment(Token **rest, Token *token)
{
    if (token_equal(token, "return"))
    {
        Node *node = new_unary(NODE_RETURN, expression(&token, token->next), token);
        *rest = skip(token, ";");
        return node;
    }

    if (token_equal(token, "if"))
    {
        Node *node = new_node(NODE_IF, token);
        token = skip(token->next, "(");
        node->cond = expression(&token, token);
        token = skip(token, ")");
        node->then = statment(&token, token);
        if (token_equal(token, "else"))
            node->els = statment(&token, token->next);
        *rest = token;
        return node;
    }

    if (token_equal(token, "for"))
    {
        Node *node = new_node(NODE_FOR, token);
        token = skip(token->next, "(");

        node->init = expression_statement(&token, token);

        if (!token_equal(token, ";"))
            node->cond = expression(&token, token);
        token = skip(token, ";");

        if (!token_equal(token, ")"))
            node->increment = expression(&token, token);
        token = skip(token, ")");

        node->then = statment(rest, token);
        return node;
    }
    if (token_equal(token, "while"))
    {
        Node *node = new_node(NODE_FOR, token);
        token = skip(token->next, "(");
        node->cond = expression(&token, token);
        token = skip(token, ")");
        node->then = statment(rest, token);
        return node;
    }

    if (token_equal(token, "{"))
        return compound_statement(rest, token->next);

    return expression_statement(rest, token);
}

static Node *compound_statement(Token **rest, Token *token)
{
    Node head = {};
    Node *cur = &head;
    while (!token_equal(token, "}"))
    {
        cur = cur->next = statment(&token, token);
        add_node_type(cur);
    }

    Node *node = new_node(NODE_BLOCK, token);
    node->body = head.next;
    *rest = token->next;
    return node;
}

static Node *expression_statement(Token **rest, Token *token)
{
    if (token_equal(token, ";"))
    {
        *rest = token->next;
        return new_node(NODE_BLOCK, token);
    }

    Node *node = new_unary(NODE_EXPR_STMT, expression(&token, token), token);
    *rest = skip(token, ";");
    return node;
}

static Node *expression(Token **rest, Token *token)
{
    return assign(rest, token);
}

static Node *assign(Token **rest, Token *token)
{
    Node *node = equality(&token, token);
    if (token_equal(token, "="))
        node = new_binary(NODE_ASSIGN, node, assign(&token, token->next), token);
    *rest = token;
    return node;
}

static Node *equality(Token **rest, Token *token)
{
    Node *node = relational(&token, token);

    while (token_equal(token, "==") || token_equal(token, "!="))
    {
        NodeKind kind = token_equal(token, "==") ? NODE_EQ : NODE_NE;
        node = new_binary(kind, node, relational(&token, token->next), token);
    }

    *rest = token;
    return node;
}

static Node *relational(Token **rest, Token *token)
{
    Node *node = add(&token, token);

    return get_comparison_node(rest, token, node);
}

static Node *new_add(Node *lhs, Node *rhs, Token *token)
{
    add_node_type(lhs);
    add_node_type(rhs);

    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_binary(NODE_ADD, lhs, rhs, token);

    if (lhs->type->base && rhs->type->base)
        error_at(token->location, "invalid operands");

    if (!lhs->type->base && rhs->type->base)
    {
        Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    rhs = new_binary(NODE_MUL, rhs, new_number(8, token), token);
    return new_binary(NODE_ADD, lhs, rhs, token);
}

static Node *new_sub(Node *lhs, Node *rhs, Token *token)
{
    add_node_type(lhs);
    add_node_type(rhs);

    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_binary(NODE_SUB, lhs, rhs, token);

    if (lhs->type->base && is_integer(rhs->type))
    {
        rhs = new_binary(NODE_MUL, rhs, new_number(8, token), token);
        add_node_type(rhs);
        Node *node = new_binary(NODE_SUB, lhs, rhs, token);
        node->type = lhs->type;
        return node;
    }

    // ptr - ptr, which returns how many elements are between the two.
    if (lhs->type->base && rhs->type->base)
    {
        Node *node = new_binary(NODE_SUB, lhs, rhs, token);
        node->type = ty_int;
        return new_binary(NODE_DIV, node, new_number(8, token), token);
    }

    error_at(token->location, "invalid operands");
}

static Node *add(Token **rest, Token *token)
{
    Node *node = multiply(&token, token);

    while (token_equal(token, "+") || token_equal(token, "-"))
    {
        node = token_equal(token, "+") ? new_add(node, multiply(&token, token->next), token) : new_sub(node, multiply(&token, token->next), token);
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
        node = new_binary(kind, node, unary(&token, token->next), token);
    }
    *rest = token;
    return node;
}

static Node *unary(Token **rest, Token *token)
{
    if (token_equal(token, "+"))
        return unary(rest, token->next);

    if (token_equal(token, "-"))
        return new_unary(NODE_NEG, unary(rest, token->next), token);

    if (token_equal(token, "&"))
        return new_unary(NODE_ADDR, unary(rest, token->next), token);

    if (token_equal(token, "*"))
        return new_unary(NODE_DEREF, unary(rest, token->next), token);

    return primary(rest, token);
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
        return new_var_node(var, token);
    }
    if (token->kind == TOK_NUM)
    {
        Node *node = new_number(token->value, token);
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