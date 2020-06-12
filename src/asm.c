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

const char *reg_names[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1",
    "gp", "sp", "fp", "ra"};

typedef struct
{
    int id;
} reg;

static reg *regs[32];

static FILE *asm_output = NULL;
static bool asm_is_passed = false;
static char asm_buffer[1024];

static reg *new_reg(int id)
{
    reg *r = new (reg);
    r->id = id;
    return r;
}

static void asm_out(char *format, ...)
{
    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(asm_buffer, format, aptr);
    va_end(aptr);

    fprintf(asm_output, "%s\n", asm_buffer);
}

static void asm_out_instr(char *format, ...)
{
    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(asm_buffer, format, aptr);
    va_end(aptr);

    fprintf(asm_output, "  %s\n", asm_buffer);
}

static void gen_label(char *name)
{
    asm_out("%s:", name);
}

static void rewrite_Label(ircode *code)
{
    asm_log(0, "%s", "Label");
    gen_label(code->label->name);
}
static void rewrite_Func(ircode *code)
{
    asm_log(0, "%s", "Func");
    gen_label(code->label->name);
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
    for (int i = 0; i < 32; i++)
        regs[i] = new_reg(i);
}

bool asm_has_passed()
{
    return asm_is_passed;
}

static void printHeader()
{
    fputs(".data\n", asm_output);
    fputs("_prompt: .asciiz \"Enter an integer:\"\n", asm_output);
    fputs("_ret: .asciiz \"\\n\"\n", asm_output);
    fputs(".globl main\n", asm_output);
    fputs(".text\n", asm_output);
    fputs("read:\n", asm_output);
    fputs("  li $v0, 4\n", asm_output);
    fputs("  la $a0, _prompt\n", asm_output);
    fputs("  syscall\n", asm_output);
    fputs("  jr $ra\n", asm_output);
    fputs("\n", asm_output);
    fputs("write:\n", asm_output);
    fputs("  li $v0, 1\n", asm_output);
    fputs("  syscall\n", asm_output);
    fputs("  li $v0, 4\n", asm_output);
    fputs("  la $a0, _ret\n", asm_output);
    fputs("  syscall\n", asm_output);
    fputs("  move $v0, $0\n", asm_output);
    fputs("  jr $ra\n", asm_output);
    fputs("\n", asm_output);
}

static void printOprand(irop *op, FILE *file)
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

static void asm_ir_comment(ircode *code, FILE *file)
{
    fprintf(file, "# ");
    switch (code->kind)
    {
    case IR_Label:
        fprintf(file, "LABEL %s :\n", code->label->name);
        break;
    case IR_Func:
        fprintf(file, "FUNCTION %s :\n", code->label->name);
        break;
    case IR_Assign:
        printOprand(code->assign.left, file);
        fprintf(file, " := ");
        printOprand(code->assign.right, file);
        fprintf(file, "\n");
        break;
    case IR_Add:
        printOprand(code->bop.target, file);
        fprintf(file, " := ");
        printOprand(code->bop.op1, file);
        fprintf(file, " + ");
        printOprand(code->bop.op2, file);
        fprintf(file, "\n");
        break;
    case IR_Sub:
        printOprand(code->bop.target, file);
        fprintf(file, " := ");
        printOprand(code->bop.op1, file);
        fprintf(file, " - ");
        printOprand(code->bop.op2, file);
        fprintf(file, "\n");
        break;
    case IR_Mul:
        printOprand(code->bop.target, file);
        fprintf(file, " := ");
        printOprand(code->bop.op1, file);
        fprintf(file, " * ");
        printOprand(code->bop.op2, file);
        fprintf(file, "\n");
        break;
    case IR_Div:
        printOprand(code->bop.target, file);
        fprintf(file, " := ");
        printOprand(code->bop.op1, file);
        fprintf(file, " / ");
        printOprand(code->bop.op2, file);
        fprintf(file, "\n");
        break;
    case IR_Goto:
        fprintf(file, "GOTO %s\n", code->label->name);
        break;
    case IR_Branch:
        fprintf(file, "IF ");
        printOprand(code->branch.op1, file);
        switch (code->branch.relop)
        {
        case RT_L:
            fprintf(file, " > ");
            break;
        case RT_S:
            fprintf(file, " < ");
            break;
        case RT_LE:
            fprintf(file, " >= ");
            break;
        case RT_SE:
            fprintf(file, " <= ");
            break;
        case RT_E:
            fprintf(file, " == ");
            break;
        case RT_NE:
            fprintf(file, " != ");
            break;
        }
        printOprand(code->branch.op2, file);
        fprintf(file, " GOTO %s", code->branch.target->name);
        fprintf(file, "\n");
        break;
    case IR_Return:
        fprintf(file, "RETURN ");
        printOprand(code->ret, file);
        fprintf(file, "\n");
        break;
    case IR_Dec:
        fprintf(file, "DEC %s %d\n", code->dec.op->var->name, code->dec.size);
        break;
    case IR_Arg:
        fprintf(file, "ARG ");
        printOprand(code->arg, file);
        fprintf(file, "\n");
        break;
    case IR_Call:
        fprintf(file, "%s := CALL %s\n", code->call.ret->var->name, code->call.func->name);
        break;
    case IR_Param:
        fprintf(file, "PARAM %s\n", code->param->var->name);
        break;
    case IR_Read:
        fprintf(file, "READ %s\n", code->read->var->name);
        break;
    case IR_Write:
        fprintf(file, "WRITE ");
        printOprand(code->write, file);
        fprintf(file, "\n");
        break;
    }
}

void asm_generate(ast *tree)
{
    printHeader();
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            continue;
        asm_ir_comment(code, asm_output);
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
