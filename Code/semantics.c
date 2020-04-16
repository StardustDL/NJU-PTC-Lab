#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include "semantics.h"
#include "ast.h"
#include "symbol.h"
#include "common.h"

typedef struct
{
    type *tp;
} SES_INT;

typedef struct
{
    type *tp;
} SES_FLOAT;

typedef struct
{
    symbol_table *syms;
} env;

static void analyse_EMPTY(ast *tree, env* ev)
{
    assert(tree->type == ST_EMPTY);
}
static void analyse_INT(ast *tree, env* ev)
{
    assert(tree->type == ST_INT);
    SES_INT *tag = new (SES_INT);
    tag->tp = new_type_meta(MT_INT);
    tree->tag = tag;
}
static void analyse_FLOAT(ast *tree, env* ev)
{
    assert(tree->type == ST_FLOAT);
    SES_FLOAT *tag = new (SES_FLOAT);
    tag->tp = new_type_meta(MT_FLOAT);
    tree->tag = tag;
}
static void analyse_ID(ast *tree, env* ev)
{
    assert(tree->type == ST_ID);
}
static void analyse_SEMI(ast *tree, env* ev)
{
    assert(tree->type == ST_SEMI);
}
static void analyse_COMMA(ast *tree, env* ev)
{
    assert(tree->type == ST_COMMA);
}
static void analyse_ASSIGNOP(ast *tree, env* ev)
{
    assert(tree->type == ST_ASSIGNOP);
}
static void analyse_RELOP(ast *tree, env* ev)
{
    assert(tree->type == ST_RELOP);
}
static void analyse_PLUS(ast *tree, env* ev)
{
    assert(tree->type == ST_PLUS);
}
static void analyse_MINUS(ast *tree, env* ev)
{
    assert(tree->type == ST_MINUS);
}
static void analyse_STAR(ast *tree, env* ev)
{
    assert(tree->type == ST_STAR);
}
static void analyse_DIV(ast *tree, env* ev)
{
    assert(tree->type == ST_DIV);
}
static void analyse_AND(ast *tree, env* ev)
{
    assert(tree->type == ST_AND);
}
static void analyse_OR(ast *tree, env* ev)
{
    assert(tree->type == ST_OR);
}
static void analyse_NOT(ast *tree, env* ev)
{
    assert(tree->type == ST_NOT);
}
static void analyse_DOT(ast *tree, env* ev)
{
    assert(tree->type == ST_DOT);
}
static void analyse_TYPE(ast *tree, env* ev)
{
    assert(tree->type == ST_TYPE);
}
static void analyse_LP(ast *tree, env* ev)
{
    assert(tree->type == ST_LP);
}
static void analyse_RP(ast *tree, env* ev)
{
    assert(tree->type == ST_RP);
}
static void analyse_LB(ast *tree, env* ev)
{
    assert(tree->type == ST_LB);
}
static void analyse_RB(ast *tree, env* ev)
{
    assert(tree->type == ST_RB);
}
static void analyse_LC(ast *tree, env* ev)
{
    assert(tree->type == ST_LC);
}
static void analyse_RC(ast *tree, env* ev)
{
    assert(tree->type == ST_RC);
}
static void analyse_STRUCT(ast *tree, env* ev)
{
    assert(tree->type == ST_STRUCT);
}
static void analyse_RETURN(ast *tree, env* ev)
{
    assert(tree->type == ST_RETURN);
}
static void analyse_IF(ast *tree, env* ev)
{
    assert(tree->type == ST_IF);
}
static void analyse_ELSE(ast *tree, env* ev)
{
    assert(tree->type == ST_ELSE);
}
static void analyse_WHILE(ast *tree, env* ev)
{
    assert(tree->type == ST_WHILE);
}
static void analyse_Program(ast *tree, env* ev)
{
    assert(tree->type == ST_Program);
}
static void analyse_ExtDefList(ast *tree, env* ev)
{
    assert(tree->type == ST_ExtDefList);
}
static void analyse_ExtDef(ast *tree, env* ev)
{
    assert(tree->type == ST_ExtDef);
}
static void analyse_ExtDecList(ast *tree, env* ev)
{
    assert(tree->type == ST_ExtDecList);
}
static void analyse_Specifier(ast *tree, env* ev)
{
    assert(tree->type == ST_Specifier);
}
static void analyse_StructSpecifier(ast *tree, env* ev)
{
    assert(tree->type == ST_StructSpecifier);
}
static void analyse_OptTag(ast *tree, env* ev)
{
    assert(tree->type == ST_OptTag);
}
static void analyse_Tag(ast *tree, env* ev)
{
    assert(tree->type == ST_Tag);
}
static void analyse_VarDec(ast *tree, env* ev)
{
    assert(tree->type == ST_VarDec);
}
static void analyse_FunDec(ast *tree, env* ev)
{
    assert(tree->type == ST_FunDec);
}
static void analyse_VarList(ast *tree, env* ev)
{
    assert(tree->type == ST_VarList);
}
static void analyse_ParamDec(ast *tree, env* ev)
{
    assert(tree->type == ST_ParamDec);
}
static void analyse_CompSt(ast *tree, env* ev)
{
    assert(tree->type == ST_CompSt);
}
static void analyse_StmtList(ast *tree, env* ev)
{
    assert(tree->type == ST_StmtList);
}
static void analyse_Stmt(ast *tree, env* ev)
{
    assert(tree->type == ST_Stmt);
}
static void analyse_DefList(ast *tree, env* ev)
{
    assert(tree->type == ST_DefList);
}
static void analyse_Def(ast *tree, env* ev)
{
    assert(tree->type == ST_Def);
}
static void analyse_DecList(ast *tree, env* ev)
{
    assert(tree->type == ST_DecList);
}
static void analyse_Dec(ast *tree, env* ev)
{
    assert(tree->type == ST_Dec);
}
static void analyse_Exp(ast *tree, env* ev)
{
    assert(tree->type == ST_Exp);
}
static void analyse_Args(ast *tree, env* ev)
{
    assert(tree->type == ST_Args);
}

static void analyse(ast *tree)
{
    env* ev = new(env);
    ev->syms = new_symbol_table(NULL);
    analyse_Program(tree, ev);
}

static bool enable_semantics_log = false;
static bool enable_semantics_error = true;
static bool semantics_is_passed = false;
static char semantics_buffer[1024];

void semantics_error(int type, int lineno, char *format, ...)
{
    semantics_is_passed = 0;

    if (!enable_semantics_error)
        return;

    fprintf(stderr, "Error type %d at Line %d: ", type, lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(semantics_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", semantics_buffer);
}

void semantics_log(int lineno, char *format, ...)
{
    if (!enable_semantics_log)
        return;

    fprintf(stdout, "semantics log at Line %d: ", lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(semantics_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", semantics_buffer);
}

void semantics_set_log(bool enable)
{
    enable_semantics_log = enable;
}

void semantics_set_error(bool enable)
{
    enable_semantics_error = enable;
}

void semantics_prepare()
{
    semantics_is_passed = 1;
}

bool semantics_work(ast *tree)
{
    analyse(tree);
    return semantics_has_passed();
}

bool semantics_has_passed()
{
    return semantics_is_passed;
}
