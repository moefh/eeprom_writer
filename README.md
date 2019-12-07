# EEPROM Writer

A simple 28C16 EEPROM writer using Arduino Nano or Uno.

Features:

- Command line tool to read/write EEPROM contents to/from data files or view EEPROM contents
- Arduino Uno friendly (don't use pin 13 as output)

<img src="/doc/breadboard.jpg" width="400" alt="Breadboard Photo">

<img src="/doc/schematic.png" width="400" alt="Schematic">

## Command Line Tool

The command line tool is written in C. To build it, just enter the `cmdline` directory and type `make` (make sure you have the necessary stuff installed, like `build-essential` if using Ubuntu).

The available commands are:

#### Show EEPROM contents

Show the whole EEPROM:

    ./eeprom dump

Show the first 64 bytes:

    ./eeprom dump 0 64

Show 128 (0x80) bytes starting at address 0x200:

    ./eeprom dump 0x200 0x80


#### Read EEPROM contents to file

Read whole EEPROM to `file.bin`:

    ./eeprom read file.bin

Read the first 64 bytes of EEPROM to `file.bin`:

    ./eeprom read file.bin 0 64

Read 32 bytes of EEPROM starting from address 100 to `file.bin`:

    ./eeprom read file.bin 100 32


#### Write file to EEPROM

Write the contents of `file.bin` to the EEPROM starting at address 0:

    ./eeprom write file.bin

Write the contents of `file.bin` to the EEPROM starting at address 0x400 (1024):

    ./eeprom write file.bin 0x400


## Arduino Code

The Arduino code (found in `arduino/eeprom_writer`) waits for commands from the serial and executes them. The prompt

    *READY

is written to the serial port at the start and after every command is executed successfully. If a command fails, the prompt

    *ERROR: <message>

is written to the serial port.

#### Dump

    d ADDRESS LENGTH

Read data from `ADDRESS` to `ADDRESS+LENGTH` and send it to the serial it in a classic "hexdump" format. Both numbers must be in hexadecimal.

#### Read

    r ADDRESS LENGTH

Read data from `ADDRESS` to `ADDRESS+LENGTH` and send it to the serial it in a stream of hex digits with no formatting. Both numbers must be in hexadecimal.

#### Write

    w ADDRESS LENGTH

Reads `LENGTH` pairs of hex digits (each pair representing a byte) and writes the bytes to the EEPROM starting at address `ADDRESS`. Both numbers must be in hexadecimal. I the data is not sent in 5 seconds, a timeout error is written to the serial and no data is written.
