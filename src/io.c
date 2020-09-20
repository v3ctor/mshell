#include "../include/io.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/sys.h"

#define max(x, y) ((x) < (y) ? (y) : (x))
#define min(x, y) ((x) < (y) ? (x) : (y))

void reader_init(reader *r, int fd) {
  r->fd = fd;
  r->cursor = 0;
  r->size = 0;
}

// Attempts to fill reader's buffer with data from file.

// Returns number of bytes read, -1 indicates error, and errno is set.
// If non-zero number of bytes is read, but the reading ended with an
// error, returns number of bytes read, and errno is set.
ssize_t reader_fill(reader *r) {
  ssize_t nbytes;

  nbytes = bread(r->fd, r->buffer, READER_BUFFER_SIZE);

  r->cursor = 0;
  r->size = max(nbytes, 0);

  return r->size ? r->size : nbytes;
}

// Attempts to peek next byte from buffor.

// Returns next byte from buffer, -1 indicates end of file, -2 indicates
// error, and errno is set.
int reader_peek_char(reader *r) {
  if (r->cursor >= r->size) {
    int nbytes = reader_fill(r);
    if (nbytes <= 0) {
      return nbytes - 1;
    }
  }

  return r->buffer[r->cursor];
}

// Attempts to get next byte from buffor.

// Same as reader_peek_char, but it also moves cursor by one.
int reader_get_char(reader *r) {
  int result = reader_peek_char(r);

  if (result < 0) {
    return result;
  }

  r->cursor++;
  return result;
}

ssize_t reader_read_until(reader *r, char *buffer, size_t max, char until) {
  ssize_t nbytes;
  int c;

  for (nbytes = 0; !max || (size_t)nbytes < max;) {
    if ((c = reader_get_char(r)) < 0) {
      break;
    }

    if (buffer) {
      buffer[nbytes++] = c;
    }

    if (c == until) {
      break;
    }
  }

  return nbytes ? nbytes : c + 1;
}

ssize_t reader_read_line(reader *r, char *buffer, size_t max) {
  return reader_read_until(r, buffer, max, '\n');
}

void writer_init(writer *w, int fd) {
  w->cursor = 0;
  w->fd = fd;
}

size_t writer_free_space(writer *w) { return WRITER_BUFFER_SIZE - w->cursor; }

ssize_t writer_flush(writer *w) {
  ssize_t total = 0;
  ssize_t nbytes = 0;

  while (total < w->cursor) {
    if ((nbytes = bwrite(w->fd, w->buffer, w->cursor)) <= 0) {
      break;
    }

    total += nbytes;
  }

  if (total != w->cursor) {
    memmove(w->buffer, w->buffer + total, w->cursor - total);
  }

  w->cursor -= total;
  return total ? total : nbytes;
}

ssize_t writer_write_char(writer *w, char c) {
  if (w->cursor >= WRITER_BUFFER_SIZE) {
    ssize_t nbytes = writer_flush(w);
    if (nbytes <= 0) {
      return nbytes;
    }
  }

  w->buffer[w->cursor++] = c;
  return 1;
}

ssize_t writer_write_nstr(writer *w, const char *str, size_t len) {
  if (len <= writer_free_space(w)) {
    memcpy(w->buffer + w->cursor, str, len);
    w->cursor += len;
    return len;
  }

  ssize_t total = 0;
  ssize_t nbytes = 0;

  while ((size_t)total < len) {
    size_t to_write = min(writer_free_space(w), len - total);
    memcpy(w->buffer + w->cursor, str + total, to_write);
    w->cursor += to_write;
    total += to_write;

    nbytes = writer_flush(w);
    if (nbytes <= 0) {
      break;
    }
  }

  return total ? total : nbytes;
}

ssize_t writer_write_str(writer *w, const char *str) {
  return writer_write_nstr(w, str, strlen(str));
}

void std_io_init(std_io *io) {
  reader_init(&io->in, 0);
  writer_init(&io->out, 1);
  writer_init(&io->err, 2);
}
