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

typedef unsigned int ASTD_Int;

typedef float ASTD_Float;

typedef RELOP_type ASTD_Relop;

typedef METATYPE_type ASTD_Type;

typedef char ASTD_Id[64];

typedef struct __ast
{
    SYNTAX_type type;
    bool is_empty;
    bool is_token;
    int count;
    struct __ast **children;
    int first_line;
    void *data;
    void *sem;
} ast;

ast *new_ast(int type, int first_line, int count, ...);

void delete_ast(ast *ast);

void show_ast(ast *ast, int level);

#endif