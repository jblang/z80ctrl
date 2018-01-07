# z80ctrl

This is an AVR-based bootloader and I/O firmware for a Z80 single-board computer.  The idea is to have the AVR load the SRAM with the ROM image from an SD card for the Z80 to run. The AVR will also provide debugging facilities, serial I/O, disk emulation, and potentially other peripherals for the Z80.  [Several](https://hackaday.io/project/7354-zaviour-board-avrz80-hybrid) [other](http://benryves.com/journal/3662496) people have had the idea to build a similar SBC before me. I took inspiration from these projects but I designed this specific circuit and software implementation myself without using any code from other projects.

Currently the computer consists of:

- Zilog Z84C010PEG 10MHz Z80 CPU
- Alliance AS6C4008 512KB SRAM chip
- Atmel ATmega1284p AVR microcontroller
- Microchip MCP23S18 SPI I/O expander
- 74HCT74 dual D-type flip-flop
- Pololu ls01a 4 channel level shifter
- Pololu sdc01a microSD breakout
- 20MHz crystal with 20 pf caps for AVR
- Silicon Labs CP2102 USB-to-TTL serial breakout
- LM1117T 3.3V regulator
- Various bypass and filter caps
- The prototype has been built on two full-size breadboards using dupont wire.

## Connections

I plan to eventually provide a schematic for the SBC, but for now, here is a description of how everything is connected, and the rationale.

The ATmega1284p doesn't have enough I/O to interface with all of the Z80's bus, so the I/O expander provides additional I/O.  I have tried to design the bus layout so that the most freqently-changed I/O lines connect directly on the AVR, and those less-frequently changed are on the I/O expander.  The AVR has direct connections for the LSB of the address bus, the data bus, and IO and the memory control lines (MREQ, IORQ, RD, and WR).  The AVR provides a clock to the Z80 using either Fast PWM up to 10MHz, or using a slower bit banging method with debug info logged for each clock cycle.

The MSB of the address bus the 3-bit bank address for the 512K SRAM and the remaining control lines (INT, NMI, RESET, BUSRQ, and BUSACK) are on the I/O expander. This arrangement allows the AVR to do faster DMA than putting the entire address bus on the I/O expander because the AVR only has to use the slower SPI interface to change the MSB of the address once per page of memory instead of for every byte. 

With the serial rx/tx lines, Z80 bus interface, SPI, and chip selects for the I/O expander, SD card, the AVR has 3 I/O pins left.  These can optionally be attached to the M1, RFSH, and HALT signals from the CPU, but this is not required and the signals are only used for debugging purposes.  In the future, they will more likely be used for chip selects for additional peripherals.

The specific ports and pins used for each I/O function are defined in `defines.h`.  `bus.c` provides functions to set addresses and data values and toggle the various control lines. This abstracts away the low-level details required to control these pins, and allows the client code to be written solely in terms of Z80 I/O lines (e.g., rd_hi, rd_lo, get_wr, busrq_lo, get_busack).  Convenience functions to intialize the bus, enter and exit bus-master mode, and do DMA transfers are provided.

Because the AVR cannot respond to an interrupt quickly enough to satisfy the timing demands of the Z80, the IORQ line also sets a flip-flop that will assert the WAIT line on the Z80 to add wait states until the AVR can respond.  Once the AVR has handled the interrupt, it clears the flip-flop to allow the Z80 to continue exeuction.

## Current Status

I have successfully run a simple Z80 program that adds two numbers together and then enters an infinite loop.  The state of the address, data, and control buses are logged with each clock cycle, allowing the execution of the program to be traced. The next step will be to implement interrupts to handle I/O requests. Following that, I will implement bootloading and drive emulation from the SD Card.  The end goal is to get CP/M to run.

## License

Copyright 2018 J.B. Langston

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
