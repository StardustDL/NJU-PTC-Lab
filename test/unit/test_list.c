#include "unittest.h"
#include "list.h"
#include "object.h"

testdef(list)
{
    int *i = new (int);
    *i = 1;
    int *j = new (int);
    *j = 2;
    list *a = list_pushfront(NULL, i);
    list *b = list_pushfront(a, j);

    testassert(instanceof (int, a->obj), "obj pointer failed");
    testassert(instanceof (int, b->obj), "obj pointer failed");
    testassert(b->next == a, "next pointer failed");
    testpass();
}

void test_init()
{
    testreg(list);
}
