#include "unittest.h"
#include "symbol.h"
#include "type.h"

testdef(table)
{
    symbol_table *root = new_symbol_table(NULL);
    symbol_table *child = new_symbol_table(root);

    symbol *symR = new_symbol("symR", 0, new_type_unit(), SS_DEC);
    symbol *sym = new_symbol("sym", 0, new_type_unit(), SS_DEC);
    st_pushfront(root, symR);
    st_pushfront(child, sym);
    testassert(st_findonly(child, sym->name) != NULL, "findonly failed");
    testassert(st_findonly(child, symR->name) == NULL, "findonly failed");
    testassert(st_find(child, symR->name) != NULL, "find in parent failed");
    testassert(st_find(root, sym->name) == NULL, "find in parent failed");
    testpass();
}

void test_init()
{
    testreg(table);
}
