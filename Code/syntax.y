%{
#define ERROR_LINE last_line
#define YYERROR_VERBOSE
#define YYSTYPE struct ast*

#include <limits.h>
#include "syntax.h"
#include "lex.yy.c"

extern void yyerror(const char *format, ...);
extern void syntax_error(char *format, ...);
extern void syntax_error_atline(int lineno, char *format, ...);
extern void syntax_log(char *format, ...);
extern void syntax_set_result(struct ast* result);
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
        syntax_set_result($$);
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

