/*
 * Chain of thought
 * ----------------
 * Responsibility: POSIX-core format walk (escapes, conversions, reuse);
 *                 printf_toto_run; pure helpers for unit tests.
 * Syscalls: none directly — output via try_write_all.
 * Heap: optional malloc when a conversion exceeds PRINTF_TOTO_CONV_STACK_CAP.
 * C standard: C11; inttypes strtoimax/strtoumax; snprintf to build text.
 */

#include "printf_toto_format.h"

#include "printf_toto.h"
#include "printf_toto_cli.h"
#include "printf_toto_emit.h"
#include "printf_toto_write.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int left;
    int plus;
    int space;
    int alt;
    int zero;
    int width; /* -1 = none */
    int prec;  /* -1 = none */
    int width_star;
    int prec_star;
    char spec;
} conv_spec;

static char *next_arg(char **args, int nargs, int *argi)
{
    if (*argi < nargs) {
        return args[(*argi)++];
    }
    return NULL;
}

int expand_escape(const char **pp, unsigned char *out)
{
    const char *p;
    unsigned int v;
    int digits;

    if (pp == NULL || *pp == NULL || out == NULL) {
        return 0;
    }
    p = *pp;
    switch (*p) {
    case 'a':
        *out = '\a';
        *pp = p + 1;
        return 1;
    case 'b':
        *out = '\b';
        *pp = p + 1;
        return 1;
    case 'f':
        *out = '\f';
        *pp = p + 1;
        return 1;
    case 'n':
        *out = '\n';
        *pp = p + 1;
        return 1;
    case 'r':
        *out = '\r';
        *pp = p + 1;
        return 1;
    case 't':
        *out = '\t';
        *pp = p + 1;
        return 1;
    case 'v':
        *out = '\v';
        *pp = p + 1;
        return 1;
    case '\\':
        *out = '\\';
        *pp = p + 1;
        return 1;
    default:
        break;
    }

    if (*p >= '0' && *p <= '7') {
        v = 0U;
        digits = 0;
        while (digits < 3 && *p >= '0' && *p <= '7') {
            v = (v * 8U) + (unsigned int)(*p - '0');
            p++;
            digits++;
        }
        *out = (unsigned char)(v & 0xFFU);
        *pp = p;
        return 1;
    }

    if (*p != '\0') {
        *out = (unsigned char)*p;
        *pp = p + 1;
        return 1;
    }
    return 0;
}

static int parse_int_field(const char **pp)
{
    const char *p = *pp;
    long v = 0;
    int any = 0;

    while (*p >= '0' && *p <= '9') {
        any = 1;
        v = v * 10L + (long)(*p - '0');
        if (v > (long)INT_MAX) {
            v = (long)INT_MAX;
        }
        p++;
    }
    *pp = p;
    if (!any) {
        return -1;
    }
    return (int)v;
}

static int parse_conv(const char **pp, conv_spec *cs)
{
    const char *p;

    if (pp == NULL || *pp == NULL || **pp != '%' || cs == NULL) {
        return -1;
    }
    p = *pp + 1;
    memset(cs, 0, sizeof(*cs));
    cs->width = -1;
    cs->prec = -1;

    for (;;) {
        if (*p == '-') {
            cs->left = 1;
            p++;
        } else if (*p == '+') {
            cs->plus = 1;
            p++;
        } else if (*p == ' ') {
            cs->space = 1;
            p++;
        } else if (*p == '#') {
            cs->alt = 1;
            p++;
        } else if (*p == '0') {
            cs->zero = 1;
            p++;
        } else {
            break;
        }
    }

    if (*p == '*') {
        cs->width_star = 1;
        p++;
    } else {
        cs->width = parse_int_field(&p);
    }

    if (*p == '.') {
        p++;
        if (*p == '*') {
            cs->prec_star = 1;
            p++;
        } else if (*p >= '0' && *p <= '9') {
            cs->prec = parse_int_field(&p);
        } else {
            cs->prec = 0;
        }
    }

    if (*p == '\0') {
        *pp = p;
        return -1;
    }
    cs->spec = *p;
    *pp = p + 1;
    return 0;
}

