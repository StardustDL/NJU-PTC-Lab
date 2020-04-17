#ifndef __HASH_H__
#define __HASH_H__

#include "common.h"

typedef struct
{
    ll seed;
    ll index;
    ll result;
} hasher;

hasher *new_hasher(ll seed);

void hash(hasher *core, ll value);

#endif