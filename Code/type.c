#include <string.h>
#include <stdlib.h>
#include "symbol.h"
#include "assert.h"

type *new_type(TYPE_CLASS cls)
{
    type *result = new(type);
    result->cls = cls;
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

type *new_type_func(int argc, type **args, type *ret)
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

type *new_type_unit()
{
    return new_type(TC_UNIT);
}

type *new_type_meta(METATYPE_type metatype)
{
    type *result = new_type(TC_META);
    result->metatype = metatype;
    return result;
}

bool type_full_eq(type *a, type *b, bool strict_arr)
{
    if (a == b)
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
            if (!type_full_eq(a->args[i], b->args[i], false))
                return false;
        return true;
    case TC_STRUCT:
        if (a->memc != b->memc)
            return false;
        for (int i = 0; i < a->memc; i++)
        {
            if (strcmp(a->mems[i]->name, b->mems[i]->name) != 0)
                return false;
            if (!type_full_eq(a->mems[i]->tp, b->mems[i]->tp, true))
                return false;
        }
        return true;
    }
    assert(0);
}

type *type_array_descending(type *t)
{
    assert(t->cls == TC_ARRAY);
    type *result = new_type(TC_ARRAY);
    result->rank = t->rank - 1;
    result->lens = t->lens + 1;
    return result;
}