static int count_one_conv_args(const conv_spec *cs)
{
    int n = 0;

    if (cs->width_star) {
        n++;
    }
    if (cs->prec_star) {
        n++;
    }
    switch (cs->spec) {
    case '%':
        break;
    case 's':
    case 'c':
    case 'd':
    case 'i':
    case 'u':
    case 'o':
    case 'x':
    case 'X':
        n++;
        break;
    default:
        break;
    }
    return n;
}

int count_format_args(const char *fmt)
{
    const char *p = fmt;
    int total = 0;

    if (fmt == NULL) {
        return 0;
    }
    while (*p != '\0') {
        if (*p == '\\') {
            unsigned char byte;

            p++;
            (void)expand_escape(&p, &byte);
            continue;
        }
        if (*p == '%') {
            conv_spec cs;
            const char *save = p;

            if (parse_conv(&p, &cs) != 0) {
                p = save + 1;
                continue;
            }
            total += count_one_conv_args(&cs);
            continue;
        }
        p++;
    }
    return total;
}

static intmax_t parse_signed(const char *s, int *had_error)
{
    char *end;
    intmax_t v;

    if (s == NULL) {
        return 0;
    }
    if ((s[0] == '\'' || s[0] == '"') && s[1] != '\0') {
        return (intmax_t)(unsigned char)s[1];
    }
    errno = 0;
    v = strtoimax(s, &end, 0);
    if (end == s) {
        printf_toto_emit_msg("expected a numeric value");
        *had_error = 1;
        return 0;
    }
    if (errno == ERANGE) {
        printf_toto_emit_msg("arithmetic overflow");
        *had_error = 1;
    } else if (*end != '\0') {
        printf_toto_emit_msg("value not completely converted");
        *had_error = 1;
    }
    return v;
}

static uintmax_t parse_unsigned(const char *s, int *had_error)
{
    char *end;
    uintmax_t v;

    if (s == NULL) {
        return 0;
    }
    if ((s[0] == '\'' || s[0] == '"') && s[1] != '\0') {
        return (uintmax_t)(unsigned char)s[1];
    }
    errno = 0;
    v = strtoumax(s, &end, 0);
    if (end == s) {
        printf_toto_emit_msg("expected a numeric value");
        *had_error = 1;
        return 0;
    }
    if (errno == ERANGE) {
        printf_toto_emit_msg("arithmetic overflow");
        *had_error = 1;
    } else if (*end != '\0') {
        printf_toto_emit_msg("value not completely converted");
        *had_error = 1;
    }
    return v;
}

static int pad_and_copy(char *out, size_t out_cap, size_t *out_len,
                        const char *body, size_t body_len, int width,
                        int left, int zero)
{
    size_t pad;
    size_t need;
    size_t i;
    char padch;

    if (width < 0) {
        width = 0;
    }
    pad = 0;
    if ((size_t)width > body_len) {
        pad = (size_t)width - body_len;
    }
    need = body_len + pad;
    if (need > out_cap) {
        return -1;
    }
    padch = (zero && !left) ? '0' : ' ';
    if (left) {
        memcpy(out, body, body_len);
        for (i = 0; i < pad; i++) {
            out[body_len + i] = ' ';
        }
    } else if (padch == '0' && body_len > 0 &&
               (body[0] == '+' || body[0] == '-' || body[0] == ' ')) {
        out[0] = body[0];
        for (i = 0; i < pad; i++) {
            out[1 + i] = '0';
        }
        memcpy(out + 1 + pad, body + 1, body_len - 1);
    } else if (padch == '0' && body_len >= 2 && body[0] == '0' &&
               (body[1] == 'x' || body[1] == 'X')) {
        out[0] = body[0];
        out[1] = body[1];
        for (i = 0; i < pad; i++) {
            out[2 + i] = '0';
        }
        memcpy(out + 2 + pad, body + 2, body_len - 2);
    } else {
        for (i = 0; i < pad; i++) {
            out[i] = padch;
        }
        memcpy(out + pad, body, body_len);
    }
    *out_len = need;
    return 0;
}

static int format_string_conv(const conv_spec *cs, const char *arg, char *out,
                              size_t out_cap, size_t *out_len)
{
    size_t len;
    size_t use;
    const char *s = arg != NULL ? arg : "";

    len = strlen(s);
    use = len;
    if (cs->prec >= 0 && (size_t)cs->prec < use) {
        use = (size_t)cs->prec;
    }
    return pad_and_copy(out, out_cap, out_len, s, use, cs->width, cs->left, 0);
}

