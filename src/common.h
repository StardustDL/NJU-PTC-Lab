#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>

typedef unsigned char bool;

typedef long long ll;

#define true 1
#define false 0

#define new(type) (type*)newobj(sizeof(type), #type)
#define newarr(type, count) (type**)newobjs(sizeof(type*), count, #type "*")
#define instanceof(type, ptr) instanceofobj(ptr, #type)
#define cast(type, ptr) (type*)castobj(ptr, #type)

void *newobj(size_t size, char *typename);

void *newobjs(size_t size, int count, char *typename);

bool instanceofobj(void *ptr, char *typename);

void *castobj(void *ptr, char *typename);

#endif