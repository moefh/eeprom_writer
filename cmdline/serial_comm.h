/* serial_comm.h */

#ifndef SERIAL_COMM_H_FILE
#define SERIAL_COMM_H_FILE

#include <stddef.h>

size_t serial_read_line(int fd, void *data, size_t data_size);
size_t serial_write_all(int fd, void *data, size_t data_size);

#endif /* SERIAL_COMM_H_FILE */
