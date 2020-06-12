#ifndef __ASM_H__
#define __ASM_H__

#include "ast.h"

void asm_prepare(FILE* output);

void asm_generate(ast* tree);

bool asm_has_passed();

#endif