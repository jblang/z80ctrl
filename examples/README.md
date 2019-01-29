# z80ctrl Example Programs

These are some example programs that demonstrate features of the z80ctrl and the I/O expander add-on card.

## Assembling

I have tried to stick to a common subset of assembler features that will work with any of the following cross-assemblers:

- [Sjasm](http://www.xl2s.tk/): Windows binaries available for download.  Compiles on Linux using sources on [Github](https://github.com/Konamiman/Sjasm).
- [SjasmPlus](https://sourceforge.net/projects/sjasmplus/):  Windows binaries available for download.  Compiles on Linux using sources on [Github](https://github.com/sjasmplus/sjasmplus).
- [z80asm](https://www.nongnu.org/z80asm/): Binaries available in the Debian repository. Mac binaries on [Homebrew](https://brew.sh/). Compiles on Windows.
- [z88dk](https://github.com/z88dk/z88dk)'s z80asm (not to be confused with above). Windows and Mac binaries available for download. Compiles on Linux.
- [pasmo](http://pasmo.speccy.org/): Windows and Mac binaries available for download. Binaries available in the Debain repository.

My preferred assembler is sjasm. I try to maintain compatibility with all of the above, but may not always test every change against the others.  If you have trouble assembling with any of the above, please try sjasm, and file a bug to let me know about the problem.

## About the Programs

- `graphclk.bas`: MSX-BASIC program draws a graphical clock on TMS9918A with time read from RTC.
- `hello.asm`: Simple hello world assembly program that uses the Altair UART emulation.
- `ioxblink.bas` MBASIC program that blinks an LED using the I/O expander.
- `lcd.bas`: MBASIC program that prompts for a string and prints it on an LCD hooked up to the I/O expander.
- `lcdclock.asm`: Assembly program that reads date/time from RTC and displays it on an LCD hooked up to the I/O expander.
- `rtc.bas`: MBASIC program that reads the date/time from RTC and prints it on the console.
- `z80ops.asm`: Assembly program with all Z80 opcodes, for testing disassembler.