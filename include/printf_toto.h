#ifndef PRINTF_TOTO_H
#define PRINTF_TOTO_H

#define PRINTF_TOTO_VERSION_STRING "0.1.0"

enum {
    PRINTF_TOTO_EXIT_OK = 0,
    PRINTF_TOTO_EXIT_ERR = 1,
    PRINTF_TOTO_EXIT_SIGINT = 130
};

/*
 * Precondition: argc >= 1, argv[0] is FORMAT, argv[1..] are arguments.
 * Postcondition: returns PRINTF_TOTO_EXIT_OK or PRINTF_TOTO_EXIT_ERR.
 * Write failures / SIGINT exit from within helpers and do not return.
 */
int printf_toto_run(int argc, char **argv);

#endif /* PRINTF_TOTO_H */
