/* serial.c */

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>

#include "serial.h"

static const struct {
  int err;
  const char *msg;
} serial_errors[] = {
  { SERIAL_ERR_BAD_SPEED,    "invalid speed" },
  { SERIAL_ERR_OPEN_DEVICE,  "can't open device" },
  { SERIAL_ERR_SETUP_DEVICE, "can't setup device" },
  { 0, NULL }
};

static int serial_setup(int fd, speed_t speed_setting)
{
  struct termios tty;
  if (tcgetattr(fd, &tty) != 0) {
    return -1;
  }

  cfsetispeed(&tty, speed_setting);
  cfsetospeed(&tty, speed_setting);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;       // set data bytes to 8
  tty.c_cflag &= ~(PARENB|PARODD);                  // no parity
  tty.c_cflag &= ~CSTOPB;                           // 1 stop bit
  
  tty.c_cflag &= ~CRTSCTS;                          // no hardware flow control
  tty.c_cflag |= (CREAD | CLOCAL);                  // enable receiver, ignore modem control lines

  tty.c_iflag &= ~IGNBRK;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON/XOFF flow control
  tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);   // noncanonical mode
  tty.c_iflag &= ~(ICRNL | INLCR | IGNCR);          // don't mess with cr/nl
  
  tty.c_oflag = 0;                                  // don't mess with output
  tty.c_lflag = 0;
  
  tty.c_cc[VMIN] = 1;                               // read() blocks until at least 1 byte is available
  tty.c_cc[VTIME] = 0;                              // read() blocks forever
  
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    return -1;
  }
  return 0;
}

static int get_speed(int req_speed, speed_t *ret_speed)
{
  switch (req_speed) {
  case 9600:   *ret_speed = B9600;   return 0;
  case 19200:  *ret_speed = B19200;  return 0;
  case 38400:  *ret_speed = B38400;  return 0;
  case 57600:  *ret_speed = B57600;  return 0;
  case 115200: *ret_speed = B115200; return 0;
  case 230400: *ret_speed = B230400; return 0;
  }
  return -1;
}

int serial_open(const char *device, int speed)
{
  speed_t speed_setting;
  if (get_speed(speed, &speed_setting) != 0) {
    return SERIAL_ERR_BAD_SPEED;
  }
  
  int fd = open(device, O_RDWR|O_NOCTTY|O_SYNC);
  if (fd == -1) {
    return SERIAL_ERR_OPEN_DEVICE;
  }

  if (serial_setup(fd, speed_setting) != 0) {
    close(fd);
    return SERIAL_ERR_SETUP_DEVICE;
  }
  tcflush(fd, TCIOFLUSH);

  return fd;
}

int serial_close(int fd)
{
  return close(fd);
}

const char *serial_get_error_message(int err)
{
  for (int i = 0; serial_errors[i].err != 0; i++) {
    if (serial_errors[i].err == err) {
      return serial_errors[i].msg;
    }
  }
  return NULL;
}
