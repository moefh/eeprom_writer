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

Sends the contents of the EEPROM to the standard output formatted as traditional hexdump output.

    ./eeprom dump              # the whole EEPROM
    ./eeprom dump 0 64         # 64 bytes starting at address 0
    ./eeprom dump 0x200 0x80   # 128 bytes (0x80) starting at address 512 (0x200)


#### Read EEPROM contents to file

Reads the contents of the EEPROM and writes the data to a file.

    ./eeprom read file.bin           # the whole EEPROM
    ./eeprom read file.bin 0 64      # the first 64 bytes
    ./eeprom read file.bin 100 0x20  # 32 bytes (0x20) starting at address 100


#### Write file to EEPROM

Writes the whole contents of a file to the EEPROM.

    ./eeprom write file.bin          # writes to start of the EEPROM
    ./eeprom write file.bin 0x400    # writes to the address 1024 (0x400)


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
