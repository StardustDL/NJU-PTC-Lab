#ifndef __SEMANTICS_H__
#define __SEMANTICS_H__

#include "ast.h"

void semantics_set_log(bool enable);

void semantics_prepare();

bool semantics_work(ast *tree);

bool semantics_has_passed();

#endif