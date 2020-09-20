#ifndef _V3CT0R_SYS_H
#define _V3CT0R_SYS_H

#include <sys/types.h>

int bdup2(int fd, int fd2);
__pid_t bwaitpid(__pid_t pid, int *stat_loc, int options);
ssize_t bread(int fd, char *buf, size_t s);
ssize_t bwrite(int fd, const char *buf, size_t s);
int bopen(const char *filename, int flags);
int bopenm(const char *filename, int flags, int mode);
int bclose(int fd);

#endif // _V3CT0R_SYS_H
