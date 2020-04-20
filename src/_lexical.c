#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "lexical.h"

extern void yyrestart(FILE *input_file);
extern int yylex();
extern int yylineno;
extern int yycolumn;

static bool enable_lexical_log = false;
static bool lexical_is_passed = false;
static char lexical_buffer[1024];

void lexical_error(char *format, ...)
{
    lexical_is_passed = 0;

    fprintf(stderr, "Error type A at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(lexical_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", lexical_buffer);
}

void lexical_log(char *format, ...)
{
    if (!enable_lexical_log)
        return;

    fprintf(stdout, "Lexical log at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(lexical_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", lexical_buffer);
}

void lexical_set_log(bool enable)
{
    enable_lexical_log = enable;
}

void lexical_prepare(FILE *input)
{
    yyrestart(input);
    yylineno = 1;
    yycolumn = 1;
    lexical_is_passed = true;
}

bool lexical_test()
{
    while (yylex() != 0)
        ;
    return lexical_has_passed();
}

bool lexical_has_passed()
{
    return lexical_is_passed;
}
