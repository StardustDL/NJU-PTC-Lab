#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "unittest.h"

typedef struct __test_list
{
    const char *name;
    bool (*func)();
    struct __test_list *next;
} test_list;

test_list *head = NULL;

void register_test(const char *name, bool (*f)())
{
    test_list *cur = (test_list *)malloc(sizeof(test_list));
    memset(cur, 0, sizeof(test_list));
    cur->name = name;
    cur->func = f;
    cur->next = head;
    head = cur;
}

int main()
{
    test_init();
    test_list *cur = head;
    bool pass = true;
    while (cur != NULL)
    {
        if (!cur->func())
        {
            printf("test %s FAILED\n", cur->name);
            pass = false;
        }
        else
        {
            printf("test %s passed\n", cur->name);
        }
        cur = cur->next;
    }
    return pass ? 0 : 1;
}