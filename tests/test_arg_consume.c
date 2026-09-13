/*
 * Chain of thought
 * ----------------
 * Responsibility: unit test count_format_args for reuse accounting.
 * Heap: none. C11 assert.
 */

#include "test_arg_consume.h"

#include "printf_toto_format.h"

#include <assert.h>
#include <stdio.h>

void test_arg_consume(void)
{
    assert(count_format_args("%s %d\n") == 2);
    assert(count_format_args("%%\n") == 0);
    assert(count_format_args("%s\n") == 1);
    assert(count_format_args("%*s") == 2);

    printf("PASS: arg_consume counts\n");
}
