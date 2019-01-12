# z80ctrl

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

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/sVV1PYWX18A/0.jpg)](https://www.youtube.com/watch?v=sVV1PYWX18A)

## Documentation

Much of the information that was previously here has been moved to separate wiki pages:

- [Board Assembly Instructions](https://github.com/jblang/z80ctrl/wiki/Board-Assembly-Instructions)
- [Monitor User Guide](https://github.com/jblang/z80ctrl/wiki/Monitor-User-Guide)
- [Compatible Z80 Software](https://github.com/jblang/z80ctrl/wiki/Compatible-Z80-Software)
- [Known Issues](https://github.com/jblang/z80ctrl/wiki/Known-Issues)
- [Planned Features](https://github.com/jblang/z80ctrl/wiki/Planned-Features)
- [Hardware Design Notes](https://github.com/jblang/z80ctrl/wiki/Hardware-Design-Notes)
- [Software Design Notes](https://github.com/jblang/z80ctrl/wiki/Software-Design-Notes)
- [IO Expander Board](https://github.com/jblang/z80ctrl/wiki/IO-Expander-Board)
- Check the [Wiki Index](https://github.com/jblang/z80ctrl/wiki) for other topics.


## Off-Site Resources

- [Hackaday Project Page](https://hackaday.io/project/158338-z80ctrl)
- [YouTube Playlist](https://www.youtube.com/watch?v=M8EIAxMpA-o&list=PLwP_dSEH4Oi_jAljWQ1H-TBiBL34LqYYm)
- [RC2014 Google Group](https://groups.google.com/forum/#!forum/rc2014-z80). Search for z80ctrl to find previous discussion threads.


## License

Copyright 2018 J.B. Langston

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
