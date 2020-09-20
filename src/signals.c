#include "../include/signals.h"

#include <signal.h>
#include <stddef.h>

sigset_t empty_mask;
sigset_t sigmask;

void signals_init(void) {
    sigemptyset(&sigmask);
    sigemptyset(&empty_mask);
    sigaddset(&sigmask, SIGCHLD);

    signal(SIGINT, SIG_IGN);
}

sigset_t signals_empty_mask(void) {
    return empty_mask;
}

void signals_block(void) {
    sigprocmask(SIG_BLOCK, &sigmask, NULL);
}

void signals_unblock(void) {
    sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
}