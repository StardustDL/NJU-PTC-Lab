#include "unittest.h"
#include "main.c"

testdef(basic)
{
    return true;
}

void test_init()
{
    testreg(basic);
}
