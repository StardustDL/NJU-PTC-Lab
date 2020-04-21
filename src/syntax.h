#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "ast.h"

void syntax_prepare();

syntax_tree *syntax_parse();

bool syntax_has_passed();

#endif