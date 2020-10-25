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
 * @file flash.c Support for SST39SF0x0 flash
 */

#include <util/delay.h>

#include "flash.h"
#include "bus.h"

// Reference: http://ww1.microchip.com/downloads/en/DeviceDoc/20005022C.pdf

static void flash_cmd_prefix(void)
{
        SET_ADDR(0x5555);
        SET_DATA(0xAA);
        WR_LO;
        WR_HI;
        SET_ADDR(0x2AAA);
        SET_DATA(0x55);
        WR_LO;
        WR_HI;
}

uint8_t flash_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    if (GET_BUSACK)
        return 0;
    // first two banks must be physical pages 0 and 1
    mem_bank(0, 0);
    mem_bank(1, 1);
    mem_bank(2, addr >> 14);
    DATA_OUTPUT;
    MREQ_LO;
    for (uint32_t i = 0; i < len; i++) {
        // Send byte program command sequence
        flash_cmd_prefix();
        SET_ADDR(0x5555);
        SET_DATA(0xA0);
        WR_LO;
        WR_HI;

        // Write byte
        if ((addr & 0x3fff) == 0) {
            // Load page to write into bank 3
            MREQ_HI;
            mem_bank(2, addr >> 14);
            DATA_OUTPUT;
            MREQ_LO;
        }
        SET_ADDR((addr & 0x3fff) + 0x8000);
        SET_DATA(buf[i]);
        DATA_OUTPUT;
        WR_LO;
        WR_HI;

        // Wait for write to finish
        _delay_us(20);
        addr++;
    }
    MREQ_HI;
    DATA_INPUT;
    return 1;
}


/** 
 * Erase the 4KB sector that the address falls within
 */
uint8_t flash_erase(uint32_t addr)
{
    if (GET_BUSACK)
        return 0;
    // first two banks must be physical pages 0 and 1
    mem_bank(0, 0);
    mem_bank(1, 1);
    DATA_OUTPUT;
    MREQ_LO;
    // Send erase command sequence
    flash_cmd_prefix();
    SET_ADDR(0x5555);
    SET_DATA(0x80);
    WR_LO;
    WR_HI;
    flash_cmd_prefix();
    if (addr > 0x7FFFF) {
        // Erase entire chip
        SET_ADDR(0x5555);
        SET_DATA(0x10);
        WR_LO;
        WR_HI;
        _delay_ms(100);
    } else {
        // Erase 4KB sector
        MREQ_HI;
        mem_bank(0, addr >> 14);
        DATA_OUTPUT;
        MREQ_LO;
        SET_ADDR(addr & 0x3000);
        SET_DATA(0x30);
        DATA_OUTPUT;
        WR_LO;
        WR_HI;
        _delay_ms(100);
    }
    MREQ_HI;
    DATA_INPUT;
    return 1;
}
