#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "common.h"

#define new(type) ((type*)newobj(sizeof(type), #type))
#define newarr(type, count) ((type**)newobjs(sizeof(type*), count, #type "*"))
#define delete(ptr) (deleteobj(ptr))
#define instanceof(type, ptr) (instanceofobj(ptr, #type))
#define instancearrof(type, ptr) (instanceofobj(ptr, #type "*"))
#define cast(type, ptr) ((type*)castobj(ptr, #type, __FILE__, __LINE__))
#define castarr(type, ptr) ((type**)castobj(ptr, #type "*", __FILE__, __LINE__))

void *newobj(size_t size, const char *type_name);

void *newobjs(size_t size, int count, const char *type_name);

void deleteobj(void *ptr);

bool instanceofobj(void *ptr, const char *type_name);

void *castobj(void *ptr, const char *type_name, const char *file, int line);

const char* typename(void *ptr);

#endif