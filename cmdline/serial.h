/* serial.h */

#ifndef SERIAL_H_FILE
#define SERIAL_H_FILE

#define SERIAL_ERR_BAD_SPEED     (-1)
#define SERIAL_ERR_OPEN_DEVICE   (-2)
#define SERIAL_ERR_SETUP_DEVICE  (-3)

int serial_open(const char *device, int speed);
int serial_close(int fd);
const char *serial_get_error_message(int err);

#endif /* SERIAL_H_FILE */
