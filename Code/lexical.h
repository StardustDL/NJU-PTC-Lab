#ifndef __LEXICAL_H__
#define __LEXICAL_H__

#include <stdio.h>

void lexical_set_log(int enable);

void lexical_set_error(int enable);

void lexical_prepare(FILE* input);

int lexical_has_passed();

int lexical_test();

#endif