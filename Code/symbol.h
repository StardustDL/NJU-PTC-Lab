#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "ast.h"

typedef enum
{
    TC_UNIT,
    TC_META,
    TC_ARRAY,
    TC_FUNC,
    TC_STRUCT
} TYPE_CLASS;

struct __symbol;

typedef struct __type
{
    TYPE_CLASS cls;
    union {
        METATYPE_type metatype;
        struct
        {
            struct __type *base;
            int rank;
            int *lens;
        };
        struct
        {
            int argc;
            struct __type **args;
            struct __type *ret;
        };
        struct
        {
            int memc;
            struct __symbol **mems;
        };
    };
} type;

typedef enum
{
    SS_DEC,
    SS_DEF
} SYMBOL_state;

typedef struct __symbol
{
    char name[40];
    type *tp;
    SYMBOL_state state;
} symbol;

typedef struct __symbol_item
{
    symbol *sym;
    struct __symbol_item *next;
} symbol_item;

typedef struct __symbol_table
{
    struct __symbol_table *parent;
    symbol_item *table;
} symbol_table;

type *new_type(TYPE_CLASS cls);

type *new_type_array(type *base, int rank, int *lens);

type *new_type_func(int argc, type **args, type *ret);

type *new_type_struct(int memc, symbol **mems);

type *new_type_unit();

type *new_type_meta(METATYPE_type metatype);

type *type_array_descending(type *t);

bool type_full_eq(type *a, type *b, bool strict_arr);

symbol *new_symbol(char *name, type *tp, SYMBOL_state state);

symbol_table *new_symbol_table(symbol_table *parent);

symbol *st_find(symbol_table *table, char *name);

symbol *st_findonly(symbol_table *table, char *name);

void st_pushfront(symbol_table *table, symbol *sym);

#endif