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

int list_len(list *l)
{
    list *cur = l;
    int ans = 0;
    while (cur != NULL)
    {
        ans++;
        cur = cur->next;
    }
    return ans;
}

void **list_to_arr(list *l)
{
    int len = list_len(l);
    void **result = newarr(void, len);
    list *cur = l;
    int i = 0;
    while (cur != NULL)
    {
        result[i] = cur->obj;
        cur = cur->next;
        i++;
    }
    return result;
}

void **list_revto_arr(list *l)
{
    int len = list_len(l);
    void **result = newarr(void, len);
    list *cur = l;
    int i = 0;
    while (cur != NULL)
    {
        result[len - 1 - i] = cur->obj;
        cur = cur->next;
        i++;
    }
    return result;
}