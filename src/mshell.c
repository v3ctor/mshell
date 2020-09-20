#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtins.h"
#include "../include/config.h"
#include "../include/errors.h"
#include "../include/pipeline.h"
#include "../include/io.h"
#include "../include/signals.h"
#include "../include/siparse.h"
#include "../include/utils.h"

bool is_prompt(std_io *io) {
  struct stat stat;

  if (fstat(0, &stat)) {
    writer_write_str(&io->err, strerror(errno));
    writer_flush(&io->err);
    exit(1);
  }

  return S_ISCHR(stat.st_mode);
}

void print_prompt(std_io *io) {
  static bool initialized = false;
  static bool show_prompt;

  if (!initialized) {
    show_prompt = is_prompt(io);
    initialized = true;
  }

  if (show_prompt) {
    print_bg_notes(io);
    writer_write_str(&io->out, PROMPT_STR);
    writer_flush(&io->out);
  } else {
    clear_bg_notes();
  }
}

ssize_t read_line(std_io *io, char *buffer, size_t max) {
  ssize_t nbytes = reader_read_line(&io->in, buffer, max);
  if (nbytes <= 0) {
    return _V3CT0R_SHELL_BREAK;
  }

  if (buffer[nbytes - 1] != '\n') {
    // swallow whole line
    reader_read_line(&io->in, NULL, 0);

    writer_write_str(&io->err, SYNTAX_ERROR_STR);
    writer_flush(&io->err);
    return _V3CT0R_SHELL_CONTINUE;
  }

  if (buffer[0] == '\n') {
    return _V3CT0R_SHELL_CONTINUE;
  }

  buffer[nbytes] = '\0';
  return nbytes;
}

int main(void) {
  pipeline_init();
  signals_init();

  std_io io;
  std_io_init(&io);

  char line[MAX_LINE_LENGTH + 1];
  pipelineseq *ps;
  ssize_t nbytes;

  while (true) {
    print_prompt(&io);

    nbytes = read_line(&io, line, MAX_LINE_LENGTH);
    _V3CT0R_SHELL_CHECK(nbytes);

    if (!(ps = parseline(line)) || !is_valid_pipelineseq(ps)) {
      print_syntax_error(&io);
      continue;
    }

    execute_pipeline_sequence(&io, ps);
  }

  return 0;
}
