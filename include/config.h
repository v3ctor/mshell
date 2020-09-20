#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __GNUC__ // GCC
#define _V3CT0R_UNUSED __attribute__((unused))
#elif __clang__ // clang
#define _V3CT0R_UNUSED __attribute__((unused))
#else // default
#define _V3CT0R_UNUSED
#endif

#define _V3CT0R_BG_NOTES_SIZE 2048
#define _V3CT0R_FG_NOTES_SIZE 1024

#define _V3CT0R_BG_NOTES_EXIT                                                  \
  "Background process %d terminated. (exited with status %d)\n"
#define _V3CT0R_BG_NOTES_KILL                                                  \
  "Background process %d terminated. (killed by signal %d)\n"

#define _V3CT0R_SHELL_BREAK -2
#define _V3CT0R_SHELL_CONTINUE -3
#define _V3CT0R_SHELL_CHECK(status)                                            \
  if ((status) == _V3CT0R_SHELL_BREAK) {                                       \
    break;                                                                     \
  } else if ((status) == _V3CT0R_SHELL_CONTINUE) {                             \
    continue;                                                                  \
  }

#define PROMPT_STR "$ "

#define MAX_LINE_LENGTH 2048
#define READER_BUFFER_SIZE (2 * MAX_LINE_LENGTH)
#define WRITER_BUFFER_SIZE (2 * MAX_LINE_LENGTH)
#define WRITER_FORMAT_BUFFER_SIZE WRITER_BUFFER_SIZE

#endif /* !_CONFIG_H_ */
