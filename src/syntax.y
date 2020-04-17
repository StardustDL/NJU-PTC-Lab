%{
#define ERROR_LINE last_line
#define YYERROR_VERBOSE
#define YYSTYPE ast*

#include <limits.h>
#include "syntax.h"
#include "lex.yy.c"

extern void yyerror(const char *format, ...);
extern void syntax_error(char *format, ...);
extern void syntax_error_atline(int lineno, char *format, ...);
extern void syntax_log(char *format, ...);
extern void syntax_set_result(ast* result);
%}

%locations

%token INT FLOAT ID SEMI COMMA ASSIGNOP RELOP
%token PLUS MINUS STAR DIV AND OR NOT DOT TYPE
%token LP RP LB RB LC RC
%token STRUCT RETURN IF ELSE WHILE

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
Program : ExtDefList { $$ = new_ast(ST_Program, @$.first_line, 1, $1); syntax_set_result($$); }
    | error { syntax_error_atline(@1.ERROR_LINE, "syntax error"); }
    ;
ExtDefList : ExtDef ExtDefList { $$ = new_ast(ST_ExtDefList, @$.first_line, 2, $1, $2); }
    | /* empty */ { $$ = new_ast(ST_ExtDefList, @$.first_line, 0); $$->is_empty = 1; }
    ;
ExtDef : Specifier ExtDecList SEMI { $$ = new_ast(ST_ExtDef, @$.first_line, 3, $1, $2, $3); }
    | Specifier SEMI { $$ = new_ast(ST_ExtDef, @$.first_line, 2, $1, $2); }
    | Specifier FunDec CompSt { $$ = new_ast(ST_ExtDef, @$.first_line, 3, $1, $2, $3); }
    | Specifier FunDec SEMI { $$ = new_ast(ST_ExtDef, @$.first_line, 3, $1, $2, $3); }
    | error SEMI { syntax_error_atline(@1.ERROR_LINE, "Illegal definitions"); }
    | Specifier ExtDecList error { syntax_error_atline(@2.ERROR_LINE, "Missing ';'"); }
    | Specifier error { syntax_error_atline(@1.ERROR_LINE, "Missing ';'"); }
    | Specifier FunDec error { syntax_error_atline(@2.ERROR_LINE, "Missing function body"); }
    ;
ExtDecList : VarDec { $$ = new_ast(ST_ExtDecList, @$.first_line, 1, $1); }
    | VarDec COMMA ExtDecList { $$ = new_ast(ST_ExtDecList, @$.first_line, 3, $1, $2, $3); }
    | VarDec COMMA error { syntax_error_atline(@2.ERROR_LINE, "Missing variable declaration"); }
    ;

Specifier : TYPE { $$ = new_ast(ST_Specifier, @$.first_line, 1, $1); }
    | StructSpecifier { $$ = new_ast(ST_Specifier, @$.first_line, 1, $1); }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC { $$ = new_ast(ST_StructSpecifier, @$.first_line, 5, $1, $2, $3, $4, $5); }
    | STRUCT Tag { $$ = new_ast(ST_StructSpecifier, @$.first_line, 2, $1, $2); }
    | STRUCT OptTag LC error RC { syntax_error_atline(@4.ERROR_LINE, "Illegal definitions for structure."); }
    ;
OptTag : ID  { $$ = new_ast(ST_OptTag, @$.first_line, 1, $1); }
    | /* empty */ { $$ = new_ast(ST_OptTag, @$.first_line, 0); $$->is_empty = 1; }
    ;
Tag : ID { $$ = new_ast(ST_Tag, @$.first_line, 1, $1); }
    ;

VarDec : ID { $$ = new_ast(ST_VarDec, @$.first_line, 1, $1); }
    | VarDec LB INT RB { $$ = new_ast(ST_VarDec, @$.first_line, 4, $1, $2, $3, $4); }
    | VarDec LB error RB { syntax_error_atline(@3.ERROR_LINE, "Index must be an integer"); }
    | error RB { syntax_error_atline(@1.ERROR_LINE, "Missing '['"); }
    | VarDec LB INT error { syntax_error_atline(@3.ERROR_LINE, "Missing ']'"); }
    ;
FunDec : ID LP VarList RP  { $$ = new_ast(ST_FunDec, @$.first_line, 4, $1, $2, $3, $4); }
    | ID LP RP { $$ = new_ast(ST_FunDec, @$.first_line, 3, $1, $2, $3); }
    | ID LP error RP { syntax_error_atline(@3.ERROR_LINE, "Illegal variable list for function"); }
    | error RP { syntax_error_atline(@1.ERROR_LINE, "Missing '('"); }
    | ID LP VarList error { syntax_error_atline(@3.ERROR_LINE, "Missing ')'"); }
    | ID LP error { syntax_error_atline(@2.ERROR_LINE, "Missing ')'"); }
    ;
VarList : ParamDec COMMA VarList { $$ = new_ast(ST_VarList, @$.first_line, 3, $1, $2, $3); }
    | ParamDec { $$ = new_ast(ST_VarList, @$.first_line, 1, $1); }
    | error COMMA { syntax_error_atline(@1.ERROR_LINE, "Illegal parameter declaration"); }
    | ParamDec COMMA error { syntax_error_atline(@2.ERROR_LINE, "Missing parameter declarations"); }
    | VarDec VarList error { syntax_error_atline(@2.ERROR_LINE, "Missing ',' between parameter declarations"); }
    ;
ParamDec : Specifier VarDec { $$ = new_ast(ST_ParamDec, @$.first_line, 2, $1, $2); }
    ;

CompSt : LC DefList StmtList RC { $$ = new_ast(ST_CompSt, @$.first_line, 4, $1, $2, $3, $4); }
    | error RC { syntax_error_atline(@2.ERROR_LINE, "Illegal statements"); }
    ;
