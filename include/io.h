#ifndef _V3CT0R_IO_H
#define _V3CT0R_IO_H

#include "../include/config.h"

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct {
  char buffer[READER_BUFFER_SIZE + 1];
  int fd;
  ssize_t cursor;
  ssize_t size;
} reader;

// Initializes reader for reading from given file descriptor.
void reader_init(reader *r, int fd);

// Attempts to read up to max bytes from file into buffer until first
// occurance of specified character.

// When 'max' is 0 then there's no limit on the number of bytes read
// When 'buffer' is NULL fuction will only move cursor pointer, without reading

// Returns number of bytes read into buffer, 0 indicated end of file,
// -1 indicates error, and errno is set.
ssize_t reader_read_until(reader *r, char *buffer, size_t max, char until);

// Wrapper around reader_read_until(r, buffer, max, '\n');
ssize_t reader_read_line(reader *r, char *buffer, size_t max);

typedef struct {
  char buffer[WRITER_BUFFER_SIZE];
  int fd;
  ssize_t cursor;
} writer;

// Initializes writer for writing to given file descriptor.
void writer_init(writer *w, int fd);

// Attempts to flush writer's buffor to file

// Returns number of bytes written 0 indicated end of file, -1 indicated
// error, and errno is set.
ssize_t writer_flush(writer *w);

// Attempts to write one character into writer.

// Returns number of bytes written, 0 indicated end of file, -1 indicated
// error, and errno is set.
ssize_t writer_write_char(writer *w, char c);

// Attempts to write len bytes from str into writer.

// Returns number of bytes writtenm, 0 indicated end of file, -1 indicated
// error, and errno is set.
ssize_t writer_write_nstr(writer *w, const char *str, size_t len);

// Wrapper around writer_write_nstr(w, str, strlen(str));
ssize_t writer_write_str(writer *w, const char *str);

// Writes snprintf(buffer, size, format, __VA_ARGS__) into file.

// Retuns number of bytes written, 0 indicated end of file, -1 indicated
// error, and errno is set (possibly from snprintf).
#define writer_writer_nformat(w, size, format, ...)                            \
  ({                                                                           \
    char buffer[(size)];                                                       \
    int status = snprintf(buffer, (size), (format), __VA_ARGS__);              \
    status <= 0 ? status : writer_write_nstr((w), (buffer), status);           \
  })

// Writes snprintf(buffer, WRITER_FORMAT_BUFFER_SIZE, format, __VA_ARGS__) into
// file.

// Retuns number of bytes written, 0 indicated end of file, -1 indicated
// error, and errno is set (possibly from snprintf).
#define writer_write_format(w, format, ...)                                    \
  writer_writer_nformat((w), WRITER_FORMAT_BUFFER_SIZE, (format), __VA_ARGS__)

typedef struct {
  reader in;
  writer out;
  writer err;
} std_io;

// Initializes std_io with file descriptors 0, 1 and 2.
void std_io_init(std_io *io);

#endif // _V3CT0R_IO_H
