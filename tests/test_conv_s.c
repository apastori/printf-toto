/*
 * Chain of thought
 * ----------------
 * Responsibility: unit test %s conversion into a stack buffer.
 * Heap: none. C11 assert.
 */

#include "test_conv_s.h"

#include "printf_toto_format.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_conv_s(void)
{
    const char *p = "%s";
    char *args[] = { "hello" };
    char out[32];
    size_t out_len = 0;
    int argi = 0;
    int had_error = 0;
    int rc;

    rc = format_one_specifier(&p, args, 1, &argi, out, sizeof(out), &out_len,
                              &had_error);
    assert(rc == 0);
    assert(had_error == 0);
    assert(out_len == 5);
    assert(memcmp(out, "hello", 5) == 0);
    assert(argi == 1);

    printf("PASS: conv_s basic\n");
}
