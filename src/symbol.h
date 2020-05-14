#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "list.h"

typedef enum
{
    RT_L,
    RT_S,
    RT_LE,
    RT_SE,
    RT_E,
    RT_NE
} relop_type;

typedef enum
{
    MT_INT,
    MT_FLOAT
} metatype_type;

typedef enum
{
    TC_UNIT,
    TC_ANY,
    TC_NEVER,
    TC_META,
    TC_ARRAY,
    TC_FUNC,
    TC_STRUCT,
    TC_TYPE
} type_class;

struct __symbol_table;

typedef struct __type
{
    type_class cls;
    ll hash;
    union {
        metatype_type metatype;
        struct
        {
            struct __type *base;
            int rank;
            int *lens;
        };
        struct
        {
            int argc;
            struct __symbol **args;
            struct __type *ret;
        };
        struct
        {
            int memc;
            struct __symbol **mems;
        };
        struct
        {
            struct __type *tp;
        };
    };
} type;

typedef enum
{
    SS_DEC,
    SS_DEF
} symbol_state;

typedef struct __symbol
{
    char name[64];
    int lineno;
    type *tp;
    char** names;
    symbol_state state;
} symbol;

typedef struct __symbol_table
{
    struct __symbol_table *parent;
    list *table;
} symbol_table;

symbol *new_symbol(char *name, int lineno, type *tp, symbol_state state);

symbol_table *new_symbol_table(symbol_table *parent);

symbol *st_find(symbol_table *table, char *name);

symbol *st_findonly(symbol_table *table, char *name);

int st_len(symbol_table *table);

symbol** st_to_arr(symbol_table *table);

symbol** st_revto_arr(symbol_table *table);

void st_add(symbol_table *table, symbol *sym);

#endif