static int format_char_conv(const conv_spec *cs, const char *arg, char *out,
                            size_t out_cap, size_t *out_len)
{
    char body[1];

    if (arg == NULL || arg[0] == '\0') {
        body[0] = '\0';
    } else {
        body[0] = arg[0];
    }
    return pad_and_copy(out, out_cap, out_len, body, 1, cs->width, cs->left, 0);
}

static int format_int_conv(const conv_spec *cs, const char *arg, char *out,
                           size_t out_cap, size_t *out_len, int *had_error)
{
    char num[128];
    char fbuf[32];
    char *f;
    int n;
    intmax_t sv;
    uintmax_t uv;
    int is_signed = 0;

    f = fbuf;
    *f++ = '%';
    if (cs->plus) {
        *f++ = '+';
    } else if (cs->space) {
        *f++ = ' ';
    }
    if (cs->alt) {
        *f++ = '#';
    }
    if (cs->prec >= 0) {
        *f++ = '.';
        *f++ = '*';
    }
    switch (cs->spec) {
    case 'd':
    case 'i':
        is_signed = 1;
        *f++ = 'j';
        *f++ = 'd';
        break;
    case 'u':
        *f++ = 'j';
        *f++ = 'u';
        break;
    case 'o':
        *f++ = 'j';
        *f++ = 'o';
        break;
    case 'x':
        *f++ = 'j';
        *f++ = 'x';
        break;
    case 'X':
        *f++ = 'j';
        *f++ = 'X';
        break;
    default:
        return -1;
    }
    *f = '\0';

    if (is_signed) {
        sv = parse_signed(arg != NULL ? arg : "0", had_error);
        if (cs->prec >= 0) {
            n = snprintf(num, sizeof(num), fbuf, cs->prec, sv);
        } else {
            n = snprintf(num, sizeof(num), fbuf, sv);
        }
    } else {
        uv = parse_unsigned(arg != NULL ? arg : "0", had_error);
        if (cs->prec >= 0) {
            n = snprintf(num, sizeof(num), fbuf, cs->prec, uv);
        } else {
            n = snprintf(num, sizeof(num), fbuf, uv);
        }
    }
    if (n < 0 || (size_t)n >= sizeof(num)) {
        return -1;
    }
    return pad_and_copy(out, out_cap, out_len, num, (size_t)n, cs->width,
                        cs->left, cs->zero && cs->prec < 0);
}

static int get_star_width(char **args, int nargs, int *argi, int *had_error,
                          int *left_out)
{
    const char *s = next_arg(args, nargs, argi);
    intmax_t v;

    if (s == NULL) {
        return 0;
    }
    v = parse_signed(s, had_error);
    if (v < 0) {
        *left_out = 1;
        v = -v;
    }
    if (v > (intmax_t)INT_MAX) {
        return INT_MAX;
    }
    return (int)v;
}

int format_one_specifier(const char **pp, char **args, int nargs, int *argi,
                         char *out, size_t out_cap, size_t *out_len,
                         int *had_error)
{
    conv_spec cs;
    char *arg;
    int local_err = 0;
    int *errp = had_error != NULL ? had_error : &local_err;
    char msg[64];

    if (out_len != NULL) {
        *out_len = 0;
    }
    if (pp == NULL || *pp == NULL || **pp != '%' || out == NULL ||
        out_len == NULL || argi == NULL) {
        return -1;
    }
    if (parse_conv(pp, &cs) != 0) {
        printf_toto_emit_msg("missing conversion specifier");
        *errp = 1;
        return 0;
    }

    if (cs.width_star) {
        cs.width = get_star_width(args, nargs, argi, errp, &cs.left);
    }
    if (cs.prec_star) {
        const char *s = next_arg(args, nargs, argi);
        intmax_t v;

        if (s == NULL) {
            cs.prec = 0;
        } else {
            v = parse_signed(s, errp);
            if (v < 0) {
                cs.prec = -1;
            } else if (v > (intmax_t)INT_MAX) {
                cs.prec = INT_MAX;
            } else {
                cs.prec = (int)v;
            }
        }
    }

    if (cs.spec == '%') {
        if (out_cap < 1) {
            return -1;
        }
        out[0] = '%';
        *out_len = 1;
        return 0;
    }

    switch (cs.spec) {
    case 's':
        arg = next_arg(args, nargs, argi);
        return format_string_conv(&cs, arg, out, out_cap, out_len);
    case 'c':
        arg = next_arg(args, nargs, argi);
        return format_char_conv(&cs, arg, out, out_cap, out_len);
    case 'd':
    case 'i':
    case 'u':
    case 'o':
    case 'x':
    case 'X':
        arg = next_arg(args, nargs, argi);
        return format_int_conv(&cs, arg, out, out_cap, out_len, errp);
    default:
        snprintf(msg, sizeof(msg), "invalid conversion specifier '%c'",
                 cs.spec);
        printf_toto_emit_msg(msg);
        *errp = 1;
        return 0;
    }
}

