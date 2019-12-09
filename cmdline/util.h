/* util.h */

#ifndef UTIL_H_FILE
#define UTIL_H_FILE

#define READ_FILE_ERR_OPEN   (-1)
#define READ_FILE_ERR_SIZE   (-2)
#define READ_FILE_ERR_MEM    (-3)
#define READ_FILE_ERR_READ   (-4)

struct eeprom_range {
  unsigned int addr;
  unsigned int len;
};

struct file_data {
  char *data;
  size_t size;
};

long parse_number(const char *str);
int parse_eeprom_address(struct eeprom_range *range, char **options);
int parse_eeprom_range(struct eeprom_range *out, char **options);
int read_file(struct file_data *data, const char *filename);
const char *get_read_file_error_message(int code);

#endif /* UTIL_H_FILE */
