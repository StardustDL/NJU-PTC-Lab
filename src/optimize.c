
#include "optimize.h"
#include "object.h"

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
                code->assign.left->var->isused = true;
            if (code->assign.right->kind != IRO_Constant)
                code->assign.right->var->isused = true;
            break;
        case IR_Add:
        case IR_Sub:
        case IR_Mul:
        case IR_Div:
            if (code->bop.op1->kind != IRO_Constant)
                code->bop.op1->var->isused = true;
            if (code->bop.op2->kind != IRO_Constant)
                code->bop.op2->var->isused = true;
            break;
        case IR_Branch:
            if (code->branch.op1->kind != IRO_Constant)
                code->branch.op1->var->isused = true;
            if (code->branch.op2->kind != IRO_Constant)
                code->branch.op2->var->isused = true;
            break;
        case IR_Return:
            if (code->ret->kind != IRO_Constant)
                code->ret->var->isused = true;
            break;
        case IR_Dec:
            break;
        case IR_Arg:
            if (code->arg->kind != IRO_Constant)
                code->arg->var->isused = true;
            break;
        case IR_Call:
            break;
        case IR_Param:
            break;
        case IR_Read:
            break;
        case IR_Write:
            if (code->write->kind != IRO_Constant)
                code->write->var->isused = true;
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
            if (code->assign.left->var->isused == false)
            {
                code->ignore = true;
            }
            break;
        case IR_Add:
        case IR_Sub:
        case IR_Mul:
        case IR_Div:
            if (code->bop.target->var->isused == false)
            {
                code->ignore = true;
            }
            break;
        case IR_Branch:
            break;
        case IR_Return:
            break;
        case IR_Dec:
            if (code->dec.op->var->isused == false)
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

    int count = 0;
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            count++;
    }
    return count;
}