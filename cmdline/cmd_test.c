/* cmd_test.c */

#include <stdlib.h>
#include <stdio.h>

#include "eeprom.h"
#include "serial.h"
#include "serial_comm.h"

void cmd_test(char *progname, struct cmdline *options)
{
  int fd = serial_open(options->port, options->speed);
  if (fd < 0) {
    printf("%s: %s\n", progname, serial_get_error_message(fd));
    exit(1);
  }
  
  char buf[1024];
  size_t buf_len;
  while (1) {
    buf_len = serial_read_line(fd, buf, sizeof(buf));
    printf("%.*s", (int) buf_len, buf);
    fflush(stdout);
  }
}
