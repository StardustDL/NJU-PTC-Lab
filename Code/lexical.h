#ifndef __LEXICAL_H__
#define __LEXICAL_H__

#include <stdio.h>
#include "common.h"

void lexical_set_log(bool enable);

void lexical_set_error(bool enable);

void lexical_prepare(FILE* input);

bool lexical_has_passed();

bool lexical_test();

#endif