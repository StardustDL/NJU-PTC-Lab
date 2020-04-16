#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbol.h"
#include "common.h"

symbol *new_symbol(char *name, type *tp, SYMBOL_state state)
{
    symbol *result = new (symbol);
    strcpy(result->name, name);
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
    symbol_item *si = new (symbol_item);
    si->sym = sym;
    si->next = table->table;
    table->table = si;
}

int st_len(symbol_table *table)
{
    symbol_item *cur = table->table;
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
    symbol_item *cur = table->table;
    int i = 0;
    while (cur != NULL)
    {
        result[i] = cur->sym;
        cur = cur->next;
        i++;
    }
    return result;
}

symbol **st_revto_arr(symbol_table *table)
{
    int len = st_len(table);
    symbol **result = newarr(symbol, len);
    symbol_item *cur = table->table;
    int i = 0;
    while (cur != NULL)
    {
        result[len - 1 - i] = cur->sym;
        cur = cur->next;
        i++;
    }
    return result;
}