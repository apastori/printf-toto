/*
 * Chain of thought
 * ----------------
 * Responsibility: meta --help/--version (sole operand), SIGPIPE/SIGINT setup.
 * Syscalls: write for help/version; sigaction/signal; _exit on stop.
 * Heap: none.
 * C standard: C11; POSIX sigaction on non-Windows; signal() on _WIN32.
 */

#include "printf_toto_cli.h"

#include "printf_toto.h"
#include "printf_toto_write.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t printf_toto_stop_requested;

static void on_sigint(int signo)
{
    (void)signo;
    printf_toto_stop_requested = 1;
}

void printf_toto_exit_if_stop_requested(void)
{
    if (printf_toto_stop_requested) {
        _exit(PRINTF_TOTO_EXIT_SIGINT);
    }
}

int install_sigpipe_ignore(void)
{
#if defined(_WIN32)
    /* No SIGPIPE on Windows — broken pipe surfaces as write errno. */
    return 0;
#else
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) != 0) {
        return -1;
    }
    return 0;
#endif
}

int install_sigint_handler(void)
{
    printf_toto_stop_requested = 0;
#if defined(_WIN32)
    if (signal(SIGINT, on_sigint) == SIG_ERR) {
        return -1;
    }
    return 0;
#else
    {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = on_sigint;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGINT, &sa, NULL) != 0) {
            return -1;
        }
        return 0;
    }
#endif
}

void print_version(void)
{
    const char *line = "printf-toto " PRINTF_TOTO_VERSION_STRING "\n";

    try_write_all(line, strlen(line));
}

void print_help(void)
{
    static const char help[] =
        "Usage: printf-toto FORMAT [ARGUMENT]...\n"
        "   or: printf-toto --help\n"
        "   or: printf-toto --version\n"
        "\n"
        "Format and print ARGUMENT(s) under control of FORMAT (POSIX core).\n"
        "Conversions: %s %c %d %i %u %o %x %X %%\n"
        "Meta flags (--help/--h, --version/--v) only when sole operand.\n";

    try_write_all(help, sizeof(help) - 1U);
}

int scan_meta_flags(int argc, char **argv)
{
    const char *a;

    // Check if there are exactly two arguments and if the second argument is not NULL
    // In case the user provided --help or --version as operands
    if (argc != 2 || argv == NULL || argv[1] == NULL) {
        return 0;
    }
    a = argv[1];
    if (strcmp(a, "--help") == 0 || strcmp(a, "--h") == 0) {
        print_help();
        return 1;
    }
    if (strcmp(a, "--version") == 0 || strcmp(a, "--v") == 0) {
        print_version();
        return 1;
    }
    return 0;
}
