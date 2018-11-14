# IO Expander Board

This board adds two additional MCP23S17 I/O expanders and a battery-backed real-time clock to the SPI bus that z80ctrl exports on the user pins of the RC2014 backplane.  This provides 4 additional 8-bit GPIO ports that can be made available from z80ctrl to the Z80.  

## Addressing

All I/O expanders share the same IOXCS chip select line and are assigned an address which can be configured using 3 jumpers next to each I/O expander (`JP1`-`JP3` for `U3` and `JP4`-`JP6` for `U2`). For each jumper, the upper position assigns a value of 0 and the lower position assigns a value of 1 for the corresponding address bit. An address of 0 should not be used for either I/O expander because it will cause contention on the SPI bus with the I/O expander on the main z80ctrl board. It should be possible to add up to 7 I/O expanders on multiple boards.

## Reset Line

REV4 of the z80ctrl board optionally exports the AVR reset line on pin D15 of the RC2014 bus, which is normally unused. This signal can be connected to the RESET pin of the I/O expanders by jumpering `J11` in the right position. This will reset the GPIO expanders whenever the AVR on the z80ctrl board is reset.  If the AVR reset line is not connected to D15, then the RESET pin of the I/O expanders should be tied to VCC by jumpering `J11` in the left position.

## GPIO Connectors

Four 8-bit GPIO connectors are provided. Two sets of VCC and GND pins are provided on either end of the connector. The pinout is as follows:

```
VCC 0 2 4 6 VCC
GND 1 3 5 7 GND
```

The top of the connector is the row further away from the PCB.

## SPI Connectors

Pins GPB4-7 on the first I/O expander have been ORed with the AUXCS2 line. This provides up to 4 additional SPI chip selects, which are exported along with SCK, MISO, and MOSI on 4 additional SPI headers. The chip selects are also passed through an inverter to provide active HIGH chip selects if required. The pinout of each connector is as follows:

```
CS /CS MOSI MISO SCK GND VCC
```

CS is the active HIGH chip select, and /CS is the active LOW chip select.

A chip select can be activated by setting one of the 4 CS lines (GPB4-GPB7) low on the I/O expander and then switching the AVR's chip select back to AUXCS2.  The AVR can now talk to the SPI device connected to the corresponding SPI port.  When the AVR is done, it switches the chip select back to IOXCS, which cause the OR gate to deassert all 4 extended CS lines.  This will allow the AVR communicate with the IO expander to raise the CS line without interference from the device that was previously selected.

## RTC

The board also includes a DS1306+ SPI RTC.  The RTC is connected to the AUXCS1 line, so it can be addressed directly from the AVR without having to use any of the IO expander's pins for chip selects.  The RTC is provided backup power from a CR1216 lithium coin cell battery.  The z80ctrl board will be able to use the RTC to set the dates on files in its FAT filesystem, as well as make the time available to the Z80.

## Bill of Materials

| Reference | Part |
|-|-|
| J1/J2 | 2x40 right angle pin header with extra pins removed |
| JP1-JP6, J11 | 1x3 straight pin header |
| | Jumper blocks for JP1-JP6 and J11 |
| J3-J4 | 1x7 right angle pin header |
| J7-J10 | 2x6 right angle pin header |
| U1 | 74HCT32 quad OR gate |
| U5 | 74HCT04 hex inverter |
| U2, U3 | MCP23S17 16-bit SPI I/O expanders | 
| U4 | DS1306+ Serial Real-Time Clock |
| Y1 | 32 kHz Crystal (Citizen CFS-20632768DZFB or similar) |
| BT1 | CR1216 battery holder (Keystone 3001 or similar) |

The configuration of this board is quite flexible and many components are optional:

- The RTC (U4) may be omitted, and if multiple boards are installed in your RC2014, only one of them should include an RTC to prevent conetnion.  If the RTC is omitted, BT1 and Y1 can also be omitted.  
- The inverter (U5) is required by the RTC, but if the RTC is omitted, the inverter may also be omitted unless active high CS outputs are desired for the SPI ports.
- The OR gate (U1) can be omitted if the SPI ports are not needed.
- Either or both of the I/O expanders (U2, U3) can be omitted. The address jumpers for the unused I/O expanders can also be omitted.  Note that the SPI ports cannot be used if U2 is omitted.
- Headers can be omitted for the GPIO or SPI ports not currently needed. Waiting to add the headers until needed will make the card easier to handle.
- Any decoupling capacitors used for the omitted chips can be omitted.

## References

- [DS1306 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS1306.pdf)
- [MCP32S17 Datasheet](http://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf)