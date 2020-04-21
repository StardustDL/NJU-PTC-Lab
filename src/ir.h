#ifndef __IR_H__
#define __IR_H__

#include "ast.h"

void ir_prepare();

bool ir_work(syntax_tree *tree);

bool ir_has_passed();

#endif