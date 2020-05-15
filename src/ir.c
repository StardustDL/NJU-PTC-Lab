// #define DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "ast.h"
#include "symbol.h"
#include "type.h"
#include "common.h"
#include "object.h"
#include "debug.h"
#include "semantics.h"

void ir_log(int lineno, char *format, ...);

static list *irs = NULL;

#pragma region helper functions

static void push_ircode(ircode *code)
{
    irs = list_pushfront(irs, code);
}

static irvar *new_var()
{
    static int count = 0;

    Assert(count >= 0, "too many var");
    count++;
    irvar *var = new (irvar);
    sprintf(var->name, "t%d", count);
    return var;
}

static irlabel *new_named_label(const char *name)
{
    irlabel *l = new (irlabel);
    strcpy(l->name, name);
    return l;
}

static irlabel *new_label()
{
    static int count = 0;

    Assert(count >= 0, "too many label");
    count++;
    irlabel *l = new (irlabel);
    sprintf(l->name, "l%d", count);
    return l;
}

static irop *op_var(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Variable;
    op->var = var;
    return op;
}

static irop *op_ref(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Ref;
    op->var = var;
    return op;
}

static irop *op_deref(irvar *var)
{
    irop *op = new (irop);
    op->kind = IRO_Deref;
    op->var = var;
    return op;
}

static irop *op_const(int value)
{
    irop *op = new (irop);
    op->kind = IRO_Constant;
    op->value = value;
    return op;
}

static void gen_label(irlabel *label)
{
    ircode *c = new (ircode);
    c->kind = IR_Label;
    c->label = label;
    push_ircode(c);
}

static void gen_func(irlabel *label)
{
    ircode *c = new (ircode);
    c->kind = IR_Func;
    c->label = label;
    push_ircode(c);
}

