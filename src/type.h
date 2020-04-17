#ifndef __TYPE_H__
#define __TYPE_H__

#include "symbol.h"

bool type_full_eq(type *a, type *b, bool strict_arr);

bool type_can_call(type *a);

bool type_can_index(type *a);

bool type_can_member(type *a);

symbol *type_can_membername(type *a, char *name);

bool type_can_logic(type *a);

bool type_can_arithmetic(type *a);

bool type_can_arithmetic2(type *a, type *b);

#endif