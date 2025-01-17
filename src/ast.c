#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"
#include "object.h"
#include "debug.h"

irop *op_var(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Variable;
    op->var = var;
    return op;
}

irop *op_ref(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Ref;
    op->var = var;
    return op;
}

irop *op_deref(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Deref;
    op->var = var;
    return op;
}

irop *op_const(int value)
{
    irop *op = new (irop);
    op->kind = IRO_Constant;
    op->value = value;
    return op;
}

irop *op_rval(irvar *var)
{
    if (var->isref)
    {
        return op_deref(var);
    }
    else
    {
        return op_var(var);
    }
}

syntax_tree *new_syntax_tree(int type, int first_line, int count, ...)
{
    Assert(count >= 0, "syntax_tree children count < 0");
    syntax_tree *result = new (syntax_tree);
    result->type = type;
    result->first_line = first_line;
    result->is_empty = false;
    result->is_token = false;
    result->count = count;
    result->sem = NULL;
    if (count > 0)
    {
        result->children = newarr(syntax_tree, count);
        va_list valist;
        va_start(valist, count);
        for (int i = 0; i < count; i++)
            result->children[i] = va_arg(valist, syntax_tree *);
    }
    else
        result->children = NULL;
    return result;
}
void delete_syntax_tree(syntax_tree *tree)
{
    if (tree == NULL)
        return;

    for (int i = 0; i < tree->count; i++)
        delete_syntax_tree(tree->children[i]);
    delete (tree);
}

const char *get_syntax_type_name(int type)
{
    const char *result = NULL;
    switch (type)
    {
    case ST_EMPTY:
        result = "EMPTY";
        break;
    case ST_INT:
        result = "INT";
        break;
    case ST_FLOAT:
        result = "FLOAT";
        break;
    case ST_ID:
        result = "ID";
        break;
    case ST_SEMI:
        result = "SEMI";
        break;
    case ST_COMMA:
        result = "COMMA";
        break;
    case ST_ASSIGNOP:
        result = "ASSIGNOP";
        break;
    case ST_RELOP:
        result = "RELOP";
        break;
    case ST_PLUS:
        result = "PLUS";
        break;
    case ST_MINUS:
        result = "MINUS";
        break;
    case ST_STAR:
        result = "STAR";
        break;
    case ST_DIV:
        result = "DIV";
        break;
    case ST_AND:
        result = "AND";
        break;
    case ST_OR:
        result = "OR";
        break;
    case ST_NOT:
        result = "NOT";
        break;
    case ST_DOT:
        result = "DOT";
        break;
    case ST_TYPE:
        result = "TYPE";
        break;
    case ST_LP:
        result = "LP";
        break;
    case ST_RP:
        result = "RP";
        break;
    case ST_LB:
        result = "LB";
        break;
    case ST_RB:
        result = "RB";
        break;
    case ST_LC:
        result = "LC";
        break;
    case ST_RC:
        result = "RC";
        break;
    case ST_STRUCT:
        result = "STRUCT";
        break;
    case ST_RETURN:
        result = "RETURN";
        break;
    case ST_IF:
        result = "IF";
        break;
    case ST_ELSE:
        result = "ELSE";
        break;
    case ST_WHILE:
        result = "WHILE";
        break;
    case ST_Program:
        result = "Program";
        break;
    case ST_ExtDefList:
        result = "ExtDefList";
        break;
    case ST_ExtDef:
        result = "ExtDef";
        break;
    case ST_ExtDecList:
        result = "ExtDecList";
        break;
    case ST_Specifier:
        result = "Specifier";
        break;
    case ST_StructSpecifier:
        result = "StructSpecifier";
        break;
    case ST_OptTag:
        result = "OptTag";
        break;
    case ST_Tag:
        result = "Tag";
        break;
    case ST_VarDec:
        result = "VarDec";
        break;
    case ST_FunDec:
        result = "FunDec";
        break;
    case ST_VarList:
        result = "VarList";
        break;
    case ST_ParamDec:
        result = "ParamDec";
        break;
    case ST_CompSt:
        result = "CompSt";
        break;
    case ST_StmtList:
        result = "StmtList";
        break;
    case ST_Stmt:
        result = "Stmt";
        break;
    case ST_DefList:
        result = "DefList";
        break;
    case ST_Def:
        result = "Def";
        break;
    case ST_DecList:
        result = "DecList";
        break;
    case ST_Dec:
        result = "Dec";
        break;
    case ST_Exp:
        result = "Exp";
        break;
    case ST_Args:
        result = "Args";
        break;
    }
    return result;
}
void _show_syntax_tree(syntax_tree *tree, int level)
{
    AssertNotNull(tree);
    if (tree->is_empty)
        return;
    for (int i = 0; i < level; i++)
        printf("  ");
    printf("%s", get_syntax_type_name(tree->type));
    if (tree->is_token)
    {
        switch (tree->type)
        {
        case ST_ID:
            printf(": %s", *cast(sytd_id, tree->data));
            break;
        case ST_TYPE:
            switch (*cast(sytd_type, tree->data))
            {
            case MT_INT:
                printf(": %s", "int");
                break;
            case MT_FLOAT:
                printf(": %s", "float");
                break;
            }
            break;
        case ST_INT:
            printf(": %u", *cast(sytd_int, tree->data));
            break;
        case ST_FLOAT:
            printf(": %.6f", *cast(sytd_float, tree->data));
            break;
        }
    }
    else
    {
        printf(" (%d)", tree->first_line);
    }
    puts("");
    for (int i = 0; i < tree->count; i++)
    {
        syntax_tree *child = tree->children[i];
        _show_syntax_tree(child, level + 1);
    }
}

void show_syntax_tree(syntax_tree *tree)
{
    _show_syntax_tree(tree, 0);
}