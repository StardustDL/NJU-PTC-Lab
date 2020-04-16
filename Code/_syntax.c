#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "lexical.h"

extern int yylineno;
extern int yyparse();

static ast *syntax_result = NULL;
static bool enable_syntax_log = false;
static bool syntax_is_passed = false;
static char syntax_buffer[1024];

void syntax_set_result(struct ast *result)
{
    syntax_result = result;
}

void syntax_error(char *format, ...)
{
    syntax_is_passed = false;

    fprintf(stderr, "Error type B at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", syntax_buffer);
}

void syntax_error_atline(int lineno, char *format, ...)
{
    syntax_is_passed = false;

    fprintf(stderr, "Error type B at Line %d: ", lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", syntax_buffer);
}

void syntax_log(char *format, ...)
{
    if (!enable_syntax_log)
        return;

    fprintf(stdout, "Syntax log at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", syntax_buffer);
}

void yyerror(const char *format, ...)
{
    syntax_is_passed = false;

    if (strcmp(format, "syntax error") == 0)
    {
        syntax_log("syntax error");
        return;
    }
    else
    {
    }

    fprintf(stderr, "Error type B at Line %d: ", yylineno);

    if (strstr(format, "syntax error, unexpect"))
    {
        va_list aptr;
        int ret;

        va_start(aptr, format);

        vsprintf(syntax_buffer, format + 15, aptr);
        va_end(aptr);

        fprintf(stderr, "%c%s.\n", toupper(*(format + 14)), syntax_buffer);
    }
    else
    {
        va_list aptr;
        int ret;

        va_start(aptr, format);

        vsprintf(syntax_buffer, format, aptr);
        va_end(aptr);

        fprintf(stderr, "%s.\n", syntax_buffer);
    }
}

void syntax_set_log(bool enable)
{
    enable_syntax_log = enable;
}

void syntax_prepare()
{
    syntax_is_passed = true;
    syntax_result = NULL;
}

bool syntax_has_passed()
{
    return syntax_is_passed && lexical_has_passed();
}

ast *syntax_work()
{
    yyparse();
    if (syntax_has_passed())
    {
        return syntax_result;
    }
    else
    {
        return NULL;
    }
}