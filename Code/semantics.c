#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "semantics.h"
#include "ast.h"

static bool enable_semantics_log = false;
static bool enable_semantics_error = true;
static bool semantics_is_passed = false;
static char semantics_buffer[1024];

void semantics_error(int type, int lineno, char *format, ...)
{
    semantics_is_passed = 0;

    if (!enable_semantics_error)
        return;

    fprintf(stderr, "Error type %d at Line %d: ", type, lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(semantics_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", semantics_buffer);
}

void semantics_log(int lineno, char *format, ...)
{
    if (!enable_semantics_log)
        return;

    fprintf(stdout, "semantics log at Line %d: ", lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(semantics_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", semantics_buffer);
}

void semantics_set_log(bool enable)
{
    enable_semantics_log = enable;
}

void semantics_set_error(bool enable)
{
    enable_semantics_error = enable;
}

void semantics_prepare()
{
    semantics_is_passed = 1;
}

bool semantics_work(ast* tree)
{
    return semantics_has_passed();
}

bool semantics_has_passed()
{
    return semantics_is_passed;
}
