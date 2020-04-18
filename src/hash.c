#include "hash.h"
#include "object.h"
#include <assert.h>

static ll MOD = 1e9 + 7;

hasher *new_hasher(ll seed)
{
    hasher *result = new (hasher);
    result->seed = seed;
    return result;
}

void hash(hasher *core, ll value)
{
    assert(core != NULL);
    ll r = (core->result * core->seed + value) % MOD;
    core->result = r;
}