#ifndef PRINTF_TOTO_CLI_H
#define PRINTF_TOTO_CLI_H

/* Sole-operand meta flags. Returns 1 if handled (caller should exit 0). */
int scan_meta_flags(int argc, char **argv);

void print_help(void);
void print_version(void);

int install_sigpipe_ignore(void);
int install_sigint_handler(void);
void printf_toto_exit_if_stop_requested(void);

#endif /* PRINTF_TOTO_CLI_H */
