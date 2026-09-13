/*
 * Chain of thought
 * ----------------
 * Responsibility: stderr diagnostics only (never stdout).
 * Syscalls: write to STDERR_FILENO / fprintf(stderr) for messages.
 * Heap: none.
 * C standard: C11 + POSIX strerror.
 */

#include "printf_toto_emit.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void printf_toto_emit_error(const char *context)
{
    const char *reason = strerror(errno);

    if (context == NULL) {
        context = "?";
    }
    if (reason == NULL) {
        reason = "Unknown error";
    }
    fprintf(stderr, "printf-toto: %s: %s\n", context, reason);
}

void printf_toto_emit_msg(const char *msg)
{
    if (msg == NULL) {
        msg = "";
    }
    fprintf(stderr, "printf-toto: %s\n", msg);
}
