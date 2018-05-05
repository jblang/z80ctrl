# z80ctrl

This is an AVR-based bootloader and I/O firmware for a Z80-based computer.  I have made a [series of videos](https://www.youtube.com/watch?v=M8EIAxMpA-o&list=PLwP_dSEH4Oi_jAljWQ1H-TBiBL34LqYYm) demonstrating the design and features of the system.  In brief, the AVR loads an SRAM with code for the Z80 to run. Once the Z80 is running, AVR also provides serial I/O, disk emulation, debugging facilities, and potentially other peripherals for the Z80.

[Several](http://benryves.com/journal/3662496) [other](https://hackaday.io/project/7354-zaviour-board-avrz80-hybrid) [people](https://www.apress.com/us/book/9781484214268) [had](https://hackaday.io/project/19000-a-4-4ics-z80-homemade-computer-on-breadboard) the idea to combine an AVR with a Z80 before me, but these projects either didn't share any code or didn't provide all the capabilities that I wanted. I have taken inspiration from these projects but I designed this specific circuit and software implementation myself. The innovations not present in previous projects are described in the design notes section below.

## Hardware

The first version of this computer was designed on a breadboard. The [first video](https://www.youtube.com/watch?v=M8EIAxMpA-o) on YouTube explains the design of the breadboarded circuit. I have kept the bill of materials and table of pin-to-pin connections for the original breadboarded circuit in [breadboard.md](breadboard.md) for posterity.

Later I designed a plugin module for the [RC2014](https://rc2014.co.uk/) retrocomputer kit. My board will replace the clock, serial, rom, and CF boards, so that all you need for a fully functional computer is my module plus the Backplane Pro, Z80 CPU board, and 64K RAM board.

Besides the reduced part count, my implementation has a few other advantages over the stock RC2014. It can read disk images off a FAT32-formatted SD Card and has full compatiblity with unmodified Altair 8800 disk images and disk images from the SIMH emulator project. It also has single-step debugging, breakpoints, and watchpoints that will trace the execution of the Z80 in real-time.

I have created a [schematic](hardware/z80ctrl.pdf) and PCB design for the proposed plug-in module in KiCad. All the design files are located in the `hardware` subdirectory. These can be used to manufacture a PCB at suppliers like [OSH Park](https://oshpark.com/) or [Seeed Studio](https://www.seeedstudio.com/fusion_pcb.html).  The [third](https://www.youtube.com/watch?v=_iZxZO_yfaE) YouTube video covers the design of the REV1 hardware.

### Known Issues

After manufacturing the board, I discovered several problems:

- The position of the CTS and VCC pin are swapped on the serial connectors. This doesn't affect anything unless you try to power the board from the serial connector by jumpering JP1 or JP2. It can be fixed by crossing the CTS and VCC wires running from the FTDI adapter to the Serial connector on the board, or simply leaving JP1 and JP2 unpopulated and powering the board from an external supply via the RC2014 bus.
- The position of SPIA0 and SPIA1 on the 74HCT139N are swapped.  This has been fixed in software by swapping the definitions for SD_SEL and AUX1_SEL in defines.h.  The originally intended addresses will be used if `CORRECT_SPIADDR` is defined; otherwise the addresses will be swapped so they match those on the REV1 board.
- The DTR trace near the edge of the board runs too close to the Serial 1 pads. On some boards, the solder mask near the ground pin does not cover the DTR trace, so a solder bridge can short them together. This can prevent the AVR from automatically resetting when programmed by avrdude and can damage the serial to USB adapter.  This issue occurred on two of the three OSH Park-manufactured boards that I received. Other boards built by Seeed Studio using the same design files did not exhibit this problem.  The problem can be fixed by cutting the Serial 1 DTR trace on either side of the GND pad and soldering a bodge wire from the DTR pad of the Serial 1 connector to the DTR pad of C7.
- Mislabeled and mismatched values are indicated in the schematic and silkscreen for C2 and C3. They should both be 18 pf for the crystal I used, or whatever is specified for your crystal.  However, the value is not critical, and I have found that it also works with 20 pf capacitors.
- The original layout of the SCK track is not optimal although it does not cause an issue at the 10MHz top speed of the SPI bus. SCK runs to the SD card first and then back to the IO expander, whereas MISO and MOSI run to the IO expander and then to the SD card. At higher frequencies, the differences in length could cause signal skew and interfere with the operation of SPI.

These defects have been corrected in the updated KiCad design files as of May 2018, but the new design is untested. If you have either positive or negative experiences using the new design files, please let me know.  The original design is available under the REV1 tag and all known defects can be worked around using the techniques described above.

### Design Notes

The ATmega1284p doesn't have enough I/O to interface with all of the Z80's bus lines, so I used an I/O expander to provide two additional 8-bit I/O ports.  I have tried to optimize performance by connecting the most frequently-changed signals directly to the AVR, and connecting those less-frequently changed to the I/O expander.  The AVR has direct connections for the LSB of the address bus, the data bus, and following control lines: MREQ, IORQ, RD, WR, M1 and HALT.  

The MSB of the address bus, the 3-bit bank address for the 512K SRAM, and the remaining control lines--INT, NMI, RESET, BUSRQ, and BUSACK--are on the I/O expander. This arrangement allows the AVR to do faster DMA than would be possible if the entire address bus were on the I/O expander. With just the upper 8 bits of the address on the expander, the AVR only has to use the slower SPI interface to change the MSB of the address bus once every 256 bytes of memory instead of for every byte. 

The AVR produces a 10MHz clock signal for the Z80 using hardware PWM. However, care must be taken to avoid bus contention when switching the bus directions.  Therefore, the AVR brings the clock under software control to perform cycle-exact timing when responding to I/O requests. Software control of the clock also allows the bus state to be traced for each clock cycle if desired.  Under software control, the clock can achieve a maximum frequency of about 4MHz. When tracing information is being logged, the attainable clock rate slows down to several hundred KHz.

Because the AVR cannot respond to an I/O request quickly enough to satisfy the timing demands of the Z80, the IORQ line also sets a flip-flop that asserts the WAIT line on the Z80 to add wait states until the AVR can respond.  The AVR polls the IORQ line in a tight loop and services the request when IORQ goes low. When the AVR has finished servicing the I/O request, it brings the reset line on the flip-flop low to deassert the WAIT line on the Z80.  It then manually clocks the Z80 until the IORQ line goes high. At this point, it ensures that its data port is in input mode, releases the reset line on the flip flop, and restarts the PWM clock, allowing the Z80 to continue execution at full speed.

The HALT input on the AVR is connected to the Z80 through a diode so that it can also be grounded using a push button without shorting the halt pin on the Z80.  Thus, it is possible to return to the AVR-based monitor system automatically when the Z80 executes a halt instruction, or manually by pressing the halt button.  PB6 has the manual pullup resistor enabled so that the halt signal is high (inactive) when neither the HALT signal is asserted by the Z80 nor the button is pressed.

PB0 on the AVR is used as an output to flash an LED by the bootloader, so it cannot safely be connected to an output signal on the Z80. In the current design, it is connected to the IOACK line on the 74HCT139, so it can be safely used as an output during the boot process.

I have added a 74HCT139 decoder to allow the AVR to be assigned to none, all, or a selectable block of 64 I/O ports ($00-$3F, $40-$7F, $80-$BF and $C0-$FF). The flip flop will only trigger a wait state if the I/O address is within the selected range. This will allow the use of other peripherals alongside the AVR.  I'm using half of a 74HC139 dual 2-to-4 decoder to do this, and the other half is used as an SPI chip select multiplexer to allow the AVR to control up to 4 SPI peripherals with just 2 I/O pins.  The SPI signals are exported to the user pins on the RC2014 bus.  The RC2014 bus lines USER1-6 are connected to SCK, MISO, MOSI, IO expander chip select, and 2 unassigned chip select lines, respectively.  The traces to these pads can be cut or the pins can be omitted if this is not desired. The MCP23S17 I/O expander has a 3-bit serial addressing scheme so it should be possible to add up to 7 additional I/O expanders to the bus with only a single chip select line shared between all 8 I/O expanders.  The I/O expander on the z80ctrl board is hardcoded to address 0.

## Software

Watch the [second](https://www.youtube.com/watch?v=5hJ0k5ZuQRE) YouTube video linked above for a demo of the monitor software.

### AVR

The AVR contains a built-in monitor program to allow control of the Z80.  When the AVR starts up, it presents a `z80ctrl>` prompt. Typing `help` at the prompt will list all available commands. Commands are provided to dump memory, fill memory, load and save Intel HEX files, mount and unmount disk images, and start the Z80 running from any address.

The layout of the source code is as follows:

- `defines.h` contains the specific ports and pins used for the Z80 bus interface and SPI chip selects.
- `bus.h` provides macros to set the address and data bus and read or toggle the various control lines. This abstracts control of these signals away from the specific hardware implementation. It should be possible, for example, to port this code to an ATmega2560 without needing an IO expander, simply by modifying `bus.h` and `defines.h`.
- `bus.c` provides functions to intialize the bus, control the clock, and enter and exit bus-master mode.
- `memory.c` contains functions to do DMA transfers, and various convenience functions to dump, fill, and verify blocks of memory.
- `spi.c` and `iox.c` provide functions for the low-level control of the SPI bus in general and the IO Expander specifically.
- `ff.c`, `ffsystem.c`, `ffunicode.c`, and `mmc_avr_spi.c` are from the FatFS project and provide access via SPI to the FAT32 filesystem contained on the microSD card.
- `z80.c` has functions run and debug the Z80 and handle I/O requests from it. 
- `diskemu.c` emulates an Altair 88-DISK controller to allow running CP/M and other disk-based programs.
- `ihex.c` contains functions to read and write Intel HEX files.
- `cli.c` contains the command-line monitor interface for controlling the Z80 and loading memory via the AVR.  
- The `uart.c` and `uart.h` files borrowed from the avr-libc [stdio demo](http://www.nongnu.org/avr-libc/user-manual/group__stdiodemo.html) allow the use of the C stdio library over serial.
- `z80ctrl.c` contains the main function that starts initializes the bus and starts the command-line monitor interface. 

The AVR is programmed with the [MightyCore](https://github.com/MCUdude/MightyCore) bootloader to allow it to be reprogrammed through the serial interface.  Although I am using the MightyCore bootloader, I do not use the Arduino libraries; only avr-libc. I use the [fatfs](http://elm-chan.org/fsw/ff/00index_e.html) library to provide access to a FAT32-formatted SD-card containing ROM and disk images for the Z80.

### Z80

The current version of this software is able to run many unmodified 8080 and Z80 programs and disk images from the following sources:

- [Altair Clone](http://altairclone.com) project's [download](http://altairclone.com/downloads/) site.  The disk images here should be booted using the standard Altair Disk Bootloader via the `dboot` monitor command.
- [SIMH AltairZ80](https://schorn.ch/altair.html) emulator project. These disk images should be booted using the SIMH bootloader via the `sboot` monitor command.
- [Z80pack](http://www.autometer.de/unix4fun/z80pack/) hex files and disk images for the Altair 8800 and IMSAI 8080.
- Note that Altair Basic will not run on the Z80. This is due to subtle incompatibilites between the 8080 and Z80 status registers and is not anything specific to my project.
- For cross-assembling your own programs, I suggest using [z80asm](http://www.nongnu.org/z80asm/) for Zilog mnemonics, or [asm8080](http://asm8080.sourceforge.net/) for Intel mnemonics. After assembling the programs, a hex file can be generated using the `objcopy` command from GNU binutils.  For the built-in `altmon`, `sboot`, and `dboot` monitor commands, I used `xxd -i` to generate C header files with an array containing the machine code.

### Planned Features

Here are some items on my TODO list, just in the order they occurred to me, not necessarily the order I'll work on them:

- Improve Z80 debugger to decode assembly operands and output them in opfetch watch mode (initial version checked in, but may have some bugs).
- Improve Z80 debugger to allow single stepping of individual instructions rather requiring multiple and varying steps per instruction due to the differing number of clock cycles for different instructions.
- Monitor command to control Z80 clock rate.
- Monitor command to load and save binary files to and from memory.
- Monitor command to disassemble data in memory.
- autoexec.bat file to be run by monitor at startup.
- Disk convenience commands for the monitor (erase files, create blank disk images, change directory, etc.)
- Implement DMA for disk emulation so that the AVR will copy an entire sector of data to memory at a time instead of one byte at a time. This should speed up the disk performance dramatically.  It will also require a custom CP/M BIOS so it will be a considerable effort.  
- Implement pass-through timer interrupts for the Z80. This will allow more accurate timing e.g., for playing music on Ed Brindley's sound board.  Due to the INT pin being on the IO expander, the minimum timing resolution will be limited, but sub-millisecond resolution should be doable. Microsecond resolution is probably not.
- Implement interrupt-driven serial with a ring buffer and software flow control (XON/XOFF).  The AVR doesn't support hardware flow control, and currently it's easy to overrun the buffer by sending data too fast (e.g., via copy/paste or XMODEM).
- Implement a second serial port so that debugging can be on a separate console from Z80 program output.
- Possibly implement PS/2 keyboard interface on Serial 2 header (mutually exclusive with above, configured via compile-time or run-time configuration).
- Fix bug that causes monitor CLI to go into an infinite loop spewing z80ctrl> repeatedly when the HALT button is pressed while the Z80 is outputting something.
- Add other SPI peripherals and implement I/O port interfaces to them for the Z80. Examples: real-time clock, GPIO, 7-segment display, LCD display, etc.
- Possibly emulate other disk formats or peripherals.
- Investigate RomWBW support.

 I would welcome suggestions for additional features (although I don't promise I will implement them myself).  I would also be thrilled with pull requests, but if you're interested in working on anything, let's coordinate so we don't duplicate efforts.

## License

Copyright 2018 J.B. Langston

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
