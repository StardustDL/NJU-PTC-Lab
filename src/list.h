#ifndef __LIST_H__
#define __LIST_H__

#include "common.h"

typedef struct __list{
    void* obj;
    struct __list *next;
}list;

list* new_list();

list* list_pushfront(list* l, void* obj);

#endif