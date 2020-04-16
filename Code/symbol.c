#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbol.h"

symbol *new_symbol(char *name, type *tp, SYMBOL_state state)
{
    symbol *result = (symbol *)malloc(sizeof(symbol));
    memset(result, 0, sizeof(symbol));
    strcpy(result->name, name);
    result->tp = tp;
    result->state = state;
    return result;
}

symbol_table *new_symbol_table(symbol_table *parent)
{
    symbol_table *result = (symbol_table *)malloc(sizeof(symbol_table));
    memset(result, 0, sizeof(symbol_table));
    result->parent = parent;
    result->table = NULL;
    return result;
}

symbol *st_findonly(symbol_table *table, char *name)
{
    symbol_item *cur = table->table;
    while (cur != NULL)
    {
        if (strcmp(cur->sym->name, name) == 0)
            return cur->sym;
        cur = cur->next;
    }
    return NULL;
}

symbol *st_find(symbol_table *table, char *name)
{
    symbol_table *cur = table;
    while (cur != NULL)
    {
        symbol *res = st_findonly(table, name);
        if (res != NULL)
            return res;
        cur = cur->parent;
    }
    return NULL;
}

void st_pushfront(symbol_table *table, symbol *sym)
{
    assert(st_findonly(table, sym->name) == NULL);
    symbol_item *si = (symbol_item *)malloc(sizeof(symbol_item));
    memset(si, 0, sizeof(symbol_item));
    si->sym = sym;
    si->next = table->table;
    table->table = si;
}