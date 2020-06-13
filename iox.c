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
 * @file iox.c MCP21S17 convenience functions
 */

#include "iox.h"
#include "rtc.h"
#include "spi.h"

void iox_init(void)
{
    spi_init();
    // Enable individually addressable chips
    iox_write(0, IOCON0, (1 << HAEN));
}

void iox_begin(uint8_t mode, uint8_t addr)
{
    IOX_SEL;
    spi_exchange(SPI_ADDR | mode);
    spi_exchange(addr);
}

void iox_end(void)
{
    AUX2_SEL;
}

uint8_t iox_read(uint8_t chipaddr, uint8_t regaddr)
{
    uint8_t data;
    iox_begin(READ | (chipaddr << 1), regaddr);
    data = spi_exchange(0);
    iox_end();
    return data;
}

uint16_t iox_read16(uint8_t chipaddr, uint8_t regaddr)
{
    uint16_t data;
    iox_begin(READ | (chipaddr << 1), regaddr);
    data = spi_exchange(0);
    data |= spi_exchange(0) << 8;
    iox_end();
    return data;
}

void iox_write(uint8_t chipaddr, uint8_t regaddr, uint8_t data)
{
    iox_begin(WRITE | (chipaddr << 1), regaddr);
    spi_exchange(data);
    iox_end();
}

void iox_extcs_init(uint8_t addr)
{
    iox_write(addr, IODIRB0, iox_read(addr, IODIRB0) & 0x0F);
    iox_write(addr, GPIOB0, iox_read(addr, GPIOB0) & 0xF0);
}

void iox_extcs_lo(uint8_t c)
{
    uint8_t addr = (c / 4) * 2 + 1;
    uint8_t pin = (c % 4) + 4;
    iox_write(addr, GPIOB0, iox_read(addr, GPIOA0) & ~(1 << pin));
}

void iox_extcs_hi(uint8_t c)
{
    uint8_t addr = (c / 4) * 2 + 1;
    uint8_t pin = (c % 4) + 4;
    iox_write(addr, GPIOB0, iox_read(addr, GPIOA0) | (1 << pin));
}

/**
 * IO registers exposed to Z80
 */
#ifdef IOX_BASE
#define IOX_DEVPORT IOX_BASE
#define IOX_REGPORT IOX_BASE+1
#define IOX_VALPORT IOX_BASE+2
#define IOX_RTC 0
#define IOX_GPIO_MIN 1
#define IOX_GPIO_MAX 7

static uint8_t iox_dev = 0;
static uint8_t iox_reg = 0;

void iox_setdev(uint8_t dev)
{
    iox_dev = dev;
}

uint8_t iox_getdev()
{
    return iox_dev;
}

void iox_setreg(uint8_t reg)
{
    iox_reg = reg;
}

uint8_t iox_getreg()
{
    return iox_reg;
}

void iox_writeval(uint8_t data)
{
    if (iox_dev == IOX_RTC) {
#ifdef DS1306_RTC
        rtc_write1(iox_reg, data);
#endif
    } else if (iox_dev >= IOX_GPIO_MIN && iox_dev <= IOX_GPIO_MAX) {
        iox_write(iox_dev, iox_reg, data);
    }
}

uint8_t iox_readval()
{
    if (iox_dev == IOX_RTC) {
#ifdef DS1306_RTC
        return rtc_read1(iox_reg);
#endif
    } else if (iox_dev >= IOX_GPIO_MIN && iox_dev <= IOX_GPIO_MAX) {
        return iox_read(iox_dev, iox_reg);
    }
}
#endif