static void gen_assign(irop *left, irop *right)
{
    Assert(left->kind == IRO_Variable || left->kind == IRO_Deref, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Assign;
    c->assign.left = left;
    c->assign.right = right;
    push_ircode(c);
}

static void gen_add(irop *target, irop *op1, irop *op2)
{
    Assert(op1->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Add;
    c->bop.target = target;
    c->bop.op1 = op1;
    c->bop.op2 = op2;
    push_ircode(c);
}

static void gen_sub(irop *target, irop *op1, irop *op2)
{
    Assert(op1->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Sub;
    c->bop.target = target;
    c->bop.op1 = op1;
    c->bop.op2 = op2;
    push_ircode(c);
}

static void gen_mul(irop *target, irop *op1, irop *op2)
{
    Assert(op1->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Mul;
    c->bop.target = target;
    c->bop.op1 = op1;
    c->bop.op2 = op2;
    push_ircode(c);
}

static void gen_div(irop *target, irop *op1, irop *op2)
{
    Assert(op1->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Div;
    c->bop.target = target;
    c->bop.op1 = op1;
    c->bop.op2 = op2;
    push_ircode(c);
}

static void gen_goto(irlabel *label)
{
    ircode *c = new (ircode);
    c->kind = IR_Goto;
    c->label = label;
    push_ircode(c);
}

static void gen_branch(relop_type relop, irop *op1, irop *op2, irlabel *target)
{
    ircode *c = new (ircode);
    c->kind = IR_Branch;
    c->branch.relop = relop;
    c->branch.op1 = op1;
    c->branch.op2 = op2;
    c->branch.target = target;
    push_ircode(c);
}

static void gen_return(irop *ret)
{
    ircode *c = new (ircode);
    c->kind = IR_Return;
    c->ret = ret;
    push_ircode(c);
}

static void gen_dec(irop *op, int size)
{
    Assert(op->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Dec;
    c->dec.op = op;
    c->dec.size = size;
    push_ircode(c);
}

static void gen_arg(irop *arg)
{
    ircode *c = new (ircode);
    c->kind = IR_Arg;
    c->arg = arg;
    push_ircode(c);
}

static void gen_param(irop *param)
{
    Assert(param->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Param;
    c->param = param;
    push_ircode(c);
}

static void gen_read(irop *read)
{
    Assert(read->kind == IRO_Variable, "wrong op type");
    ircode *c = new (ircode);
    c->kind = IR_Read;
    c->read = read;
    push_ircode(c);
}

static void gen_write(irop *write)
{
    ircode *c = new (ircode);
    c->kind = IR_Write;
    c->write = write;
    push_ircode(c);
}

#pragma endregion

#pragma region
static void translate_Program(syntax_tree *tree);
static void translate_ExtDefList(syntax_tree *tree);
static void translate_ExtDef(syntax_tree *tree);
static void translate_FunDec(syntax_tree *tree);
static void translate_VarDec(syntax_tree *tree);
static void translate_CompSt(syntax_tree *tree);
static void translate_StmtList(syntax_tree *tree);
static void translate_Stmt(syntax_tree *tree);
static void translate_DefList(syntax_tree *tree);
static void translate_Def(syntax_tree *tree);
static void translate_DecList(syntax_tree *tree);
static void translate_Dec(syntax_tree *tree);
static void translate_Exp(syntax_tree *tree);
static void translate_Args(syntax_tree *tree);
#pragma endregion

static void translate_Program(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "Program");
    // Program : ExtDefList
    //     ;

    AssertEq(tree->type, ST_Program);
    translate_ExtDefList(tree->children[0]);
}
static void translate_ExtDefList(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "ExtDefList");
    // ExtDefList : ExtDef ExtDefList
    //     | /* empty */
    //     ;

    AssertEq(tree->type, ST_ExtDefList);

    if (tree->count == 2)
    {
        translate_ExtDef(tree->children[0]);
        translate_ExtDefList(tree->children[1]);
    }
}
static void translate_ExtDef(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "ExtDef");
    // ExtDef : Specifier ExtDecList SEMI
    //     | Specifier SEMI
    //     | Specifier FunDec CompSt
    //     | Specifier FunDec SEMI
    //     ;
    AssertEq(tree->type, ST_ExtDef);

    switch (tree->children[1]->type)
    {
    case ST_ExtDecList:
    {
        panic("Don't support gloabl var");
    }
    break;
    case ST_FunDec:
    {
        translate_FunDec(tree->children[1]);

        if (tree->children[2]->type == ST_CompSt) // function definition
        {
            translate_CompSt(tree->children[2]);
        }
        else if (tree->children[2]->type == ST_SEMI) // function declare
        {
            panic("Don't support functioin declare");
        }
    }
    break;
    }
}
static void translate_FunDec(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "FunDec");
    // FunDec : ID LP VarList RP
    //     | ID LP RP
    //     ;
    AssertEq(tree->type, ST_FunDec);
    AssertNotNull(tree->sem);
    SES_FunDec *sem = cast(SES_FunDec, tree->sem);
    symbol *sym = st_find(tree->ev->syms, sem->sym->name);
    AssertNotNull(sym);

    irlabel *label = new_named_label(sym->name);
    sym->ir = label;
    gen_func(label);
    int i = 0;
    env *subev = sem->ev;
    while (i < sym->tp->argc)
    {
        char *paramname = sym->tp->args[i]->name;
        symbol *param = st_findonly(subev->syms, paramname);
        AssertNotNull(param);
        irvar *var = new_var();
        param->ir = var;

        gen_param(op_var(var));
        i++;
    }
}
static void translate_VarDec(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "VarDec");
    // VarDec : ID
    //     | VarDec LB INT RB
    //     ;
    AssertEq(tree->type, ST_VarDec);

    AssertNotNull(tree->sem);

    SES_VarDec *sem = cast(SES_VarDec, tree->sem);

    AssertNotNull(sem->sym);
    AssertNotNull(sem->sym->tp);

    switch (sem->sym->tp->cls)
    {
    case TC_STRUCT:
        //TODO struct dec
        break;
    case TC_ARRAY:
        //TODO arr dec
        break;
    case TC_META:
        sem->sym->ir = new_var();
        break;
    default:
        panic("unexpect dec type %d.", sem->sym->tp->cls);
    }
}
static void translate_CompSt(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "CompSt");
    // CompSt : LC DefList StmtList RC
    //     ;
    AssertEq(tree->type, ST_CompSt);

    translate_DefList(tree->children[1]);

    translate_StmtList(tree->children[2]);
}
static void translate_StmtList(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "StmtList");
    // StmtList : Stmt StmtList
    //     | /* empty */
    //     ;
    AssertEq(tree->type, ST_StmtList);

    if (tree->count > 0)
    {
        // TODO
        // translate_Stmt(tree->children[0]);
        translate_StmtList(tree->children[1]);
    }
}
// static void translate_Stmt(syntax_tree *tree)
// {
//     ir_log(tree->first_line, "%s", "Stmt");
//     // Stmt : Exp SEMI
//     //     | CompSt
//     //     | RETURN Exp SEMI
//     //     | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
//     //     | IF LP Exp RP Stmt ELSE Stmt
//     //     | WHILE LP Exp RP Stmt
//     //     ;
//     AssertEq(tree->type, ST_Stmt);
//     switch (tree->children[0]->type)
//     {
//     case ST_Exp: // Exp SEMI
//         translate_Exp(tree->children[0]);
//         break;
//     case ST_CompSt: // CompSt
//         translate_CompSt(tree->children[0]);
//         break;
//     case ST_RETURN: // RETURN Exp SEMI
//     {
//         AssertNotNull(ev->ret_type);
//         void exp = translate_Exp(tree->children[1]);
//         if (!type_full_eq(ev->ret_type, exp->tp, false))
//             error_return_type(tree->children[1]->first_line);
//     }
//     break;
//     case ST_IF:
//     {
//         void exp = translate_Exp(tree->children[2]);
//         if (!type_can_logic(exp->tp))
//             error_op_type(tree->children[2]->first_line);
//         if (tree->count == 7) // IF LP Exp RP Stmt ELSE Stmt
//         {
//             translate_Stmt(tree->children[4]);
//             translate_Stmt(tree->children[6]);
//         }
//         else // IF LP Exp RP Stmt
//             translate_Stmt(tree->children[4]);
//     }
//     break;
//     case ST_WHILE: // WHILE LP Exp RP Stmt
//     {
//         void exp = translate_Exp(tree->children[2]);
//         if (!type_can_logic(exp->tp))
//             error_op_type(tree->children[2]->first_line);
//         translate_Stmt(tree->children[4]);
//     }
//     break;
//     }
// }
static void translate_DefList(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "DefList");
    // DefList : Def DefList
    //     | /* empty */
    //     ;
    AssertEq(tree->type, ST_DefList);

    if (tree->count > 0)
    {
        translate_Def(tree->children[0]);
        translate_DefList(tree->children[1]);
    }
}
static void translate_Def(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "Def");
    // Def : Specifier DecList SEMI
    //     ;
    AssertEq(tree->type, ST_Def);

    translate_DecList(tree->children[1]);
}
static void translate_DecList(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "DecList");
    // DecList : Dec
    //     | Dec COMMA DecList
    //     ;
    AssertEq(tree->type, ST_DecList);

    translate_Dec(tree->children[0]);
    if (tree->count > 1)
        translate_DecList(tree->children[2]);
}
static void translate_Dec(syntax_tree *tree)
{
    ir_log(tree->first_line, "%s", "Dec");
    // Dec : VarDec
    //     | VarDec ASSIGNOP Exp
    //     ;
    AssertEq(tree->type, ST_Dec);

    translate_VarDec(tree->children[0]);
    if (tree->count > 1)
    {
        // TODO
        // translate_Exp(tree->children[2]);
    }
}

