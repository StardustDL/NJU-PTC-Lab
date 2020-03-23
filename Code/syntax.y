%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <assert.h>

#include "syntax.h"
#define ERROR_LINE last_line
#define YYERROR_VERBOSE

#define YYSTYPE struct ast*

#include "lex.yy.c"

void yyerror(const char *format, ...);

static struct ast* syntax_result = NULL;
static int enable_syntax_log = 0;
static int syntax_is_passed = 1;
static char syntax_buffer[1024];
static void syntax_error(char *format, ...);
static void syntax_error_atline(int lineno, char *format, ...);
static void syntax_log(char *format, ...);

const char* get_syntax_type_name(int type);
%}

%locations

%token INT
%token FLOAT
%token ID
%token SEMI COMMA ASSIGNOP
%token RELOP 
%token PLUS MINUS STAR DIV
%token AND OR NOT
%token DOT
%token TYPE
%token LP RP LB RB LC RC
%token STRUCT RETURN IF ELSE WHILE
%token TEOF

/*%type Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag
%type VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args*/

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left LP RP LB RB DOT
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList {
        $$ = new_ast(ST_Program);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
        syntax_result = $$;
        // show_ast($$, 0);
    }
    | error {
        syntax_error_atline(@1.ERROR_LINE, "syntax error");
    }
    ;
