/* cmd_write.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eeprom.h"
#include "serial.h"
#include "serial_comm.h"
#include "util.h"

static void wait_prompt(int fd, char *buf, size_t buf_size)
{
  while (1) {
    size_t buf_len = serial_read_line(fd, buf, buf_size);
    if (buf_len == 0 || buf[0] == '*') {
      break;
    }
  }
}

static char nibble_to_hex(int n)
{
  if (n >=  0 && n <=  9) return n + '0';
  if (n >= 10 && n <= 15) return n - 10 + 'a';
  return '0';
}

static size_t format_write_data(char *out, const char *data, size_t pos, size_t chunk_size)
{
  for (size_t i = 0; i < chunk_size; i++) {
    out[2*i + 0] = nibble_to_hex(((unsigned char)data[pos+i] & 0xf0) >> 4);
    out[2*i + 1] = nibble_to_hex(((unsigned char)data[pos+i] & 0x0f) >> 0);
  }
  out[2*chunk_size] = '\n';
  return 2*chunk_size + 1;
}

void cmd_write(char *progname, struct cmdline *options)
{
  // read options
  char *filename = options->command[1];
  if (filename == NULL) {
    printf("%s: filename required for 'read' command\n", progname);
    exit(1);
  }
  struct eeprom_range range;
  if (parse_eeprom_range(&range, &options->command[2])) {
    printf("%s: invalid address\n", progname);
    exit(1);
  }

  // read file
  struct file_data data;
  int err = read_file(&data, filename);
  if (err != 0) {
    printf("%s: error reading '%s': %s\n", progname, filename, get_read_file_error_message(err));
    exit(1);
  }
  if (range.addr + data.size > EEPROM_SIZE) {
    if (range.addr == 0) {
      printf("%s: file '%s' doesn't fit in EEPROM\n", progname, filename);
    } else {
      printf("%s: file '%s' doesn't fit in EEPROM at address 0x%04x\n", progname, filename, range.addr);
    }
    exit(1);
  }

  // open serial
  int fd = serial_open(options->port, options->speed);
  if (fd < 0) {
    printf("%s: %s\n", progname, serial_get_error_message(fd));
    exit(1);
  }

  char buf[1024];

  // wait for first prompt
  wait_prompt(fd, buf, sizeof(buf));

  // write data in chunks of 256 bytes (chunk size must be less than (sizeof(buf)/2-1)
  unsigned int end = range.addr + (unsigned int) data.size;
  for (unsigned int pos = range.addr; pos < end; pos += 256) {
    unsigned int chunk_size = (end-pos > 256) ? 256 : end-pos;

    // write command
    snprintf(buf, sizeof(buf), "w %x %x\n", pos, chunk_size);
    size_t cmd_buf_len = strlen(buf);
    if (serial_write_all(fd, buf, cmd_buf_len) != cmd_buf_len) {
      printf("%s: error writing command to serial\n", progname);
      exit(1);
    }

    // write data
    size_t data_buf_len = format_write_data(buf, data.data, pos, chunk_size);
    if (serial_write_all(fd, buf, data_buf_len) != data_buf_len) {
      printf("%s: error writing data to serial\n", progname);
      exit(1);
    }

    // wait for next prompt
    wait_prompt(fd, buf, sizeof(buf));
  }
  
  free(data.data);
  serial_close(fd);
}
