#include <string.h>
#include "optimize.h"
#include "object.h"
#include "debug.h"

static void optimizeDupLabel(ast *tree)
{
    for (int i = 0; i < tree->len;)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
        {
            i++;
            continue;
        }
        switch (code->kind)
        {
        case IR_Label:
        {
            int j = i + 1;
            for (; j < tree->len; j++)
            {
                ircode *tc = cast(ircode, tree->codes[j]);
                if (tc->ignore)
                    continue;
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
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            continue;
        if (code->kind == IR_Assign)
        {
            if (code->assign.left->kind == IRO_Variable && code->assign.right->kind != IRO_Deref)
            {
                irvar *var = code->assign.left->var;
                irop *value = code->assign.right;
                if (var->usedTime == 1 && var->assignTime == 1)
                {
                    ircode *use = cast(ircode, var->usedCode);
                    if (use->ignore)
                        continue;
                    switch (use->kind)
                    {
                    case IR_Assign:
                        if (use->assign.right->kind == IRO_Variable && use->assign.right->var == var)
                        {
                            use->assign.right = value;
                            code->ignore = true;
                        }
                        break;
                    case IR_Add:
                    case IR_Sub:
                    case IR_Mul:
                    case IR_Div:
                        if (use->bop.op1->kind == IRO_Variable && use->bop.op1->var == var)
                        {
                            use->bop.op1 = value;
                            code->ignore = true;
                        }
                        else if (use->bop.op2->kind == IRO_Variable && use->bop.op2->var == var)
                        {
                            use->bop.op2 = value;
                            code->ignore = true;
                        }
                        break;
                    case IR_Branch:
                        if (use->branch.op1->kind == IRO_Variable && use->branch.op1->var == var)
                        {
                            use->branch.op1 = value;
                            code->ignore = true;
                        }
                        else if (use->branch.op2->kind == IRO_Variable && use->branch.op2->var == var)
                        {
                            use->bop.op2 = value;
                            code->ignore = true;
                        }
                        break;
                    case IR_Return:
                        if (use->ret->kind == IRO_Variable && use->ret->var == var)
                        {
                            use->ret = value;
                            code->ignore = true;
                        }
                        break;
                    case IR_Arg:
                        if (use->arg->kind == IRO_Variable && use->arg->var == var)
                        {
                            use->arg = value;
                            code->ignore = true;
                        }
                        break;
                    case IR_Read:
                        break;
                    case IR_Write:
                        if (use->write->kind == IRO_Variable && use->write->var == var)
                        {
                            use->write = value;
                            code->ignore = true;
                        }
                        break;
                    }
                }
            }
        }
    }
}

static void optimizeDupGoto(ast *tree)
{
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
        {
            continue;
        }
        switch (code->kind)
        {
        case IR_Goto:
        {
            bool isdup = false;
            for (int j = i + 1; j < tree->len; j++)
            {
                ircode *tc = cast(ircode, tree->codes[j]);
                if (tc->ignore)
                    continue;
                if (tc->kind != IR_Label)
                    break;
                if (strcmp(tc->label->name, code->label->name) == 0)
                {
                    isdup = true;
                    break;
                }
            }
            if (isdup)
                code->ignore = true;
        }
        break;
        }
    }
}

static void optimizeDeadAssign(ast *tree)
{
    for (list *l = tree->vars; l != NULL; l = l->next)
    {
        irvar *var = cast(irvar, l->obj);
        var->assignTime = 0;
        var->usedCode = NULL;
        var->usedTime = 0;
    }
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            continue;
        switch (code->kind)
        {
        case IR_Label:
            break;
        case IR_Func:
            break;
        case IR_Assign:
            code->assign.left->var->assignTime++;
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
            code->dec.op->var->assignTime++;
            break;
        case IR_Arg:
            if (code->arg->kind != IRO_Constant)
            {
                code->arg->var->usedTime++;
                code->arg->var->usedCode = code;
            }
            break;
        case IR_Call:
            code->call.ret->var->assignTime++;
            break;
        case IR_Param:
            break;
        case IR_Read:
            code->read->var->assignTime++;
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

static void optimizeConstExp(ast *tree)
{
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        switch (code->kind)
        {
        case IR_Add:
            if (code->bop.op1->kind == IRO_Constant && code->bop.op2->kind == IRO_Constant)
            {
                int result = code->bop.op1->value + code->bop.op2->value;
                code->kind = IR_Assign;
                code->assign.left = code->bop.target;
                code->assign.right = op_const(result);
            }
        case IR_Sub:
            if (code->bop.op1->kind == IRO_Constant && code->bop.op2->kind == IRO_Constant)
            {
                int result = code->bop.op1->value - code->bop.op2->value;
                code->kind = IR_Assign;
                code->assign.left = code->bop.target;
                code->assign.right = op_const(result);
            }
        case IR_Mul:
            if (code->bop.op1->kind == IRO_Constant && code->bop.op2->kind == IRO_Constant)
            {
                int result = code->bop.op1->value * code->bop.op2->value;
                code->kind = IR_Assign;
                code->assign.left = code->bop.target;
                code->assign.right = op_const(result);
            }
        case IR_Div:
            if (code->bop.op1->kind == IRO_Constant && code->bop.op2->kind == IRO_Constant)
            {
                if (code->bop.op2->value != 0)
                {
                    int result = code->bop.op1->value / code->bop.op2->value;
                    code->kind = IR_Assign;
                    code->assign.left = code->bop.target;
                    code->assign.right = op_const(result);
                }
                else
                {
                    // UB div 0
                    code->kind = IR_Assign;
                    code->assign.left = code->bop.target;
                    code->assign.right = op_const(0);
                }
            }
            break;
        case IR_Branch:
            if (code->branch.op1->kind == IRO_Constant && code->branch.op2->kind == IRO_Constant)
            {
                bool result = false;
                switch (code->branch.relop)
                {
                case RT_L:
                    result = code->branch.op1->value > code->branch.op2->value;
                    break;
                case RT_S:
                    result = code->branch.op1->value < code->branch.op2->value;
                    break;
                case RT_LE:
                    result = code->branch.op1->value >= code->branch.op2->value;
                    break;
                case RT_SE:
                    result = code->branch.op1->value <= code->branch.op2->value;
                    break;
                case RT_E:
                    result = code->branch.op1->value == code->branch.op2->value;
                    break;
                case RT_NE:
                    result = code->branch.op1->value != code->branch.op2->value;
                    break;
                }
                if (result)
                {
                    code->kind = IR_Goto;
                    code->label = code->branch.target;
                }
            }
            break;
        case IR_Return:
            break;
        case IR_Dec:
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
    const int T = 100;

    for (int i = 0; i < T; i++)
    {
        optimizeDeadAssign(tree);
        optimizeDupLabel(tree);
        optimizeDupVar(tree);
        optimizeConstExp(tree);
        optimizeDupGoto(tree);
    }

    int count = 0;
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
        if (code->ignore)
            count++;
    }
    return count;
}