ExtDefList : ExtDef ExtDefList { 
        $$ = new_ast(ST_ExtDefList);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | /* empty */ {
        $$ = new_ast(ST_ExtDefList);
        $$->first_line = @$.first_line;
        $$->is_empty = 1;
    }
    ;
ExtDef : Specifier ExtDecList SEMI {
        $$ = new_ast(ST_ExtDef);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Specifier SEMI {
        $$ = new_ast(ST_ExtDef);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Specifier FunDec CompSt {
        $$ = new_ast(ST_ExtDef);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error SEMI {
        syntax_error_atline(@1.ERROR_LINE, "Illegal definitions");
    }
    | Specifier ExtDecList error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ';'");
    }
    | Specifier error {
        syntax_error_atline(@1.ERROR_LINE, "Missing ';'");
    }
    | Specifier FunDec error {
        syntax_error_atline(@2.ERROR_LINE, "Missing function body");
    }
    ;
ExtDecList : VarDec {
        $$ = new_ast(ST_ExtDecList);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec COMMA ExtDecList {
        $$ = new_ast(ST_ExtDecList);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec COMMA error {
        syntax_error_atline(@2.ERROR_LINE, "Missing variable declaration");
    }
    ;

Specifier : TYPE {
        $$ = new_ast(ST_Specifier);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | StructSpecifier {
        $$ = new_ast(ST_Specifier);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = new_ast(ST_StructSpecifier);
        pushfront_child($$, $5);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | STRUCT Tag {
        $$ = new_ast(ST_StructSpecifier);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | STRUCT OptTag LC error RC {
        syntax_error_atline(@4.ERROR_LINE, "Illegal definitions for structure.");
    }
    ;
OptTag : ID  {
        $$ = new_ast(ST_OptTag);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | /* empty */ {
        $$ = new_ast(ST_OptTag);
        $$->first_line = @$.first_line;
        $$->is_empty = 1;
    }
    ;
Tag : ID {
        $$ = new_ast(ST_Tag);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    ;

VarDec : ID {
        $$ = new_ast(ST_VarDec);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec LB INT RB {
        $$ = new_ast(ST_VarDec);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec LB error RB {
        syntax_error_atline(@3.ERROR_LINE, "Index must be an integer");
    }
    | error RB {
        syntax_error_atline(@1.ERROR_LINE, "Missing '['");
    }
    | VarDec LB INT error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ']'");
    }
    ;
FunDec : ID LP VarList RP  {
        $$ = new_ast(ST_FunDec);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ID LP RP {
        $$ = new_ast(ST_FunDec);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ID LP error RP {
        syntax_error_atline(@3.ERROR_LINE, "Illegal variable list for function");
    }
    | error RP {
        syntax_error_atline(@1.ERROR_LINE, "Missing '('");
    }
    | ID LP VarList error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ')'");
    }
    | ID LP error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ')'");
    }
    ;
VarList : ParamDec COMMA VarList {
        $$ = new_ast(ST_VarList);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ParamDec {
        $$ = new_ast(ST_VarList);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error COMMA {
        syntax_error_atline(@1.ERROR_LINE, "Illegal parameter declaration");
    }
    | ParamDec COMMA error {
        syntax_error_atline(@2.ERROR_LINE, "Missing parameter declarations");
    }
    | VarDec VarList error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ',' between parameter declarations");
    }
    ;
ParamDec : Specifier VarDec {
        $$ = new_ast(ST_ParamDec);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    ;

CompSt : LC DefList StmtList RC {
        $$ = new_ast(ST_CompSt);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error RC {
        syntax_error_atline(@2.ERROR_LINE, "Illegal statements");
    }
    ;
StmtList : Stmt StmtList {
        $$ = new_ast(ST_StmtList);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | /* empty */ {
        $$ = new_ast(ST_StmtList);
        $$->is_empty = 1;
    }
    ;
Stmt : Exp SEMI {
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | CompSt {
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | RETURN Exp SEMI {
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $5);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $7);
        pushfront_child($$, $6);
        pushfront_child($$, $5);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | WHILE LP Exp RP Stmt {
        $$ = new_ast(ST_Stmt);
        pushfront_child($$, $5);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error SEMI {
        syntax_error_atline(@1.ERROR_LINE, "Illegal statement");
    }
    | Exp error {
        syntax_error_atline(@1.ERROR_LINE, "Missing ';'");
    }
    | RETURN Exp error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ';'");
    }
    | RETURN error {
        syntax_error_atline(@1.ERROR_LINE, "Missing return-expression");
    }
    | IF LP Exp error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ')'");
    }
    | IF LP error {
        syntax_error_atline(@2.ERROR_LINE, "Missing condition");
    }
    | IF error {
        syntax_error_atline(@1.ERROR_LINE, "Missing '('");
    }
    | WHILE LP Exp error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ')'");
    }
    | WHILE LP error {
        syntax_error_atline(@2.ERROR_LINE, "Missing condition");
    }
    | WHILE error {
        syntax_error_atline(@1.ERROR_LINE, "Missing '('");
    }
    ;

DefList : Def DefList {
        $$ = new_ast(ST_DefList);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | /* empty */ {
        $$ = new_ast(ST_DefList);
        $$->is_empty = 1;
    }
    ;
Def : Specifier DecList SEMI {
        $$ = new_ast(ST_Def);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Specifier error SEMI {
        syntax_error_atline(@1.ERROR_LINE, "Illegal declarations");
    }
    | Specifier DecList error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ';'");
    }
    ;
DecList : Dec {
        $$ = new_ast(ST_DecList);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Dec COMMA DecList {
        $$ = new_ast(ST_DecList);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error COMMA {
        syntax_error_atline(@1.ERROR_LINE, "Illegal declaration");
    }
    | Dec COMMA error {
        syntax_error_atline(@2.ERROR_LINE, "Missing declarations");
    }
Dec : VarDec {
        $$ = new_ast(ST_Dec);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec ASSIGNOP Exp {
        $$ = new_ast(ST_Dec);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | VarDec ASSIGNOP error {
        syntax_error_atline(@2.ERROR_LINE, "Missing expression for assignment");
    }
    ;

Exp : Exp ASSIGNOP Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp AND Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp OR Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp RELOP Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp PLUS Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp MINUS Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp STAR Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp DIV Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | LP Exp RP {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | MINUS Exp %prec NEG {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | NOT Exp {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ID LP Args RP {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ID LP RP {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp LB Exp RB {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $4);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp DOT ID {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | ID {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | INT {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | FLOAT {
        $$ = new_ast(ST_Exp);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | LP Exp error {
        syntax_error_atline(@2.ERROR_LINE, "Missing ')'");
    }
    | LP error RP {
        syntax_error_atline(@1.ERROR_LINE, "Illegal expression");
    }
    | ID LP Args error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ')'");
    }
    | Exp LB Exp error {
        syntax_error_atline(@3.ERROR_LINE, "Missing ']'");
    }
    | Exp LB error RB {
        syntax_error_atline(@2.ERROR_LINE, "Missing index-expression");
    }
    | Exp DOT error {
        syntax_error_atline(@2.ERROR_LINE, "Missing identifier");
    }
    ;
Args : Exp COMMA Args {
        $$ = new_ast(ST_Args);
        pushfront_child($$, $3);
        pushfront_child($$, $2);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | Exp {
        $$ = new_ast(ST_Args);
        pushfront_child($$, $1);
        $$->first_line = @$.first_line;
    }
    | error COMMA {
        syntax_error_atline(@1.ERROR_LINE, "Illegal argument");
    }
    | Exp COMMA error {
        syntax_error_atline(@2.ERROR_LINE, "Missing argument");
    }
    ;
%%
void yyerror(const char *format, ...) {
    syntax_is_passed = 0;

    if (strcmp(format, "syntax error") == 0)
    {
        syntax_log("syntax error");
        return;
    }
    else
    {

    }

    fprintf(stderr, "Error type B at Line %d: ", yylineno);

    if (strstr(format, "syntax error, unexpect"))
    {
        va_list aptr;
        int ret;

        va_start(aptr, format);
        
        vsprintf(syntax_buffer, format + 15, aptr);
        va_end(aptr);

        fprintf(stderr, "%c%s.\n", toupper(*(format + 14)), syntax_buffer);
    }
    else
    {
        va_list aptr;
        int ret;

        va_start(aptr, format);
        
        vsprintf(syntax_buffer, format, aptr);
        va_end(aptr);

        fprintf(stderr, "%s.\n", syntax_buffer);
    }
}

static void syntax_error(char *format, ...)
{
    syntax_is_passed = 0;

    fprintf(stderr, "Error type B at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", syntax_buffer);
}

static void syntax_error_atline(int lineno, char *format, ...)
{
    syntax_is_passed = 0;

    fprintf(stderr, "Error type B at Line %d: ", lineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stderr, "%s.\n", syntax_buffer);
}

static void syntax_log(char *format, ...)
{
    if (!enable_syntax_log)
        return;

    fprintf(stdout, "Syntax log at Line %d: ", yylineno);

    va_list aptr;
    int ret;

    va_start(aptr, format);
    vsprintf(syntax_buffer, format, aptr);
    va_end(aptr);

    fprintf(stdout, "%s\n", syntax_buffer);
}

struct ast* new_ast(int type)
{
    struct ast* result = (struct ast*) malloc(sizeof(struct ast));
    result->type = type;
    result->is_empty = 0;
    result->is_token = 0;
    result->children = NULL;
    result->first_line = 0;
    return result;
}
void free_ast(struct ast* ast)
{
    if (ast == NULL)
        return;

    free_ast_list(ast->children);
    free(ast);
}
struct ast_list* new_ast_list(struct ast* ast)
{
    struct ast_list* result = (struct ast_list*) malloc(sizeof(struct ast_list));
    result->head = ast;
    result->next = NULL;
    return result;
}
void free_ast_list(struct ast_list* ast_list)
{
    while (ast_list != NULL)
    {
        free_ast(ast_list->head);
        struct ast_list* next = ast_list->next;
        free(ast_list);
        ast_list = next;
    }
}
void pushfront_child(struct ast* parent, struct ast* child)
{
    assert(parent != NULL);
    assert(child != NULL);
    struct ast_list* list = new_ast_list(child);
    list->next = parent->children;
    parent->children = list;
}

const char* get_syntax_type_name(int type)
{
    const char* result = NULL;
    switch(type)
    {
        case ST_EMPTY:
            result = "EMPTY";
            break;
        case ST_INT:
            result = "INT";
            break;
        case ST_FLOAT:
            result = "FLOAT";
            break;
        case ST_ID:
            result = "ID";
            break;
        case ST_SEMI:
            result = "SEMI";
            break;
        case ST_COMMA:
            result = "COMMA";
            break;
        case ST_ASSIGNOP:
            result = "ASSIGNOP";
            break;
        case ST_RELOP:
            result = "RELOP";
            break;
        case ST_PLUS:
            result = "PLUS";
            break;
        case ST_MINUS:
            result = "MINUS";
            break;
        case ST_STAR:
            result = "STAR";
            break;
        case ST_DIV:
            result = "DIV";
            break;
        case ST_AND:
            result = "AND";
            break;
        case ST_OR:
            result = "OR";
            break;
        case ST_NOT:
            result = "NOT";
            break;
        case ST_DOT:
            result = "DOT";
            break;
        case ST_TYPE:
            result = "TYPE";
            break;
        case ST_LP:
            result = "LP";
            break;
        case ST_RP:
            result = "RP";
            break;
        case ST_LB:
            result = "LB";
            break;
        case ST_RB:
            result = "RB";
            break;
        case ST_LC:
            result = "LC";
            break;
        case ST_RC:
            result = "RC";
            break;
        case ST_STRUCT:
            result = "STRUCT";
            break;
        case ST_RETURN:
            result = "RETURN";
            break;
        case ST_IF:
            result = "IF";
            break;
        case ST_ELSE:
            result = "ELSE";
            break;
        case ST_WHILE:
            result = "WHILE";
            break;
        case ST_Program:
            result = "Program";
            break;
        case ST_ExtDefList:
            result = "ExtDefList";
            break;
        case ST_ExtDef:
            result = "ExtDef";
            break;
        case ST_ExtDecList:
            result = "ExtDecList";
            break;
        case ST_Specifier:
            result = "Specifier";
            break;
        case ST_StructSpecifier:
            result = "StructSpecifier";
            break;
        case ST_OptTag:
            result = "OptTag";
            break;
        case ST_Tag:
            result = "Tag";
            break;
        case ST_VarDec:
            result = "VarDec";
            break;
        case ST_FunDec:
            result = "FunDec";
            break;
        case ST_VarList:
            result = "VarList";
            break;
        case ST_ParamDec:
            result = "ParamDec";
            break;
        case ST_CompSt:
            result = "CompSt";
            break;
        case ST_StmtList:
            result = "StmtList";
            break;
        case ST_Stmt:
            result = "Stmt";
            break;
        case ST_DefList:
            result = "DefList";
            break;
        case ST_Def:
            result = "Def";
            break;
        case ST_DecList:
            result = "DecList";
            break;
        case ST_Dec:
            result = "Dec";
            break;
        case ST_Exp:
            result = "Exp";
            break;
        case ST_Args:
            result = "Args";
            break;
    }
    return result;
}
void show_ast(struct ast* ast, int level)
{
    assert(ast != NULL);
    if (ast->is_empty)
        return;
    for(int i = 0; i < level; i++)
        printf("  ");
    printf("%s", get_syntax_type_name(ast->type));
    if (ast->is_token)
    {
        switch (ast->type)
        {
            case ST_ID:
                printf(": %s", ast->t_str);
                break;
            case ST_TYPE:
                switch (ast->t_type)
                {
                    case TYPE_INT:
                        printf(": %s", "int");
                        break;
                    case TYPE_FLOAT:
                        printf(": %s", "float");
                        break;
                }
                break;
            case ST_INT:
                printf(": %u", ast->t_uint);
                break;
            case ST_FLOAT:
                printf(": %.6f", ast->t_float);
                break;
        }
    }
    else
    {
        printf(" (%d)", ast->first_line);
    }
    puts("");
    struct ast_list* children = ast->children;
    while (children != NULL)
    {
        show_ast(children->head, level + 1);
        children = children->next;
    }
}

void syntax_set_log(int enable)
{
    enable_syntax_log = enable;
}

void syntax_prepare()
{
    syntax_is_passed = 1;
}

int syntax_has_passed()
{
    return syntax_is_passed && lexical_has_passed();
}

struct ast* syntax_work()
{
    yyparse();
    if (syntax_has_passed())
    {
        return syntax_result;
    }
    else
    {
        return NULL;
    }
}