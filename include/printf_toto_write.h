#ifndef PRINTF_TOTO_WRITE_H
#define PRINTF_TOTO_WRITE_H

#include <stddef.h>

/* Remap Windows pipe EINVAL to EPIPE when appropriate. */
void maybe_normalize_broken_pipe_errno(int fd);

/*
 * Write n bytes fully. Retries partial writes and EINTR (unless stop).
 * On EPIPE / fatal write: emit + exit(PRINTF_TOTO_EXIT_ERR).
 * On SIGINT stop: _exit(PRINTF_TOTO_EXIT_SIGINT).
 */
void try_write_all(const void *buf, size_t n);

#endif /* PRINTF_TOTO_WRITE_H */
