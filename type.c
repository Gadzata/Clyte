#include "Clyte.h"

Type *ty_char = &(Type){TYPE_CHAR, 1};
Type *ty_int = &(Type){TYPE_INT, 8};

bool is_integer(Type *type)
{
    return type->kind == TYPE_CHAR || type->kind == TYPE_INT;
}

Type *pointer_to(Type *base)
{
    Type *type = calloc(1, sizeof(Type));
    type->kind = TYPE_PTR;
    type->size = 8;
    type->base = base;
    return type;
}

Type *function_type(Type *return_type)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_FUNC;
    ty->return_type = return_type;
    return ty;
}

Type *copy_type(Type *type)
{
    Type *ret = calloc(1, sizeof(Type));
    *ret = *type;
    return ret;
}

Type *array_of(Type *base, int len)
{
    Type *type = calloc(1, sizeof(Type));
    type->kind = TYPE_ARRAY;
    type->size = base->size * len;
    type->base = base;
    type->array_len = len;
    return type;
}

void add_node_type(Node *node)
{
    if (!node || node->type)
        return;

    add_node_type(node->lhs);
    add_node_type(node->rhs);
    add_node_type(node->cond);
    add_node_type(node->then);
    add_node_type(node->els);
    add_node_type(node->init);
    add_node_type(node->increment);

    for (Node *n = node->body; n; n = n->next)
        add_node_type(n);

    for (Node *n = node->arguments_list; n; n = n->next)
        add_node_type(n);

    switch (node->kind)
    {
    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_NEG:
        node->type = node->lhs->type;
        return;
    case NODE_ASSIGN:
        if (node->lhs->type->kind == TYPE_ARRAY)
            error_at(node->lhs->token->location, "not an lvalue");
        node->type = node->lhs->type;
        return;
    case NODE_EQ:
    case NODE_NE:
    case NODE_LT:
    case NODE_LE:
    case NODE_NUM:
    case NODE_FUNCTION_CALL:
        node->type = ty_int;
        return;
    case NODE_VAR:
        node->type = node->var->type;
        return;
    case NODE_ADDR:
        if (node->lhs->type->kind == TYPE_ARRAY)
            node->type = pointer_to(node->lhs->type->base);
        else
            node->type = pointer_to(node->lhs->type);
        return;
    case NODE_DEREF:
        if (!node->lhs->type->base)
            error_at(node->token->location, "invalid pointer dereference");
        node->type = node->lhs->type->base;
        return;
    }
}