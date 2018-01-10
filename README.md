# z80ctrl

This is an AVR-based bootloader and I/O firmware for a Z80 single-board computer.  The idea is to have the AVR load the SRAM with the ROM image from an SD card for the Z80 to run. The AVR will also provide debugging facilities, serial I/O, disk emulation, and potentially other peripherals for the Z80.  [Several](https://hackaday.io/project/7354-zaviour-board-avrz80-hybrid) [other](http://benryves.com/journal/3662496) people have had the idea to build a similar SBC before me. I took inspiration from these projects but I designed this specific circuit and software implementation myself without using any code from other projects.


## Hardware

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

![breadboard](breadboard.jpg)

I plan to eventually provide a schematic for the SBC, but for now, here is a general description of how everything is connected, and the rationale for doing it this way.

The ATmega1284p doesn't have enough I/O to interface with all of the Z80's bus, so the I/O expander provides additional I/O.  I have tried to design the bus configuration so that the most freqently-changed lines connect directly on the AVR, and those less-frequently changed are on the I/O expander.  The AVR has direct connections for the LSB of the address bus, the data bus, and IO and the memory control lines (MREQ, IORQ, RD, and WR).  The AVR provides a clock to the Z80 using either Fast PWM up to 10MHz, or using a slower bit banging method with debug info logged for each clock cycle.

The MSB of the address bus, the 3-bit bank address for the 512K SRAM, and the remaining control lines (INT, NMI, RESET, BUSRQ, and BUSACK) are on the I/O expander. This arrangement allows the AVR to do faster DMA than would be possible if the entire address bus were on the I/O expander. With just the upper 8 bits of the address on the expander, the AVR only has to use the slower SPI interface to change the MSB of the address once every 256 bytes of memory instead of for every byte. 

With the serial rx/tx lines, the Z80 bus interface, SPI, and chip selects for the I/O expander and SD card, the AVR has 3 I/O pins left.  These can optionally be attached to the M1, RFSH, and HALT signals from the CPU, but this is not required and the signals are only used for debugging purposes.  In the future, I will likely use them for chip selects for additional peripherals.

Because the AVR cannot respond to an I/O request quickly enough to satisfy the timing demands of the Z80, the IORQ line also sets a flip-flop that will assert the WAIT line on the Z80 to add wait states until the AVR can respond.  Once the AVR has detected the IO request, it stops the 10MHz PWM-based clock and bit-bangs the clock until it has completed the cycle-exact timing required for the I/O operation.  When it's finished, it clears the flip-flop and restarts the fast clock to allow the Z80 to continue execution at full speed.

## Software

The specific ports and pins used for each function are defined in `defines.h`.  `bus.h` provides macros to set addresses and data values and toggle the various control lines. This abstracts away the low-level details required to control these pins, and allows the client code to be written solely in terms of Z80 I/O lines (e.g., RD_HI, RD_LO, GET_WR, BUSRQ_LO, GET_BUSACK, and so on). Convenience functions to intialize the bus, enter and exit bus-master mode, do DMA transfers, and trace the bus status during program exection are provided in `bus.c`.  `z80ctrl.c` contains the main function that loads the test program into memory, resets the processor, and starts it running. Eventually some of this logic should be moved into separate functions in `bus.c`.

As an early test, I successfully ran a simple "hello, world" program (`hello.asm`) that I wrote in Z80 assembly language. I implemented I/O ports for the UART that are compatible with the Altair 8800 2SIO board.  With this in place, I have gotten a simple monitor program (`turnmon.asm`) for the Altair 8800 to run unmodified. Altair 4K BASIC starts to load (prompts for memory, terminal width, etc.) but then hangs before issuing the ready prompt and I haven't figured out what the problem is yet. Following that, I will implement bootloading and drive emulation from the SD Card with the goal of getting CP/M to run.

I am cross-assembling the programs using `z80asm` for Zilog mnemonics, or `asm8080` for Intel mnemonics. After assembling the programs, I use `xxd -i` to generate a C header file with an array containing the machine code, include the header file into my main program, and use my memory transfer functions to copy them to the external SRAM.

![monitor](monitor.png)

## License

Copyright 2018 J.B. Langston

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
