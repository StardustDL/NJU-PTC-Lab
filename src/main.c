#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "lexical.h"
#include "syntax.h"
#include "semantics.h"

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

static char *get_option(int argc, char **argv)
{
    if (argc > 2)
    {
        return argv[2];
    }
    else
    {
        return "";
    }
}

static bool try_lexical(FILE *input)
{
    lexical_prepare(input);
    bool result = lexical_test();
    fclose(input);
    return result;
}

static ast *try_syntax(FILE *input)
{
    lexical_prepare(input);
    syntax_prepare();
    ast *result = syntax_work();
    fclose(input);
    return result;
}

static bool try_semantics(ast *ast)
{
    semantics_prepare();
    bool result = semantics_work(ast);
    return result;
}

int main(int argc, char **argv)
{
    FILE *input = get_input_file(argc, argv);

    if (input == NULL)
    {
        return 1;
    }

    char *option = get_option(argc, argv);
    if (strcmp(option, "--lexcial") == 0)
    {
        if (try_lexical(input))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if (strcmp(option, "--syntax") == 0)
    {
        ast *tree = try_syntax(input);
        if (tree != NULL)
        {
            show_ast(tree, 0);
            return 0;
        }
        else
        {
            return 1;
        }
    }

    ast *tree = try_syntax(input);
    if (tree == NULL)
    {
        return 1;
    }

    // semantics_set_log(true);

    if (!try_semantics(tree))
        return 1;

    return 0;
}
