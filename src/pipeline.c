#include "../include/pipeline.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtins.h"
#include "../include/debug.h"
#include "../include/errors.h"
#include "../include/signals.h"
#include "../include/siparse.h"
#include "../include/sys.h"

typedef struct bg_notes_entry {
  __pid_t pid;
  int status;
  struct bg_notes_entry *next;
} bg_notes_entry;

volatile bg_notes_entry bg_notes[_V3CT0R_BG_NOTES_SIZE];
volatile int bg_notes_size = 0;

void add_bg_note(__pid_t pid, int status) {
  if (bg_notes_size == _V3CT0R_BG_NOTES_SIZE) {
    return;
  }

  dbg("add_bg_note %d %d\n", pid, status);
  bg_notes[bg_notes_size].pid = pid;
  bg_notes[bg_notes_size].status = status;
  ++bg_notes_size;
}

void process_bg_notes(std_io *io, bool print) {
  signals_block();
  dbg("process_bg_notes %d\n", print);

  for (int i = 0; i < bg_notes_size; ++i) {
    __pid_t pid = bg_notes[i].pid;
    int status = bg_notes[i].status;

    if (print) {
      char *format = NULL;

      if (WIFEXITED(status)) {
        format = _V3CT0R_BG_NOTES_EXIT;
      } else if (WIFSIGNALED(status)) {
        format = _V3CT0R_BG_NOTES_KILL;
      }

      if (format) {
        writer_write_format(&io->out, format, pid, WEXITSTATUS(status));
      }
    }
  }

  bg_notes_size = 0;
  signals_unblock();
}

void print_bg_notes(std_io *io) { process_bg_notes(io, true); }

void clear_bg_notes(void) { process_bg_notes(NULL, false); }

typedef struct fg_child_entry {
  __pid_t pid;
} fg_child_entry;

volatile fg_child_entry fg_children[_V3CT0R_FG_NOTES_SIZE];
volatile int fg_children_size = 0;
volatile int fg_count = 0;

void add_fg_child(__pid_t pid) {
  if (fg_children_size == _V3CT0R_FG_NOTES_SIZE) {
    return;
  }

  dbg("add_fg_child %d\n", pid);
  fg_children[fg_children_size].pid = pid;
  ++fg_children_size;
}

bool remove_fg_child(__pid_t pid) {
  for (int i = 0; i < fg_children_size; ++i) {
    if (fg_children[i].pid == pid) {
      fg_children[i].pid = fg_children[fg_children_size - 1].pid;
      --fg_children_size;
      return true;
    }
  }

  return false;
}

void child_handler(_V3CT0R_UNUSED int signum) {
  int prev_errno = errno;

  __pid_t wait_status;
  int child_status;

  while ((wait_status = waitpid(-1, &child_status, WNOHANG)) > 0) {
    if (!remove_fg_child(wait_status)) {
      add_bg_note(wait_status, child_status);
    } else {
      --fg_count;
    }
  }

  errno = prev_errno;
}

void pipeline_init(void) {
  struct sigaction act;
  act.sa_flags = 0;
  act.sa_handler = child_handler;
  sigemptyset(&act.sa_mask);
  sigaction(SIGCHLD, &act, NULL);
}

int try_execute_builtin(std_io *io, char **argv, bool *found) {
  *found = false;

  int i = 0;
  for (i = 0; builtins_table[i].name; ++i) {
    if (!strcmp(builtins_table[i].name, argv[0])) {
      *found = true;
      dbg("\ttry_execute_builtin %s OK\n", argv[0]);
      return builtins_table[i].fun(io, argv);
    }
  }

  dbg("\ttry_execute_builtin %s ERR\n", argv[0]);
  return 0;
}

void apply_redirs(std_io *io, command *cmd) {
  if (!cmd || !cmd->redirs || !cmd->redirs->r) {
    return;
  }

  redirseq *current = cmd->redirs;
  redirseq *last = cmd->redirs;

  do {
    int cmd_flags = current->r->flags;
    const char *filename = current->r->filename;

    if (IS_RIN(cmd_flags)) {
      errno = 0;
      bclose(0);

      if (errno) {
        writer_write_str(&io->err, strerror(errno));
        writer_flush(&io->err);
        exit(0);
      }

      int fd = bopen(filename, O_RDONLY);

      if (fd == -1 || fd) {
        print_redir_error_and_exit(io, filename);
      }

      dbg("\t|%s| !stdin = %s\n", cmd->args->arg, filename);
    } else /*if (IS_RAPPEND(cmd_flags) || IS_ROUT(cmd_flags))*/ {
      int open_flags = O_CREAT | O_WRONLY;
      if (IS_RAPPEND(cmd_flags)) {
        open_flags |= O_APPEND;
      } else /*if (IS_ROUT(cmd_flags))*/ {
        open_flags |= O_TRUNC;
      }

      errno = 0;
      bclose(1);

      if (errno) {
        writer_write_str(&io->err, strerror(errno));
        writer_flush(&io->err);
        exit(0);
      }

      int fd = bopenm(filename, open_flags, 0644);
      if (fd == -1 || fd != 1) {
        print_redir_error_and_exit(io, filename);
      }

      dbg("\t|%s| !stdout = %s\n", cmd->args->arg, filename);
    }

    current = current->next;
  } while (current != last);
}

