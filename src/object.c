#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "object.h"

static const int OBJMAGIC = 21687894;

typedef struct
{
    int magic;
    const char *type_name;
} objheader;

void *newobj(size_t size, const char *type_name)
{
    size_t soh = sizeof(objheader);
    char *result = malloc(soh + size);
    memset(result, 0, soh + size);

    objheader *oh = (objheader *)result;
    oh->magic = OBJMAGIC;
    oh->type_name = type_name;
    return (void *)(result + soh);
}

void *newobjs(size_t size, int count, const char *type_name)
{
    return newobj(size * count, type_name);
}

void deleteobj(void *ptr)
{
    size_t soh = sizeof(objheader);
    objheader *oh = (objheader *)(((char *)ptr) - soh);
    assert(oh->magic == OBJMAGIC);
    free((void *)oh);
}

const char *typename(void *ptr)
{
    size_t soh = sizeof(objheader);
    objheader *oh = (objheader *)(((char *)ptr) - soh);
    assert(oh->magic == OBJMAGIC);
    return oh->type_name;
}

bool instanceofobj(void *ptr, const char *type_name)
{
    const char *name = typename(ptr);
    if (name == type_name || strcmp(name, type_name) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void *castobj(void *ptr, const char *type_name)
{
    assert(instanceofobj(ptr, type_name));
    return ptr;
}