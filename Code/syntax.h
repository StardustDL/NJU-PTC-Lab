#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "ast.h"

void syntax_set_log(bool enable);

void syntax_prepare();

struct ast* syntax_work();

bool syntax_has_passed();

#endif