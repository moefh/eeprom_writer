# EEPROM Writer

A simple 28C16 EEPROM writer for Arduino Nano or Uno based on Ben Eater's [EEPROM programmer](https://github.com/beneater/eeprom-programmer).

This project includes:
- An arduino project that accepts commands from the serial USB to read or write the EEPROM
- A command line tool for Linux (might work on Mac?) written in C to read and write data files to the EEPROM

The circuit is heavily based on Ben Eater's [circuit](https://www.youtube.com/watch?v=K88pgWhEb1M) with a few changes to make it more robust when using an Arduino Uno. The main changes are:

- Don't use pin D13 for write enable (which is used by the Arduino Uno duuring boot). Instead we use the digital output of analog pins A0 to A2 to control the EEPROM chip enable, write enable and output enable pins.
- Add 10k resistors to pull down the shift register inputs do they don't float wildly while the Arduino is starting
- Add 10k resistors to pull up the enable pins of the EEPROM (chip enable, write enable and output enable) to make it more reliable (especially when arduino is starting up). 


## Circuit

<img src="/doc/schematic.png" width="400" alt="Schematic">


<img src="/doc/breadboard.jpg" width="400" alt="Breadboard Photo">


## Command Line Tool

The command line tool is written in C. To build it, just enter the `cmdline` directory and type `make` (make sure you have the necessary stuff installed, like `build-essential` if using Ubuntu).

The available commands are:

#### Show EEPROM contents

    eeprom dump [ADDRESS [LENGTH]]

Sends the contents of the EEPROM to the standard output formatted as traditional hexdump output. Examples:

    ./eeprom dump              # the whole EEPROM
    ./eeprom dump 0 64         # 64 bytes starting at address 0
    ./eeprom dump 0x200 0x80   # 128 bytes (0x80) starting at address 512 (0x200)


#### Read EEPROM contents to file

    eeprom read FILE [ADDRESS [LENGTH]]

Reads the contents of the EEPROM and writes the data to a file. Examples:

    ./eeprom read file.bin           # the whole EEPROM
    ./eeprom read file.bin 0 64      # the first 64 bytes
    ./eeprom read file.bin 100 0x20  # 32 bytes (0x20) starting at address 100


#### Write file to EEPROM

    eeprom read FILE [ADDRESS]

Writes the whole contents of a file to the EEPROM. Examples:

    ./eeprom write file.bin          # writes to start of the EEPROM
    ./eeprom write file.bin 0x400    # writes to the address 1024 (0x400)


## Arduino Project

The Arduino program (found in `arduino/eeprom_writer`) waits for commands from the serial and executes them. The prompt

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
