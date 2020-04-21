#include "unittest.h"
#include "object.h"

testdef(object)
{
    int* i = new(int);
    testassert(instanceof(int, i), "instanceof failed");
    testassert(!instanceof(double, i), "instanceof failed");
    int* j = cast(int, i);
    testassert(i == j, "cast failed");
    delete(i);
    testpass();
}

testdef(arr)
{
    int** i = newarr(int, 5);
    testassert(instancearrof(int, i), "instancearrof failed");
    testassert(!instancearrof(double, i), "instancearrof failed");
    int** j = castarr(int, i);
    testassert(i == j, "castarr failed");
    delete(i);
    testpass();
}

void test_init()
{
    testreg(object);
    testreg(arr);
}
