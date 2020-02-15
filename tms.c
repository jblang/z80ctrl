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
uint8_t textfg;
uint8_t textbg;
int16_t cursorpos;
uint8_t undercursor;
uint8_t pixelfg;
uint8_t pixelbg;
uint8_t pixelmode;

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
    tms_writereg(7, (textfg << 4) | textbg);
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
    textfg = 0xf;
    textbg = 0;
    nametab = 0x3800;     
    colortab = 0x2000;
    pattab = 0;
    satab = 0x3bc0;
    sptab = 0x1800;
    switch (mode) {
        case TMS_TEXT:
            tms_loadfont(font);
            textbg = 0x4;
            break;
        case TMS_BITMAP:
            tms_bitmapname();
            break;
        case TMS_MULTICOLOR:
            tms_multicolorname();
            break;
    }
    modereg = mode;
    cursorpos = 0;
    tms_config();
}

const uint8_t defcolors[] PROGMEM = {
    0x0, 0x6, 0xc, 0xa, 0x4, 0xd, 0x7, 0xe, 
    0xe, 0x9, 0x3, 0xb, 0x5, 0xd, 0x7, 0xf
};

uint8_t colormap[] = {
    0x0, 0x6, 0xc, 0xa, 0x4, 0xd, 0x7, 0xe, 
    0xe, 0x9, 0x3, 0xb, 0x5, 0xd, 0x7, 0xf
};

#define DEFCOLOR(i) (pgm_read_byte(&defcolors[i]))

void vdu_color(uint8_t c)
{
    if (c >= 128)
        textbg = colormap[c & 0xf];
    else
        textfg = colormap[c & 0xf];
    tms_config();
}

void vdu_gcolor(uint8_t m, uint8_t c)
{
    pixelmode = m % 5;
    if (c >= 128)
        pixelbg = colormap[c & 0xf];
    else
        pixelfg = colormap[c & 0xf];
    tms_config();
}

void vdu_lcolor(uint8_t l, uint8_t p)
{
    colormap[l & 0xf] = p & 0xf;
}

void vdu_dcolor()
{
    for (uint8_t i = 0; i < 16; i++)
        colormap[i] = DEFCOLOR(i);
}

void vdu_flash()
{
    uint8_t save = textbg;
    textbg = ~textbg;
    tms_config();
    _delay_ms(100);
    textbg = save;
    tms_config();
}

void vdu_pos(uint8_t x, uint8_t y)
{
    x %= 40;
    y %= 24;
    tms_fill(nametab + cursorpos, 0, 1);
    cursorpos = x * 40 + y;
}

void vdu_scroll(int16_t lines)
{
    uint8_t buf[1000];
    if (lines > 0) {
        tms_read(nametab + lines * 40, buf, 960 - lines * 40);
        for (uint16_t i = 960 - lines * 40; i < 960; i++)
            buf[i] = 0;
        tms_write(nametab, buf, 960);
    } else if (lines < 0) {
        tms_read(nametab, buf, 960 + lines * 40);
        for (uint16_t i = 960 - lines * 40; i < 960; i++)
            buf[i] = 0;
        tms_write(nametab, buf, 960);
    }
}

void vdu_clearall()
{
    tms_fill(nametab, 0, 960);
}

void vdu_cleartostart()
{
    tms_fill(nametab, 0, cursorpos);
}

void vdu_cleartoend()
{
    tms_fill(nametab + cursorpos, 0, 960 - cursorpos);
}

void vdu_cleargraph()
{
    
}

void vdu_update()
{
    static int16_t prevpos;
    uint8_t lines = 0;
    if (cursorpos < 0) {
        lines = (cursorpos - 40) / 40;
        cursorpos = 40 - (cursorpos % 40);
    } else if (cursorpos > 959) {
        lines = (cursorpos - 920) / 40;
        cursorpos = 920 + (cursorpos % 40);
    }

    tms_write(nametab + prevpos, &undercursor, 1);
    vdu_scroll(lines);
    tms_read(nametab + cursorpos, &undercursor, 1);
    tms_fill(nametab + cursorpos, 0xde, 1);
    prevpos = cursorpos;
}

