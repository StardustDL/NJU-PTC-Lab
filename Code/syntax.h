#ifndef __SYNTAX_H__
#define __SYNTAX_H__

struct ast
{
    int type;
    int is_empty;
    int is_token;
    struct ast_list* children;
    int first_line;
    union 
    {
        unsigned int t_uint;
        float t_float;
        int t_type;
        char t_str[64];
    };
};
struct ast_list {
    struct ast* head;
    struct ast_list* next;
};

void syntax_set_log(int enable);

void syntax_prepare();

struct ast* syntax_work();

int syntax_has_passed();

struct ast* new_ast(int type);
void free_ast(struct ast* ast);
struct ast_list* new_ast_list(struct ast* ast);
void free_ast_list(struct ast_list* ast_list);
void pushfront_child(struct ast* parent, struct ast* child);
void show_ast(struct ast* ast, int level);

enum SYNTAX_TYPE
{
    ST_EMPTY,
    ST_INT,
    ST_FLOAT,
    ST_ID,
    ST_SEMI,
    ST_COMMA,
    ST_ASSIGNOP,
    ST_RELOP,
    ST_PLUS,
    ST_MINUS,
    ST_STAR,
    ST_DIV,
    ST_AND,
    ST_OR,
    ST_NOT,
    ST_DOT,
    ST_TYPE,
    ST_LP,
    ST_RP,
    ST_LB,
    ST_RB,
    ST_LC,
    ST_RC,
    ST_STRUCT,
    ST_RETURN,
    ST_IF,
    ST_ELSE,
    ST_WHILE,
    ST_Program,
    ST_ExtDefList,
    ST_ExtDef,
    ST_ExtDecList,
    ST_Specifier,
    ST_StructSpecifier,
    ST_OptTag,
    ST_Tag,
    ST_VarDec,
    ST_FunDec,
    ST_VarList,
    ST_ParamDec,
    ST_CompSt,
    ST_StmtList,
    ST_Stmt,
    ST_DefList,
    ST_Def,
    ST_DecList,
    ST_Dec,
    ST_Exp,
    ST_Args
};

#endif