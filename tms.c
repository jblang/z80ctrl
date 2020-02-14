#include <stdio.h>
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
#include "font.h"
#include "stdlib.h"

#define TMS_RAM TMS_BASE
#define TMS_REG TMS_BASE+1

uint16_t modereg;
uint16_t nametab;
uint16_t colortab;
uint16_t pattab;
uint16_t satab;
uint16_t sptab;
uint8_t fgcolor;
uint8_t bgcolor;

uint8_t _tms_write(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace)
{
    addr &= 0x3fff;
    addr |= 0x4000;
    DATA_OUTPUT;
    io_out(TMS_REG, addr & 0xff);
    _delay_us(2);
    io_out(TMS_REG, addr >> 8);
    _delay_us(2);
    for (uint16_t i = 0; i < len; i++) {
        if (pgmspace)
            io_out(TMS_RAM, pgm_read_byte(&buf[i]));
        else
            io_out(TMS_RAM, buf[i]);
        if (modereg & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
    return 1;
}

uint8_t tms_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    DATA_OUTPUT;
    addr &= 0x3fff;
    io_out(TMS_REG, addr & 0xff);
    _delay_us(2);
    io_out(TMS_REG, addr >> 8);
    _delay_us(2);
    DATA_INPUT;
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = io_in(TMS_RAM);
        if (modereg & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
    return 1;
}

uint8_t tms_readreg()
{
    return io_in(TMS_REG);
}

void tms_writereg(uint8_t reg, uint8_t data)
{
    reg &= 0x07;
    reg |= 0x80;
    io_out(TMS_REG, data);
    _delay_us(2);
    io_out(TMS_REG, reg);
    _delay_us(2);
}

void tms_config()
{
    nametab &= 0x3c00;
    if (modereg & 2) {
        colortab &= 0x2000;
        pattab &= 0x2000;
    } else {
        colortab &= 0x3fc0;
        pattab &= 0x3800;
    }
    satab &= 0x3f80;
    sptab &= 0x3800;
    tms_writereg(0, modereg & 0xff);
    tms_writereg(1, modereg >> 8);
    tms_writereg(2, nametab / 0x400);
    if (modereg & 2) {
        tms_writereg(3, colortab / 0x40 + 0x7f);
        tms_writereg(4, pattab / 0x800 + 3);
    } else {
        tms_writereg(3, colortab / 0x40);
        tms_writereg(4, pattab / 0x800);
    }
    tms_writereg(5, satab / 0x80);
    tms_writereg(6, sptab / 0x800);
    tms_writereg(7, (fgcolor << 4) | bgcolor);
}

tms_stat tms_status(void)
{
    tms_stat s;
    uint8_t data = tms_readreg();
    s.interrupt = data >> 7;
    s.fifthsprite = (data >> 6) & 1;
    s.coincidence = (data >> 5) & 1;
    s.spriteno = data & 0x1f;
    return s;
}

void tms_fill(uint16_t addr, uint8_t val, uint16_t len)
{
    addr &= 0x3fff;
    addr |= 0x4000;
    io_out(TMS_REG, addr & 0xff);
    _delay_us(2);
    io_out(TMS_REG, addr >> 8);
    _delay_us(2);
    for (uint16_t i = 0; i < len; i++) {
        io_out(TMS_RAM, val);
        if (modereg & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
}

#define tms_loadfont(font) tms_write_P(pattab, (font), 0x800);

void tms_bitmapname()
{
    uint8_t buf[768];
    for (uint16_t i = 0; i < 768; i++)
        buf[i] = i & 0xff;
    tms_write(nametab, buf, 768);
}

void tms_multicolorname()
{
    uint8_t buf[768];
    uint8_t i = 0;
    for (uint8_t s = 0; s < 6; s++)
        for (uint8_t l = 0; l < 4; l++)
            for (uint8_t b = 0; b < 32; b++)
                buf[i++] = 32 * s + b;
    tms_write(nametab, buf, 768);
}

void tms_init(uint16_t mode)
{
    modereg = 0x8000;
    tms_config();
    tms_fill(0, 0, 0x3fff);
    fgcolor = 0xf;
    bgcolor = 0;
    nametab = 0x3800;     
    colortab = 0x2000;
    pattab = 0;
    satab = 0x3bc0;
    sptab = 0x1800;
    switch (mode) {
        case TMS_TEXT:
            tms_loadfont(font);
            bgcolor = 0x4;
            break;
        case TMS_BITMAP:
            tms_bitmapname();
            break;
        case TMS_MULTICOLOR:
            tms_multicolorname();
            break;
    }
    modereg = mode;
    tms_config();
}

#define TERM_NORMAL 0
#define TERM_ESC 1
#define TERM_CSI 2

uint8_t colormap[] = {
    0x1, 0x6, 0xc, 0xa, 0x4, 0xd, 0x7, 0xe, 
    0xe, 0x9, 0x3, 0xb, 0x5, 0xd, 0x7, 0xf
};

void tms_putchar(char c)
{
    static uint16_t pos = 0;
    static uint8_t mode = TERM_NORMAL;
    static uint8_t escidx = 0;
    static char escbuf[16];
    static char prev;

    uint8_t prev_bus = bus_mode;
    if (bus_mode != BUS_MASTER)
        bus_master();

    if (mode == TERM_ESC) {
        if (c == 'c')
            tms_init(TMS_TEXT); // reset terminal
        if (c == '[') 
            mode = TERM_CSI;
        else
            mode = TERM_NORMAL;
    } else if (mode == TERM_CSI) {
        if (0x30 <= c && c <= 0x3B && escidx < 15) {
            escbuf[escidx++] = c;
        } else {
            escbuf[escidx] = 0;
            char *end;
            if (c == 'J') {
                int n = strtoul(escbuf, &end, 10);
                if (n == 1) {
                    // clear to beginning
                    tms_fill(nametab, 0, pos);
                } else if (n == 2 || n == 3) {
                    // clear all
                    tms_fill(nametab, 0, 0x3c0);
                } else {
                    // clear to end
                    tms_fill(nametab + pos, 0, 0x3c0 - pos);
                }
            } else if (c == 'H') {
                int n = strtoul(escbuf, &end, 10) % 24;
                if (*end != 0) {
                    char *next = end + 1;
                    int m = strtoul(next, &end, 10) % 40;
                    tms_fill(nametab + pos, 0, 1);
                    pos = n * 40 + m;
                }
            } else if (c == 'm') {
                char *next = escbuf;
                do {
                    int m = strtoul(next, &end, 10);
                    next = end + 1;
                    if (30 <= m && m <= 37)
                        fgcolor = colormap[m - 30];
                    else if (40 <= m && m <= 47)
                        bgcolor = colormap[m - 40];
                    else if (90 <= m && m <= 97)
                        fgcolor = colormap[m - 90 + 8];
                    else if (100 <= m && m <= 107)
                        bgcolor = colormap[m - 100 + 8];
                } while (*end != 0);
                tms_config();
            }
            mode = TERM_NORMAL;
        }
    } else {
        switch (c) {
            case '\r':
                break;
            case '\n':
                tms_fill(nametab + pos, 0, 1);
                // go to start of next line
                pos = ((pos / 40) * 40) + 40;
                break;
            case '\f':  // clear screen
                tms_fill(nametab, 0, 0x3c0);
                pos = 0;
                break;
            case '\t':
                tms_fill(nametab + pos, 0, 1);
                pos = ((pos / 4) * 4) + 4;
                break;
            case '\b':  // backspace
            case 0x7f:  // delete
                tms_fill(nametab + pos, 0, 1);
                pos--;
                break;
            case '\e':
                mode = TERM_ESC;
                escidx = 0;
                break;
            default:    // normal character
                // CR without LF moves to start of current line
                if (prev == '\r')
                    pos = ((pos / 40) * 40);
                tms_write(nametab + pos, &c, 1);
                pos++;
                break;       
        }
    }

    // scroll the screen if necessary
    if (pos >= 960) {
        uint8_t buf[1000];
        tms_read(nametab + 40, buf, 920);
        for (uint16_t i = 920; i < 960; i++)
            buf[i] = 0;
        tms_write(nametab, buf, 960);
        pos = 920;
    }
    prev = c;

    // show cursor
    tms_fill(nametab + pos, 0xde, 1);

    if (prev_bus != BUS_MASTER)
        bus_slave();
}