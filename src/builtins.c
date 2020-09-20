#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/builtins.h"
#include "../include/config.h"

int echo(std_io *, char *[]);
int lcd(std_io *, char *[]);
int lexit(std_io *, char *[]);
int lkill(std_io *, char *[]);
int lls(std_io *, char *[]);
int undefined(std_io *, char *[]);

builtin_pair builtins_table[] = {
    {"exit", &lexit},  {"lecho", &echo}, {"lcd", &lcd}, {"cd", &lcd},
    {"lkill", &lkill}, {"lls", &lls},    {NULL, NULL}};

int echo(std_io *io, char *argv[]) {
  int i = 1;
  if (argv[i]) {
    writer_write_str(&io->out, argv[i++]);
  }
  while (argv[i]) {
    writer_write_str(&io->out, " ");
    writer_write_str(&io->out, argv[i++]);
  }
  writer_write_str(&io->out, "\n");
  writer_flush(&io->out);
  return 0;
}

int lcd(_V3CT0R_UNUSED std_io *io, char *argv[]) {
  char *path = argv[1] ? argv[1] : getenv("HOME");
  if (path == NULL || (argv[1] && argv[2]) || chdir(path)) {
    return BUILTIN_ERROR;
  }

  return 0;
}

int lexit(_V3CT0R_UNUSED std_io *io, _V3CT0R_UNUSED char *argv[]) {
  exit(0);

  return 0;
}

int lkill(_V3CT0R_UNUSED std_io *io, char *argv[]) {
  if (!argv[1]) {
    return BUILTIN_ERROR;
  }

  long sig = SIGTERM;
  int pid_pos = 1;
  long pid;

  if (argv[1][0] == '-') {
    errno = 0;
    pid_pos = 2;
    sig = strtol(argv[1] + 1, NULL, 10);

    if (errno) {
      return BUILTIN_ERROR;
    }
  }

  if (!argv[pid_pos] || argv[pid_pos + 1]) {
    return BUILTIN_ERROR;
  }

  errno = 0;
  char *end_ptr;
  pid = strtol(argv[pid_pos], &end_ptr, 10);
  if (errno || *end_ptr != '\0' || kill(pid, sig)) {
    return BUILTIN_ERROR;
  }

  return 0;
}

int lls(std_io *io, _V3CT0R_UNUSED char *argv[]) {
  DIR *dir = opendir(".");
  if (dir == NULL) {
    return BUILTIN_ERROR;
  }

  struct dirent *file = NULL;
  errno = 0;

  while ((file = readdir(dir)) != NULL) {
    if (file->d_name[0] != '.') {
      writer_write_format(&io->out, "%s\n", file->d_name);
    }
    errno = 0;
  }

  if (closedir(dir)) {
    return BUILTIN_ERROR;
  }

  // readdir error
  if (errno) {
    return BUILTIN_ERROR;
  }

  writer_flush(&io->out);
  return 0;
}

int undefined(std_io *io, char *argv[]) {
  writer_write_format(&io->err, "Command %s undefined.\n", argv[0]);
  writer_flush(&io->err);
  return BUILTIN_ERROR;
}
