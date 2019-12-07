/* cmd_dump.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eeprom.h"
#include "serial.h"
#include "serial_comm.h"
#include "util.h"

/*
 * Show contents of EEPROM.  Accepted command line formats:
 *
 *   dump                    # dump whole EEPROM
 *   dump address            # dump from address to end
 *   dump address length     # dump from address to address+length
 *
 */
void cmd_dump(char *progname, struct cmdline *options)
{
  // read options
  struct eeprom_range range;
  if (parse_eeprom_range(&range, &options->command[1])) {
    printf("%s: invalid range\n", progname);
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
  while (1) {
    buf_len = serial_read_line(fd, buf, sizeof(buf));
    if (buf_len == 0 || buf[0] == '*') {
      break;
    }
  }

  // send dump command
  snprintf(buf, sizeof(buf), "d %x %x\n", range.addr, range.len);
  buf_len = strlen(buf);
  if (serial_write_all(fd, buf, buf_len) != buf_len) {
    printf("ERROR: can't write to serial\n");
    exit(1);
  }

  // read dump contents
  while (1) {
    buf_len = serial_read_line(fd, buf, sizeof(buf));
    if (buf_len == 0 || buf[0] == '*') {
      break;
    }
    printf("%.*s", (int) buf_len, buf);
    fflush(stdout);
  }

  serial_close(fd);
}
