/* serial_comm.c */

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>

#include "serial_comm.h"

static char read_buf[4096];
static size_t read_buf_len = 0;

static size_t read_data(int fd, char *buf, size_t buf_len)
{
  while (1) {
    ssize_t n = read(fd, buf, buf_len);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return 0;
    }
    return n;
  }
}

size_t serial_read_line(int fd, void *data, size_t data_size)
{
  char *newline = memchr(read_buf, '\n', read_buf_len);
  while (newline == NULL && read_buf_len < sizeof(read_buf)) {
    size_t n = read_data(fd, read_buf + read_buf_len, sizeof(read_buf) - read_buf_len);
    read_buf_len += n;
    newline = memchr(read_buf, '\n', read_buf_len);
  }

  size_t copy_len = (newline == NULL) ? read_buf_len : ((size_t) (newline - read_buf + 1));
  if (copy_len > data_size) copy_len = data_size;
  if (copy_len > 0) {
    memcpy(data, read_buf, copy_len);
    memmove(read_buf, read_buf + copy_len, read_buf_len - copy_len);
    read_buf_len -= copy_len;
  }
  return copy_len;
}

size_t serial_write_all(int fd, void *data, size_t data_size)
{
  char *buf = data;
  size_t buf_written = 0;

  while (buf_written < data_size) {
    ssize_t n = write(fd, buf + buf_written, data_size - buf_written);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return buf_written;
    }
    buf_written += n;
  }
  return buf_written;
}
