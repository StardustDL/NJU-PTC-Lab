#ifndef __TYPE_H__
#define __TYPE_H__

#include "symbol.h"

type *new_type_array(type *base, int rank, int *lens);

type *new_type_func(int argc, symbol **args, type *ret);

type *new_type_struct(int memc, symbol **mems);

type *new_type_type(type* tp);

type *new_type_any();

type *new_type_unit();

type *new_type_never();

type *new_type_meta(metatype_type metatype);

type *type_array_descending(type *t);

void show_type(type *a);

bool type_full_eq(type *a, type *b, bool strict_arr);

bool type_is_type(type *a);

bool type_can_call(type *a);

bool type_can_index(type *a);

bool type_can_member(type *a);

symbol *type_can_membername(type *a, char *name);

bool type_can_logic(type *a);

bool type_can_arithmetic(type *a);

bool type_can_arithmetic2(type *a, type *b);

int type_sizeof(type* a);

#endif