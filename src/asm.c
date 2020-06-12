// #define DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "ast.h"
#include "symbol.h"
#include "type.h"
#include "common.h"
#include "object.h"
#include "debug.h"
#include "semantics.h"

void asm_log(int lineno, char *format, ...);

static FILE *asm_output = NULL;
static bool asm_is_passed = false;
static char asm_buffer[1024];

void asm_error(int type, int lineno, char *format, ...)
{
    asm_is_passed = 0;

    fprintf(stderr, "Error type %d at Line %d: ", type, lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(asm_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", asm_buffer);
}

void asm_log(int lineno, char *format, ...)
{
#ifdef DEBUG

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(asm_buffer, format, aptr);
    va_end(aptr);

#endif

    Info("Line %d: %s", lineno, asm_buffer);
}

void asm_prepare(FILE *output)
{
    asm_is_passed = true;
    asm_output = output;
}

bool asm_has_passed()
{
    return asm_is_passed;
}

static void printHeader()
{
    fputs(".data\n", asm_output);
    fputs("_prompt: .asciiz \"Enter an integer:\"\n",asm_output);
    fputs("_ret: .asciiz \"\\n\"\n",asm_output);
    fputs(".globl main\n",asm_output);
    fputs(".text\n",asm_output);
    fputs("read:\n",asm_output);
    fputs("  li $v0, 4\n",asm_output);
    fputs("  la $a0, _prompt\n",asm_output);
    fputs("  syscall\n",asm_output);
    fputs("  jr $ra\n",asm_output);
    fputs("\n",asm_output);
    fputs("write:\n",asm_output);
    fputs("  li $v0, 1\n",asm_output);
    fputs("  syscall\n",asm_output);
    fputs("  li $v0, 4\n",asm_output);
    fputs("  la $a0, _ret\n",asm_output);
    fputs("  syscall\n",asm_output);
    fputs("  move $v0, $0\n",asm_output);
    fputs("  jr $ra\n",asm_output);
    fputs("\n",asm_output);
}

static void printOprand(irop *op)
{
    switch (op->kind)
    {
    case IRO_Variable:
        fprintf(asm_output, "%s", op->var->name);
        break;
    case IRO_Constant:
        fprintf(asm_output, "#%d", op->value);
        break;
    case IRO_Deref:
        fprintf(asm_output, "*%s", op->var->name);
        break;
    case IRO_Ref:
        fprintf(asm_output, "&%s", op->var->name);
        break;
    }
}

static void rewrite_Label(ircode *code)
{
    asm_log(0, "%s", "Label");
}
static void rewrite_Func(ircode *code)
{
    asm_log(0, "%s", "Func");
}
static void rewrite_Assign(ircode *code)
{
    asm_log(0, "%s", "Assign");
}
static void rewrite_Add(ircode *code)
{
    asm_log(0, "%s", "Add");
}
static void rewrite_Sub(ircode *code)
{
    asm_log(0, "%s", "Sub");
}
static void rewrite_Mul(ircode *code)
{
    asm_log(0, "%s", "Mul");
}
static void rewrite_Div(ircode *code)
{
    asm_log(0, "%s", "Div");
}
static void rewrite_Goto(ircode *code)
{
    asm_log(0, "%s", "Goto");
}
static void rewrite_Branch(ircode *code)
{
    asm_log(0, "%s", "Branch");
}
static void rewrite_Return(ircode *code)
{
    asm_log(0, "%s", "Return");
}
static void rewrite_Dec(ircode *code)
{
    asm_log(0, "%s", "Dec");
}
static void rewrite_Arg(ircode *code)
{
    asm_log(0, "%s", "Arg");
}
static void rewrite_Call(ircode *code)
{
    asm_log(0, "%s", "Call");
}
static void rewrite_Param(ircode *code)
{
    asm_log(0, "%s", "Param");
}
static void rewrite_Read(ircode *code)
{
    asm_log(0, "%s", "Read");
}
static void rewrite_Write(ircode *code)
{
    asm_log(0, "%s", "Write");
}

void asm_generate(ast *tree)
{
    printHeader();
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            continue;
        switch (code->kind)
        {
        case IR_Label:
            rewrite_Label(code);
            break;
        case IR_Func:
            rewrite_Func(code);
            break;
        case IR_Assign:
            rewrite_Assign(code);
            break;
        case IR_Add:
            rewrite_Add(code);
            break;
        case IR_Sub:
            rewrite_Sub(code);
            break;
        case IR_Mul:
            rewrite_Mul(code);
            break;
        case IR_Div:
            rewrite_Div(code);
            break;
        case IR_Goto:
            rewrite_Goto(code);
            break;
        case IR_Branch:
            rewrite_Branch(code);
            break;
        case IR_Return:
            rewrite_Return(code);
            break;
        case IR_Dec:
            rewrite_Dec(code);
            break;
        case IR_Arg:
            rewrite_Arg(code);
            break;
        case IR_Call:
            rewrite_Call(code);
            break;
        case IR_Param:
            rewrite_Param(code);
            break;
        case IR_Read:
            rewrite_Read(code);
            break;
        case IR_Write:
            rewrite_Write(code);
            break;
        }
    }
}
