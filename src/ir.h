#ifndef __IR_H__
#define __IR_H__

#include "ast.h"

void ir_prepare();

ast* ir_translate(syntax_tree *tree);

void ir_linearise(ast* tree, FILE* file);

bool ir_has_passed();

#endif