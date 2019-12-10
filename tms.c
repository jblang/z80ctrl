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

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "tms.h"
#include "bus.h"

#define TMS_RAM TMS_BASE
#define TMS_REG TMS_BASE+1

#define TMS_WAIT 8

uint8_t _tms_write(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace)
{
    if (bus_mode != BUS_MASTER)
        return 0;
    addr &= 0x3fff;
    addr |= 0x4000;
    DATA_OUTPUT;
    io_out(TMS_REG, addr & 0xff);
    _delay_us(TMS_WAIT);
    io_out(TMS_REG, addr >> 8);
    _delay_us(TMS_WAIT);
    for (uint16_t i = 0; i < len; i++) {
        if (pgmspace)
            io_out(TMS_RAM, pgm_read_byte(&buf[i]));
        else
            io_out(TMS_RAM, buf[i]);
        _delay_us(TMS_WAIT);
    }
    return 1;
}

uint8_t tms_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    if (bus_mode != BUS_MASTER)
        return 0;
    DATA_OUTPUT;
    addr &= 0x3fff;
    io_out(TMS_REG, addr & 0xff);
    _delay_us(TMS_WAIT);
    io_out(TMS_REG, addr >> 8);
    _delay_us(TMS_WAIT);
    DATA_INPUT;
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = io_in(TMS_RAM);
        _delay_us(TMS_WAIT);
    }
    return 1;
}