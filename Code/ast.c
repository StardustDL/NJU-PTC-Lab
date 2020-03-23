#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"

struct ast* new_ast(int type)
{
    struct ast* result = (struct ast*) malloc(sizeof(struct ast));
    result->type = type;
    result->is_empty = 0;
    result->is_token = 0;
    result->children = NULL;
    result->first_line = 0;
    return result;
}
void free_ast(struct ast* ast)
{
    if (ast == NULL)
        return;

    free_ast_list(ast->children);
    free(ast);
}
struct ast_list* new_ast_list(struct ast* ast)
{
    struct ast_list* result = (struct ast_list*) malloc(sizeof(struct ast_list));
    result->head = ast;
    result->next = NULL;
    return result;
}
void free_ast_list(struct ast_list* ast_list)
{
    while (ast_list != NULL)
    {
        free_ast(ast_list->head);
        struct ast_list* next = ast_list->next;
        free(ast_list);
        ast_list = next;
    }
}
void pushfront_child(struct ast* parent, struct ast* child)
{
    assert(parent != NULL);
    assert(child != NULL);
    struct ast_list* list = new_ast_list(child);
    list->next = parent->children;
    parent->children = list;
}

const char* get_syntax_type_name(int type)
{
    const char* result = NULL;
    switch(type)
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
void show_ast(struct ast* ast, int level)
{
    assert(ast != NULL);
    if (ast->is_empty)
        return;
    for(int i = 0; i < level; i++)
        printf("  ");
    printf("%s", get_syntax_type_name(ast->type));
    if (ast->is_token)
    {
        switch (ast->type)
        {
            case ST_ID:
                printf(": %s", ast->t_str);
                break;
            case ST_TYPE:
                switch (ast->t_type)
                {
                    case TYPE_INT:
                        printf(": %s", "int");
                        break;
                    case TYPE_FLOAT:
                        printf(": %s", "float");
                        break;
                }
                break;
            case ST_INT:
                printf(": %u", ast->t_uint);
                break;
            case ST_FLOAT:
                printf(": %.6f", ast->t_float);
                break;
        }
    }
    else
    {
        printf(" (%d)", ast->first_line);
    }
    puts("");
    struct ast_list* children = ast->children;
    while (children != NULL)
    {
        show_ast(children->head, level + 1);
        children = children->next;
    }
}