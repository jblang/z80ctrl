## z80ctrl with AVR128DB64

This is a experimental design for a new z80ctrl using the [AVR128DB64](https://www.microchip.com/en-us/product/avr128db64) microcontroller. 

**Important: This board has not been tested and is not yet complete.  Manufacture at your own risk.**

This part has several advantages over the current design using an ATmega1284P and I/O expander:

- Is a 64-pin part with more than enough I/O to interface with the RC2014 bus without any I/O expander. 
- Has configurable logic used to assert the wait signal on I/O requests without having to use an external NAND gate.
- Has multi-voltage I/O to interface the SD card directly without any level shifters.

The disadvantage is that it's a fine-pitched surface mount part, but since JLCPCB and PCBWay now offer board assembly service, it should be possible to get a fully assembled board from them.

### RAM

This board combines features of the previous through-hole z80ctrl board plus the RAM and RTC from the companion CPU/RAM/RTC board. The CPU is still on a separate board so that z80ctrl can be used with other CPUs.

The RAM uses the same bank switching scheme as the CPU/RAM/RTC board, with two 32K windows into 512K of RAM. The banks for the upper and lower window are stored in the upper and lower nybbles of a 74HCT574, respectively.  A 74HCT257 multiplexer selects between the two windows based on state of A15.  Bank switching on this board can be disabled via the slide switch, in which case A15 is connected directly to the SRAM and the output of the 74HCT257 is disabled.  A16-A18 are connected directly to the bus, so with bank switching disabled, a CPU with more address lines can be used. The AVR is connected to A16-A18 as well so it can work with all 512KB of RAM in either mode.

### RTC

The RTC used on the board is a [Microchip MCP2940N](https://www.microchip.com/en-us/product/mcp7940n) I2C RTC. The bus can be shared with additional I2C peripherals assuming they do not use the same address as the RTC.  

### Serial Buses

The 3-volt SPI, I2C, and UART signals are exposed on the [UEXT header](https://www.olimex.com/Products/Modules/UEXT/). The CS pin on the UEXT header is shared with the SD card, so the SD card must be removed if connecting an SPI device to the UEXT header.  Alternatively, if UART functionality is not needed, the 3-volt RX and TX pins can be used as alternate chip selects.

The 5V level-shifted I2C bus can optionally be exposed on USER6 and USER7 pins of the RC2014 bus (the same pins used for I2C by the SC126). Connecting and disconnecting the signals from the bus is controlled by a dip switch, which also allows connecting or disconnecting the RX, TX, RX2, and TX2 lines to the RC2014 bus. Unlike previous versions of z80ctrl, the SPI bus is no longer exposed on the RC2014 user pins.