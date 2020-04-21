#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "symbol.h"
#include "common.h"
#include "object.h"

symbol *new_symbol(char *name, int lineno, type *tp, symbol_state state)
{
    symbol *result = new (symbol);
    strcpy(result->name, name);
    result->lineno = lineno;
    result->tp = tp;
    result->state = state;
    return result;
}

symbol_table *new_symbol_table(symbol_table *parent)
{
    symbol_table *result = new (symbol_table);
    result->parent = parent;
    result->table = NULL;
    return result;
}

symbol *st_findonly(symbol_table *table, char *name)
{
    list *cur = table->table;
    while (cur != NULL)
    {
        symbol *sym = cast(symbol, cur->obj);
        if (strcmp(sym->name, name) == 0)
            return sym;
        cur = cur->next;
    }
    return NULL;
}

symbol *st_find(symbol_table *table, char *name)
{
    symbol_table *cur = table;
    while (cur != NULL)
    {
        symbol *res = st_findonly(cur, name);
        if (res != NULL)
            return res;
        cur = cur->parent;
    }
    return NULL;
}

void st_add(symbol_table *table, symbol *sym)
{
    AssertIsNull(st_findonly(table, sym->name));

    // printf("st->push %s\n", sym->name);

    table->table = list_pushfront(table->table, sym);
}

int st_len(symbol_table *table)
{
    list *cur = table->table;
    int ans = 0;
    while (cur != NULL)
    {
        ans++;
        cur = cur->next;
    }
    return ans;
}

symbol **st_to_arr(symbol_table *table)
{
    int len = st_len(table);
    symbol **result = newarr(symbol, len);
    list *cur = table->table;
    int i = 0;
    while (cur != NULL)
    {
        result[i] = cast(symbol, cur->obj);
        cur = cur->next;
        i++;
    }
    return result;
}

symbol **st_revto_arr(symbol_table *table)
{
    int len = st_len(table);
    symbol **result = newarr(symbol, len);
    list *cur = table->table;
    int i = 0;
    while (cur != NULL)
    {
        result[len - 1 - i] = cast(symbol, cur->obj);
        cur = cur->next;
        i++;
    }
    return result;
}