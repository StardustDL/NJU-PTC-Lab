#ifndef __AST_H__
#define __AST_H__

#include "common.h"

typedef enum
{
    ST_EMPTY,
    ST_INT,
    ST_FLOAT,
    ST_ID,
    ST_SEMI,
    ST_COMMA,
    ST_ASSIGNOP,
    ST_RELOP,
    ST_PLUS,
    ST_MINUS,
    ST_STAR,
    ST_DIV,
    ST_AND,
    ST_OR,
    ST_NOT,
    ST_DOT,
    ST_TYPE,
    ST_LP,
    ST_RP,
    ST_LB,
    ST_RB,
    ST_LC,
    ST_RC,
    ST_STRUCT,
    ST_RETURN,
    ST_IF,
    ST_ELSE,
    ST_WHILE,
    ST_Program,
    ST_ExtDefList,
    ST_ExtDef,
    ST_ExtDecList,
    ST_Specifier,
    ST_StructSpecifier,
    ST_OptTag,
    ST_Tag,
    ST_VarDec,
    ST_FunDec,
    ST_VarList,
    ST_ParamDec,
    ST_CompSt,
    ST_StmtList,
    ST_Stmt,
    ST_DefList,
    ST_Def,
    ST_DecList,
    ST_Dec,
    ST_Exp,
    ST_Args
} SYNTAX_type;

typedef enum
{
    RELOP_L,
    RELOP_S,
    RELOP_LE,
    RELOP_SE,
    RELOP_E,
    RELOP_NE
} RELOP_type;

typedef enum
{
    MT_INT,
    MT_FLOAT
} METATYPE_type;

typedef struct __ast
{
    SYNTAX_type type;
    bool is_empty;
    bool is_token;
    int count;
    struct __ast **children;
    int first_line;
    union {
        unsigned int t_uint;
        float t_float;
        int t_type;
        char t_str[64];
    };
    void *tag;
} ast;

ast *new_ast(int type, int first_line, int count, ...);

void free_ast(ast *ast);

void show_ast(ast *ast, int level);

#endif