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
#include <stdlib.h>
#include <string.h>

#include "tms.h"
#include "bus.h"
#include "font.h"
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

void tms_literal(uint8_t c)
{
    undercursor = c;
    tms_write(nametab + cursorpos, &c, 1);
    cursorpos++;
}

void tms_scroll(int16_t lines)
{
    uint8_t buf[1000];
    if (lines > 0) {
        tms_read(nametab + lines * 40, buf, 960 - lines * 40);
        for (uint16_t i = 960 - lines * 40; i < 960; i++)
            buf[i] = 0;
        tms_write(nametab, buf, 960);
    } else if (lines < 0) {
        lines = -lines;
        tms_read(nametab, buf + lines * 40, 960 - lines * 40);
        for (uint16_t i = 0; i < lines * 40; i++)
            buf[i] = 0;
        tms_write(nametab, buf, 960);
    }
}

void tms_update()
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
    tms_scroll(lines);
    tms_read(nametab + cursorpos, &undercursor, 1);
    tms_fill(nametab + cursorpos, 0xde, 1);
    prevpos = cursorpos;
}

void tms_delete()
{
    cursorpos--;
    undercursor = 0;
}

void tms_home()
{
    cursorpos = 0;
}

void tms_cleartext()
{
    tms_fill(nametab, 0, 960);
}

void tms_cleargraph()
{
}

void tms_cursorup(uint8_t c)
{
    cursorpos -= 40 * c;
}

void tms_cursordown(uint8_t c)
{
    cursorpos += 40 * c;
}

void tms_cursorleft(uint8_t c)
{
    cursorpos -= c;
}

void tms_cursorright(uint8_t c)
{
    cursorpos += c;
}

void tms_startline()
{
    cursorpos = ((cursorpos / 40) * 40);
}