// static void translate_Exp(syntax_tree *tree)
// {
//     ir_log(tree->first_line, "%s", "Exp");
//     // Exp : Exp ASSIGNOP Exp
//     //     | Exp AND Exp
//     //     | Exp OR Exp
//     //     | Exp RELOP Exp
//     //     | Exp PLUS Exp
//     //     | Exp MINUS Exp
//     //     | Exp STAR Exp
//     //     | Exp DIV Exp
//     //     | LP Exp RP
//     //     | MINUS Exp %prec NEG
//     //     | NOT Exp
//     //     | ID LP Args RP
//     //     | ID LP RP
//     //     | Exp LB Exp RB
//     //     | Exp DOT ID
//     //     | ID
//     //     | INT
//     //     | FLOAT
//     //     ;
//     AssertEq(tree->type, ST_Exp);

//     void tag = new (SES_Exp);

//     switch (tree->count)
//     {
//     case 1:
//         switch (tree->children[0]->type)
//         {
//         case ST_INT: // INT
//         {
//             tag->tp = new_type_meta(MT_INT);
//         }
//         break;
//         case ST_FLOAT: // FLOAT
//         {
//             tag->tp = new_type_meta(MT_FLOAT);
//         }
//         break;
//         case ST_ID: // ID
//         {
//             symbol *val = get_symbol_by_id(tree->children[0]);
//             if (val == NULL)
//             {
//                 error_var_nodef(tree->first_line, *cast(sytd_id, tree->children[0]->data));
//                 tag->tp = new_type_never();
//             }
//             else
//             {
//                 tag->tp = val->tp;
//             }
//         }
//         break;
//         }
//         break;
//     case 2:
//     {
//         void exp = translate_Exp(tree->children[1]);
//         switch (tree->children[0]->type)
//         {
//         case ST_MINUS: // MINUS Exp
//             if (!type_can_arithmetic(exp->tp))
//                 error_op_type(tree->children[1]->first_line);
//             tag->tp = exp->tp;
//             break;
//         case ST_NOT: // NOT Exp
//         {
//             if (!type_can_logic(exp->tp))
//                 error_op_type(tree->children[1]->first_line);
//             tag->tp = exp->tp;
//         }
//         break;
//         default:
//             panic("unexpect exp");
//             break;
//         }
//     }
//     break;
//     case 3:
//     {
//         switch (tree->children[0]->type)
//         {
//         case ST_LP: // LP Exp RP
//         {
//             void exp = translate_Exp(tree->children[1]);
//             tag->tp = exp->tp;
//         }
//         break;
//         case ST_ID: // ID LP RP
//         {
//             symbol *val = get_symbol_by_id(tree->children[0]);
//             if (val == NULL)
//             {
//                 error_func_nodef(tree->first_line, *cast(sytd_id, tree->children[0]->data));
//                 tag->tp = new_type_never();
//             }
//             else if (!type_can_call(val->tp))
//             {
//                 error_call(tree->first_line);
//                 tag->tp = new_type_never();
//             }
//             else
//             {
//                 if (val->tp->argc != 0)
//                     error_call_type(tree->first_line);
//                 tag->tp = val->tp->ret;
//             }
//         }
//         break;
//         default:
//             switch (tree->children[1]->type)
//             {
//             case ST_DOT: // Exp DOT ID
//             {
//                 void exp = translate_Exp(tree->children[0]);
//                 char *name = *cast(sytd_id, tree->children[2]->data);
//                 if (!type_can_member(exp->tp))
//                 {
//                     error_member(tree->first_line);
//                     tag->tp = new_type_never();
//                 }
//                 else
//                 {
//                     symbol *member = type_can_membername(exp->tp, name);
//                     if (member == NULL)
//                     {
//                         error_member_nodef(tree->children[2]->first_line, name);
//                         tag->tp = new_type_never();
//                     }
//                     else
//                     {
//                         tag->tp = member->tp;
//                     }
//                 }
//             }
//             break;
//             case ST_AND: // Exp AND Exp, Exp OR Exp
//             case ST_OR:
//             {
//                 tag = new (SES_Exp);
//                 void exp1 = translate_Exp(tree->children[0]);
//                 void exp2 = translate_Exp(tree->children[2]);
//                 if (!type_can_logic(exp1->tp))
//                 {
//                     error_op_type(tree->children[0]->first_line);
//                     tag->tp = new_type_meta(MT_INT);
//                 }
//                 else if (!type_can_logic(exp2->tp))
//                 {
//                     error_op_type(tree->children[2]->first_line);
//                     tag->tp = new_type_meta(MT_INT);
//                 }
//                 else
//                 {
//                     tag->tp = exp1->tp;
//                 }
//             }
//             break;
//             case ST_ASSIGNOP: // Exp ASSIGNOP Exp
//             {
//                 tag = new (SES_Exp);
//                 void exp1 = translate_Exp(tree->children[0]);
//                 void exp2 = translate_Exp(tree->children[2]);

