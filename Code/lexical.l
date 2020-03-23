%option yylineno
%{
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include "ast.h"
#include "lexical.h"
#include "syntax.tab.h"

int yycolumn = 1;

extern void lexical_error(char *format, ...);
extern void lexical_log(char *format, ...);

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng; yylval = new_ast(ST_EMPTY); yylval->first_line = yylloc.first_line; yylval->is_token = 1;
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
    } while (c != 0);
    if (!flag)
        lexical_error("Missing '*/' for block comments", yylineno);
    lexical_log("Block comment");
}
"*/" {
    lexical_error("Missing '/*' for block comments", yylineno);  // */
}
(0|[1-9][0-9]*) {
    // decimal integer
    unsigned int result = 0;
    {
        long long temp = 0;
        for (int i = 0; i < yyleng; i++)
        {
            temp = temp * 10;
            char c = yytext[i];
            temp = temp + (c - '0');
            if (temp > UINT_MAX)
            {
                lexical_error("Too large decimal number '%s'", yytext);
            }
        }
        result = (unsigned int) temp;
    }
    lexical_log("INT '%s' -> %u", yytext, result);
    yylval->type = ST_INT;
    yylval->t_uint = result;
    return INT;
}
(0[0-9]+) {
    // octal integer
    unsigned int result = 0;
    {
        for (int i = 1; i < yyleng; i++)
        {
            char c = yytext[i];
            if (!(c >= '0' && c <= '7'))
            {
                lexical_error("Illegal octal number '%s'", yytext);
            }
        }
        long long temp = 0;
        for (int i = 1; i < yyleng; i++)
        {
            temp = temp * 8;
            char c = yytext[i];
            temp = temp + (c - '0');
            if (temp > UINT_MAX)
            {
                lexical_error("Too large octal number '%s'", yytext);
            }
        }
        result = (unsigned int) temp;
    }
    lexical_log("INT '%s' -> %u", yytext, result);
    yylval->type = ST_INT;
    yylval->t_uint = result;
    return INT;
}
0[xX]({digit}|{letter})* {
    // hexadecimal integer
    unsigned int result = 0;
    {
        if (yyleng == 2)
        {
            lexical_error("Empty hexadecimal number '%s'", yytext);
        }
        long long temp = 0;
        for (int i = 2; i < yyleng; i++)
        {
            char c = yytext[i];
            if (!(c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f' || c >= '0' && c <= '9'))
            {
                lexical_error("Illegal hexadecimal number '%s'", yytext);
            }
        }
        for (int i = 2; i < yyleng; i++)
        {
            temp = temp * 16;
            char c = yytext[i];
            if (c >= '0' && c <= '9')
                temp = temp + (c - '0');
            else
            {
                c = tolower(c);
                temp = temp + 10 + (c - 'a');
            }
            if (temp > UINT_MAX)
            {
                lexical_error("Too large hexadecimal number '%s'", yytext);
            }
        }
        result = (unsigned int) temp;
    }
    lexical_log("INT '%s' -> %u", yytext, result);
    yylval->type = ST_INT;
    yylval->t_uint = result;
    return INT;
}
{digit}+\.{digit}+ {
    // normal float
    float result = 0;
    {
        double temp = 0, factor = 1.0;
        int i = 0;
        for (; i < yyleng && yytext[i] != '.'; i++)
        {
            char c = yytext[i];
            temp = temp * 10 + (c - '0');
        }
        for (i++; i < yyleng; i++)
        {
            char c = yytext[i];
            factor /= 10.0;
            temp += factor * (c - '0');
        }
        result = (float) temp;
    }
    lexical_log("FLOAT '%s' -> %f", yytext, result);
    yylval->type = ST_FLOAT;
    yylval->t_float = result;
    return INT;
}
({digit}+\.{digit}*|\.{digit}+)[eE][+-]?{digit}+ {
    // scientific notation float
    float result = 0;
    {
        double temp = 0, factor = 1.0;
        int i = 0;
        for (; i < yyleng && yytext[i] != '.'; i++)
        {
            char c = yytext[i];
            temp = temp * 10 + (c - '0');
        }
        for (i++; i < yyleng && yytext[i] != 'e' && yytext[i] != 'E'; i++)
        {
            char c = yytext[i];
            factor /= 10.0;
            temp += factor * (c - '0');
        }
        {
            int exp = 0, isneg = 0;
            i++;
            if (yytext[i] == '+') {
                isneg = 0;
                i++;
            }
            else if (yytext[i] == '-') {
                isneg = 1;
                i++;
            }
            for(; i < yyleng; i++)
            {
                char c = yytext[i];
                exp = exp * 10 + (c - '0');
            }
            double base = isneg ? 0.1 : 10;
            double fac = 1.0;
            while (exp > 0) { // quick pow for base^exp
                if (exp & 1)
                    fac *= base;
                base *= base;
                exp >>= 1;
            }
            temp *= fac; 
        }
        result = (float) temp;
    }
    lexical_log("FLOAT '%s' -> %f", yytext, result);
    yylval->type = ST_FLOAT;
    yylval->t_float = result;
    return FLOAT;
}
{digit}+\.|\.{digit}+ { lexical_error("Illegal float number '%s'", yytext); /* illegal normal float */ }
({digit}+\.{digit}*|\.{digit}+)[eE] { lexical_error("Illegal float number '%s'", yytext); /* illegal scientific notation float */ }
"." { SIMPLE_OP(DOT); }
"int"|"float" { yylval->t_type = yytext[0] == 'i' ? TYPE_INT : TYPE_FLOAT; SIMPLE_OP(TYPE); }
"struct" { SIMPLE_OP(STRUCT); }
"return" { SIMPLE_OP(RETURN); }
"if" { SIMPLE_OP(IF); }
"else" { SIMPLE_OP(ELSE); }
"while" { SIMPLE_OP(WHILE); }
(_|{letter})(_|{letter}|{digit})* { strcpy(yylval->t_str, yytext); SIMPLE_OP(ID); }
";" { SIMPLE_OP(SEMI); }
"," { SIMPLE_OP(COMMA); }
"=" { SIMPLE_OP(ASSIGNOP); }
">"|"<"|">="|"<="|"=="|"!=" {
    if(yyleng == 1)
    {
        yylval->t_type = yytext[0] == '>' ? RELOP_L : RELOP_S;
    }
    else
    {
        switch(yytext[0])
        {
            case '>':
                yylval->t_type = RELOP_LE;
                break;
            case '<':
                yylval->t_type = RELOP_SE;
                break;
            case '=':
                yylval->t_type = RELOP_E;
                break;
            case '!':
                yylval->t_type = RELOP_NE;
                break;
        }
    }
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
