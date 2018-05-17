# z80ctrl

![z80ctrl photo](board.jpg)

z80ctrl is an AVR-based bootloader, debugger, and I/O device for a Z80 retrocomputer with the following features:

- Runs CP/M software including WordStar, Zork, Ladder, Turbo Pascal and more!
- Plug-in module for [RC2014](https://rc2014.co.uk/) retrocomputer replaces the clock, serial, ROM, and CF boards
- Reads HEX files and disk images from a FAT32-formatted SD Card
- Provides Altair and RC2014-compatible serial port
- Compatible with unmodified Altair disk images from the SIMH emulator project
- Monitor with memory inspector and built-in Z80 disassembler
- Single-step debugging, breakpoints, and watchpoints to trace Z80 bus activity in real-time
- Flexible MIT-licensed firmware can be the basis of your own SBC or breadboard computer
- Expandable SPI bus allows bridging other SPI peripherals to the Z80

I have made a [series of videos](https://www.youtube.com/watch?v=M8EIAxMpA-o&list=PLwP_dSEH4Oi_jAljWQ1H-TBiBL34LqYYm) demonstrating the design and features of the system.

## Software

The AVR firmware contains a built-in monitor program to allow control of the Z80.  When the AVR starts up, it presents a `z80ctrl>` prompt. The following commands are provided:

- Dump, disassemble, or fill memory (`dump`, `disasm`, `fill`)
- List files on the SD card (`dir`)
- Load and save Intel HEX files (`loadhex`, `savehex`)
- Mount, unmount, and boot from disk images (`mount`, `unmount`, `sboot`, `dboot`)
- Set breakpoints and watchpoints, and debug a program (`break`, `watch`, `debug`, `step`)
- Start the Z80 running from any address (`run`)
- Clear the screen (`cls`)
- List available commands (`help`)

Watch this [YouTube video](https://www.youtube.com/watch?v=5hJ0k5ZuQRE) for a demo of the monitor software.

## Hardware

I have created a [schematic](hardware/z80ctrl.pdf) and PCB design for the RC2014 plug-in module in KiCad. All the design files are located in the `hardware` subdirectory. You can order PCBs directly on [OSH Park](https://oshpark.com/shared_projects/Bn3c0pEt). If you want to manufacture them elsewhere, you will need to generate Gerber files from the KiCad project.

My board will replace the clock, serial, rom, and CF boards, so that all you need for a fully functional computer is my module plus the Backplane Pro, Z80 CPU board, and 64K RAM board.

Watch this [YouTube video](https://www.youtube.com/watch?v=_iZxZO_yfaE) for a tour of the PCB.

## Documentation

Much of the information that was previously here has been moved to separate wiki pages:

- [Compatible Z80 Software](https://github.com/jblang/z80ctrl/wiki/Compatible-Z80-Software)
- [Known Hardware Issues](https://github.com/jblang/z80ctrl/wiki/Known-Hardware-Issues)
- [Planned Features](https://github.com/jblang/z80ctrl/wiki/Planned-Features)
- [Hardware Design Notes](https://github.com/jblang/z80ctrl/wiki/Hardware-Design-Notes)
- [Software Design Notes](https://github.com/jblang/z80ctrl/wiki/Software-Design-Notes)
- [Breadboard Instructions](https://github.com/jblang/z80ctrl/wiki/Breadboard-Configuration)
- Check the [Wiki Index](https://github.com/jblang/z80ctrl/wiki) for other topics.


## Other Sites

Additional information available elsewhere:

- [Hackaday Project Page](https://hackaday.io/project/158338-z80ctrl)
- [YouTube Playlist](https://www.youtube.com/watch?v=M8EIAxMpA-o&list=PLwP_dSEH4Oi_jAljWQ1H-TBiBL34LqYYm)
- [OSH Park Shared Project (REV2)](https://oshpark.com/shared_projects/Bn3c0pEt)
- [Bill of Materials](https://docs.google.com/viewer?a=v&pid=forums&srcid=MDg4ODQwMTE2ODM0ODg2OTc5MTYBMDYxMjQ0OTAwNDMxMzE1MTEyNTYBd3dvUkxweVlBd0FKATAuMQEBdjI&authuser=0)
- [Current Google Groups Thread](https://groups.google.com/forum/#!topic/rc2014-z80/yc8g_ugiseo) (use this thread or the Hackaday Project Page to ask for help)
- [Older Google Groups Thread](https://groups.google.com/forum/#!searchin/rc2014-z80/z80ctrl%7Csort:date/rc2014-z80/wi8MCKlJdlo/xWQDF-IaAgAJ) (please reply to the current thread instead)
- [Even Older Google Groups Thread](https://groups.google.com/forum/#!searchin/rc2014-z80/z80ctrl%7Csort:date/rc2014-z80/EukbCJAxxIY/uYzqrVTcAgAJ)


## License

Copyright 2018 J.B. Langston

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
