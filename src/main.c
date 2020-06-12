#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ast.h"
#include "lexical.h"
#include "syntax.h"
#include "semantics.h"
#include "ir.h"
#include "asm.h"

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

static FILE *get_ir_file(int argc, char **argv)
{
    if (argc > 2 && strncmp(argv[2], "--", 2) != 0)
    {
        FILE *f;
        if (!(f = fopen(argv[2], "w")))
        {
            perror(argv[2]);
            return NULL;
        }
        return f;
    }
    else
    {
        return stdout;
    }
}

static FILE *get_asm_file(int argc, char **argv)
{
    if (argc > 2 && strncmp(argv[2], "--", 2) != 0)
    {
        FILE *f;
        if (!(f = fopen(argv[2], "w")))
        {
            perror(argv[2]);
            return NULL;
        }
        return f;
    }
    else
    {
        return stdout;
    }
}

static char *get_option(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "--", 2) == 0)
        {
            return argv[i];
        }
    }
    return "";
}

static bool try_lexical(FILE *input)
{
    lexical_prepare(input);
    bool result = lexical_test();
    return result;
}

static syntax_tree *try_syntax(FILE *input)
{
    lexical_prepare(input);
    syntax_prepare();
    syntax_tree *result = syntax_parse();
    return result;
}

static bool try_semantics(syntax_tree *tree)
{
    semantics_prepare();
    bool result = semantics_analyse(tree);
    return result;
}

static ast *try_ir(syntax_tree *tree)
{
    ir_prepare();
    ast *at = ir_translate(tree);
    return at;
}

int main(int argc, char **argv)
{
    FILE *input = get_input_file(argc, argv);

    if (input == NULL)
        return 1;

    char *option = get_option(argc, argv);

    if (strcmp(option, "--lexcial") == 0)
    {
        int exitcode = try_lexical(input) ? 0 : 1;
        fclose(input);
        return exitcode;
    }

    syntax_tree *tree = try_syntax(input);
    fclose(input);
    if (tree == NULL)
        return 1;

    if (strcmp(option, "--syntax") == 0)
    {
        show_syntax_tree(tree);
        return 0;
    }

    if (!try_semantics(tree))
        return 1;

    if (strcmp(option, "--semantics") == 0)
        return 0;

    ast *at = try_ir(tree);

    if (at == NULL)
        return 1;

    if (strcmp(option, "--ir") == 0)
    {
        FILE *irfile = get_ir_file(argc, argv);

        ir_linearise(at, irfile);

        if (irfile != stdout)
            fclose(irfile);

        return 0;
    }

    FILE *asmfile = get_asm_file(argc, argv);

    asm_prepare(asmfile);

    asm_generate(at);

    if (asmfile != stdout)
        fclose(asmfile);

    return 0;
}
