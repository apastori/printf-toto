/*
 * Chain of thought
 * ----------------
 * Responsibility: unit test %% via format_one_specifier.
 * Heap: none. C11 assert.
 */

#include "test_conv_percent.h"

#include "printf_toto_format.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_conv_percent(void)
{
    const char *p = "%%";
    char out[8];
    size_t out_len = 0;
    int argi = 0;
    int had_error = 0;
    int rc;

    rc = format_one_specifier(&p, NULL, 0, &argi, out, sizeof(out), &out_len,
                              &had_error);
    assert(rc == 0);
    assert(had_error == 0);
    assert(out_len == 1);
    assert(out[0] == '%');
    assert(*p == '\0');
    assert(argi == 0);

    printf("PASS: conv_percent\n");
}
