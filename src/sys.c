#include "../include/sys.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int bdup2(int fd, int fd2) {
  int status;

  while ((status = dup2(fd, fd2)) == -1 && errno == EINTR)
    ;

  return status;
}

__pid_t bwaitpid(__pid_t pid, int *stat_loc, int options) {
  __pid_t result;

  while ((result = waitpid(pid, stat_loc, options)) == -1 && errno == EINTR)
    ;

  return result;
}

ssize_t bread(int fd, char *buf, size_t s) {
  ssize_t result;

  while ((result = read(fd, buf, s)) == -1 && errno == EINTR)
    ;

  return result;
}

ssize_t bwrite(int fd, const char *buf, size_t s) {
  ssize_t result;

  while ((result = write(fd, buf, s)) == -1 && errno == EINTR)
    ;

  return result;
}

int bopen(const char *filename, int flags) {
  int status;

  while ((status = open(filename, flags)) == -1 && errno == EINTR)
    ;

  return status;
}

int bopenm(const char *filename, int flags, int mode) {
  int status;

  while ((status = open(filename, flags, mode)) == -1 && errno == EINTR)
    ;

  return status;
}

int bclose(int fd) {
  int status;

  while ((status = close(fd)) == -1 && errno == EINTR)
    ;

  return status;
}