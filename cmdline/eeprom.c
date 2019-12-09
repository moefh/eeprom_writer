/* eeprom.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include "eeprom.h"
#include "util.h"

#define DEFAULT_PORT  "/dev/ttyUSB0"
#define DEFAULT_SPEED 115200

struct command {
  const char *cmd;
  void (*func)(char *progname, struct cmdline *options);
};

static const struct command command_table[] = {
  { "read",  cmd_read },
  { "dump",  cmd_dump },
  { "write", cmd_write },
  { "test",  cmd_test },
  { NULL, NULL }
};

static void print_help(char *progname)
{
  printf("%s [options] command\n", progname);
  printf("\n");
  printf("options:\n");
  printf("  -h                        show this help\n");
  printf("  -port PORT                set serial port (default: %s)\n", DEFAULT_PORT);
  printf("  -speed SPEED              set serial speed (default: %d)\n", DEFAULT_SPEED);
  printf("\n");
  printf("commands:\n");
  printf("  dump  [ADDR [LEN]]        show EEPROM data\n");
  printf("  read  FILE [ADDR [LEN]]   copy EEPROM to file\n");
  printf("  write FILE [ADDR]         copy file to EEPROM\n");
  printf("\n");
  printf("Source code available at: https://github.com/moefh/eeprom_writer\n");
}

static int parse_cmdline(int argc, char *argv[], struct cmdline *opt)
{
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i] + 1, "speed") == 0) {
        if (argv[i+1] == NULL) {
          printf("%s: '-speed' requires an argument\n", argv[0]);
          return -1;
        }
        long n = parse_number(argv[i+1]);
        if (n < 0 || n > INT_MAX) {
          printf("%s: invalid speed: '%s'\n", argv[0], argv[i+1]);
          return -1;
        }
        opt->speed = (int) n;
        i++;
      } else if (strcmp(argv[i] + 1, "port") == 0) {
        if (argv[i+1] == NULL) {
          printf("%s: '-port' requires an argument\n", argv[0]);
          return -1;
        }
        opt->port = argv[i+1];
        i++;
      } else if (strcmp(argv[i] + 1, "h") == 0) {
        print_help(argv[0]);
        return -1;
      } else {
        printf("%s: invalid option: '%s'\n", argv[0], argv[i]);
        return -1;
      }
    } else {
      opt->command = &argv[i];
      return 0;
    }
  }
  print_help(argv[0]);
  return -1;
}

static const struct command *get_command(const struct command *table, const char *name)
{
  for (int i = 0; table[i].cmd != NULL; i++) {
    if (strcmp(name, table[i].cmd) == 0) {
      return &table[i];
    }
  }
  return NULL;
}

static void run_command(char *progname, struct cmdline *options)
{
  const struct command *cmd = get_command(command_table, options->command[0]);
  if (cmd == NULL) {
    printf("%s: invalid command '%s'\n", progname, options->command[0]);
    exit(1);
  }

  cmd->func(progname, options);
}

int main(int argc, char *argv[])
{
  struct cmdline options = {
    .port  = DEFAULT_PORT,
    .speed = DEFAULT_SPEED,
  };

  if (parse_cmdline(argc, argv, &options) != 0) {
    exit(1);
  }

  run_command(argv[0], &options);
  
  return 0;
}
