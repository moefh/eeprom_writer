/* util.c */

#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "eeprom.h"

const struct {
  int code;
  const char *msg;
} read_file_error_table[] = {
  { READ_FILE_ERR_OPEN, "can't open file" },
  { READ_FILE_ERR_SIZE, "can't get file size" },
  { READ_FILE_ERR_MEM,  "out of memory" },
  { READ_FILE_ERR_READ, "can't read file" },
  { 0, "unknown error" }
};

const char *get_read_file_error_message(int code)
{
  for (int i = 0; read_file_error_table[i].code != 0; i++) {
    if (read_file_error_table[i].code == code || read_file_error_table[i].code == 0) {
      return read_file_error_table[i].msg;
    }
  }
  return NULL; // shoudn't happen
}

long parse_number(const char *str)
{
  char *end;
  long n = strtol(str, &end, 0);
  if (end == str || *end != '\0') {
    return -1;
  }
  return n;
}

int parse_eeprom_address(struct eeprom_range *range, char **options)
{
  if (options[0] == NULL) {
    range->addr = 0;
  } else {
    long n = parse_number(options[0]);
    if (n < 0) return -1;
    range->addr = (unsigned int) n;
  }

  if (range->addr >= EEPROM_SIZE) {
    return -1;
  }
  return 0;
}

int parse_eeprom_range(struct eeprom_range *range, char **options)
{
  if (options[0] == NULL) {
    range->addr = 0;
    range->len = EEPROM_SIZE;
  } else {
    long n = parse_number(options[0]);
    if (n < 0) return -1;
    range->addr = (unsigned int) n;
    if (options[1] != NULL) {
      n = parse_number(options[1]);
      if (n < 0) return -1;
      range->len = (unsigned int) n;
    } else {
      range->len = range->addr - EEPROM_SIZE;
    }
  }

  if (range->addr >= EEPROM_SIZE || range->addr + range->len > EEPROM_SIZE) {
    return -1;
  }
  
  return 0;
}

int read_file(struct file_data *data, const char *filename)
{
  int ret = 0;
  char *buf = NULL;
  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    ret = READ_FILE_ERR_OPEN;
    goto err;
  }

  // get file size
  if (fseek(f, 0L, SEEK_END) < 0) {
    ret = READ_FILE_ERR_SIZE;
    goto err;
  }
  long file_size = ftell(f);
  if (file_size < 0) {
    ret = READ_FILE_ERR_SIZE;
    goto err;
  }
  if (fseek(f, 0L, SEEK_SET) < 0) {
    ret = READ_FILE_ERR_SIZE;
    goto err;
  }
  
  // read whole file
  buf = malloc((size_t) file_size);
  if (buf == NULL) {
    ret = READ_FILE_ERR_MEM;
    goto err;
  }
  if (fread(buf, 1, (size_t) file_size, f) != (size_t) file_size) {
    ret = READ_FILE_ERR_READ;
    goto err;
  }

  fclose(f);
  data->size = (size_t) file_size;
  data->data = buf;
  return 0;
  
err:
  free(buf);
  if (f) {
    fclose(f);
  }
  return ret;
}
