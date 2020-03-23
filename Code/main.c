#include <stdio.h>
#include <string.h>
#include "lexical.h"
#include "syntax.h"

static FILE *get_input_file(int argc, char **argv)
{
    if (argc > 1)
    {
        FILE *f;
        if (!(f = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return NULL;
        }
        return f;
    }
    else
    {
        return stdin;
    }
}

static int try_lexical(FILE *input)
{
    lexical_prepare(input);
    int result = lexical_test();
    fclose(input);
    return result;
}

static struct ast* try_syntax(FILE *input)
{
    lexical_prepare(input);
    syntax_prepare();
    struct ast* result = syntax_work();
    fclose(input);
    return result;
}

int main(int argc, char **argv)
{
    // lexical_set_log(1);
    // syntax_set_log(1);

    FILE *input = get_input_file(argc, argv);

    if (input == NULL)
    {
        return 1;
    }

    lexical_set_error(0);
    try_lexical(input);
    lexical_set_error(1);

    input = get_input_file(argc, argv);

    struct ast* ast = try_syntax(input);
    if (ast != NULL)
    {
        show_ast(ast, 0);
        return 0;
    }
    else
    {
        return 1;
    }    
}
