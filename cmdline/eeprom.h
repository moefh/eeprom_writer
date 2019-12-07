/* eeprom.h */

#ifndef EEPROM_H_FILE
#define EEPROM_H_FILE

#define EEPROM_SIZE (2*1024)

struct cmdline {
  char  *port;
  int    speed;
  char **command;
};

void cmd_dump(char *progname, struct cmdline *options);
void cmd_read(char *progname, struct cmdline *options);
void cmd_write(char *progname, struct cmdline *options);
void cmd_test(char *progname, struct cmdline *options);

#endif /* EEPROM_H_FILE */
