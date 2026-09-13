/*
 * Chain of thought
 * ----------------
 * Responsibility: invoke all pure-helper test suites in fixed order.
 * Heap: none. C11.
 */

#include "test_arg_consume.h"
#include "test_conv_d.h"
#include "test_conv_percent.h"
#include "test_conv_s.h"
#include "test_escape_basic.h"

int main(void)
{
    test_escape_basic();
    test_conv_percent();
    test_conv_s();
    test_conv_d();
    test_arg_consume();
    return 0;
}