int execute_command(std_io *io, command *cmd, bool builtins, bool in_background,
                    int in_fd, int out_fd, int close_fd) {
  argseq *it = cmd->args;
  argseq *last = cmd->args;
  char *argv[MAX_LINE_LENGTH + 1];
  size_t cursor = 0;
  pid_t pid;

  dbg("execute_command");
  pdbg(" |%s|", it->arg);
  argv[cursor++] = it->arg;
  for (it = it->next; it != last; it = it->next) {
    pdbg(" |%s|", it->arg);
    argv[cursor++] = it->arg;
  }
  pdbg("\n");
  argv[cursor++] = NULL;

  if (builtins) {
    bool found;
    int builtin_status = try_execute_builtin(io, argv, &found);

    if (found) {
      if (builtin_status == BUILTIN_ERROR) {
        print_builtin_error(io, argv[0]);
      }

      return 0;
    }
  }

  if ((pid = fork()) == -1) {
    writer_write_str(&io->err, strerror(errno));
    writer_flush(&io->err);
    return 0;
  }

  if (pid == 0) {
    std_io_init(io);

    if (in_background) {
      setsid();
    }
    signal(SIGINT, SIG_DFL);

    if (in_fd != -1) {
      dbg("\t|%s| stdin = %d\n", argv[0], in_fd);
      bdup2(in_fd, 0);
      bclose(in_fd);
    }

    if (out_fd != -1) {
      dbg("\t|%s| stdout = %d\n", argv[0], out_fd);
      bdup2(out_fd, 1);
      bclose(out_fd);
    }

    if (close_fd != -1) {
      dbg("\t|%s| close = %d\n", argv[0], close_fd);
      bclose(close_fd);
    }

    apply_redirs(io, cmd);
    signals_unblock();
    execvp(argv[0], argv);
    print_exec_error_and_exit(io, argv[0]);
  } else if (!in_background) {
    add_fg_child(pid);
    return 1;
  }

  return 0;
}

void execute_pipeline(std_io *io, pipeline *pl) {
  if (!pl->commands || !pl->commands->com) {
    return;
  }

  const bool is_single = pl->commands->com == pl->commands->next->com;
  const bool has_redirs = pl->commands->com->redirs;
  const bool in_background = IS_INBACKGROUND(pl->flags);
  const bool can_be_builtin = is_single && !has_redirs && !in_background;

  signals_block();

  if (is_single) {
    fg_count += execute_command(io, pl->commands->com, can_be_builtin,
                                in_background, -1, -1, -1);
  } else {
    commandseq *curr = pl->commands;
    commandseq *last = pl->commands;

    int pipefd[2];
    int std_in = -1;
    int std_out = -1;

    // First command
    pipe(pipefd);
    dbg("pipe = [%d, %d]\n", pipefd[0], pipefd[1]);
    std_in = pipefd[0];
    std_out = pipefd[1];
    fg_count += execute_command(io, curr->com, false, in_background, -1,
                                std_out, std_in);
    bclose(std_out);

    // Middle commands
    for (curr = curr->next; curr->next != last; curr = curr->next) {

      pipe(pipefd);
      dbg("pipe = [%d, %d]\n", pipefd[0], pipefd[1]);
      std_out = pipefd[1];
      fg_count += execute_command(io, curr->com, false, in_background, std_in,
                                  std_out, pipefd[0]);
      bclose(std_out);
      bclose(std_in);
      std_in = pipefd[0];
    }

    // Last command
    fg_count +=
        execute_command(io, curr->com, false, in_background, std_in, -1, -1);
    bclose(std_in);
  }

  sigset_t empty_mask = signals_empty_mask();
  while (fg_count) {
    dbg("wait for %d children\n", fg_count);
    sigsuspend(&empty_mask);
  }

  dbg("done waiting\n");

  signals_unblock();
}

void execute_pipeline_sequence(std_io *io, pipelineseq *seq) {
  pipelineseq *last = seq;
  pipelineseq *curr = seq;

  do {
    execute_pipeline(io, curr->pipeline);
    curr = curr->next;
  } while (curr != last);
}

bool is_valid_pipeline(pipeline *pl) {
  if (!pl || !pl->commands) {
    return true;
  }

  if (!pl->commands->com && pl->commands == pl->commands->next) {
    return true;
  }

  commandseq *curr_cs = pl->commands;
  commandseq *last_cs = pl->commands;

  do {
    if (!curr_cs || !curr_cs->com) {
      return false;
    }

    curr_cs = curr_cs->next;
  } while (curr_cs != last_cs);

  return true;
}

bool is_valid_pipelineseq(pipelineseq *ps) {
  if (!ps || (!ps->pipeline && ps->next == ps)) {
    return true;
  }

  pipelineseq *curr_ps = ps;
  pipelineseq *last_ps = ps;

  do {
    pipeline *pl = curr_ps->pipeline;
    if (!pl || !is_valid_pipeline(pl)) {
      return false;
    }

    curr_ps = curr_ps->next;
  } while (curr_ps != last_ps);

  return true;
}
