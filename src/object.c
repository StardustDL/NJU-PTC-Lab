#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "object.h"

static const int OBJMAGIC = 21687894;

typedef struct
{
    int magic;
    const char *typename;
} objheader;

void *newobj(size_t size, const char *typename)
{
    size_t soh = sizeof(objheader);
    char *result = malloc(soh + size);
    memset(result, 0, soh + size);

    objheader *oh = (objheader *)result;
    oh->magic = OBJMAGIC;
    oh->typename = typename;
    return (void *)(result + soh);
}

void *newobjs(size_t size, int count, const char *typename)
{
    return newobj(size * count, typename);
}

bool instanceofobj(void *ptr, const char *typename)
{
    size_t soh = sizeof(objheader);
    objheader *oh = (objheader *)(((char *)ptr) - soh);
    assert(oh->magic == OBJMAGIC);
    if (oh->typename == typename || strcmp(oh->typename, typename) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void *castobj(void *ptr, const char *typename)
{
    assert(instanceofobj(ptr, typename));
    return ptr;
}