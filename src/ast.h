#ifndef __AST_H__
#define __AST_H__

#include "common.h"
#include "symbol.h"

typedef struct
{
    symbol_table *syms;
    type *declare_type;
    type *ret_type;
    bool in_struct;
    bool in_vardec;
} env;

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
    env *ev;
} syntax_tree;

syntax_tree *new_syntax_tree(int type, int first_line, int count, ...);

void delete_syntax_tree(syntax_tree *tree);

void show_syntax_tree(syntax_tree *tree);

typedef enum
{
    IR_Label,
    IR_Func,
    IR_Assign,
    IR_Add,
    IR_Sub,
    IR_Mul,
    IR_Div,
    IR_Goto,
    IR_Branch,
    IR_Return,
    IR_Dec,
    IR_Arg,
    IR_Call,
    IR_Param,
    IR_Read,
    IR_Write
} irc_type;

typedef enum
{
    IRO_Variable,
    IRO_Constant,
    IRO_Ref,
    IRO_Deref,
} irop_type;

typedef struct
{
    char name[64];
} irvar;

typedef struct
{
    char name[64];
} irlabel;

typedef struct
{
    irop_type kind;
    union {
        irvar *var;
        int value;
    };
} irop;

typedef struct
{
    irc_type kind;
    union {
        struct
        {
            irop *left, *right;
        } assign;
        struct
        {
            irop *op1, *op2, *target;
        } bop;
        irlabel *label;
        struct
        {
            irop *op1, *op2;
            relop_type relop;
            irlabel *target;
        } branch;
        irop *ret;
        struct
        {
            irop *op;
            int size;
        } dec;
        irop *arg;
        irop *param;
        irop *read;
        irop *write;
        struct
        {
            irlabel *func;
            irop *ret;
        } call;
    };
} ircode;

typedef struct
{
    int len;
    void **codes;
} ast;

#endif