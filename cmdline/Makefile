
CC = gcc
CFLAGS = -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS =

OBJS = eeprom.o serial.o serial_comm.o util.o cmd_read.o cmd_dump.o cmd_write.o cmd_test.o
LIBS =

.PHONY: all clean

all: eeprom

clean:
	rm -f *.o *~ eeprom

eeprom: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
