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
#include "tmsfont.h"
#include "bus.h"

#define TMS_RAM TMS_BASE
#define TMS_REG TMS_BASE+1

#define TMS_WAIT 8

uint8_t tms_readreg()
{
    return io_in_bare(TMS_REG);
}

void tms_writereg(uint8_t reg, uint8_t data)
{
    reg &= 0x07;
    reg |= 0x80;
    io_out_bare(TMS_REG, data);
    _delay_us(TMS_WAIT);
    io_out_bare(TMS_REG, reg);
    _delay_us(TMS_WAIT);
}

void tms_configure(tms_conf c)
{
    if (bus_master()) {
        DATA_OUTPUT;
        tms_writereg(0, ((c.mode & 1) << 1) | c.extvid);
        tms_writereg(1, (c.ramsize << 7) | (c.blank << 6) | (c.intenable << 5) | 
            ((c.mode & 6)  << 2) | (c.spritesize << 1) | c.spritemag);
        tms_writereg(2, c.nametable);
        tms_writereg(3, c.colortable);
        tms_writereg(4, c.patterngen);
        tms_writereg(5, c.spriteattr);
        tms_writereg(6, c.spritepat);
        tms_writereg(7, (c.textcolor << 4) | c.bgcolor);
        bus_slave();
    }
}

tms_stat tms_status(void)
{
    tms_stat s;
    if (bus_master()) {
        DATA_INPUT;
        uint8_t data = tms_readreg();
        s.interrupt = data >> 7;
        s.fifthsprite = (data >> 6) & 1;
        s.coincidence = (data >> 5) & 1;
        s.spriteno = data & 0x1f;
        bus_slave();
    }
    return s;
}

void tms_clear(void)
{
    if (bus_master()) {
        DATA_OUTPUT;
        io_out_bare(TMS_REG, 0);
        _delay_us(TMS_WAIT);
        io_out_bare(TMS_REG, 0x40);
        _delay_us(TMS_WAIT);
        for (uint16_t i = 0; i < 0x4000; i++) {
            io_out_bare(TMS_RAM, 0);
            _delay_us(TMS_WAIT);
        }
        bus_slave();
    }
}

void _tms_write(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace)
{
    if (bus_master()) {
        addr &= 0x3fff;
        addr |= 0x4000;
        DATA_OUTPUT;
        io_out_bare(TMS_REG, addr & 0xff);
        _delay_us(TMS_WAIT);
        io_out_bare(TMS_REG, addr >> 8);
        _delay_us(TMS_WAIT);
        for (uint16_t i = 0; i < len; i++) {
            if (pgmspace)
                io_out_bare(TMS_RAM, pgm_read_byte(&buf[i]));
            else
                io_out_bare(TMS_RAM, buf[i]);
            _delay_us(TMS_WAIT);
        }
        bus_slave();
    }
}

void tms_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    if (bus_master()) {
        DATA_OUTPUT;
        addr &= 0x3fff;
        io_out_bare(TMS_REG, addr & 0xff);
        _delay_us(TMS_WAIT);
        io_out_bare(TMS_REG, addr >> 8);
        _delay_us(TMS_WAIT);
        DATA_INPUT;
        for (uint16_t i = 0; i < len; i++) {
            buf[i] = io_in_bare(TMS_RAM);
            _delay_us(TMS_WAIT);
        }
        bus_slave();
    }
}

#define tms_loadfont(font) tms_write_P(0x800, (font), 0x800);

tms_conf tms_init()
{
    tms_clear();
    tms_conf c;
    c.mode = TMS_TEXTMODE;
    c.extvid = 0;
    c.ramsize = 1;
    c.blank = 1;
    c.intenable = 0;
    c.spritesize = 0;
    c.spritemag = 0;
    c.colortable = 0;
    c.patterngen = 0x01;
    c.spriteattr = 0;
    c.spritepat = 0;
    c.textcolor = 0xf;
    c.bgcolor = 0;
    tms_configure(c);
    tms_loadfont(tms_font);

    return c;
}

void tms_putchar(uint8_t c)
{
    static uint16_t pos = 0;
    if (c == '\n') {
        pos = ((pos / 40) * 40) + 40;
    } else {
        tms_write(pos, &c, 1);
        pos = (pos + 1) % 960;
    }
}