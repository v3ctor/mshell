#include "../include/errors.h"

#include <errno.h>
#include <stdlib.h>

void print_syntax_error(std_io *io) {
  writer_write_str(&io->err, SYNTAX_ERROR_STR);
  writer_flush(&io->err);
}

void print_exec_error_and_exit(std_io *io, const char *name) {
  writer_write_format(&io->err, "%s: %s", name, EXEC_ERROR);
  writer_flush(&io->err);
  exit(EXEC_FAILURE);
}

void print_redir_error_and_exit(std_io *io, const char *name) {
  writer_write_format(&io->err, "%s: %s", name, REDIR_ERROR);
  writer_flush(&io->err);
  exit(REDIR_FAILURE);
}

void print_builtin_error(std_io *io, const char *name) {
  writer_write_format(&io->err, "Builtin %s error.\n", name);
  writer_flush(&io->err);
}
