#ifndef PRINTF_TOTO_FORMAT_H
#define PRINTF_TOTO_FORMAT_H

#include <stddef.h>

/* Documented stack cap for a single conversion result before heap fallback. */
#define PRINTF_TOTO_CONV_STACK_CAP 65536

/*
 * Precondition: *pp points at the character after '\\' in a format string.
 * Postcondition: *pp advanced past the escape; *out holds the byte if any.
 * Returns 1 if a byte was produced, 0 otherwise.
 */
int expand_escape(const char **pp, unsigned char *out);

/*
 * Count arguments consumed by one full pass of fmt (* width/precision count).
 * %% and escapes/literals consume zero. Unknown %X still may consume args
 * for flags/width/precision before the letter.
 */
int count_format_args(const char *fmt);

/*
 * Format one conversion starting at *pp (must be '%').
 * Advances *pp past the specifier. Uses args[*argi] when available; updates *argi.
 * Writes into out[0..out_cap). Sets *out_len. Sets *had_error on conversion issues.
 * Returns 0 on success, -1 if out_cap is too small.
 */
int format_one_specifier(const char **pp, char **args, int nargs, int *argi,
                         char *out, size_t out_cap, size_t *out_len,
                         int *had_error);

#endif /* PRINTF_TOTO_FORMAT_H */
