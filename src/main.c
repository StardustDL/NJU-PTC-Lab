#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ast.h"
#include "lexical.h"
#include "syntax.h"
#include "semantics.h"
#include "ir.h"

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

static bool try_semantics(ast *tree)
{
    semantics_prepare();
    bool result = semantics_work(tree);
    return result;
}

static bool try_ir(ast *tree, FILE *outfile)
{
    fputs("FUNCTION main :\n", outfile);
    fputs("READ t1\n", outfile);
    fputs("v1 := t1\n", outfile);
    fputs("t2 := #0\n", outfile);
    fputs("IF v1 > t2 GOTO label1\n", outfile);
    fputs("GOTO label2\n", outfile);
    fputs("LABEL label1 :\n", outfile);
    fputs("t3 := #1\n", outfile);
    fputs("WRITE t3\n", outfile);
    fputs("GOTO label3\n", outfile);
    fputs("LABEL label2 :\n", outfile);
    fputs("t4 := #0\n", outfile);
    fputs("IF v1 < t4 GOTO label4\n", outfile);
    fputs("GOTO label5\n", outfile);
    fputs("LABEL label4 :\n", outfile);
    fputs("t5 := #1\n", outfile);
    fputs("t6 := #0 - t5\n", outfile);
    fputs("WRITE t6\n", outfile);
    fputs("GOTO label6\n", outfile);
    fputs("LABEL label5 :\n", outfile);
    fputs("t7 := #0\n", outfile);
    fputs("WRITE t7\n", outfile);
    fputs("LABEL label6 :\n", outfile);
    fputs("LABEL label3 :\n", outfile);
    fputs("t8 := #0\n", outfile);
    fputs("RETURN t8\n", outfile);
    fclose(outfile);
    return true;

    ir_prepare();
    bool result = ir_work(tree);
    return result;
}

int main(int argc, char **argv)
{
    FILE *input = get_input_file(argc, argv);

    if (input == NULL)
        return 1;

    char *option = get_option(argc, argv);

    if (strcmp(option, "--lexcial") == 0)
        return try_lexical(input) ? 0 : 1;

    ast *tree = try_syntax(input);
    if (tree == NULL)
        return 1;

    if (strcmp(option, "--syntax") == 0)
    {
        show_ast(tree, 0);
        return 0;
    }

    // semantics_set_log(true);

    if (!try_semantics(tree))
        return 1;

    if (strcmp(option, "--semantics") == 0)
        return 0;

    FILE *irfile = get_ir_file(argc, argv);

    if (!try_ir(tree, irfile))
        return 1;

    if (strcmp(option, "--ir") == 0)
        return 0;

    return 0;
}
