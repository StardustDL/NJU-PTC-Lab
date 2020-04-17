#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>

typedef unsigned char bool;

typedef long long ll;

#define true 1
#define false 0

#define new(type) (type*)newobj(sizeof(type))
#define newarr(type, count) (type**)newobjs(sizeof(type*), count)

void* newobj(size_t size);

void* newobjs(size_t size, int count);

#endif