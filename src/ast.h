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

typedef enum
{
    OP_Assign,
    OP_And,
    OP_Or,
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_DIV,
    OP_NEG,
    OP_NOT,
} op_type;

typedef struct
{
    int count;
    void **children;
} ast;

typedef struct
{
    type *tp;
    union {
        unsigned int vint;
        float vfloat;
    };
} ast_exp_const;

typedef struct
{
    symbol* sym;
} ast_exp_id;

typedef struct
{
    op_type type;
    void *left;
    void *right;
    type *tp;
} ast_exp_bop;

typedef struct
{
    symbol* sym;
    void** args;
} ast_exp_call;

typedef struct
{
    symbol* sym;
    int offset;
} ast_exp_offset;

typedef struct
{
    void *exp;
} ast_return;

typedef struct
{
    void *cond;
    void *body;
} ast_if;

typedef struct
{
    void *cond;
    void *body;
} ast_while;

typedef struct
{
    symbol *sym;
    void *init;
} ast_var;

typedef struct
{
    symbol *sym;
    void* body;
} ast_func;

#endif