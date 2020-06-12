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

const char *store_tvars = "bee845c68e1c_store_tvars";
const char *load_tvars = "bee845c68e1c_load_tvars";
static void **vars = NULL;
static ast *ast_tree = NULL;

const char *reg_names[32] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1",
    "$gp", "$sp", "$fp", "$ra"};

typedef struct
{
    int id;
} reg;

static reg *regs[32];

static FILE *asm_output = NULL;
static bool asm_is_passed = false;
static char asm_buffer[1024];

#pragma region helper functions

static reg *new_reg(int id)
{
    reg *r = new (reg);
    r->id = id;
    return r;
}

static reg *get_reg_rs() // ra for var store
{
    return regs[24];
}

static reg *get_reg_fp()
{
    return regs[30];
}

static reg *get_reg_ra()
{
    return regs[31];
}

static reg *get_reg_sp()
{
    return regs[29];
}

static reg *get_reg_v0()
{
    return regs[2];
}

static reg *get_reg_a0()
{
    return regs[4];
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

static void gen_label(const char *name)
{
    asm_out("%s:", name);
}

static void gen_lw(reg *rt, reg *rs, short imm)
{
    asm_out_instr("lw %s, %d(%s)", reg_names[rt->id], (int)imm, reg_names[rs->id]);
}

static void gen_sw(reg *rt, reg *rs, short imm)
{
    asm_out_instr("sw %s, %d(%s)", reg_names[rt->id], (int)imm, reg_names[rs->id]);
}

static void gen_la(reg *dest, const char *addr)
{
    asm_out_instr("la %s, %s", reg_names[dest->id], addr);
}

static void gen_li(reg *dest, int imm)
{
    asm_out_instr("li %s, %d", reg_names[dest->id], imm);
}

static void gen_move(reg *dest, reg *src)
{
    asm_out_instr("move %s, %s", reg_names[dest->id], reg_names[src->id]);
}

static void gen_bgt(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("bgt %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_bge(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("bge %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_blt(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("blt %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_ble(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("ble %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_beq(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("beq %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_bne(reg *src1, reg *src2, const char *label)
{
    asm_out_instr("bne %s, %s, %s", reg_names[src1->id], reg_names[src2->id], label);
}

static void gen_add(reg *rd, reg *rs, reg *rt)
{
    asm_out_instr("add %s, %s, %s", reg_names[rd->id], reg_names[rs->id], reg_names[rt->id]);
}

static void gen_addi(reg *rt, reg *rs, short imm)
{
    asm_out_instr("addi %s, %s, %d", reg_names[rt->id], reg_names[rs->id], (int)imm);
}

static void gen_sub(reg *rd, reg *rs, reg *rt)
{
    asm_out_instr("sub %s, %s, %s", reg_names[rd->id], reg_names[rs->id], reg_names[rt->id]);
}

static void gen_mul(reg *rd, reg *rs, reg *rt)
{
    asm_out_instr("mul %s, %s, %s", reg_names[rd->id], reg_names[rs->id], reg_names[rt->id]);
}

static void gen_div(reg *rd, reg *rs, reg *rt)
{
    asm_out_instr("div %s, %s, %s", reg_names[rd->id], reg_names[rs->id], reg_names[rt->id]);
}

static void gen_j(const char *label)
{
    asm_out_instr("j %s", label);
}

static void gen_jal(const char *label)
{
    asm_out_instr("jal %s", label);
}

static void gen_jr(reg *r)
{
    asm_out_instr("jr %s", reg_names[r->id]);
}

static void gen_push(reg *r)
{
    reg *sp = get_reg_sp();
    gen_addi(sp, sp, -4);
    gen_sw(r, sp, 0);
}

static void gen_pop(reg *r)
{
    reg *sp = get_reg_sp();
    gen_lw(r, sp, 0);
    gen_addi(sp, sp, 4);
}

static reg *get_reg()
{
    static int current = 8;

    reg *r = regs[current];
    current++;
    if (current > 23)
        current = 8;
    return r;
}

static void prepare_var(irvar *var, reg *r)
{
    reg *temp = get_reg();
    gen_la(temp, var->name);
    gen_lw(r, temp, 0);
}

static void prepare_oprand(irop *op, reg *r)
{
    reg *temp = get_reg();
    switch (op->kind)
    {
    case IRO_Variable:
        prepare_var(op->var, r);
        break;
    case IRO_Constant:
        gen_li(r, op->value);
        break;
    case IRO_Deref:
        gen_la(temp, op->var->name);
        gen_lw(temp, temp, 0);
        gen_lw(r, temp, 0);
        break;
    case IRO_Ref: // DEC set var's data with addr
        gen_la(temp, op->var->name);
        gen_lw(r, temp, 0);
        break;
    }
}

static void apply_var(irvar *var, reg *r)
{
    reg *temp = get_reg();
    gen_la(temp, var->name);
    gen_sw(r, temp, 0);
}

static void apply_oprand(irop *op, reg *r)
{
    reg *temp = get_reg();
    switch (op->kind)
    {
    case IRO_Variable:
        apply_var(op->var, r);
        break;
    case IRO_Constant:
        panic("Try to apply constant oprand in left");
        break;
    case IRO_Deref:
        gen_la(temp, op->var->name);
        gen_lw(temp, temp, 0);
        gen_sw(r, temp, 0);
        break;
    case IRO_Ref:
        panic("Try to apply ref oprand in left");
        break;
    }
}

static void gen_store_vars()
{
    asm_out("# store_vars");
    reg *r = get_reg();
    for (int i = 0; i < ast_tree->var_count; i++)
    {
        irvar *var = cast(irvar, vars[i]);
        prepare_var(var, r);
        gen_push(r);
    }
}

static void gen_load_vars()
{
    asm_out("# load_vars");
    reg *r = get_reg();
    for (int i = ast_tree->var_count - 1; i >= 0; i--)
    {
        irvar *var = cast(irvar, vars[i]);
        gen_pop(r);
        apply_var(var, r);
    }
}

#pragma endregion

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
    reg *right = get_reg();
    prepare_oprand(code->assign.right, right);
    apply_oprand(code->assign.left, right);
}
static void rewrite_Add(ircode *code)
{
    asm_log(0, "%s", "Add");
    reg *op1 = get_reg(), *op2 = get_reg(), *res = get_reg();
    prepare_oprand(code->bop.op1, op1);
    prepare_oprand(code->bop.op2, op2);
    gen_add(res, op1, op2);
    apply_oprand(code->bop.target, res);
}
static void rewrite_Sub(ircode *code)
{
    asm_log(0, "%s", "Sub");
    reg *op1 = get_reg(), *op2 = get_reg(), *res = get_reg();
    prepare_oprand(code->bop.op1, op1);
    prepare_oprand(code->bop.op2, op2);
    gen_sub(res, op1, op2);
    apply_oprand(code->bop.target, res);
}
static void rewrite_Mul(ircode *code)
{
    asm_log(0, "%s", "Mul");
    reg *op1 = get_reg(), *op2 = get_reg(), *res = get_reg();
    prepare_oprand(code->bop.op1, op1);
    prepare_oprand(code->bop.op2, op2);
    gen_mul(res, op1, op2);
    apply_oprand(code->bop.target, res);
}
static void rewrite_Div(ircode *code)
{
    asm_log(0, "%s", "Div");
    reg *op1 = get_reg(), *op2 = get_reg(), *res = get_reg();
    prepare_oprand(code->bop.op1, op1);
    prepare_oprand(code->bop.op2, op2);
    gen_div(res, op1, op2);
    apply_oprand(code->bop.target, res);
}
static void rewrite_Goto(ircode *code)
{
    asm_log(0, "%s", "Goto");
    gen_j(code->label->name);
}
static void rewrite_Branch(ircode *code)
{
    asm_log(0, "%s", "Branch");
    reg *op1 = get_reg(), *op2 = get_reg();
    prepare_oprand(code->branch.op1, op1);
    prepare_oprand(code->branch.op2, op2);
    switch (code->branch.relop)
    {
    case RT_L: // >
        gen_bgt(op1, op2, code->branch.target->name);
        break;
    case RT_S: // <
        gen_blt(op1, op2, code->branch.target->name);
        break;
    case RT_LE: // >=
        gen_bge(op1, op2, code->branch.target->name);
        break;
    case RT_SE: // <=
        gen_ble(op1, op2, code->branch.target->name);
        break;
    case RT_E: // ==
        gen_beq(op1, op2, code->branch.target->name);
        break;
    case RT_NE: // !=
        gen_bne(op1, op2, code->branch.target->name);
        break;
    }
}
static void rewrite_Return(ircode *code)
{
    asm_log(0, "%s", "Return");
    prepare_oprand(code->ret, get_reg_v0());
    gen_jr(get_reg_ra());
}
static void rewrite_Dec(ircode *code)
{
    asm_log(0, "%s", "Dec");
    reg *sp = get_reg_sp(), *r = get_reg();
    gen_li(r, code->dec.size);
    gen_sub(sp, sp, r);
    apply_oprand(code->dec.op, sp);
}
static bool is_incall = false;
static void prepare_call()
{
    if (!is_incall)
    {
        gen_store_vars();
        reg *ra = get_reg_ra(), *fp = get_reg_fp(), *sp = get_reg_sp();
        gen_push(ra);
        gen_push(fp);
        gen_move(fp, sp);
        is_incall = true;
    }
}
static void end_call()
{
    Assert(is_incall, "Not incall");
    reg *ra = get_reg_ra(), *fp = get_reg_fp(), *sp = get_reg_sp();
    gen_move(sp, fp);
    gen_pop(fp);
    gen_pop(ra);
    gen_load_vars();
    is_incall = false;
}
static void rewrite_Arg(ircode *code)
{
    asm_log(0, "%s", "Arg");
    prepare_call();
    reg *r = get_reg();
    prepare_oprand(code->arg, r);
    gen_push(r);
}
static void rewrite_Call(ircode *code)
{
    asm_log(0, "%s", "Call");
    prepare_call();
    gen_jal(code->call.func->name);
    end_call();
    reg *res = get_reg();
    gen_move(res, get_reg_v0());
    apply_oprand(code->call.ret, res);
}
static void rewrite_Param(ircode *code)
{
    asm_log(0, "%s", "Param");
    reg *r = get_reg();
    gen_pop(r);
    apply_oprand(code->param, r);
}
static void rewrite_Read(ircode *code)
{
    asm_log(0, "%s", "Read");
    reg *ra = get_reg_ra();
    gen_push(ra);
    gen_jal("read");
    gen_pop(ra);
    apply_oprand(code->read, get_reg_v0());
}
static void rewrite_Write(ircode *code)
{
    asm_log(0, "%s", "Write");
    reg *ra = get_reg_ra();
    reg *r = get_reg();
    prepare_oprand(code->write, r);
    gen_move(get_reg_a0(), r);
    gen_push(ra);
    gen_jal("write");
    gen_pop(ra);
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

static void printHeader(ast *tree)
{
    fputs(".data\n", asm_output);
    fputs("_prompt: .asciiz \"Enter an integer:\"\n", asm_output);
    fputs("_ret: .asciiz \"\\n\"\n", asm_output);

    for (list *l = tree->vars; l != NULL; l = l->next)
    {
        irvar *var = cast(irvar, l->obj);
        asm_out("%s: .word 0", var->name);
    }

    fputs(".globl main\n", asm_output);
    fputs(".text\n", asm_output);
    fputs("read:\n", asm_output);
    fputs("  li $v0, 4\n", asm_output);
    fputs("  la $a0, _prompt\n", asm_output);
    fputs("  syscall\n", asm_output);
    fputs("  li $v0, 5\n", asm_output);
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
    ast_tree = tree;
    vars = list_revto_arr(tree->vars);
    printHeader(tree);
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
