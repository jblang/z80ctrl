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
 * @file mcp23s17.c MCP23S17 convenience functions
 */

#include <stdio.h>

#include "harvard.h"

#include "mcp23s17.h"
#include "rtc.h"
#include "avr_spi.h"

// translation table from bank 0 to bank 1 addresses
const uint8_t iox_bank1[] PROGMEM = {
    0x00, 0x10, // IODIR
    0x01, 0x11, // IPOL
    0x02, 0x12, // GPINTEN
    0x03, 0x13, // DEFVAL
    0x04, 0x14, // INTCON
    0x05, 0x15, // IOCON
    0x06, 0x16, // GPPU
    0x07, 0x17, // INTF
    0x08, 0x18, // INTCAP
    0x09, 0x19, // GPIO
    0x0a, 0x1a // OLAT
};

#define IOX_SIGNATURE 0x40
#define IOX_WRITE 0
#define IOX_READ 1
#define IOX_END AUX2_SEL

// local copy of register values
uint8_t iox0_registers[] = {
    0xff, 0xff, // IODIR
    0x00, 0x00, // IPOL
    0x00, 0x00, // GPINTEN
    0x00, 0x00, // DEFVAL
    0x00, 0x00, // INTCON
    0x00, 0x00, // IOCON
    0x00, 0x00, // GPPU
    0x00, 0x00, // INTF
    0x00, 0x00, // INTCAP
    0x00, 0x00, // GPIO
    0x00, 0x00 // OLAT
};

// Banks of IO expanders 1-7
uint8_t iox_banks = 0;

void iox_init(void)
{
    spi_init();
    // Enable individually addressable chips
    iox0_set(IOCON, HAEN);

    // Disable sequential operation
    iox0_set(IOCON, SEQOP);
}

void iox_begin(uint8_t chip, uint8_t mode, uint8_t reg)
{
    IOX_SEL;
    if (iox_banks & (1 << chip))
        reg = pgm_read_byte(&iox_bank1[reg]);
    spi_exchange(IOX_SIGNATURE | (chip << 1) | mode);
    spi_exchange(reg);
}

uint8_t iox_read(uint8_t chip, uint8_t reg)
{
    chip &= 7;
    uint8_t data;
    iox_begin(chip, IOX_READ, reg);
    data = spi_exchange(0);
    IOX_END;
    return data;
}

void iox_write(uint8_t chip, uint8_t reg, uint8_t data)
{
    chip &= 7;
    if (chip != 0) {
        iox_begin(chip, IOX_WRITE, reg);
        spi_exchange(data);
        IOX_END;
        if (reg == IOCON || reg == IOCONB) {
            if (data & BANK)
                iox_banks |= (1 << chip);
            else
                iox_banks &= ~(1 << chip);
        }
    }
}

void iox0_begin(uint8_t mode, uint8_t reg)
{
    if (reg == IOCONB)
        reg = IOCON;
    if (iox0_registers[IOCON] & BANK)
        reg = pgm_read_byte(&iox_bank1[reg]);
    IOX_SEL;
    spi_exchange(IOX_SIGNATURE | mode);
    spi_exchange(reg);
}

uint8_t iox0_read(uint8_t reg)
{
    iox0_begin(IOX_READ, reg);
    uint8_t data = spi_exchange(0);
    IOX_END;
    return data;
}

uint16_t iox0_read16(uint8_t reg)
{
    iox0_begin(IOX_READ, reg);
    uint16_t data = spi_exchange(0);
    data |= spi_exchange(0) << 8;
    IOX_END;
    return data;
}

void iox0_write(uint8_t reg, uint8_t data)
{
    iox0_registers[reg] = data;
    iox0_begin(IOX_WRITE, reg);
    spi_exchange(data);
    IOX_END;
}

void iox0_set(uint8_t reg, uint8_t mask)
{
    iox0_write(reg, iox0_registers[reg] | mask);
}

void iox0_clear(uint8_t reg, uint8_t mask)
{
    iox0_write(reg, iox0_registers[reg] & ~mask);
}