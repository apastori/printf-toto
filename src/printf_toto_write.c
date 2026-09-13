/*
 * Chain of thought
 * ----------------
 * Responsibility: raw write(2) of already-formatted bytes; EPIPE normalize.
 * Syscalls: write(STDOUT_FILENO), _exit/exit on failure paths.
 * Heap: none in the write loop.
 * C standard: C11 + POSIX unistd/errno; _WIN32 for CRT pipe quirks.
 */

#include "printf_toto_write.h"

#include "printf_toto.h"
#include "printf_toto_cli.h"
#include "printf_toto_emit.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(_WIN32)
#include <io.h>
#endif

void maybe_normalize_broken_pipe_errno(int fd)
{
#if defined(_WIN32)
    /*
     * MinGW CRT may report EINVAL instead of EPIPE on a broken pipe.
     * Remap when the fd is not a tty and errno is EINVAL.
     */
    if (errno == EINVAL && !_isatty(fd)) {
        errno = EPIPE;
    }
#else
    (void)fd;
#endif
}

void try_write_all(const void *buf, size_t n)
{
    const char *p = (const char *)buf;
    size_t left = n;

    while (left > 0) {
        ssize_t w;

        printf_toto_exit_if_stop_requested();
        w = write(STDOUT_FILENO, p, left);
        if (w < 0) {
            if (errno == EINTR) {
                printf_toto_exit_if_stop_requested();
                continue;
            }
            maybe_normalize_broken_pipe_errno(STDOUT_FILENO);
            printf_toto_emit_error("write");
            exit(PRINTF_TOTO_EXIT_ERR);
        }
        if (w == 0) {
            errno = EIO;
            printf_toto_emit_error("write");
            exit(PRINTF_TOTO_EXIT_ERR);
        }
        p += (size_t)w;
        left -= (size_t)w;
    }
}