StmtList : Stmt StmtList { $$ = new_ast(ST_StmtList, @$.first_line, 2, $1, $2); }
    | /* empty */ { $$ = new_ast(ST_StmtList, @$.first_line, 0); $$->is_empty = 1; }
    ;
Stmt : Exp SEMI { $$ = new_ast(ST_Stmt, @$.first_line, 2, $1, $2); }
    | CompSt { $$ = new_ast(ST_Stmt, @$.first_line, 1, $1); }
    | RETURN Exp SEMI { $$ = new_ast(ST_Stmt, @$.first_line, 3, $1, $2, $3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{ $$ = new_ast(ST_Stmt, @$.first_line, 5, $1, $2, $3, $4, $5); }
    | IF LP Exp RP Stmt ELSE Stmt { $$ = new_ast(ST_Stmt, @$.first_line, 7, $1, $2, $3, $4, $5, $6, $7); }
    | WHILE LP Exp RP Stmt { $$ = new_ast(ST_Stmt, @$.first_line, 5, $1, $2, $3, $4, $5); }
    | error SEMI { syntax_error_atline(@1.ERROR_LINE, "Illegal statement"); }
    | Exp error { syntax_error_atline(@1.ERROR_LINE, "Missing ';'"); }
    | RETURN Exp error { syntax_error_atline(@2.ERROR_LINE, "Missing ';'"); }
    | RETURN error { syntax_error_atline(@1.ERROR_LINE, "Missing return-expression"); }
    | IF LP Exp error { syntax_error_atline(@3.ERROR_LINE, "Missing ')'"); }
    | IF LP error { syntax_error_atline(@2.ERROR_LINE, "Missing condition"); }
    | IF error { syntax_error_atline(@1.ERROR_LINE, "Missing '('"); }
    | WHILE LP Exp error { syntax_error_atline(@3.ERROR_LINE, "Missing ')'"); }
    | WHILE LP error { syntax_error_atline(@2.ERROR_LINE, "Missing condition"); }
    | WHILE error { syntax_error_atline(@1.ERROR_LINE, "Missing '('"); }
    ;

DefList : Def DefList { $$ = new_ast(ST_DefList, @$.first_line, 2, $1, $2); }
    | /* empty */ { $$ = new_ast(ST_DefList, @$.first_line, 0); $$->is_empty = 1; }
    ;
Def : Specifier DecList SEMI { $$ = new_ast(ST_Def, @$.first_line, 3, $1, $2, $3); }
    | Specifier error SEMI { syntax_error_atline(@1.ERROR_LINE, "Illegal declarations"); }
    | Specifier DecList error { syntax_error_atline(@2.ERROR_LINE, "Missing ';'"); }
    ;
DecList : Dec { $$ = new_ast(ST_DecList, @$.first_line, 1, $1); }
    | Dec COMMA DecList { $$ = new_ast(ST_DecList, @$.first_line, 3, $1, $2, $3); }
    | error COMMA { syntax_error_atline(@1.ERROR_LINE, "Illegal declaration"); }
    | Dec COMMA error { syntax_error_atline(@2.ERROR_LINE, "Missing declarations"); }
    ;
Dec : VarDec { $$ = new_ast(ST_Dec, @$.first_line, 1, $1); }
    | VarDec ASSIGNOP Exp { $$ = new_ast(ST_Dec, @$.first_line, 3, $1, $2, $3); }
    | VarDec ASSIGNOP error { syntax_error_atline(@2.ERROR_LINE, "Missing expression for assignment"); }
    ;

Exp : Exp ASSIGNOP Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp AND Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp OR Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp RELOP Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp PLUS Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp MINUS Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp STAR Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp DIV Exp { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | LP Exp RP { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | MINUS Exp %prec NEG { $$ = new_ast(ST_Exp, @$.first_line, 2, $1, $2); }
    | NOT Exp { $$ = new_ast(ST_Exp, @$.first_line, 2, $1, $2); }
    | ID LP Args RP { $$ = new_ast(ST_Exp, @$.first_line, 4, $1, $2, $3, $4); }
    | ID LP RP { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | Exp LB Exp RB { $$ = new_ast(ST_Exp, @$.first_line, 4, $1, $2, $3, $4); }
    | Exp DOT ID { $$ = new_ast(ST_Exp, @$.first_line, 3, $1, $2, $3); }
    | ID { $$ = new_ast(ST_Exp, @$.first_line, 1, $1); }
    | INT { $$ = new_ast(ST_Exp, @$.first_line, 1, $1); }
    | FLOAT { $$ = new_ast(ST_Exp, @$.first_line, 1, $1); }
    | LP Exp error { syntax_error_atline(@2.ERROR_LINE, "Missing ')'"); }
    | LP error RP { syntax_error_atline(@1.ERROR_LINE, "Illegal expression"); }
    | ID LP Args error { syntax_error_atline(@3.ERROR_LINE, "Missing ')'"); }
    | Exp LB Exp error { syntax_error_atline(@3.ERROR_LINE, "Missing ']'"); }
    | Exp LB error RB { syntax_error_atline(@2.ERROR_LINE, "Missing index-expression"); }
    | Exp DOT error { syntax_error_atline(@2.ERROR_LINE, "Missing identifier"); }
    ;
Args : Exp COMMA Args { $$ = new_ast(ST_Args, @$.first_line, 3, $1, $2, $3); }
    | Exp { $$ = new_ast(ST_Args, @$.first_line, 1, $1); }
    | error COMMA { syntax_error_atline(@1.ERROR_LINE, "Illegal argument"); }
    | Exp COMMA error { syntax_error_atline(@2.ERROR_LINE, "Missing argument"); }
    ;
%%

