#include "list.h"
#include "debug.h"
#include "object.h"

list *new_list()
{
    list *result = new (list);
    result->obj = NULL;
    result->next = NULL;
    return result;
}

list *list_pushfront(list *l, void *obj)
{
    list *result = new_list();
    result->obj = obj;
    result->next = l;
    return result;
}