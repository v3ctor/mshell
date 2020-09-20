#ifndef _V3ECT0R_ERRORS_H
#define _V3ECT0R_ERRORS_H

#include "io.h"

#define SYNTAX_ERROR_STR "Syntax error.\n"

#define REDIR_DEFAULT_ERROR "redirs error\n"
#define REDIR_EACCESS_ERROR "permission denied\n"
#define REDIR_ENOENT_ERROR "no such file or directory\n"
#define REDIR_FAILURE 127
#define REDIR_ERROR                                                            \
  (errno == ENOENT                                                             \
       ? REDIR_ENOENT_ERROR                                                    \
       : errno == EACCES ? REDIR_EACCESS_ERROR : REDIR_DEFAULT_ERROR)

#define EXEC_DEFAULT_ERROR "exec error\n"
#define EXEC_EACCESS_ERROR "permission denied\n"
#define EXEC_ENOENT_ERROR "no such file or directory\n"
#define EXEC_FAILURE 127
#define EXEC_ERROR                                                             \
  (errno == ENOENT                                                             \
       ? EXEC_ENOENT_ERROR                                                     \
       : errno == EACCES ? EXEC_EACCESS_ERROR : EXEC_DEFAULT_ERROR)


void print_syntax_error(std_io *io);

void print_exec_error_and_exit(std_io *io, const char *name);

void print_redir_error_and_exit(std_io *io, const char *name);

void print_builtin_error(std_io *io, const char *name);

#endif // _V3ECT0R_ERRORS_H
