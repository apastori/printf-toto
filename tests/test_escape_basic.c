/*
 * Chain of thought
 * ----------------
 * Responsibility: unit test expand_escape for \n \t \\.
 * Heap: none. C11 assert.
 */

#include "test_escape_basic.h"

#include "printf_toto_format.h"

#include <assert.h>
#include <stdio.h>

void test_escape_basic(void)
{
    const char *p;
    unsigned char out;

    p = "n";
    assert(expand_escape(&p, &out) == 1);
    assert(out == '\n');
    assert(*p == '\0');

    p = "t";
    assert(expand_escape(&p, &out) == 1);
    assert(out == '\t');

    p = "\\";
    assert(expand_escape(&p, &out) == 1);
    assert(out == '\\');

    printf("PASS: escape_basic n/t/backslash\n");
}