void vdu_cursorup(uint8_t n)
{
    cursorpos -= 40 * n;
}

void vdu_cursordown(uint8_t n)
{
    cursorpos += 40 * n;
}

void vdu_cursorleft(uint8_t n)
{
    cursorpos -= n;
}

void vdu_cursorright(uint8_t n)
{
    cursorpos += n;
}

void vdu_startline()
{
    cursorpos = ((cursorpos / 40) * 40);
}

void vdu_delete()
{
    undercursor = 0;
}

void vdu_mode(uint8_t m)
{
    switch (m) {
        case 0:
            tms_init(TMS_TEXT);
            break;
        case 1:
            tms_init(TMS_TILE);
            break;
        case 2:
            tms_init(TMS_BITMAP);
            break;
        case 3:
            tms_init(TMS_MULTICOLOR);
            break;
    }
}

void vdu_program(uint8_t *b)
{

}

void vdu_plot(uint8_t m, uint16_t x, uint16_t y)
{

}

void vdu_gwindow(uint16_t lx, uint16_t by, uint16_t rx, uint16_t ty)
{
    
}

void vdu_dwindow()
{

}

void vdu_twindow(uint8_t lx, uint8_t by, uint8_t rx, uint8_t ty)
{

}

void vdu_origin(uint16_t x, uint16_t y)
{

}

void vdu_literal(uint8_t c)
{
    undercursor = c;
    tms_write(nametab + cursorpos, &c, 1);
    cursorpos++;
}

void ansi_pos(uint8_t m, uint8_t n)
{
    if (n > 0)
        n--;
    if (m > 0)
        m--;
    vdu_pos(n, m);
}

void ansi_clear(uint8_t n)
{
    if (n == 0) {
        vdu_cleartoend();
    } else if (n == 1) {
        vdu_cleartostart();
    } else if (n == 2 || n == 3) {
        vdu_clearall();
    }
}

void ansi_color(uint8_t n)
{
    if (30 <= n && n <= 37)
        textfg = DEFCOLOR(n - 30);
    else if (n == 38)
        textfg = 0;
    else if (40 <= n && n <= 47)
        textbg = DEFCOLOR(n - 40);
    else if (n == 48)
        textbg = 0;
    else if (90 <= n && n <= 97)
        textfg = DEFCOLOR(n - 90 + 8);
    else if (100 <= n && n <= 107)
        textbg = DEFCOLOR(n - 100 + 8);
    tms_config();
}

enum { 
    VDU_NORMAL,
    VDU_ESC,
    VDU_CSI,
    VDU_TCOLOR,
    VDU_GCOLOR,
    VDU_LCOLOR,
    VDU_DCOLOR,
    VDU_DISABLED,
    VDU_MODE,
    VDU_PROGRAM,
    VDU_GWINDOW,
    VDU_PLOT,
    VDU_TWINDOW,
    VDU_ORIGIN,
    VDU_POS
};

#define MAXESC 16

