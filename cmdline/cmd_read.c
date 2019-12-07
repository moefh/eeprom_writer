/* cmd_read.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eeprom.h"
#include "serial.h"
#include "serial_comm.h"
#include "util.h"

static int decode_nibble(char ch)
{
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

static int decode_and_write(FILE *f, char *buf, size_t len)
{
  int high_nibble = -1;
  for (size_t pos = 0; pos < len; pos++) {
    if (buf[pos] == '\r' || buf[pos] == '\n') {
      continue;
    }
    int nibble = decode_nibble(buf[pos]);
    if (high_nibble >= 0) {
      putc((high_nibble<<4) | nibble, f);
      high_nibble = -1;
    } else {
      high_nibble = nibble;
    }
  }
  return 0;
}

static void wait_prompt(int fd, char *buf, size_t buf_size)
{
  while (1) {
    size_t buf_len = serial_read_line(fd, buf, buf_size);
    if (buf_len == 0 || buf[0] == '*') {
      break;
    }
  }
}

/*
 * Copy EEPROM to file.  Accepted command line formats:
 *
 *   read filename                   # read whole EEPROM
 *   read filename address           # read from address to end
 *   read filename address length    # read from address to address+length
 *
 */
void cmd_read(char *progname, struct cmdline *options)
{
  // read options
  char *filename = options->command[1];
  if (filename == NULL) {
    printf("%s: filename required for 'read' command\n", progname);
    exit(1);
  }
  struct eeprom_range range;
  if (parse_eeprom_range(&range, &options->command[2])) {
    printf("%s: invalid range\n", progname);
    exit(1);
  }

  // open output file
  FILE *f = fopen(filename, "wb");
  if (f == NULL) {
    printf("%s: can't open file '%s'\n", progname, filename);
    exit(1);
  }

  // open serial
  int fd = serial_open(options->port, options->speed);
  if (fd < 0) {
    printf("%s: %s\n", progname, serial_get_error_message(fd));
    exit(1);
  }
  
  char buf[1024];
  size_t buf_len;

  // wait for prompt
  wait_prompt(fd, buf, sizeof(buf));

  // read from eeprom and write to file
  unsigned int end_addr = range.addr + range.len;
  for (unsigned int chunk_pos = range.addr; chunk_pos < end_addr; chunk_pos += 256) {
    // request chunk of 256 bytes or less
    unsigned int chunk_len = (end_addr - chunk_pos > 256) ? 256 : end_addr - chunk_pos;
    snprintf(buf, sizeof(buf), "r %x %x\n", chunk_pos, chunk_len);
    buf_len = strlen(buf);
    if (serial_write_all(fd, buf, buf_len) != buf_len) {
      printf("%s: error writing to serial\n", progname);
      exit(1);
    }
    while (1) {
      // read chunk data
      buf_len = serial_read_line(fd, buf, sizeof(buf));
      if (buf_len == 0 || buf[0] == '*') {
        break;
      }

      // write chunk to file
      if (decode_and_write(f, buf, buf_len) != 0) {
        printf("%s: invalid data read\n", progname);
        exit(1);
      }
    }
  }

  fclose(f);
  serial_close(fd);
}
