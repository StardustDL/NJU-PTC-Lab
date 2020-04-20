#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "ast.h"
#include "symbol.h"
#include "type.h"
#include "common.h"
#include "object.h"

static bool enable_ir_log = false;
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
    if (!enable_ir_log)
        return;

    fprintf(stdout, "ir log at Line %d: ", lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(ir_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", ir_buffer);
}

void ir_set_log(bool enable)
{
    enable_ir_log = enable;
}

void ir_prepare()
{
    ir_is_passed = true;
}

bool ir_work(ast *tree)
{
    return ir_has_passed();
}

bool ir_has_passed()
{
    return ir_is_passed;
}
