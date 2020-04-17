#include <stdlib.h>
#include <string.h>
#include "common.h"

void *newobj(size_t size)
{
    void *result = malloc(size);
    memset(result, 0, size);
    return result;
}

void *newobjs(size_t size, int count)
{
    return newobj(size * count);
}