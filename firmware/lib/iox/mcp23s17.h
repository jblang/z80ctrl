/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
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
 * @file mcp23s17.h MCP23S17 register definitions and convenience functions
 */

#ifndef HARDWARE_MCP23S17_H
#define HARDWARE_MCP23S17_H

#include <stdint.h>

// Register addresses in bank 0
enum {
    IODIRA = 0x00,
    IODIRB = 0x01,
    IPOLA = 0x02,
    IPOLB = 0x03,
    GPINTENA = 0x04,
    GPINTENB = 0x05,
    DEFVALA = 0x06,
    DEFVALB = 0x07,
    INTCONA = 0x08,
    INTCONB = 0x09,
    IOCONA = 0x0a,
    IOCONB = 0x0b,
    GPPUA = 0x0c,
    GPPUB = 0x0d,
    INTFA = 0x0e,
    INTFB = 0x0f,
    INTCAPA = 0x10,
    INTCAPB = 0x11,
    GPIOA = 0x12,
    GPIOB = 0x13,
    OLATA = 0x14,
    OLATB = 0x15
};

#define IOX_IODIR_P(P) IODIR##P
#define IOX_IPOL_P(P) IPOL##P
#define IOX_GPINTEN_P(P) GPINTEN##P
#define IOX_DEFVAL_P(P) DEFVAL##P
#define IOX_INTCON_P(P) INTCON##P
#define IOX_IOCON_P(P) IOCON##P
#define IOX_GPPU_P(P) GPPU##P
#define IOX_INTF_P(P) INTF##P
#define IOX_INTCAP_P(P) INTCAP##P
#define IOX_GPIO_P(P) GPIO##P
#define IOX_OLAT_P(P) OLAT##P

#define IOX_IODIR(P) IOX_IODIR_P(P)
#define IOX_IPOL(P) IOX_IPOL_P(P)
#define IOX_GPINTEN(P) IOX_GPINTEN_P(P)
#define IOX_DEFVAL(P) IOX_DEFVAL_P(P)
#define IOX_INTCON(P) IOX_INTCON_P(P)
#define IOX_IOCON(P) IOX_IOCON_P(P)
#define IOX_GPPU(P) IOX_GPPU_P(P)
#define IOX_INTF(P) IOX_INTF_P(P)
#define IOX_INTCAP(P) IOX_INTCAP_P(P)
#define IOX_GPIO(P) IOX_GPIO_P(P)
#define IOX_OLAT(P) IOX_OLAT_P(P)



// Bits in IOCON register
enum {
    INTCC = (1 << 0),
    INTPOL = (1 << 1),
    ODR = (1 << 2),
    HAEN = (1 << 3),
    DISSLW = (1 << 4),
    SEQOP = (1 << 5),
    MIRROR = (1 << 6),
    BANK = (1 << 7)
};

void iox_init(void);
uint8_t iox_read(uint8_t chipaddr, uint8_t regaddr);
uint16_t iox_read16(uint8_t chipaddr, uint8_t regaddr);
void iox_write(uint8_t chipaddr, uint8_t regaddr, uint8_t data);

void iox_extcs_init(uint8_t addr);
void iox_extcs_lo(uint8_t c);
void iox_extcs_hi(uint8_t c);

uint8_t iox0_read(uint8_t reg);
uint16_t iox0_read16(uint8_t reg);
void iox0_write(uint8_t reg, uint8_t data);
void iox0_set(uint8_t reg, uint8_t mask);
void iox0_clear(uint8_t reg, uint8_t mask);
#endif
