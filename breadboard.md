# Z80 to AVR Breadboard Configuration

## Bill of Materials

- Zilog [Z84C010PEG](http://www.mouser.com/ds/2/450/ps0178-19386.pdf) 10MHz Z80 CPU
- Alliance [AS6C4008](http://www.mouser.com/ds/2/12/AS6C4008-1265427.pdf) 512KB SRAM chip
- Atmel [ATmega1284p](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-42719-ATmega1284P_Datasheet.pdf) AVR microcontroller
- Microchip [MCP23S18](http://ww1.microchip.com/downloads/en/DeviceDoc/22103a.pdf) SPI I/O expander
- [74HCT74](http://www.ti.com/general/docs/lit/getliterature.tsp?genericPartNumber=SN74HCT74&fileType=pdf) dual D-type flip-flop
- [LM1117](http://www.ti.com/lit/ds/symlink/lm1117.pdf) 3.3V regulator
- Silicon Labs [CP2102](https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf) USB-to-TTL serial breakout
- Pololu [ls01a](https://www.pololu.com/product/2595) 4 channel level shifter
- Pololu [sdc01a](https://www.pololu.com/product/2597) microSD breakout
- 20MHz crystal with 20 pf load caps for AVR
- Various 0.01uf bypass and 10uf filter caps

## Connections

The prototype has been built on two full-size breadboards using dupont wire.  In lieu of a schematic for now, here is a table of connections. Pin names are followed by pin numbers are in parentheses.  Please refer to notes after each table for connections marked with an asterisk.

### Memory, Address, and Control Busses

| Z80          | 74HCT74    | Atmega1284P | MCP23S18    | AS6C4008 | Power |
|--------------|------------|-------------|-------------|----------|-------|
| A0 (30)      |            | PA0 (40)    |             | A0 (12)  |       |
| A1 (31)      |            | PA1 (39)    |             | A1 (11)  |       |
| A2 (32)      |            | PA2 (38)    |             | A2 (10)  |       |
| A3 (33)      |            | PA3 (37)    |             | A3 (9)   |       |
| A4 (34)      |            | PA4 (36)    |             | A4 (8)   |       |
| A5 (35)      |            | PA5 (35)    |             | A5 (7)   |       |
| A6 (36)      |            | PA6 (34)    |             | A6 (6)   |       |
| A7 (37)      |            | PA7 (33)    |             | A7 (5)   |       |
| A8 (38)      |            |             | GPA0 (20)   | A8 (27)  |       |
| A9 (39)      |            |             | GPA1 (21)   | A9 (26)  |       |
| A10 (40)     |            |             | GPA2 (22)   | A10 (23) |       |
| A11 (1)      |            |             | GPA3 (23)   | A11 (25) |       |
| A12 (2)      |            |             | GPA4 (24)   | A12 (4)  |       |
| A13 (3)      |            |             | GPA5 (25)   | A13 (28) |       |
| A14 (4)      |            |             | GPA6 (26)   | A14 (3)  |       |
| A15 (5)      |            |             | GPA7 (27)   | A15 (31) |       |
|              |            |             | GPB5 (8)    | A16 (2)  |       |
|              |            |             | GPB6 (9)    | A17 (30) |       |
|              |            |             | GPB7 (10)   | A18 (1)  |       |
| D0 (14)      |            | PC0 (22)    |             | DQ0 (13) |       |
| D1 (15)      |            | PC1 (23)    |             | DQ1 (14) |       |
| D2 (12)      |            | PC2 (24)    |             | DQ2 (15) |       |
| D3 (8)       |            | PC3 (25)    |             | DQ3 (17) |       |
| D4 (7)       |            | PC4 (26)    |             | DQ4 (18) |       |
| D5 (9)       |            | PC5 (27)    |             | DQ5 (19) |       |
| D6 (10)      |            | PC6 (28)    |             | DQ6 (20) |       |
| D7 (13)      |            | PC7 (29)    |             | DQ7 (21) |       |
| M1# (27)     |            | PB1 (2)     |             |          |       |
| MREQ# (19)   |            | PD3 (17)    |             | CE# (22) |       |
| IORQ# (20)   | 1PRE# (4)  | PD2 (16)    |             |          |       |
| RD# (21)     |            | PD5 (19)    |             | OE# (24) |       |
| WR# (22)     |            | PD4 (18)    |             | WE# (29) |       |
| RFSH# (28)   |            |             |             |          |       |
| HALT# (18)*  |            | PB2 (3)*    |             |          | GND*  |
| WAIT# (24)   | 1Q# (6)    |             |             |          |       |
| INT# (16)    |            |             | GPB0 (3)    |          |       |
| NMI# (17)    |            |             | GPB1 (4)    |          |       |
| RESET# (26)  |            |             | GPB2 (5)    |          |       |
| BUSREQ# (25) |            |             | GPB3 (6)    |          |       |
| BUSACK# (23) |            |             | GPB4 (7)    |          |       |
| CLK (6)      |            | PD6 (20)    |             |          |       |
|              | 1CLR# (1)  | PD7 (21)    |             |          |       |
|              | 1D (2)     |             |             |          | GND   |
|              | 1CLK (3)   |             |             |          | GND   |
|              | 2CLR# (13) |             |             |          | 5V    |
|              | 2D (12)    |             |             |          | 5V    |
|              | 2CLK (11)  |             |             |          | 5V    |
|              | 2PRE# (12) |             |             |          | 5V    |
|              |            | PB4 (5)     | CS# (12)    |          |       |
|              |            |             | RESET# (16) |          | 5V    |
|              |            | SCK (8)     | SCK (13)    |          |       |
|              |            | MISO (7)    | SO (15)     |          |       |
|              |            | MOSI (6)    | SI (14)     |          |       |
|              |            | PB3 (4)     |             |          |       |
|              |            | XTAL1 (13)* |             |          |       |
|              |            | XTAL2 (12)* |             |          |       |
| +5V (11)     | VCC (14)   | VCC (10)    | VDD (11)    | VCC (32) | 5V*   |
| GND (29)     | GND (7)    | GND (11)    | VSS (1)     | VSS (16) | GND*  |

Notes:
1. HALT# connected to PB6 through a diode: `PB6 ->|- HALT#`. PB6 is also connected through a momentary push button to ground.
2. Atmega1284p XTAL1 and XTAL2 connected to 20MHz full swing crystal oscillator with 20 pf load capacitors to GND on each pin.
3. Each chip has a 0.1uf bypass capacitor from 5V to GND.

### SD Card

| Atmega1284p | LS01A   | SDC01A   | LM1117T-3.3V | Power |
|-------------|---------|----------|--------------|-------|
| SCK (8)     | H1 (2)  |          |              |       |
| MISO (7)    | H2 (3)  |          |              |       |
| MOSI (6)    | H3 (4)  |          |              |       |
| PB3 (4)     | H4 (5)  |          |              |       |
|             | L1 (9)  | SCLK (5) |              |       |
|             | L2 (8)  | DO (4)   |              |       |
|             | L3 (7)  | DI (3)   |              |       |
|             | L4 (6)  | CS# (6)  |              |       |
| VCC (10)    | HV (1)  |          | INPUT*       | 5V    |
|             | LV (10) | VDD (2)  | OUTPUT*      | 3V    |
| GND (11)    |         | GND (1)  | GND*         | GND   |

Note: Input and output pins of LM117T-3.3V each have 10uf filter capacitor to GND.

### Serial Adapter

| ATmega1284p | CP2102 | Power    |
|-------------|--------|----------|
| RESET# (9)* | DTR*   | 5V, GND* |
| RXD0 (14)   | TXO    |          |
| TXD0 (15)   | RXI    |          |
| VCC (10)    | VCC    | 5V       |
| GND (11)    | GND    | GND      |

Note: RESET# is connected to DTR through a 0.1uf capacitor: `RESET# -||- DTR`. RESET# is also connected through a 10K pullup resistor to 5V and through a momentary push button to GND.