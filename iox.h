/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iox.h MCP21S17 register definitions and convenience functions
 */

#ifndef IOX_H
#define IOX_H

#include <stdint.h>

// Register addresses in bank 0
#define IODIRA0 0x00
#define IODIRB0 0x01
#define IPOLA0 0x02
#define IPOLB0 0x03
#define GPINTENA0 0x04
#define GPINTENB0 0x05
#define DEFVALA0 0x06
#define DEFVALB0 0x07
#define INTCONA0 0x08
#define INTCONB0 0x09
#define IOCON0 0x0A
#define GPPUA0 0x0C
#define GPPUB0 0x0D
#define INTFA0 0x0E
#define INTFB0 0x0F
#define INTCAPA0 0x10
#define INTCAPB0 0x11
#define GPIOA0 0x12
#define GPIOB0 0x13
#define OLATA0 0x14
#define OLATB0 0x15

// Register addresses in bank 1
#define IODIRA1 0x00
#define IPOLA1 0x01
#define GPINTENA1 0x02
#define DEFVALA1 0x03
#define INTCONA1 0x04
#define IOCON1 0x05
#define GPPUA1 0x06
#define INTFA1 0x07
#define INTCAPA1 0x08
#define GPIOA1 0x09
#define OLATA1 0x0A
#define IODIRB1 0x10
#define IPOLB1 0x11
#define GPINTENB1 0x12
#define DEFVALB1 0x13
#define INTCONB1 0x14
#define GPPUB1 0x16
#define INTFB1 0x17
#define INTCAPB1 0x18
#define GPIOB1 0x19
#define OLATB1 0x1A

// Bits in IOCON register
#define INTCC 0
#define INTPOL 1
#define ODR 2
#define HAEN 3
#define DISSLW 4
#define SEQOP 5
#define MIRROR 6
#define BANK 7

#define SPI_ADDR 0x40
#define WRITE 0
#define READ 1

void iox_init(void);
void iox_begin(uint8_t mode, uint8_t addr);
void iox_end(void);
uint8_t iox_read(uint8_t chipaddr, uint8_t regaddr);
void iox_write(uint8_t chipaddr, uint8_t regaddr, uint8_t data);

#endif
