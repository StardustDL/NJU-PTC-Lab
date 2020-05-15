#include "optimize.h"
#include "object.h"
#include <string.h>
#include "debug.h"

static void optimizeDupLabel(ast *tree)
{
    for (int i = 0; i < tree->len;)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        switch (code->kind)
        {
        case IR_Label:
        {
            int j = i + 1;
            for (; j < tree->len; j++)
            {
                ircode *tc = cast(ircode, tree->codes[j]);
                if (tc->kind != IR_Label)
                    break;
                strcpy(tc->label->name, code->label->name);
                tc->ignore = true;
            }
            i = j;
        }
        break;
        default:
            i++;
            break;
        }
    }
}

static void optimizeDupVar(ast *tree)
{
    for (int i = 0; i + 1 < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        ircode *next = cast(ircode, tree->codes[i + 1]);
        if (code->kind == IR_Assign && next->kind == IR_Assign)
        {
            if (code->assign.left->kind == IRO_Variable && code->assign.right->kind == IRO_Constant)
            {
                if (code->assign.left->var->usedTime == 1 && code->assign.left->var->usedCode == next)
                {
                    if (next->assign.right->kind == IRO_Variable && next->assign.right->var == code->assign.left->var)
                    {
                        next->assign.right = code->assign.right;
                        code->ignore = true;
                    }
                }
            }
        }
    }
}

static void optimizeDeadAssign(ast *tree)
{
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        switch (code->kind)
        {
        case IR_Label:
            break;
        case IR_Func:
            break;
        case IR_Assign:
            if (code->assign.left->kind == IRO_Deref)
            {
                code->assign.left->var->usedTime++;
                code->assign.left->var->usedCode = code;
            }
            if (code->assign.right->kind != IRO_Constant)
            {
                code->assign.right->var->usedTime++;
                code->assign.right->var->usedCode = code;
            }
            break;
        case IR_Add:
        case IR_Sub:
        case IR_Mul:
        case IR_Div:
            if (code->bop.op1->kind != IRO_Constant)
            {
                code->bop.op1->var->usedTime++;
                code->bop.op1->var->usedCode = code;
            }
            if (code->bop.op2->kind != IRO_Constant)
            {
                code->bop.op2->var->usedTime++;
                code->bop.op2->var->usedCode = code;
            }
            break;
        case IR_Branch:
            if (code->branch.op1->kind != IRO_Constant)
            {
                code->branch.op1->var->usedTime++;
                code->branch.op1->var->usedCode = code;
            }
            if (code->branch.op2->kind != IRO_Constant)
            {
                code->branch.op2->var->usedTime++;
                code->branch.op2->var->usedCode = code;
            }
            break;
        case IR_Return:
            if (code->ret->kind != IRO_Constant)
            {
                code->ret->var->usedTime++;
                code->ret->var->usedCode = code;
            }
            break;
        case IR_Dec:
            break;
        case IR_Arg:
            if (code->arg->kind != IRO_Constant)
            {
                code->arg->var->usedTime++;
                code->arg->var->usedCode = code;
            }
            break;
        case IR_Call:
            break;
        case IR_Param:
            break;
        case IR_Read:
            break;
        case IR_Write:
            if (code->write->kind != IRO_Constant)
            {
                code->write->var->usedTime++;
                code->write->var->usedCode = code;
            }
            break;
        }
    }
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        switch (code->kind)
        {
        case IR_Label:
            break;
        case IR_Func:
            break;
        case IR_Assign:
            if (code->assign.left->var->usedTime == 0)
            {
                code->ignore = true;
            }
            break;
        case IR_Add:
        case IR_Sub:
        case IR_Mul:
        case IR_Div:
            if (code->bop.target->var->usedTime == 0)
            {
                code->ignore = true;
            }
            break;
        case IR_Branch:
            break;
        case IR_Return:
            break;
        case IR_Dec:
            if (code->dec.op->var->usedTime == 0)
            {
                code->ignore = true;
            }
            break;
        case IR_Arg:
            break;
        case IR_Call:
            break;
        case IR_Param:
            break;
        case IR_Read:
            break;
        case IR_Write:
            break;
        }
    }
}

int optimize(ast *tree)
{
    optimizeDeadAssign(tree);
    optimizeDupLabel(tree);
    optimizeDupVar(tree);

    int count = 0;
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            count++;
    }
    return count;
}