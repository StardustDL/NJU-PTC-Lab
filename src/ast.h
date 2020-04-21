#ifndef __AST_H__
#define __AST_H__

#include "common.h"
#include "symbol.h"

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
} syntax_type;

typedef unsigned int sytd_int;

typedef float sytd_float;

typedef relop_type sytd_relop;

typedef metatype_type sytd_type;

typedef char sytd_id[64];

typedef struct __syntax_tree
{
    syntax_type type;
    bool is_empty;
    bool is_token;
    int count;
    struct __syntax_tree **children;
    int first_line;
    void *data;
    void *sem;
} syntax_tree;

syntax_tree *new_syntax_tree(int type, int first_line, int count, ...);

void delete_syntax_tree(syntax_tree *tree);

void show_syntax_tree(syntax_tree *tree);

#define __ast_base      \
    symbol_table *syms; \
    int count;          \
    void **children

typedef struct
{
    __ast_base;
} ast;

typedef struct
{
    type *tp;
    __ast_base;
} ast_exp;

typedef struct
{
    ast_exp *cond;
    __ast_base;
} ast_if;

typedef struct
{
    ast_exp *cond;
    __ast_base;
} ast_while;

typedef struct
{
    symbol *var;
    ast_exp *init;
    __ast_base;
} ast_def;

#endif