void tms_putchar(char c)
{
    static uint8_t mode = VDU_NORMAL;
    static uint8_t escidx = 0;
    static uint8_t esclen = 0;
    static uint8_t escbuf[MAXESC];

    if (mode == VDU_DISABLED) {
        if (c == 6) {
            mode = VDU_NORMAL;
            return;
        } else {
            return;
        }
    }

    if (bus_mode != BUS_MASTER)
        bus_master();

    if (mode == VDU_NORMAL) {
        escidx = 0;
        switch (c) {
            case 0: // null, ignore
                break;
            case 7:
                vdu_flash();
                break;
            case 8:
                vdu_cursorleft(1);
                break;
            case 9:
                vdu_cursorright(1);
                break;
            case 10:
                vdu_cursordown(1);
                break;
            case 11:
                vdu_cursorup(1);
                break;
            case 12:
                vdu_clearall();
                vdu_pos(0, 0);
                break;
            case 13:
                vdu_startline();
                break;
            case 16:
                vdu_cleargraph();
                break;
            case 17:
                mode = VDU_TCOLOR;
                esclen = 1;
                break;
            case 18:
                mode = VDU_GCOLOR;
                esclen = 2;
                break;
            case 19:
                mode = VDU_LCOLOR;
                esclen = 3;
                break;
            case 20:
                mode = VDU_DCOLOR;
                esclen = 4;
                break;
            case 21:
                mode = VDU_DISABLED;
                break;
            case 22:
                mode = VDU_MODE;
                esclen = 1;
                break;
            case 23:
                mode = VDU_PROGRAM;
                esclen = 9;
                break;
            case 24:
                mode = VDU_GWINDOW;
                esclen = 8;
                break;
            case 25:
                mode = VDU_PLOT;
                esclen = 5;
                break;
            case 26:
                vdu_dwindow();
                break;
            case 27:
                mode = VDU_ESC;
                break;
            case 28:
                mode = VDU_TWINDOW;
                esclen = 4;
                break;
            case 29:
                mode = VDU_ORIGIN;
                esclen = 4;
                break;
            case 30:
                vdu_pos(0, 0);
                break;
            case 31:
                mode = VDU_POS;
                esclen = 2;
                break;
            case 127:
                vdu_delete();
                break;
            default:
                vdu_literal(c);
                break;       
        }
    } else if (mode == VDU_ESC) {
        if (c == '[') {
            for (uint8_t i = 0; i < MAXESC; i++)
                escbuf[i] = 0;
            escidx = 0;
            mode = VDU_CSI;
        } else {
            if (c == 'c')
                tms_init(TMS_TEXT); // reset terminal
            else 
                vdu_literal(c);
            mode = VDU_NORMAL;
        }
    } else if (mode == VDU_CSI) {
        if ('0' <= c && c <= '9') {
            escbuf[escidx] = escbuf[escidx] * 10 + c - '0';
        } else if ((c == ':' || c == ';') && escidx < MAXESC-1) {
            escidx++;
        } else {
            if (c == 'J') {
                ansi_clear(escbuf[0]);
            } else if (c == 'H') {
                ansi_pos(escbuf[0], escbuf[1]);
            } else if (c == 'm') {
                for (uint8_t i = 0; i <= escidx; i++)
                    ansi_color(escbuf[i]);
            }
            mode = VDU_NORMAL;
        }
    } else if (escidx < esclen - 1) {
        escbuf[escidx++] = c;
    } else {
        escbuf[escidx] = c;
        switch (mode) {
            case VDU_TCOLOR:
                vdu_color(escbuf[0]);
                break;
            case VDU_GCOLOR:
                vdu_gcolor(escbuf[0], escbuf[1]);
                break;
            case VDU_LCOLOR:
                vdu_lcolor(escbuf[0], escbuf[1]);
                break;
            case VDU_DCOLOR:
                vdu_dcolor();
                break;
            case VDU_MODE:
                vdu_mode(escbuf[0]);
                break;
            case VDU_PROGRAM:
                vdu_program(escbuf);
                break;
            case VDU_GWINDOW:
                vdu_gwindow(escbuf[0] | escbuf[1] << 8, escbuf[2] | escbuf[3] << 8, escbuf[4] | escbuf[5] << 8, escbuf[6] | escbuf[7] << 8);
                break;
            case VDU_PLOT:
                vdu_plot(escbuf[0], escbuf[1] | escbuf[2] << 8, escbuf[3] | escbuf[4] << 8);
                break;
            case VDU_TWINDOW:
                vdu_twindow(escbuf[0], escbuf[1], escbuf[2], escbuf[3]);
                break;
            case VDU_ORIGIN:
                vdu_origin(escbuf[0] | escbuf[1] << 8, escbuf[2] | escbuf[3] << 8);
                break;
            case VDU_POS:
                vdu_pos(escbuf[0], escbuf[1]);
                break;
        }
        mode = VDU_NORMAL;
    }
    
    vdu_update();
}