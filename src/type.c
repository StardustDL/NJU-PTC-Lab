#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symbol.h"
#include "type.h"
#include "debug.h"
#include "object.h"

static type *unit = NULL;
static type *any = NULL;
static type *never = NULL;
static type *metaint = NULL, *metafloat = NULL;

static type *new_type(type_class cls)
{
    type *result = new (type);
    result->cls = cls;
    result->hash = 0;
    return result;
}

type *new_type_array(type *base, int rank, int *lens)
{
    type *result = new_type(TC_ARRAY);
    result->base = base;
    result->rank = rank;
    result->lens = lens;
    return result;
}

type *new_type_func(int argc, symbol **args, type *ret)
{
    type *result = new_type(TC_FUNC);
    result->argc = argc;
    result->args = args;
    result->ret = ret;
    return result;
}

type *new_type_struct(int memc, symbol **mems)
{
    type *result = new_type(TC_STRUCT);
    result->memc = memc;
    result->mems = mems;
    return result;
}

type *new_type_type(type *tp)
{
    type *result = new_type(TC_TYPE);
    result->tp = tp;
    return result;
}

type *new_type_unit()
{
    if (unit == NULL)
        unit = new_type(TC_UNIT);
    return unit;
}

type *new_type_any()
{
    if (any == NULL)
        any = new_type(TC_ANY);
    return any;
}

type *new_type_never()
{
    if (never == NULL)
        never = new_type(TC_NEVER);
    return never;
}

type *new_type_meta(metatype_type metatype)
{
    switch (metatype)
    {
    case MT_INT:
        if (metaint == NULL)
        {
            metaint = new_type(TC_META);
            metaint->metatype = MT_INT;
        }
        return metaint;
    case MT_FLOAT:
        if (metafloat == NULL)
        {
            metafloat = new_type(TC_META);
            metafloat->metatype = MT_FLOAT;
        }
        return metafloat;
    }
    panic("Unexpect metatype");
}

static void _show_type(type *a, int level)
{
    for (int i = 0; i < level; i++)
        printf("  ");
    switch (a->cls)
    {
    case TC_META:
        puts(a->metatype == MT_INT ? "INT" : "FLOAT");
        break;
    case TC_UNIT:
        puts("UNIT");
        break;
    case TC_ANY:
        puts("ANY");
        break;
    case TC_NEVER:
        puts("NEVER");
        break;
    case TC_ARRAY:
        printf("%dd-Array\n", a->rank);
        _show_type(a->base, level + 1);
        break;
    case TC_FUNC:
        printf("Func(argc=%d)\n", a->argc);
        for (int i = 0; i < a->argc; i++)
            _show_type(a->args[i]->tp, level + 1);
        _show_type(a->ret, level + 1);
        break;
    case TC_STRUCT:
        printf("Struct(memc=%d)\n", a->memc);
        for (int i = 0; i < a->memc; i++)
        {
            // if (strcmp(a->mems[i]->name, b->mems[i]->name) != 0) return false;
            _show_type(a->mems[i]->tp, level + 1);
        }
        break;
    case TC_TYPE:
        printf("Type\n");
        _show_type(a->tp, level + 1);
        break;
    default:
        panic("Unexpect type class %d", a->cls);
    }
}

void show_type(type *a)
{
    _show_type(a, 0);
}

bool type_full_eq(type *a, type *b, bool strict_arr)
{
    // printf("a=\n");
    // show_type(a);
    // printf("b=\n");
    // show_type(b);
    if (a == b)
        return true;
    if (a->cls == TC_ANY || b->cls == TC_ANY)
        return true;
    if (a->cls == TC_NEVER || b->cls == TC_NEVER)
        return true;
    if (a->cls != b->cls)
        return false;
    switch (a->cls)
    {
    case TC_META:
        return a->metatype == b->metatype;
    case TC_UNIT:
        return true;
    case TC_ARRAY:
        if (a->rank != b->rank)
            return false;
        if (strict_arr)
        {
            for (int i = 0; i < a->rank; i++)
                if (a->lens[i] != b->lens[i])
                    return false;
        }
        if (!type_full_eq(a->base, b->base, false))
            return false;
        return true;
    case TC_FUNC:
        if (a->argc != b->argc)
            return false;
        if (!type_full_eq(a->ret, b->ret, false))
            return false;
        for (int i = 0; i < a->argc; i++)
            if (!type_full_eq(a->args[i]->tp, b->args[i]->tp, false))
                return false;
        return true;
    case TC_STRUCT:
        if (a->memc != b->memc)
            return false;
        for (int i = 0; i < a->memc; i++)
        {
            // if (strcmp(a->mems[i]->name, b->mems[i]->name) != 0) return false;
            if (!type_full_eq(a->mems[i]->tp, b->mems[i]->tp, true))
                return false;
        }
        return true;
    }
    panic("Unexpect type class %d", a->cls);
}

int type_sizeof(type *a)
{
    switch (a->cls)
    {
    case TC_META:
        return 4;
    case TC_ARRAY:
    {
        int bsize = type_sizeof(a->base);
        int factor = 1;
        for (int i = 0; i < a->rank; i++)
            factor *= a->lens[i];
        return bsize * factor;
    }
    case TC_STRUCT:
    {
        int res = 0;
        for (int i = 0; i < a->memc; i++)
        {
            res += type_sizeof(a->mems[i]->tp);
        }
        return res;
    }
    }
    panic("Unexpect type class %d", a->cls);
}

type *type_array_descending(type *t)
{
    AssertEq(t->cls, TC_ARRAY);
    if (t->rank > 1)
    {
        type *result = new_type_array(t->base, t->rank - 1, t->lens + 1);
        return result;
    }
    else
    {
        AssertEq(t->rank, 1);
        return t->base;
    }
}

bool type_is_type(type *a)
{
    return a->cls == TC_TYPE;
}

bool type_can_call(type *a)
{
    return a->cls == TC_FUNC;
}

bool type_can_index(type *a)
{
    return a->cls == TC_ARRAY;
}

bool type_can_member(type *a)
{
    return a->cls == TC_STRUCT;
}

symbol *type_can_membername(type *a, char *name)
{
    symbol *member = NULL;
    for (int i = 0; i < a->memc; i++)
    {
        symbol *sym = a->mems[i];
        if (strcmp(sym->name, name) == 0)
        {
            member = sym;
            break;
        }
    }
    return member;
}

bool type_can_logic(type *a)
{
    return a->cls == TC_META && a->metatype == MT_INT;
}

bool type_can_arithmetic(type *a)
{
    return a->cls == TC_META;
}

bool type_can_arithmetic2(type *a, type *b)
{
    return type_can_arithmetic(a) && type_can_arithmetic(b) && a->metatype == b->metatype;
}