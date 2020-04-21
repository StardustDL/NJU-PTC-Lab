#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "ast.h"
#include "symbol.h"
#include "type.h"
#include "common.h"
#include "object.h"
#include "debug.h"

static bool ir_is_passed = false;
static char ir_buffer[1024];

void ir_error(int type, int lineno, char *format, ...)
{
    ir_is_passed = 0;

    fprintf(stderr, "Error type %d at Line %d: ", type, lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(ir_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", ir_buffer);
}

void ir_log(int lineno, char *format, ...)
{
#ifdef DEBUG

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(ir_buffer, format, aptr);
    va_end(aptr);

#endif

    Info("Line %d: %s\n", lineno, ir_buffer);
}

void ir_prepare()
{
    ir_is_passed = true;
}

bool ir_work(syntax_tree *tree)
{
    return ir_has_passed();
}

bool ir_has_passed()
{
    return ir_is_passed;
}
