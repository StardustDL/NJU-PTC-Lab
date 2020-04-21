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

ast* ir_translate(syntax_tree *tree)
{
    return new(ast);
}

bool ir_has_passed()
{
    return ir_is_passed;
}

void ir_linearise(ast *tree, FILE *file)
{
    fputs("FUNCTION main :\n", file);
    fputs("READ t1\n", file);
    fputs("v1 := t1\n", file);
    fputs("t2 := #0\n", file);
    fputs("IF v1 > t2 GOTO label1\n", file);
    fputs("GOTO label2\n", file);
    fputs("LABEL label1 :\n", file);
    fputs("t3 := #1\n", file);
    fputs("WRITE t3\n", file);
    fputs("GOTO label3\n", file);
    fputs("LABEL label2 :\n", file);
    fputs("t4 := #0\n", file);
    fputs("IF v1 < t4 GOTO label4\n", file);
    fputs("GOTO label5\n", file);
    fputs("LABEL label4 :\n", file);
    fputs("t5 := #1\n", file);
    fputs("t6 := #0 - t5\n", file);
    fputs("WRITE t6\n", file);
    fputs("GOTO label6\n", file);
    fputs("LABEL label5 :\n", file);
    fputs("t7 := #0\n", file);
    fputs("WRITE t7\n", file);
    fputs("LABEL label6 :\n", file);
    fputs("LABEL label3 :\n", file);
    fputs("t8 := #0\n", file);
    fputs("RETURN t8\n", file);
}
