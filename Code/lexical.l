%option yylineno
%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>

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
%}
digit [0-9]
letter [a-zA-Z]
%%
"//" {
    int c;
    do
    {
        c = input();
    } while (c != '\n' && c != 0);
    if (c == '\n')
    {
        unput(c);
    }
    lexical_log("Line comment");
}
"/*" {
    int c, c2, flag = 0;
    do
    {
        c = input();
        if (c == '\n')
            yycolumn = 1;
        if (c == '*')
        {
            c2 = input();
            if (c2 == '/')
            {
                flag = 1;
                break;
            }
            else
                unput(c2);
        }
    } while (c != 0);
    if (!flag)
    {
        lexical_error("Missing '*/' for block comments", yylineno);
    }
    lexical_log("Block comment");
}
"*/" {
    lexical_error("Missing '/*' for block comments", yylineno); 
    // */
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
{digit}+\.|\.{digit}+ {
    // illegal normal float
    lexical_error("Illegal float number '%s'", yytext);
}
({digit}+\.{digit}*|\.{digit}+)[eE] {
    // illegal scientific notation float
    lexical_error("Illegal float number '%s'", yytext);
}
"." {
    lexical_log("DOT %s", yytext);
    yylval->type = ST_DOT;
    return DOT;
}
"int"|"float" {
    lexical_log("TYPE %s", yytext);
    yylval->type = ST_TYPE;
    yylval->t_type = yytext[0] == 'i' ? TYPE_INT : TYPE_FLOAT;
    return TYPE;
}
"struct" {
    lexical_log("STRUCT %s", yytext);
    yylval->type = ST_STRUCT;
    return STRUCT;
}
"return" {
    lexical_log("RETURN %s", yytext);
    yylval->type = ST_RETURN;
    return RETURN;
}
"if" {
    lexical_log("IF %s", yytext);
    yylval->type = ST_IF;
    return IF;
}
"else" {
    lexical_log("ELSE %s", yytext);
    yylval->type = ST_ELSE;
    return ELSE;
}
"while" {
    lexical_log("WHILE %s", yytext);
    yylval->type = ST_WHILE;
    return WHILE;
}
(_|{letter})(_|{letter}|{digit})* {
    lexical_log("ID %s", yytext);
    yylval->type = ST_ID;
    strcpy(yylval->t_str, yytext);
    return ID;
}
";" {
    lexical_log("SEMI %s", yytext);
    yylval->type = ST_SEMI;
    return SEMI;
}
"," {
    lexical_log("COMMA %s", yytext);
    yylval->type = ST_COMMA;
    return COMMA;
}
"=" {
    lexical_log("ASSIGNOP %s", yytext);
    yylval->type = ST_ASSIGNOP;
    return ASSIGNOP;
}
">"|"<"|">="|"<="|"=="|"!=" {
    lexical_log("RELOP %s", yytext);
    yylval->type = ST_RELOP;  
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
    return RELOP;
}
"+" {
    lexical_log("PLUS %s", yytext);
    yylval->type = ST_PLUS;
    return PLUS;
}
"-" {
    lexical_log("MINUS %s", yytext);
    yylval->type = ST_MINUS;
    return MINUS;
}
"*" {
    lexical_log("STAR %s", yytext);
    yylval->type = ST_STAR;
    return STAR;
}
"/" {
    lexical_log("DIV %s", yytext);
    yylval->type = ST_DIV;
    return DIV;
}
"&&" {
    lexical_log("AND %s", yytext);
    yylval->type = ST_AND;
    return AND;
}
"||" {
    lexical_log("OR %s", yytext);
    yylval->type = ST_OR;
    return OR;
}
"!" {
    lexical_log("NOT %s", yytext);
    yylval->type = ST_NOT;
    return NOT;
}
"(" {
    lexical_log("LP %s", yytext);
    yylval->type = ST_LP;
    return LP;
}
")" {
    lexical_log("RP %s", yytext);
    yylval->type = ST_RP;
    return RP;
}
"[" {
    lexical_log("LB %s", yytext);
    yylval->type = ST_LB;
    return LB;
}
"]" {
    lexical_log("RB %s", yytext);
    yylval->type = ST_RB;
    return RB;
}
"{" {
    lexical_log("LC %s", yytext);
    yylval->type = ST_LC;
    return LC;
}
"}" {
    lexical_log("RC %s", yytext);
    yylval->type = ST_RC;
    return RC;
}
[ \t\r]+ {
    lexical_log("BLANK", yytext);
}
\n {
    lexical_log("ENTER", yytext);
    yycolumn = 1;
}
. {
    lexical_error("Mysterious characters '%s'", yytext);
}
%%
