#ifndef _V3CT0R_DEBUG_H
#define _V3CT0R_DEBUG_H

#define _V3CT0R_DEBUG_BUFFER_SIZE 1024

#ifdef DEBUG

#include <unistd.h>
#include <stdio.h>

#define _dbg(p, f, ...)                                                        \
  ({                                                                           \
    char buffer[_V3CT0R_DEBUG_BUFFER_SIZE];                                    \
    int cursor = 0;                                                            \
                                                                               \
    cursor += sprintf(buffer + cursor, "\x1B[30;1m");                          \
    if (p)                                                                     \
      cursor += sprintf(buffer + cursor, "|D| [pid %d] ", getpid());           \
    cursor += sprintf(buffer + cursor, f __VA_OPT__(, ) __VA_ARGS__);          \
    if (p)                                                                     \
      cursor += sprintf(buffer + cursor, "\x1B[0m");                           \
    fprintf(stderr, buffer);                                                   \
    fflush(stderr);                                                            \
  })

#define dbg(f, ...) _dbg(true, f, __VA_ARGS__);
#define pdbg(f, ...) _dbg(false, f, __VA_ARGS__);

#else

#define dbg(f, ...)
#define pdbg(f, ...)

#endif

#endif // _V3CT0R_DEBUG_H
