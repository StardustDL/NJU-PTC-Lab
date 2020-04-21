%option yylineno
%{
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "ast.h"
#include "object.h"
#include "lexical.h"
#include "syntax.tab.h"

int yycolumn = 1;

extern void lexical_error(char *format, ...);
extern void lexical_log(char *format, ...);

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng; yylval = new_syntax_tree(ST_EMPTY, yylloc.first_line, 0); yylval->is_token = 1;
#define SIMPLE_OP(lex_type) \
    lexical_log(#lex_type " %s", yytext); \
    yylval->type = ST_##lex_type; \
    return lex_type

%}
digit [0-9]
letter [a-zA-Z]
%%
"//" {
    int c;
    do { c = input(); } while (c != '\n' && c != 0);
    if (c == '\n') unput(c);
    lexical_log("Line comment");
}
"/*" {
    int c, c2, flag = 0;
    do
    {
        c = input();
        if (c == '\n') yycolumn = 1;
        if (c == '*')
        {
            c2 = input();
            if (c2 == '/')
            {
                flag = 1;
                break;
            }
            else unput(c2);
        }
    } while (c != 0 && c != EOF);
    if (!flag)
        lexical_error("Missing '*/' for block comments", yylineno);
    lexical_log("Block comment");
}
"*/" {
    lexical_error("Missing '/*' for block comments", yylineno);  // */
}
(0|[1-9][0-9]*) {
    // decimal integer
    yylval->type = ST_INT;
    sytd_int* data = new(sytd_int);
    *data = strtol(yytext, NULL, 10);
    lexical_log("INT '%s' -> %u", yytext, *data);
    yylval->data = data;
    return INT;
}
(0[0-9]+) {
    // octal integer
    for (int i = 1; i < yyleng; i++)
    {
        char c = yytext[i];
        if (!(c >= '0' && c <= '7'))
            lexical_error("Illegal octal number '%s'", yytext);
    }
    yylval->type = ST_INT;
    sytd_int* data = new(sytd_int);
    *data = strtol(yytext, NULL, 8);
    lexical_log("INT '%s' -> %u", yytext, *data);
    yylval->data = data;
    return INT;
}
0[xX]({digit}|{letter})* {
    // hexadecimal integer
    if (yyleng == 2)
        lexical_error("Empty hexadecimal number '%s'", yytext);
    for (int i = 2; i < yyleng; i++)
    {
        char c = yytext[i];
        if (!(c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f' || c >= '0' && c <= '9'))
            lexical_error("Illegal hexadecimal number '%s'", yytext);
    }
    yylval->type = ST_INT;
    sytd_int* data = new(sytd_int);
    *data = strtol(yytext, NULL, 16);
    lexical_log("INT '%s' -> %u", yytext, *data);
    yylval->data = data;
    return INT;
}
{digit}+\.{digit}+ { 
    sytd_float* data = new(sytd_float);
    *data = strtof(yytext, NULL);
    yylval->data = data;
    SIMPLE_OP(FLOAT); 
    /* normal float */
}
({digit}+\.{digit}*|\.{digit}+)[eE][+-]?{digit}+ { 
    sytd_float* data = new(sytd_float);
    *data = strtof(yytext, NULL);
    yylval->data = data;
    SIMPLE_OP(FLOAT); 
    /* scientific notation float */ 
}
{digit}+\.|\.{digit}+ { lexical_error("Illegal float number '%s'", yytext); /* illegal normal float */ }
({digit}+\.{digit}*|\.{digit}+)[eE] { lexical_error("Illegal float number '%s'", yytext); /* illegal scientific notation float */ }
"." { SIMPLE_OP(DOT); }
"int"|"float" { 
    sytd_type* data = new(sytd_type);
    *data = yytext[0] == 'i' ? MT_INT : MT_FLOAT;
    yylval->data = data;
    SIMPLE_OP(TYPE); 
}
"struct" { SIMPLE_OP(STRUCT); }
"return" { SIMPLE_OP(RETURN); }
"if" { SIMPLE_OP(IF); }
"else" { SIMPLE_OP(ELSE); }
"while" { SIMPLE_OP(WHILE); }
(_|{letter})(_|{letter}|{digit})* {
    sytd_id* data = new(sytd_id);
    strcpy(*data, yytext); 
    yylval->data = data;
    SIMPLE_OP(ID);
}
";" { SIMPLE_OP(SEMI); }
"," { SIMPLE_OP(COMMA); }
"=" { SIMPLE_OP(ASSIGNOP); }
">"|"<"|">="|"<="|"=="|"!=" {
    sytd_relop* data = new(sytd_relop);
    if(yyleng == 1)
    {
        *data = yytext[0] == '>' ? RT_L : RT_S;
    }
    else
    {
        switch(yytext[0])
        {
            case '>':
                *data = RT_LE;
                break;
            case '<':
                *data = RT_SE;
                break;
            case '=':
                *data = RT_E;
                break;
            case '!':
                *data = RT_NE;
                break;
        }
    }
    yylval->data = data;
    SIMPLE_OP(RELOP);
}
"+" { SIMPLE_OP(PLUS); }
"-" { SIMPLE_OP(MINUS); }
"*" { SIMPLE_OP(STAR); }
"/" { SIMPLE_OP(DIV); }
"&&" { SIMPLE_OP(AND); }
"||" { SIMPLE_OP(OR); }
"!" { SIMPLE_OP(NOT); }
"(" { SIMPLE_OP(LP); }
")" { SIMPLE_OP(RP); }
"[" { SIMPLE_OP(LB); }
"]" { SIMPLE_OP(RB); }
"{" { SIMPLE_OP(LC); }
"}" { SIMPLE_OP(RC); }
[ \t\r]+ { lexical_log("BLANK", yytext); }
\n { lexical_log("ENTER", yytext); yycolumn = 1; }
. { lexical_error("Mysterious characters '%s'", yytext); }
%%
