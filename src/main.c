/*
 * Chain of thought
 * ----------------
 * Responsibility: thin main — meta flags, signal setup, dispatch run.
 * Syscalls: via cli/write helpers only.
 * Heap: none here.
 * C standard: C11.
 */

#include "printf_toto.h"
#include "printf_toto_cli.h"
#include "printf_toto_emit.h"

int main(int argc, char **argv)
{
    if (scan_meta_flags(argc, argv)) {
        return PRINTF_TOTO_EXIT_OK;
    }

    if (argc < 2) {
        printf_toto_emit_msg("missing operand");
        print_help();
        return PRINTF_TOTO_EXIT_ERR;
    }

    if (install_sigpipe_ignore() != 0) {
        printf_toto_emit_error("sigpipe");
        return PRINTF_TOTO_EXIT_ERR;
    }
    if (install_sigint_handler() != 0) {
        printf_toto_emit_error("sigint");
        return PRINTF_TOTO_EXIT_ERR;
    }

    return printf_toto_run(argc - 1, argv + 1);
}
