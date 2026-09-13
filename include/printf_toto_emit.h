#ifndef PRINTF_TOTO_EMIT_H
#define PRINTF_TOTO_EMIT_H

/* errno-backed: printf-toto: <context>: <strerror(errno)> */
void printf_toto_emit_error(const char *context);

/* conversion/format diagnostic: printf-toto: <message>\n */
void printf_toto_emit_msg(const char *msg);

#endif /* PRINTF_TOTO_EMIT_H */