void tms_pos(uint8_t x, uint8_t y)
{
    x %= 40;
    y %= 24;
    cursorpos = y * 40 + x;
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

void vdu_color(uint8_t *p)
{
    if (p[1] >= 128)
        textbg = colormap[p[1] & 0xf];
    else
        textfg = colormap[p[1] & 0xf];
    tms_config();
}

void vdu_gcolor(uint8_t *p)
{
    pixelmode = p[1] % 5;
    if (p[2] >= 128)
        pixelbg = colormap[p[2] & 0xf];
    else
        pixelfg = colormap[p[2] & 0xf];
    tms_config();
}

void vdu_lcolor(uint8_t *p)
{
    colormap[p[1] & 0xf] = p[2] & 0xf;
}

void vdu_dcolor(uint8_t *p)
{
    for (uint8_t i = 0; i < 16; i++)
        colormap[i] = DEFCOLOR(i);
}

void vdu_flash(uint8_t *p)
{
    uint8_t save = textbg;
    textbg = ~textbg;
    tms_config();
    _delay_ms(100);
    textbg = save;
    tms_config();
}

void vdu_pos(uint8_t *p)
{
    tms_pos(p[1], p[2]);
}

void vdu_home(uint8_t *p)
{
    tms_home();
}

void vdu_cleartext(uint8_t *p)
{
    tms_fill(nametab, 0, 960);
}

void vdu_cleargraph(uint8_t *p)
{
}

void vdu_cursorup(uint8_t *p)
{
    tms_cursorup(1);
}

void vdu_cursordown(uint8_t *p)
{
    tms_cursordown(1);
}

void vdu_cursorleft(uint8_t *p)
{
    tms_cursorleft(1);
}

void vdu_cursorright(uint8_t *p)
{
    tms_cursorright(1);
}

void vdu_startline(uint8_t *p)
{
    tms_startline();
}

void vdu_mode(uint8_t *p)
{
    switch (p[1]) {
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

void vdu_program(uint8_t *p)
{

}

void vdu_plot(uint8_t *p)
{

}

void vdu_gwindow(uint8_t *p)
{
    
}

void vdu_dwindow(uint8_t *p)
{

}

void vdu_twindow(uint8_t *p)
{

}

void vdu_origin(uint8_t *p)
{

}

void ansi_clearscreen(uint8_t n)
{
    if (n == 0) {
        tms_fill(nametab + cursorpos, 0, 960 - cursorpos);
    } else if (n == 1) {
        tms_fill(nametab, 0, cursorpos);
    } else if (n == 2 || n == 3) {
        tms_fill(nametab, 0, 960);
    }
}

void ansi_clearline(uint8_t n)
{
    int startline = (cursorpos / 40) * 40;
    int column = cursorpos % 40;
    if (n == 0) {
        tms_fill(nametab + cursorpos, 0, 40 - column);
    } else if (n == 1) {
        tms_fill(nametab + startline, 0, column);
    } else if (n == 2) {
        tms_fill(nametab + startline, 0, 40);
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

void ansi_command(uint8_t command, uint8_t paramcnt, uint8_t params[])
{
    uint8_t cnt = params[0] == 0 ? 1 : params[0];
    uint8_t cnt2 = params[1] == 0 ? 1 : params[1];
    switch (command) {
        case 'A':
            tms_cursorup(cnt);
            break;
        case 'B':
            tms_cursordown(cnt);
            break;
        case 'C':
            tms_cursorright(cnt);
            break;
        case 'D':
            tms_cursorleft(cnt);
            break;
        case 'E':
            tms_startline();
            tms_cursordown(cnt);
            break;
        case 'F':
            tms_startline();
            tms_cursorup(cnt);
            break;
        case 'G':
            tms_startline();
            tms_cursorright(cnt - 1);
        case 'H':
            tms_pos(cnt2 - 1, cnt - 1);
            break;
        case 'J':
            ansi_clearscreen(params[0]);
            break;
        case 'K':
            ansi_clearline(params[0]);
            break;
        case 'S':
            tms_scroll(cnt);
            break;
        case 'T':
            tms_scroll(-cnt);
            break;
        case 'm':
            for (uint8_t i = 0; i <= paramcnt; i++)
                ansi_color(params[i]);
            break;
    }
}

uint8_t const vdu_length[] PROGMEM = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 0, 1, 9, 8, 5, 0, 1, 4, 4, 0, 2
};

void * const vdu_functions[] PROGMEM = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    vdu_flash,
    vdu_cursorleft,
    vdu_cursorright,
    vdu_cursordown,
    vdu_cursorup,
    vdu_cleartext,
    vdu_startline,
    NULL,
    NULL,
    vdu_cleargraph,
    vdu_color,
    vdu_gcolor,
    vdu_lcolor,
    vdu_dcolor,
    NULL,
    vdu_mode,
    vdu_program,
    vdu_gwindow,
    vdu_plot,
    vdu_dwindow,
    NULL,
    vdu_twindow,
    vdu_origin,
    vdu_home,
    vdu_pos
};

#define MAXPARAM 10

enum vdu_mode {
    VDU_NORMAL,
    VDU_COMMAND,
    VDU_DISABLED,
    VDU_ESC,
    VDU_CSI
};

void tms_putchar(char c)
{
    static uint8_t mode = VDU_NORMAL;
    static uint8_t paramidx = 0;
    static uint8_t paramlen = 0;
    static uint8_t parambuf[MAXPARAM];

    if (mode == VDU_DISABLED) {
        if (c == 6)
            mode = VDU_NORMAL;
        return;
    }

    if (bus_mode != BUS_MASTER)
        bus_master();

    if (mode == VDU_NORMAL) {
        if (c == 21) {
        } else if (c == '\e') {
            mode = VDU_ESC;
        } else if (c == 0x7f) {
            tms_delete();
        } else if (c < 32) {
            paramidx = 0;
            paramlen = pgm_read_byte(&vdu_length[c]);
            mode = VDU_COMMAND;
        } else {
            tms_literal(c);
        }
    } else if (mode == VDU_ESC) {
        if (c == '[') {
            memset(parambuf, 0, MAXPARAM);
            paramidx = 0;
            mode = VDU_CSI;
        } else {
            if (c == 'c') {   // reset terminal
                tms_init(TMS_TEXT);
            } else {
                tms_literal(c);
            }
            mode = VDU_NORMAL;
        }
    } else if (mode == VDU_CSI) {
        if ('0' <= c && c <= '9') {
            parambuf[paramidx] = parambuf[paramidx] * 10 + c - '0';
        } else if ((c == ':' || c == ';') && paramidx < MAXPARAM-1) {
            paramidx++;
        } else {
            ansi_command(c, paramidx, parambuf);
            mode = VDU_NORMAL;
        }
    }

    if (mode == VDU_COMMAND) {
        parambuf[paramidx++] = c;
        if (paramidx > paramlen) {
            void (*vdu_func)(uint8_t[]) = pgm_read_ptr(&vdu_functions[parambuf[0]]);
            if (vdu_func)
                vdu_func(parambuf);
            mode = VDU_NORMAL;
        }
    }

    tms_update();
}