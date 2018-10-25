# IO Expander Board

This board adds two additional MCP23S17 I/O expanders to the SPI bus that z80ctrl exports on the user pins of the RC2014 backplane.  This provides 4 additional 8-bit GPIO ports that can be made available from z80ctrl to the Z80.  

All I/O expanders share the same IOXCS chip select line and are assigned an address which can be configured using 3 jumpers next to each I/O expander. By assigning different addresses to the I/O expanders, it should be possible to add multiple I/O expander boards to the RC2014 bus, in case 4 GPIO ports isn't enough for some reason.

Pins GPB4-7 on the first I/O expander have been ORed with the AUXCS2 line. This provides up to 4 additional SPI chip selects, which are exported along with SCK, MISO, and MOSI on 4 additional SPI headers. When you set one of the 4 CS lines low on the I/O expander, and then switch the AVR's chip select back to AUXCS2, the corresponding OR gate will go low, and the AVR can now talk to whatever SPI device you have connected to the SPI port.  When the AVR is done, it switches the chip select back to IOXCS, which cause the OR gate to set all 4 extended CS lines high so the AVR can tell the IO expander to raise the CS line without interference from the device that was previously selected.

My plan is to add a SPI RTC to this board as well, but I haven't finished that yet.  The RTC will be tied to the AUXCS1 line, so it can be addressed directly from the AVR without having to use any of the IO expander's pins for chip selects.
The z80ctrl board will be able to use the RTC to set the dates on files in its FAT filesystem, as well as make the time aviailable to the Z80.
