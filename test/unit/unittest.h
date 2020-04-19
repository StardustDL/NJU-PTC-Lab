#ifndef __UNITTEST_H__
#define __UNITTEST_H__

typedef unsigned char bool;

typedef long long ll;

#define true 1
#define false 0

void register_test(const char *name, bool (*f)());
void test_init();

#define testreg(name) register_test(#name, test_##name)

#define testdef(name) bool test_##name()

#define testfail() do{return false;}while(false)

#define testassert(cond) do{if(!(cond))testfail();}while(false)

#endif