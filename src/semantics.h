#ifndef __SEMANTICS_H__
#define __SEMANTICS_H__

#include "ast.h"

typedef type *SES_TYPE;

typedef struct
{
    type *tp;
    char *struct_name;
} SES_Specifier;

typedef struct
{
    symbol *sym;
    int lineno;
    env *ev;
} SES_FunDec;

typedef char *SES_Tag;

typedef struct __SES_Exp
{
    type *tp;
    int lineno;
    struct __SES_Exp *next;
} SES_Exp;

typedef struct __SES_VarDec
{
    symbol *sym;
    bool hasinit;
    int lineno;
    list *lens;
    struct __SES_VarDec *next;
} SES_VarDec;

symbol *get_symbol_by_id(syntax_tree *tree, env *ev);

void semantics_prepare();

bool semantics_analyse(syntax_tree *tree);

bool semantics_has_passed();

#endif