//                 bool isrval = true;
//                 if (tree->children[0]->count == 1 && tree->children[0]->children[0]->type == ST_ID)
//                     isrval = false;
//                 else if (tree->children[0]->count == 4 && tree->children[0]->children[1]->type == ST_LB)
//                     isrval = false;
//                 else if (tree->children[0]->count == 3 && tree->children[0]->children[1]->type == ST_DOT)
//                     isrval = false;
//                 if (isrval)
//                 {
//                     error_assign_rval(tree->first_line);
//                     tag->tp = new_type_never();
//                 }
//                 else if (!type_full_eq(exp1->tp, exp2->tp, false))
//                 {
//                     error_assign_type(tree->first_line);
//                     tag->tp = new_type_never();
//                 }
//                 else
//                 {
//                     tag->tp = exp1->tp;
//                 }
//             }
//             break;
//             default: // PLUS, MINUS, ...
//             {
//                 tag = new (SES_Exp);
//                 void exp1 = translate_Exp(tree->children[0]);
//                 void exp2 = translate_Exp(tree->children[2]);
//                 if (!type_can_arithmetic(exp1->tp))
//                 {
//                     error_op_type(tree->children[0]->first_line);
//                     tag->tp = new_type_meta(MT_INT);
//                 }
//                 else if (!type_can_arithmetic(exp2->tp))
//                 {
//                     error_op_type(tree->children[2]->first_line);
//                     tag->tp = new_type_meta(MT_INT);
//                 }
//                 else if (!type_can_arithmetic2(exp1->tp, exp2->tp))
//                 {
//                     error_op_type(tree->children[2]->first_line);
//                     tag->tp = new_type_meta(MT_INT);
//                 }
//                 else
//                 {
//                     tag->tp = exp1->tp;
//                 }
//             }
//             break;
//             }
//             break;
//         }
//     }
//     break;
//     case 4:
//     {
//         if (tree->children[0]->type == ST_ID) // ID LP Args RP
//         {
//             symbol *val = get_symbol_by_id(tree->children[0]);
//             void args = translate_Args(tree->children[2]);
//             tag = new (SES_Exp);
//             if (val == NULL)
//             {
//                 error_func_nodef(tree->first_line, *cast(sytd_id, tree->children[0]->data));
//                 tag->tp = new_type_never();
//             }
//             else if (!type_can_call(val->tp))
//             {
//                 error_call(tree->first_line);
//                 tag->tp = new_type_never();
//             }
//             else
//             {
//                 int i = 0;
//                 while (args != NULL && i < val->tp->argc)
//                 {
//                     if (!type_full_eq(args->tp, val->tp->args[i], false))
//                     {
//                         error_call_type(args->lineno);
//                     }
//                     args = args->next;
//                     i++;
//                 }
//                 if (args != NULL || i != val->tp->argc)
//                 {
//                     error_call_type(tree->first_line);
//                 }
//                 tag->tp = val->tp->ret;
//             }
//         }
//         else // Exp LB Exp RB
//         {
//             tag = new (SES_Exp);
//             void exp1 = translate_Exp(tree->children[0]);
//             void exp2 = translate_Exp(tree->children[2]);
//             if (exp2->tp->cls != TC_META || exp2->tp->metatype != MT_INT)
//             {
//                 error_index_arg(tree->children[2]->first_line);
//                 tag->tp = type_array_descending(exp1->tp);
//             }
//             else if (!type_can_index(exp1->tp))
//             {
//                 error_index(tree->first_line);
//                 tag->tp = new_type_any();
//             }
//             else
//             {
//                 tag->tp = type_array_descending(exp1->tp);
//             }
//         }
//     }
//     break;
//     }
//     tag->lineno = tree->first_line;
//     tree->sem = tag;
//     return tag;
// }
// static void translate_Args(syntax_tree *tree)
// {
//     ir_log(tree->first_line, "%s", "Args");
//     // Args : Exp COMMA Args
//     //     | Exp
//     //     ;
//     AssertEq(tree->type, ST_Args);