static void emit_chunk(const char *buf, size_t n)
{
    if (n == 0) {
        return;
    }
    printf_toto_exit_if_stop_requested();
    try_write_all(buf, n);
}

static int print_formatted(const char *format, char **args, int nargs,
                           int *args_used_out, int *had_error)
{
    const char *p = format;
    int argi = 0;
    char stack_buf[PRINTF_TOTO_CONV_STACK_CAP];
    char lit[256];
    size_t lit_len = 0;

    *args_used_out = 0;
    while (*p != '\0') {
        printf_toto_exit_if_stop_requested();

        if (*p == '\\') {
            unsigned char byte;

            p++;
            if (expand_escape(&p, &byte)) {
                if (lit_len + 1 >= sizeof(lit)) {
                    emit_chunk(lit, lit_len);
                    lit_len = 0;
                }
                lit[lit_len++] = (char)byte;
            }
            continue;
        }

        if (*p != '%') {
            if (lit_len + 1 >= sizeof(lit)) {
                emit_chunk(lit, lit_len);
                lit_len = 0;
            }
            lit[lit_len++] = *p++;
            continue;
        }

        if (lit_len > 0) {
            emit_chunk(lit, lit_len);
            lit_len = 0;
        }

        {
            size_t out_len = 0;
            int rc;
            char *heap = NULL;
            char *dest = stack_buf;
            size_t cap = sizeof(stack_buf);
            const char *save_p = p;
            int save_argi = argi;

            rc = format_one_specifier(&p, args, nargs, &argi, dest, cap,
                                      &out_len, had_error);
            if (rc < 0) {
                p = save_p;
                argi = save_argi;
                heap = (char *)malloc(PRINTF_TOTO_CONV_STACK_CAP * 4U);
                if (heap == NULL) {
                    printf_toto_emit_error("malloc");
                    exit(PRINTF_TOTO_EXIT_ERR);
                }
                dest = heap;
                cap = PRINTF_TOTO_CONV_STACK_CAP * 4U;
                rc = format_one_specifier(&p, args, nargs, &argi, dest, cap,
                                          &out_len, had_error);
                if (rc < 0) {
                    free(heap);
                    printf_toto_emit_msg("formatted conversion too large");
                    *had_error = 1;
                    continue;
                }
            }
            emit_chunk(dest, out_len);
            free(heap);
        }
    }

    if (lit_len > 0) {
        emit_chunk(lit, lit_len);
    }
    *args_used_out = argi;
    return argi;
}

int printf_toto_run(int argc, char **argv)
{
    const char *format;
    char **args;
    int nargs;
    int status = PRINTF_TOTO_EXIT_OK;
    int had_error = 0;

    if (argc < 1 || argv == NULL || argv[0] == NULL) {
        printf_toto_emit_msg("missing operand");
        return PRINTF_TOTO_EXIT_ERR;
    }

    format = argv[0];
    args = argv + 1;
    nargs = argc - 1;

    do {
        int used = 0;

        (void)print_formatted(format, args, nargs, &used, &had_error);
        if (used <= 0) {
            break;
        }
        args += used;
        nargs -= used;
    } while (nargs > 0);

    if (had_error) {
        status = PRINTF_TOTO_EXIT_ERR;
    }
    return status;
}
