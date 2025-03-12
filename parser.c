#include "Clyte.h"

Bindable *locals;

static Type *declare_type(Token **rest, Token *token);
static Type *declarator(Token **rest, Token *token, Type *type);
static Node *declaration(Token **rest, Token *token);
static Node *compound_statement(Token **rest, Token *token);
static Node *statement(Token **rest, Token *token);
static Node *expression_statement(Token **rest, Token *token);
static Node *expression(Token **rest, Token *token);
static Node *assign(Token **rest, Token *token);
static Node *equality(Token **rest, Token *token);
static Node *relational(Token **rest, Token *token);
static Node *add(Token **rest, Token *token);
static Node *multiply(Token **rest, Token *token);
static Node *postfix(Token **rest, Token *tok);
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

static Bindable *new_local_var(char *name, Type *type)
{
    Bindable *var = calloc(1, sizeof(Bindable));
    var->var_name = name;
    var->next = locals;
    var->type = type;
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

static int get_number(Token *token)
{
    if (token->kind != TOK_NUM)
        error_at(token->location, "expected a number");
    return token->value;
}

static Type *function_params(Token **rest, Token *token, Type *type)
{
    Type head = {};
    Type *cur = &head;

    while (!token_equal(token, ")"))
    {
        if (cur != &head)
            token = skip(token, ",");
        Type *basety = declare_type(&token, token);
        Type *ty = declarator(&token, token, basety);
        cur = cur->next = copy_type(ty);
    }

    type = function_type(type);
    type->parameters = head.next;
    *rest = token->next;
    return type;
}

static Type *type_suffix(Token **rest, Token *token, Type *type)
{
    if (token_equal(token, "("))
        return function_params(rest, token->next, type);

    if (token_equal(token, "["))
    {
        int token_value = get_number(token->next);
        token = skip(token->next->next, "]");
        type = type_suffix(rest, token, type);
        return array_of(type, token_value);
    }

    *rest = token;
    return type;
}

static char *get_identation(Token *token)
{
    if (token->kind != TOK_IDENT)
        error_at(token->location, "expected an identifier");
    return strndup(token->location, token->length);
}

static Type *declare_type(Token **rest, Token *token)
{
    *rest = skip(token, "int");
    return ty_int;
}

// declarator = "*"* ident
static Type *declarator(Token **rest, Token *token, Type *type)
{
    while (consume_token(&token, token, "*"))
        type = pointer_to(type);

    if (token->kind != TOK_IDENT)
        error_at(token->location, "expected a variable name");

    type = type_suffix(rest, token->next, type);
    type->name = token;
    return type;
}

static Node *declaration(Token **rest, Token *token)
{
    Type *basety = declare_type(&token, token);

    Node head = {};
    Node *cur = &head;
    int i = 0;

    while (!token_equal(token, ";"))
    {
        if (i++ > 0)
            token = skip(token, ",");

        Type *type = declarator(&token, token, basety);
        Bindable *var = new_local_var(get_identation(type->name), type);

        if (!token_equal(token, "="))
            continue;

        Node *lhs = new_var_node(var, type->name);
        Node *rhs = assign(&token, token->next);
        Node *node = new_binary(NODE_ASSIGN, lhs, rhs, token);
        cur = cur->next = new_unary(NODE_EXPR_STMT, node, token);
    }

    Node *node = new_node(NODE_BLOCK, token);
    node->body = head.next;
    *rest = token->next;
    return node;
}

static void create_param_local_vars(Type *param)
{
    if (param)
    {
        create_param_local_vars(param->next);
        new_local_var(get_identation(param->name), param);
    }
}

// Main parsing logic
// Parsing based of priority
// Higher priority/precedence gets parsed frist
static Node *statement(Token **rest, Token *token)
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
        node->then = statement(&token, token);
        if (token_equal(token, "else"))
            node->els = statement(&token, token->next);
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

        node->then = statement(rest, token);
        return node;
    }
    if (token_equal(token, "while"))
    {
        Node *node = new_node(NODE_FOR, token);
        token = skip(token->next, "(");
        node->cond = expression(&token, token);
        token = skip(token, ")");
        node->then = statement(rest, token);
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
        if (token_equal(token, "int"))
            cur = cur->next = declaration(&token, token);
        else
            cur = cur->next = statement(&token, token);
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

    rhs = new_binary(NODE_MUL, rhs, new_number(lhs->type->base->size, token), token);
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
        rhs = new_binary(NODE_MUL, rhs, new_number(lhs->type->base->size, token), token);
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
        return new_binary(NODE_DIV, node, new_number(lhs->type->base->size, token), token);
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

    return postfix(rest, token);
}

static Node *postfix(Token **rest, Token *token)
{
    Node *node = primary(&token, token);

    while (token_equal(token, "["))
    {
        Token *start = token;
        Node *index = expression(&token, token->next);
        token = skip(token, "]");
        node = new_unary(NODE_DEREF, new_add(node, index, start), start);
    }
    *rest = token;
    return node;
}

static Node *function_call(Token **rest, Token *token)
{
    Token *start = token;
    token = token->next->next;

    Node head = {};
    Node *cur = &head;

    while (!token_equal(token, ")"))
    {
        if (cur != &head)
            token = skip(token, ",");
        cur = cur->next = assign(&token, token);
    }

    *rest = skip(token, ")");

    Node *node = new_node(NODE_FUNCTION_CALL, start);
    node->function_name = strndup(start->location, start->length);
    node->arguments_list = head.next;
    return node;
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
        if (token_equal(token->next, "("))
            return function_call(rest, token);

        Bindable *var = find_variable(token);
        if (!var)
            error_at(token->location, "undefined variable");
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

static Function *function(Token **rest, Token *token)
{
    Type *type = declare_type(&token, token);
    type = declarator(&token, token, type);

    locals = NULL;

    Function *function = calloc(1, sizeof(Function));
    function->name = get_identation(type->name);
    create_param_local_vars(type->parameters);
    function->parameters = locals;

    token = skip(token, "{");
    function->body = compound_statement(rest, token);
    function->locals = locals;
    return function;
}

// program = function-definition*
Function *parse(Token *token)
{
    Function head = {};
    Function *cur = &head;

    while (token->kind != TOK_EOF)
        cur = cur->next = function(&token, token);
    return head.next;
}