//     void first = translate_Exp(tree->children[0]);
//     if (tree->count > 1)
//         first->next = translate_Args(tree->children[2]);
//     tree->sem = first;
//     return first;
// }

static bool ir_is_passed = false;
static char ir_buffer[1024];

void ir_error(int type, int lineno, char *format, ...)
{
    ir_is_passed = 0;

    fprintf(stderr, "Error type %d at Line %d: ", type, lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(ir_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", ir_buffer);
}

void ir_log(int lineno, char *format, ...)
{
#ifdef DEBUG

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(ir_buffer, format, aptr);
    va_end(aptr);

#endif

    Info("Line %d: %s", lineno, ir_buffer);
}

void ir_prepare()
{
    ir_is_passed = true;
    irs = NULL;
}

ast *ir_translate(syntax_tree *tree)
{
    ast *result = new (ast);

    translate_Program(tree);

    result->len = list_len(irs);
    result->codes = list_revto_arr(irs);
    return result;
}

bool ir_has_passed()
{
    return ir_is_passed;
}

void printOprand(irop *op, FILE *file)
{
    switch (op->kind)
    {
    case IRO_Variable:
        fprintf(file, "%s", op->var->name);
        break;
    case IRO_Constant:
        fprintf(file, "#%d", op->value);
        break;
    case IRO_Deref:
        fprintf(file, "*%s", op->var->name);
        break;
    case IRO_Ref:
        fprintf(file, "&%s", op->var->name);
        break;
    }
}

void ir_linearise(ast *tree, FILE *file)
{
    ir_log(0, "ir.len: %d", tree->len);
    for (int i = 0; i < tree->len; i++)
    {
        ircode *code = cast(ircode, tree->codes[i]);
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
            fprintf(file, "GOTO %s :\n", code->label->name);
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
}
