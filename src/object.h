#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "common.h"

#define new(type) (type*)newobj(sizeof(type), #type)
#define newarr(type, count) (type**)newobjs(sizeof(type*), count, #type "*")
#define instanceof(type, ptr) instanceofobj(ptr, #type)
#define cast(type, ptr) (type*)castobj(ptr, #type)

void *newobj(size_t size, const char *typename);

void *newobjs(size_t size, int count, const char *typename);

bool instanceofobj(void *ptr, const char *typename);

void *castobj(void *ptr, const char *typename);

#endif