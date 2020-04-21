#ifndef __UNITTEST_H__
#define __UNITTEST_H__

#define DEBUG
#include "debug.h"

typedef unsigned char bool;

typedef long long ll;

#define true 1
#define false 0

void register_test(const char *name, bool (*f)());
void test_init();

#define testreg(name) register_test(#name, test_##name)

#define testdef(name) bool test_##name()

#define testfail(format, ...) do{Error(format, ## __VA_ARGS__); return false;}while(false)
#define testpass(format, ...) do{return true;}while(false)

#define testassert(cond, format, ...) do{if(!(cond))testfail(format, ## __VA_ARGS__);}while(false)

#endif